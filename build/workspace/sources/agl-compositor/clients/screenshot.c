/*
 * Copyright 2024 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <pixman.h>
#include <cairo.h>
#include <getopt.h>
#include <assert.h>

#include <wayland-client.h>
#include "weston-output-capture-client-protocol.h"
#include "shared/os-compatibility.h"
#include "shared/xalloc.h"
#include "shared/file-util.h"
#include "shared/pixel-formats.h"
#include "shared/helpers.h"
#include "shared/string-helpers.h"

static int opts = 0x0;

#define OPT_SCREENSHOT_OUTPUT		1
#define OPT_SHOW_ALL_OUTPUTS		2
#define OPT_SCREENSHOT_ALL_OUTPUTS	3

struct screenshooter_app {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_shm *shm;
	struct weston_capture_v1 *capture_factory;

	struct wl_list output_list; /* struct screenshooter_output::link */

	bool retry;
	bool failed;
	int waitcount;
};

struct screenshooter_buffer {
	size_t len;
	void *data;
	struct wl_buffer *wl_buffer;
	pixman_image_t *image;
};

struct screenshooter_output {
	struct screenshooter_app *app;
	struct wl_list link; /* struct screenshooter_app::output_list */

	struct wl_output *wl_output;
	int offset_x, offset_y;
	char *name;

	struct weston_capture_source_v1 *source;

	int buffer_width;
	int buffer_height;
	const struct pixel_format_info *fmt;
	struct screenshooter_buffer *buffer;
};

struct buffer_size {
	int width, height;

	int min_x, min_y;
	int max_x, max_y;
};

static struct screenshooter_buffer *
screenshot_create_shm_buffer(struct screenshooter_app *app,
			     size_t width, size_t height,
			     const struct pixel_format_info *fmt)
{
	struct screenshooter_buffer *buffer;
	struct wl_shm_pool *pool;
	int fd;
	size_t bytes_pp;
	size_t stride;

	assert(width > 0);
	assert(height > 0);
	assert(fmt && fmt->bpp > 0);
	assert(fmt->pixman_format);

	buffer = xzalloc(sizeof *buffer);

	bytes_pp = fmt->bpp / 8;
	stride = width * bytes_pp;
	buffer->len = stride * height;

	assert(width == stride / bytes_pp);
	assert(height == buffer->len / stride);

	fd = os_create_anonymous_file(buffer->len);
	if (fd < 0) {
		fprintf(stderr, "creating a buffer file for %zd B failed: %s\n",
			buffer->len, strerror(errno));
		free(buffer);
		return NULL;
	}

	buffer->data = mmap(NULL, buffer->len, PROT_READ | PROT_WRITE,
			    MAP_SHARED, fd, 0);
	if (buffer->data == MAP_FAILED) {
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		close(fd);
		free(buffer);
		return NULL;
	}

	pool = wl_shm_create_pool(app->shm, fd, buffer->len);
	close(fd);
	buffer->wl_buffer =
		wl_shm_pool_create_buffer(pool, 0, width, height, stride,
					  pixel_format_get_shm_format(fmt));
	wl_shm_pool_destroy(pool);

	buffer->image = pixman_image_create_bits(fmt->pixman_format,
						 width, height,
						 buffer->data, stride);
	if (!buffer->image) {
		fprintf(stderr, "Failed to create buffer image!\n");
		close(fd);
		free(buffer);
		return NULL;
	}

	return buffer;
}

static void
screenshooter_buffer_destroy(struct screenshooter_buffer *buffer)
{
	if (!buffer)
		return;

	pixman_image_unref(buffer->image);
	munmap(buffer->data, buffer->len);
	wl_buffer_destroy(buffer->wl_buffer);
	free(buffer);
}

static void
capture_source_handle_format(void *data,
			     struct weston_capture_source_v1 *proxy,
			     uint32_t drm_format)
{
	struct screenshooter_output *output = data;

	assert(output->source == proxy);

	output->fmt = pixel_format_get_info(drm_format);
}

static void
capture_source_handle_size(void *data,
			   struct weston_capture_source_v1 *proxy,
			   int32_t width, int32_t height)
{
	struct screenshooter_output *output = data;

	assert(width > 0);
	assert(height > 0);

	output->buffer_width = width;
	output->buffer_height = height;
}

static void
capture_source_handle_complete(void *data,
			       struct weston_capture_source_v1 *proxy)
{
	struct screenshooter_output *output = data;

	output->app->waitcount--;
}

static void
capture_source_handle_retry(void *data,
			    struct weston_capture_source_v1 *proxy)
{
	struct screenshooter_output *output = data;

	output->app->waitcount--;
	output->app->retry = true;
}

static void
capture_source_handle_failed(void *data,
			     struct weston_capture_source_v1 *proxy,
			     const char *msg)
{
	struct screenshooter_output *output = data;

	output->app->waitcount--;
	output->app->failed = true;

	if (msg)
		fprintf(stderr, "Output capture error: %s\n", msg);
}

static const struct weston_capture_source_v1_listener capture_source_handlers = {
	.format = capture_source_handle_format,
	.size = capture_source_handle_size,
	.complete = capture_source_handle_complete,
	.retry = capture_source_handle_retry,
	.failed = capture_source_handle_failed,
};

static void
display_handle_geometry(void *data,
		struct wl_output *wl_output,
		int x, int y,
		int physical_width,
		int physical_height,
		int subpixel,
		const char *make,
		const char *model,
		int32_t transform)
{
}

static void
display_handle_mode(void *data,
		struct wl_output *wl_output,
		uint32_t flags,
		int width,
		int height,
		int refresh)
{
}

static void
display_handle_scale(void *data,
		struct wl_output *wl_output,
		int scale)
{
}

static void
display_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
	struct screenshooter_output *output = data;
	output->name = strdup(name);
}

static void
display_handle_description(void *data, struct wl_output *wl_output, const char *desc)
{
}

static void
display_handle_done(void *data,
		struct wl_output *wl_output)
{
}


static const struct wl_output_listener output_listener = {
	display_handle_geometry,
	display_handle_mode,
	display_handle_done,
	display_handle_scale,
	display_handle_name,
	display_handle_description,
};

static void
create_output(struct screenshooter_app *app, uint32_t output_name, uint32_t version)
{
	struct screenshooter_output *output;

	version = MIN(version, 4);
	output = xzalloc(sizeof *output);
	output->app = app;
	output->wl_output = wl_registry_bind(app->registry, output_name,
					     &wl_output_interface, version);
	if (!output->wl_output) {
		fprintf(stderr, "Failed to get bind output!\n");
		exit(EXIT_FAILURE);
	}

	wl_output_add_listener(output->wl_output, &output_listener, output);

	output->source = weston_capture_v1_create(app->capture_factory,
						  output->wl_output,
						  WESTON_CAPTURE_V1_SOURCE_FRAMEBUFFER);
	if (!output->source) {
		fprintf(stderr, "Failed to get a capture source!\n");
		exit(EXIT_FAILURE);
	}
	weston_capture_source_v1_add_listener(output->source,
					      &capture_source_handlers, output);

	wl_list_insert(&app->output_list, &output->link);
}

static void
destroy_output(struct screenshooter_output *output)
{
	weston_capture_source_v1_destroy(output->source);

	if (wl_output_get_version(output->wl_output) >= WL_OUTPUT_RELEASE_SINCE_VERSION)
		wl_output_release(output->wl_output);
	else
		wl_output_destroy(output->wl_output);

	screenshooter_buffer_destroy(output->buffer);
	wl_list_remove(&output->link);
	free(output->name);
	free(output);
}

static void
handle_global(void *data, struct wl_registry *registry,
	      uint32_t name, const char *interface, uint32_t version)
{
	struct screenshooter_app *app = data;

	if (strcmp(interface, wl_output_interface.name) == 0) {
		create_output(app, name, version);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		app->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		/*
		 * Not listening for format advertisements,
		 * weston_capture_source_v1.format event tells us what to use.
		 */
	} else if (strcmp(interface, weston_capture_v1_interface.name) == 0) {
		app->capture_factory = wl_registry_bind(registry, name,
							&weston_capture_v1_interface,
							1);
	}
}

static void
handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	/* Dynamic output removals will just fail the respective shot. */
}

static const struct wl_registry_listener registry_listener = {
	handle_global,
	handle_global_remove
};

static void
screenshooter_output_capture(struct screenshooter_output *output)
{
	screenshooter_buffer_destroy(output->buffer);
	output->buffer = screenshot_create_shm_buffer(output->app,
						      output->buffer_width,
						      output->buffer_height,
						      output->fmt);
	if (!output->buffer) {
		fprintf(stderr, "Failed to create output buffer\n");
		exit(EXIT_FAILURE);
	}

	weston_capture_source_v1_capture(output->source,
					 output->buffer->wl_buffer);
	output->app->waitcount++;
}

static void
screenshot_write_png_per_output(const struct buffer_size *buff_size,
				struct screenshooter_output *output,
				const char *fn)
{
	pixman_image_t *shot;
	cairo_surface_t *surface;
	FILE *fp;
	char filepath[PATH_MAX];
	char *filename_to_write;

	shot = pixman_image_create_bits(PIXMAN_a8r8g8b8,
					buff_size->width, buff_size->height,
					NULL, 0);
	if (!shot) {
		fprintf(stderr, "Failed to create shot\n");
		exit(EXIT_FAILURE);
	}


	pixman_image_composite32(PIXMAN_OP_SRC,
				 output->buffer->image, /* src */
				 NULL, /* mask */
				 shot, /* dest */
				 0, 0, /* src x,y */
				 0, 0, /* mask x,y */
				 output->offset_x, output->offset_y, /* dst x,y */
				 output->buffer_width, output->buffer_height);

	surface = cairo_image_surface_create_for_data((void *)pixman_image_get_data(shot),
						      CAIRO_FORMAT_ARGB32,
						      pixman_image_get_width(shot),
						      pixman_image_get_height(shot),
						      pixman_image_get_stride(shot));
	if (fn)
		str_printf(&filename_to_write, "agl-screenshot-%s-", fn);
	else
		str_printf(&filename_to_write, "agl-screenshot-");

	fp = file_create_dated(getenv("XDG_PICTURES_DIR"), filename_to_write,
			       ".png", filepath, sizeof(filepath));
	if (fp) {
		fclose(fp);
		cairo_surface_write_to_png(surface, filepath);
	}
	cairo_surface_destroy(surface);
	pixman_image_unref(shot);
}

static void
screenshot_set_buffer_size_per_output(struct buffer_size *buff_size,
				      struct screenshooter_output *output)
{
	buff_size->min_x = MIN(buff_size->min_x, output->offset_x);
	buff_size->min_y = MIN(buff_size->min_y, output->offset_y);
	buff_size->max_x = MAX(buff_size->max_x, output->offset_x + output->buffer_width);
	buff_size->max_y = MAX(buff_size->max_y, output->offset_y + output->buffer_height);

	buff_size->width = buff_size->max_x - buff_size->min_x;
	buff_size->height = buff_size->max_y - buff_size->min_y;
}

static void
screenshot_compute_output_offset(int *pos, struct screenshooter_output *sh_output)
{
	sh_output->offset_x = *pos;
	*pos += sh_output->buffer_width;
}

static struct screenshooter_output *
agl_shooter_search_for_output(const char *output_name,
			      struct screenshooter_app *app)
{
	struct screenshooter_output *found_output = NULL;
	struct screenshooter_output *output;

	if (!output_name)
		return found_output;

	wl_list_for_each(output, &app->output_list, link) {
		if (output->name && strcmp(output->name, output_name) == 0) {
			found_output = output;
			break;
		}
	}

	return found_output;
}

static char *
agl_shooter_search_get_output_name(struct screenshooter_output *sh_output)
{
	struct screenshooter_app *app;
	struct screenshooter_output *output;

	if (!sh_output)
		return NULL;

	app = sh_output->app;

	wl_list_for_each(output, &app->output_list, link) {
		if (output == sh_output) {
			return output->name;
		}
	}

	return NULL;
}

static void
agl_shooter_display_all_outputs(struct screenshooter_app *app)
{
	struct screenshooter_output *output;

	wl_list_for_each(output, &app->output_list, link) {
		fprintf(stdout, "Output '%s'\n", output->name);
	}
}

static void
agl_shooter_screenshot_output(struct screenshooter_output *output, int *pos)
{
	struct buffer_size buff_size = {};
	struct screenshooter_app *app = output->app;
	char *output_name;

	do {
		app->retry = false;
		screenshooter_output_capture(output);

		while (app->waitcount > 0 && !app->failed) {
			if (wl_display_dispatch(app->display) < 0)
				app->failed = true;
			assert(app->waitcount >= 0);
		}
	} while (app->retry && !app->failed);

	if (!app->failed) {
		screenshot_compute_output_offset(pos, output);
		screenshot_set_buffer_size_per_output(&buff_size, output);

		output_name = agl_shooter_search_get_output_name(output);
		assert(output_name);
		screenshot_write_png_per_output(&buff_size, output, output_name);
	} else {
		fprintf(stderr, "Error: screenshot or protocol failure\n");
	}
}


static void
agl_shooter_screenshot_all_outputs(struct screenshooter_app *app)
{
	struct screenshooter_output *output;
	int pos = 0;

	wl_list_for_each(output, &app->output_list, link)
		agl_shooter_screenshot_output(output, &pos);
}

static void
print_usage_and_exit(void)
{
	fprintf(stderr, "./agl-screenshooter [-o OUTPUT_NAME] [-l] [-a]\n");

	fprintf(stderr, "\t-o OUTPUT_NAME -- take a screenshot of the output "
				"specified by OUTPUT_NAME\n");
	fprintf(stderr, "\t-a  -- take a screenshot of all the outputs found\n");
	fprintf(stderr, "\t-l  -- list all the outputs found\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct wl_display *display;
	struct screenshooter_output *output;
	struct screenshooter_output *sh_output = NULL;
	struct screenshooter_output *tmp_output;
	struct screenshooter_app app = {};

	int c, option_index;
	char *output_name = NULL;
	int pos = 0;

	wl_list_init(&app.output_list);

	static struct option long_options[] = {
		{"output", 	required_argument, 0,  'o' },
		{"list", 	required_argument, 0,  'l' },
		{"all", 	required_argument, 0,  'a' },
		{"help",	no_argument      , 0,  'h' },
		{0, 0, 0, 0}
	};

	while ((c = getopt_long(argc, argv, "o:lah",
				long_options, &option_index)) != -1) {
		switch (c) {
		case 'o':
			output_name = optarg;
			opts |= (1 << OPT_SCREENSHOT_OUTPUT);
			break;
		case 'l':
			opts |= (1 << OPT_SHOW_ALL_OUTPUTS);
			break;
		case 'a':
			opts |= (1 << OPT_SCREENSHOT_ALL_OUTPUTS);
			break;
		default:
			print_usage_and_exit();
		}
	}

	display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "failed to create display: %s\n",
			strerror(errno));
		return -1;
	}

	app.display = display;
	app.registry = wl_display_get_registry(display);
	wl_registry_add_listener(app.registry, &registry_listener, &app);

	/* Process wl_registry advertisements */
	wl_display_roundtrip(display);

	if (!app.shm) {
		fprintf(stderr, "Error: display does not support wl_shm\n");
		return -1;
	}
	if (!app.capture_factory) {
		fprintf(stderr, "Error: display does not support weston_capture_v1\n");
		return -1;
	}

	/* Process initial events for wl_output and weston_capture_source_v1 */
	wl_display_roundtrip(display);

	if (opts & (1 << OPT_SHOW_ALL_OUTPUTS)) {
		agl_shooter_display_all_outputs(&app);
		return EXIT_SUCCESS;
	}

	if (opts & (1 << OPT_SCREENSHOT_ALL_OUTPUTS)) {
		agl_shooter_screenshot_all_outputs(&app);
		return EXIT_SUCCESS;
	}

	sh_output = NULL;
	if (output_name)
		sh_output = agl_shooter_search_for_output(output_name, &app);

	if (!sh_output && (opts & (1 << OPT_SCREENSHOT_OUTPUT))) {
		fprintf(stderr, "Could not find an output matching '%s'\n",
				output_name);
		return EXIT_FAILURE;
	}

	/* if we're still here just pick the first one available
	 * and use that. Still useful in case we are run without
	 * any args whatsoever */
	if (!sh_output)
		sh_output = container_of(app.output_list.next,
					 struct screenshooter_output, link);

	/* take a screenshot only of that specific output */
	agl_shooter_screenshot_output(sh_output, &pos);

	wl_list_for_each_safe(output, tmp_output, &app.output_list, link)
		destroy_output(output);

	weston_capture_v1_destroy(app.capture_factory);
	wl_shm_destroy(app.shm);
	wl_registry_destroy(app.registry);
	wl_display_disconnect(display);

	return 0;
}
