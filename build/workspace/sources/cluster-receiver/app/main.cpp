/*
 * Copyright © 2020, 2024 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define GST_USE_UNSTABLE_API
#define HAVE_MEMFD_CREATE

#include <string>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <signal.h>
#include <wayland-client.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <glib.h>

#include "xdg-shell-client-protocol.h"
#include "zalloc.h"

#include <gst/gst.h>

#include <gst/video/videooverlay.h>
#include <gst/wayland/wayland.h>

#include "AglShellGrpcClient.h"
#include "SerialDbusBridge.h"


#if !GST_CHECK_VERSION(1, 22, 0)
#define gst_is_wl_display_handle_need_context_message gst_is_wayland_display_handle_need_context_message
#define gst_wl_display_handle_context_new gst_wayland_display_handle_context_new
#endif

#define WINDOW_WIDTH	640
#define WINDOW_HEIGHT	720

#define WINDOW_POS_X	640
#define WINDOW_POS_Y	180

unsigned g_window_width = WINDOW_WIDTH;
unsigned g_window_height = WINDOW_HEIGHT;
unsigned g_window_pos_x = WINDOW_POS_X;
unsigned g_window_pos_y = WINDOW_POS_Y;

// C++ requires a cast and we in wayland we do the cast implictly
#define WL_ARRAY_FOR_EACH(pos, array, type) \
	for (pos = (type)(array)->data; \
	     (const char *) pos < ((const char *) (array)->data + (array)->size); \
	     (pos)++)

struct display {
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_output *wl_output;
	struct wl_shm *shm;

	struct {
		int width;
		int height;
	} output_data;

	struct xdg_wm_base *wm_base;

	int has_xrgb;
};

struct buffer {
	struct wl_buffer *buffer;
	void *shm_data;
	int busy;
};

struct window {
	struct display *display;

	int x, y;
	int width, height;

	struct wl_surface *surface;

	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	bool wait_for_configure;

	int fullscreen, maximized;

	struct buffer buffers[2];
	struct wl_callback *callback;
};


struct cluster_receiver_data {
	struct window *window;

	GstElement *pipeline;
	GstVideoOverlay *overlay;
};

static int running = 1;

static bool
env_flag_enabled(const char *name)
{
	const char *value = getenv(name);
	if (!value || value[0] == '\0')
		return false;

	if (strcmp(value, "0") == 0 ||
	    strcasecmp(value, "false") == 0 ||
	    strcasecmp(value, "no") == 0 ||
	    strcasecmp(value, "off") == 0)
		return false;

	return true;
}

static void
redraw(void *data, struct wl_callback *callback, uint32_t time);

static void
paint_pixels(void *image, int width, int height)
{
	memset(image, 0x00, width * height * 4);
}

static void
buffer_release(void *data, struct wl_buffer *buffer)
{
	struct buffer *mybuf = static_cast<struct buffer *>(data);
	(void) buffer;

	mybuf->busy = 0;
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

static int
os_fd_set_cloexec(int fd)
{
	long flags;

	if (fd == -1)
		return -1;

	flags = fcntl(fd, F_GETFD);
	if (flags == -1)
		return -1;

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		return -1;

	return 0;
}

static int
set_cloexec_or_close(int fd)
{
	if (os_fd_set_cloexec(fd) != 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static int
create_tmpfile_cloexec(char *tmpname)
{
	int fd;

#ifdef HAVE_MKOSTEMP
	fd = mkostemp(tmpname, O_CLOEXEC);
	if (fd >= 0)
		unlink(tmpname);
#else
	fd = mkstemp(tmpname);
	if (fd >= 0) {
		fd = set_cloexec_or_close(fd);
		unlink(tmpname);
	}
#endif

	return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 *
 * If the C library implements posix_fallocate(), it is used to
 * guarantee that disk space is available for the file at the
 * given size. If disk space is insufficient, errno is set to ENOSPC.
 * If posix_fallocate() is not supported, program may receive
 * SIGBUS on accessing mmap()'ed file contents instead.
 *
 * If the C library implements memfd_create(), it is used to create the
 * file purely in memory, without any backing file name on the file
 * system, and then sealing off the possibility of shrinking it.  This
 * can then be checked before accessing mmap()'ed file contents, to
 * make sure SIGBUS can't happen.  It also avoids requiring
 * XDG_RUNTIME_DIR.
 */
static int
os_create_anonymous_file(off_t size)
{
	static const char weston_template[] = "/weston-shared-XXXXXX";
	const char *path;
	char *name;
	int fd;
	int ret;

#ifdef HAVE_MEMFD_CREATE
	fd = memfd_create("weston-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
	if (fd >= 0) {
		/* We can add this seal before calling posix_fallocate(), as
		 * the file is currently zero-sized anyway.
		 *
		 * There is also no need to check for the return value, we
		 * couldn't do anything with it anyway.
		 */
		fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK);
	} else
#endif
	{
		path = getenv("XDG_RUNTIME_DIR");
		if (!path) {
			errno = ENOENT;
			return -1;
		}

		name = static_cast<char *>(malloc(strlen(path) + sizeof(weston_template)));
		if (!name)
			return -1;

		strcpy(name, path);
		strcat(name, weston_template);

		fd = create_tmpfile_cloexec(name);

		free(name);

		if (fd < 0)
			return -1;
	}

#ifdef HAVE_POSIX_FALLOCATE
	do {
		ret = posix_fallocate(fd, 0, size);
	} while (ret == EINTR);
	if (ret != 0) {
		close(fd);
		errno = ret;
		return -1;
	}
#else
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
#endif

	return fd;
}

static int
create_shm_buffer(struct display *display, struct buffer *buffer,
		  int width, int height, uint32_t format)
{
	struct wl_shm_pool *pool;
	int fd, size, stride;
	void *data;

	stride = width * 4;
	size = stride * height;

	fd = os_create_anonymous_file(size);
	if (fd < 0) {
		fprintf(stderr, "creating a buffer file for %d B failed: %s\n",
				size, strerror(errno));
		return -1;
	}

	data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	pool = wl_shm_create_pool(display->shm, fd, size);
	buffer->buffer = wl_shm_pool_create_buffer(pool, 0, width,
						   height, stride, format);
	wl_buffer_add_listener(buffer->buffer, &buffer_listener, buffer);
	wl_shm_pool_destroy(pool);
	close(fd);

	buffer->shm_data = data;
	return 0;
}

static struct buffer *
get_next_buffer(struct window *window)
{
	struct buffer *buffer;
	int ret = 0;

	if (!window->buffers[0].busy)
		buffer = &window->buffers[0];
	else if (!window->buffers[1].busy)
		buffer = &window->buffers[1];
	else
		return NULL;

	if (!buffer->buffer) {
		fprintf(stdout, "get_next_buffer() buffer is not set, setting with "
				"width %d, height %d\n", window->width, window->height);
		ret = create_shm_buffer(window->display, buffer, window->width,
					window->height, WL_SHM_FORMAT_XRGB8888);

		if (ret < 0)
			return NULL;

		/* paint the padding */
		memset(buffer->shm_data, 0x00, window->width * window->height * 4);
	}

	return buffer;
}


static const struct wl_callback_listener frame_listener = {
	redraw
};

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
        struct window *window = static_cast<struct window *>(data);
        struct buffer *buffer;
	(void) time;

        buffer = get_next_buffer(window);
        if (!buffer) {
                fprintf(stderr,
                        !callback ? "Failed to create the first buffer.\n" :
                        "Both buffers busy at redraw(). Server bug?\n");
                abort();
        }

	// do the actual painting
	paint_pixels(buffer->shm_data, window->width, window->height);

        wl_surface_attach(window->surface, buffer->buffer, 0, 0);
        wl_surface_damage(window->surface, 0, 0, window->width, window->height);

        if (callback)
                wl_callback_destroy(callback);

        window->callback = wl_surface_frame(window->surface);
        wl_callback_add_listener(window->callback, &frame_listener, window);
        wl_surface_commit(window->surface);

        buffer->busy = 1;
}

static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
	struct display *d = static_cast<struct display *>(data);
	(void) wl_shm;

	if (format == WL_SHM_FORMAT_XRGB8888)
		d->has_xrgb = true;
}

static const struct wl_shm_listener shm_listener = {
	shm_format
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	(void) data;
        xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
        xdg_wm_base_ping,
};

static void
display_handle_geometry(void *data, struct wl_output *wl_output,
			int x, int y, int physical_width, int physical_height,
			int subpixel, const char *make, const char *model, int transform)
{
	(void) data;
	(void) wl_output;
	(void) x;
	(void) y;
	(void) physical_width;
	(void) physical_height;
	(void) subpixel;
	(void) make;
	(void) model;
	(void) transform;
}

static void
display_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		    int width, int height, int refresh)
{
	struct display *d = static_cast<struct display *>(data);
	(void) refresh;

	if (wl_output == d->wl_output && (flags & WL_OUTPUT_MODE_CURRENT)) {
		d->output_data.width = width;
		d->output_data.height = height;

		fprintf(stdout, "Found output with width %d and height %d\n",
				d->output_data.width, d->output_data.height);
	}
}

static void
display_handle_scale(void *data, struct wl_output *wl_output, int scale)
{
	(void) data;
	(void) wl_output;
	(void) scale;
}

static void
display_handle_done(void *data, struct wl_output *wl_output)
{
	(void) data;
	(void) wl_output;
}

static void
display_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
	(void) data;
	(void) wl_output;
	(void) name;
}

static void
display_handle_desc(void *data, struct wl_output *wl_output, const char *desc)
{
	(void) data;
	(void) wl_output;
	(void) desc;
}

static const struct wl_output_listener output_listener = {
	display_handle_geometry,
	display_handle_mode,
	display_handle_done,
	display_handle_scale,
	display_handle_name,
	display_handle_desc,
};

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
		       const char *interface, uint32_t version)
{
	struct display *d = static_cast<struct display *>(data);
	(void) version;

	if (strcmp(interface, "wl_compositor") == 0) {
		d->wl_compositor =
			static_cast<struct wl_compositor *>(wl_registry_bind(registry, id,
						    &wl_compositor_interface, 1));
	} else if (strcmp(interface, "xdg_wm_base") == 0) {
		d->wm_base = static_cast<struct xdg_wm_base *>(wl_registry_bind(registry,
				id, &xdg_wm_base_interface, 1));
		xdg_wm_base_add_listener(d->wm_base, &xdg_wm_base_listener, d);
	} else if (strcmp(interface, "wl_shm") == 0) {
		d->shm = static_cast<struct wl_shm *>(wl_registry_bind(registry,
				id, &wl_shm_interface, 1));
		wl_shm_add_listener(d->shm, &shm_listener, d);
	} else if (strcmp(interface, "wl_output") == 0) {
		d->wl_output = static_cast<struct wl_output *>(wl_registry_bind(registry, id,
					     &wl_output_interface, 1));
		wl_output_add_listener(d->wl_output, &output_listener, d);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *reg, uint32_t id)
{
	(void) data;
	(void) reg;
	(void) id;
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove,
};


static void
error_cb(GstBus *bus, GstMessage *msg, gpointer user_data)
{
	(void) bus;
	struct cluster_receiver_data *d =
		static_cast<struct cluster_receiver_data *>(user_data);

	gchar *debug = NULL;
	GError *err = NULL;

	gst_message_parse_error(msg, &err, &debug);

	g_print("Error: %s\n", err->message);
	g_error_free(err);

	if (debug) {
		g_print("Debug details: %s\n", debug);
		g_free(debug);
	}

	gst_element_set_state(d->pipeline, GST_STATE_NULL);
}

static GstBusSyncReply
bus_sync_handler(GstBus *bus, GstMessage *message, gpointer user_data)
{
	(void) bus;
	struct cluster_receiver_data *d =
		static_cast<struct cluster_receiver_data *>(user_data);

	if (gst_is_wl_display_handle_need_context_message(message)) {
		GstContext *context;
		struct wl_display *display_handle = d->window->display->wl_display;

		context = gst_wl_display_handle_context_new(display_handle);
		gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)), context);

		goto drop;
	} else if (gst_is_video_overlay_prepare_window_handle_message(message)) {
		struct wl_surface *window_handle = d->window->surface;

		/* GST_MESSAGE_SRC(message) will be the overlay object that we
		 * have to use. This may be waylandsink, but it may also be
		 * playbin. In the latter case, we must make sure to use
		 * playbin instead of waylandsink, because playbin resets the
		 * window handle and render_rectangle after restarting playback
		 * and the actual window size is lost */
		d->overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));

		g_print("setting window handle and size (%d x %d) w %d, h %d\n",
				d->window->x, d->window->y,
				d->window->width, d->window->height);

		gst_video_overlay_set_window_handle(d->overlay, (guintptr) window_handle);
		gst_video_overlay_set_render_rectangle(d->overlay,
						       d->window->x, d->window->y,
						       d->window->width, d->window->height);

		goto drop;
	}

	return GST_BUS_PASS;

drop:
	gst_message_unref(message);
	return GST_BUS_DROP;
}

static void
handle_xdg_surface_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
	struct window *window = static_cast<struct window *>(data);

	xdg_surface_ack_configure(surface, serial);

	if (window->wait_for_configure) {
		redraw(window, NULL, 0);
		window->wait_for_configure = false;
	}
}

static const struct xdg_surface_listener xdg_surface_listener = {
	handle_xdg_surface_configure,
};

static void
handle_xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
			      int32_t width, int32_t height,
			      struct wl_array *states)
{
	struct window *window = static_cast<struct window *>(data);
	uint32_t *p;
	(void) xdg_toplevel;

	window->fullscreen = 0;
	window->maximized = 0;

	// use our own macro as C++ can't typecast from (void *) directly
	WL_ARRAY_FOR_EACH(p, states, uint32_t *) {
		uint32_t state = *p; 
		switch (state) {
		case XDG_TOPLEVEL_STATE_FULLSCREEN:
			window->fullscreen = 1;
			break;
		case XDG_TOPLEVEL_STATE_MAXIMIZED:
			window->maximized = 1;
			break;
		}
	}

	fprintf(stdout, "Got handle_xdg_toplevel_configure() "
			"width %d, height %d, full %d, max %d\n", width, height,
			window->fullscreen, window->maximized);

	if (width > 0 && height > 0) {
		if (!window->fullscreen && !window->maximized) {
			window->width = width;
			window->height = height;
		}
		window->width = width;
		window->height = height;
	} else if (!window->fullscreen && !window->maximized) {
		if (width == 0)
			window->width = g_window_width;
		else
			window->width = width;

		if (height == 0)
			window->height = g_window_height;
		else
			window->height = height;
	}

	fprintf(stdout, "settting  width %d, height %d\n",
			window->width, window->height);
}

static void
handle_xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	(void) xdg_toplevel;
	(void) data;
	running = 0;
}

static void
handle_xdg_toplevel_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
                                     int32_t width, int32_t height)
{
	(void) data;
	(void) xdg_toplevel;
	(void) width;
	(void) height;
}


static void
handle_xdg_toplevel_wm_caps(void *data, struct xdg_toplevel *xdg_toplevel, 
			    struct wl_array *caps)
{
	(void) data;
	(void) xdg_toplevel;
	(void) caps;
}


static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	handle_xdg_toplevel_configure,
	handle_xdg_toplevel_close,
	handle_xdg_toplevel_configure_bounds,
	handle_xdg_toplevel_wm_caps,
};

static struct window *
create_window(struct display *display, int width, int height, const char *app_id)
{
	struct window *window;

	assert(display->wm_base != NULL);

	window = static_cast<struct window *>(zalloc(sizeof(*window)));
	if (!window)
		return NULL;

	window->callback = NULL;
	window->display = display;
	window->width = width;
	window->height = height;
	window->surface = wl_compositor_create_surface(display->wl_compositor);

	if (display->wm_base) {
		window->xdg_surface =
			xdg_wm_base_get_xdg_surface(display->wm_base, window->surface);
		assert(window->xdg_surface);

		xdg_surface_add_listener(window->xdg_surface,
					 &xdg_surface_listener, window);
		window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
		assert(window->xdg_toplevel);

		xdg_toplevel_add_listener(window->xdg_toplevel,
					  &xdg_toplevel_listener, window);

		xdg_toplevel_set_app_id(window->xdg_toplevel, app_id);

		wl_surface_commit(window->surface);
		window->wait_for_configure = true;
	}

	return window;
}


static void
destroy_window(struct window *window)
{
	if (window->callback)
		wl_callback_destroy(window->callback);

	if (window->xdg_toplevel)
		xdg_toplevel_destroy(window->xdg_toplevel);

	if (window->xdg_surface)
		xdg_surface_destroy(window->xdg_surface);

	wl_surface_destroy(window->surface);
	free(window);
}

static void
signal_int(int sig, siginfo_t *si, void *_unused)
{
	(void) sig;
	(void) _unused;
	(void) si;

	running = 0;
}

static struct display *
create_display(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	struct display *display;

	display = static_cast<struct display *>(zalloc(sizeof(*display)));
	if (display == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	display->wl_display = wl_display_connect(NULL);
	assert(display->wl_display);

	display->has_xrgb = false;
	display->wl_registry = wl_display_get_registry(display->wl_display);

	wl_registry_add_listener(display->wl_registry, &registry_listener, display);
	wl_display_roundtrip(display->wl_display);

	if (display->shm == NULL) {
		fprintf(stderr, "No wl_shm global\n");
		return NULL;
	}

	wl_display_roundtrip(display->wl_display);

	if (!display->has_xrgb) {
		fprintf(stderr, "WL_SHM_FORMAT_XRGB32 not available\n");
		return NULL;
	}

	return display;
}

static void
destroy_display(struct display *display)
{
	if (display->shm)
		wl_shm_destroy(display->shm);

	if (display->wm_base)
		xdg_wm_base_destroy(display->wm_base);

	if (display->wl_compositor)
		wl_compositor_destroy(display->wl_compositor);

	wl_registry_destroy(display->wl_registry);
	wl_display_flush(display->wl_display);
	wl_display_disconnect(display->wl_display);
	free(display);
}

void
read_config(void)
{
	GKeyFile *conf_file;
	gchar *value;
	GError *err = NULL;

	bool ret;
	int n;
	unsigned width, height, x, y;

	// Load settings from configuration file if it exists
	conf_file = g_key_file_new();

	ret = g_key_file_load_from_dirs(conf_file, "AGL.conf",
					(const gchar**) g_get_system_config_dirs(),
					NULL, G_KEY_FILE_KEEP_COMMENTS, NULL);
	if (!ret)
		return;

	value = g_key_file_get_string(conf_file, "receiver", "geometry", &err);
	if (!value) {
		return;
	}

	n = sscanf(value, "%ux%u+%u,%u", &width, &height, &x, &y);
	if (n == 4) {
		g_window_width = width;
		g_window_height = height;
		g_window_pos_x = x;
		g_window_pos_y = y;
		fprintf(stdout, "Using window geometry %dx%d+%d,%d\n",
				g_window_width, g_window_height,
				g_window_pos_x, g_window_pos_y);
	} else {
		fprintf(stderr, "Invalid value for \"geometry\" key\n!");
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	struct sigaction sa;
	struct cluster_receiver_data receiver_data = {};
	struct display *display;
	struct window *window;

	std::string role = "cluster-receiver";
	SerialDbusBridge serial_bridge;

	sa.sa_sigaction = signal_int;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND | SA_SIGINFO;
	sigaction(SIGINT, &sa, NULL);

	int gargc = 2;
	char **gargv = (char**) malloc(2 * sizeof(char*));
	gargv[0] = strdup(argv[0]);
	gargv[1] = strdup("--gst-debug-level=2");

	std::string pipeline_str = \
		"rtpbin name=rtpbin udpsrc "
		"caps=\"application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=JPEG,payload=26\" "
		"port=5005 ! rtpbin.recv_rtp_sink_0 rtpbin. ! "
		"rtpjpegdepay ! jpegdec ! waylandsink";

	read_config();

	serial_bridge.start();

	if (env_flag_enabled("CLUSTER_RECEIVER_HEADLESS")) {
		std::cout << "cluster-receiver running headless serial D-Bus backend" << std::endl;
		while (running)
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

		serial_bridge.stop();
		return 0;
	}

	gst_init(&gargc, &gargv);

	std::cout << "Using pipeline: " << pipeline_str << std::endl;

	display = create_display(argc, argv);
	if (!display) {
		serial_bridge.stop();
		return -1;
	}

	GrpcClient *client = new GrpcClient(true);


	// this will set-up the x and y position to the same app as this one.
	// Note, that in this example, the surface area is the same as the
	// output dimensions streamed by the remote compositor
	client->SetAppFloat(role, g_window_pos_x, g_window_pos_y);

	// we use the role to set a correspondence between the top level
	// surface and our application, with the previous call letting the
	// compositor know that we're one and the same
	//
	window = create_window(display, g_window_width, g_window_height, role.c_str());

	if (!window) {
		serial_bridge.stop();
		delete client;
		return -1;
	}

	window->display = display;
	receiver_data.window = window;

	/* Initialise damage to full surface, so the padding gets painted */
	wl_surface_damage(window->surface, 0, 0,
			  window->width, window->height);

	if (!window->wait_for_configure)
		redraw(window, NULL, 0);

	GstElement *pipeline = gst_parse_launch(pipeline_str.c_str(), NULL);
	if (!pipeline) {
		std::cerr << "gstreamer pipeline construction failed!" << std::endl;
		exit(1);
	}
	receiver_data.pipeline = pipeline;

	GstBus *bus = gst_element_get_bus(pipeline);
	gst_bus_add_signal_watch(bus);

	g_signal_connect(bus, "message::error", G_CALLBACK(error_cb), &receiver_data);
	gst_bus_set_sync_handler(bus, bus_sync_handler, &receiver_data, NULL);
	gst_object_unref(bus);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	std::cout << "gstreamer pipeline running" << std::endl;

	// run the application
	while (running && ret != -1)
		ret = wl_display_dispatch(display->wl_display);

	destroy_window(window);
	destroy_display(display);

	serial_bridge.stop();

	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	delete client;

	return ret;
}
