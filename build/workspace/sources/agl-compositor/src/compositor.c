/*
 * Copyright © 2012-2024 Collabora, Ltd.
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

#include "ivi-compositor.h"
#include "policy.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>

#include <libweston/backend-drm.h>
#include <libweston/backend-wayland.h>
#ifdef HAVE_BACKEND_PIPEWIRE
#include <libweston/backend-pipewire.h>
#endif
#ifdef HAVE_BACKEND_X11
#include <libweston/backend-x11.h>
#endif
#ifdef HAVE_BACKEND_RDP
#include <libweston/backend-rdp.h>
#endif
#include <libweston/libweston.h>
#include <libweston/windowed-output-api.h>
#include <libweston/config-parser.h>
#include <libweston/weston-log.h>
#include <weston.h>

#include "shared/os-compatibility.h"
#include "shared/helpers.h"

#include "config.h"
#include "agl-shell-server-protocol.h"

#include "drm-lease.h"

#define WINDOWED_DEFAULT_WIDTH 1024
#define WINDOWED_DEFAULT_HEIGHT 768

static int cached_tm_mday = -1;
static struct weston_log_scope *log_scope;

static void
weston_output_lazy_align(struct weston_output *output);

struct ivi_compositor *
to_ivi_compositor(struct weston_compositor *ec)
{
	return weston_compositor_get_user_data(ec);
}

void
ivi_process_destroy(struct wet_process *process, int status, bool call_cleanup)
{
        wl_list_remove(&process->link);
        if (call_cleanup && process->cleanup)
                process->cleanup(process, status, process->cleanup_data);
        free(process->path);
        free(process);
}

struct ivi_output_config *
ivi_init_parsed_options(struct weston_compositor *compositor)
{
	struct ivi_compositor *ivi = to_ivi_compositor(compositor);
	struct ivi_output_config *config;

	config = zalloc(sizeof *config);
	if (!config)
		return NULL;

	config->width = 0;
	config->height = 0;
	config->scale = 0;
	config->transform = UINT32_MAX;

	ivi->parsed_options = config;

	return config;
}

static void
ivi_backend_destroy(struct ivi_backend *b)
{
        wl_list_remove(&b->link);
        wl_list_remove(&b->heads_changed.link);

        free(b);
}


static void
ivi_compositor_destroy_backends(struct ivi_compositor *ivi)
{
	struct ivi_backend *b, *tmp;

	wl_list_for_each_safe(b, tmp, &ivi->backends, link)
		ivi_backend_destroy(b);
}

static void
screenshot_allow_all(struct wl_listener *l, struct weston_output_capture_attempt *att)
{
	att->authorized = true;
}


static void
sigint_helper(int sig)
{
       raise(SIGUSR2);
}

struct {
        char *name;
        enum weston_renderer_type renderer;
} renderer_name_map[] = {
        { "auto", WESTON_RENDERER_AUTO },
        { "gl", WESTON_RENDERER_GL },
        { "noop", WESTON_RENDERER_NOOP },
        { "pixman", WESTON_RENDERER_PIXMAN },
};

struct {
	char *short_name;
	char *long_name;
	enum weston_compositor_backend backend;
} backend_name_map[] = {
	{ "drm", "drm-backend.so", WESTON_BACKEND_DRM },
	{ "rdp", "rdp-backend.so", WESTON_BACKEND_RDP },
	{ "pipewire", "pipewire-backend.so", WESTON_BACKEND_PIPEWIRE },
	{ "wayland", "wayland-backend.so", WESTON_BACKEND_WAYLAND },
	{ "x11", "x11-backend.so", WESTON_BACKEND_X11 },
};

bool
get_backend_from_string(const char *name, enum weston_compositor_backend *backend)
{
	size_t i;

	for (i = 0; i < ARRAY_LENGTH(backend_name_map); i++) {
		if (strcmp(name, backend_name_map[i].short_name) == 0 ||
		    strcmp(name, backend_name_map[i].long_name) == 0) {
			*backend = backend_name_map[i].backend;
			return true;
		}
	}

	return false;
}

bool
get_renderer_from_string(const char *name, enum weston_renderer_type *renderer)
{
	size_t i;

	if (!name)
		name = "auto";

	for (i = 0; i < ARRAY_LENGTH(renderer_name_map); i++) {
		if (strcmp(name, renderer_name_map[i].name) == 0) {
			*renderer = renderer_name_map[i].renderer;
			return true;
		}
	}

	return false;
}

static void
handle_output_destroy(struct wl_listener *listener, void *data)
{
	struct ivi_output *output;

	output = wl_container_of(listener, output, output_destroy);
	assert(output->output == data);

	if (output->fullscreen_view.fs &&
	    output->fullscreen_view.fs->view) {
		weston_surface_unref(output->fullscreen_view.fs->view->surface);
		weston_buffer_destroy_solid(output->fullscreen_view.buffer_ref);
		output->fullscreen_view.fs->view = NULL;
	}

	ivi_layout_save(output->ivi, output);

	output->output = NULL;
	wl_list_remove(&output->output_destroy.link);
}

struct ivi_output *
to_ivi_output(struct weston_output *o)
{
	struct wl_listener *listener;
	struct ivi_output *output;

	listener = weston_output_get_destroy_listener(o, handle_output_destroy);
	if (!listener)
		return NULL;

	output = wl_container_of(listener, output, output_destroy);

	return output;
}

static void
ivi_output_configure_app_id(struct ivi_output *ivi_output)
{
	if (ivi_output->config) {
		if (ivi_output->app_ids != NULL)
			return;

		weston_config_section_get_string(ivi_output->config,
						 "agl-shell-app-id",
						 &ivi_output->app_ids,
						 NULL);

		if (ivi_output->app_ids == NULL)
			return;

		weston_log("Will place app_id %s on output %s\n",
				ivi_output->app_ids, ivi_output->name);
	}
}

static struct ivi_output *
ivi_ensure_output(struct ivi_compositor *ivi, struct ivi_backend *ivi_backend, 
		  char *name, struct weston_config_section *config,
		  struct weston_head *head)
{
	struct ivi_output *output = NULL;
	wl_list_for_each(output, &ivi->outputs, link) {
		if (strcmp(output->name, name) == 0) {
			free(name);
			return output;
		}
	}

	output = zalloc(sizeof *output);
	if (!output) {
		free(name);
		return NULL;
	}

	output->ivi = ivi;
	output->name = name;
	output->config = config;

	output->output =
		weston_compositor_create_output(ivi->compositor, head, head->name);
	if (!output->output) {
		free(output->name);
		free(output);
		return NULL;
	}

	/* simple_output_configure might assume we have an ivi_output created
	 * by this point, which we do but we can only link it to a
	 * weston_output through the destroy listener, so install it earlier
	 * before actually running the callback handler */
	output->output_destroy.notify = handle_output_destroy;
	weston_output_add_destroy_listener(output->output,
					   &output->output_destroy);

	if (ivi_backend->simple_output_configure) {
		int ret = ivi_backend->simple_output_configure(output->output);
		if (ret < 0) {
			weston_log("Configuring output \"%s\" failed.\n",
					weston_head_get_name(head));
			weston_output_destroy(output->output);
			ivi->init_failed = true;
			return NULL;
		}

		weston_output_lazy_align(output->output);

		if (weston_output_enable(output->output) < 0) {
			weston_log("Enabling output \"%s\" failed.\n",
					weston_head_get_name(head));
			weston_output_destroy(output->output);
			ivi->init_failed = true;
			return NULL;
		}
	}


	wl_list_insert(&ivi->outputs, &output->link);
	ivi_output_configure_app_id(output);

	return output;
}

static int
count_heads(struct weston_output *output)
{
	struct weston_head *iter = NULL;
	int n = 0;

	while ((iter = weston_output_iterate_heads(output, iter)))
		++n;

	return n;
}

static void
handle_head_destroy(struct wl_listener *listener, void *data)
{
	struct weston_head *head = data;
	struct weston_output *output;

	wl_list_remove(&listener->link);
	free(listener);

	output = weston_head_get_output(head);

	/* On shutdown path, the output might be already gone. */
	if (!output)
		return;

	/* We're the last head */
	if (count_heads(output) <= 1)
		weston_output_destroy(output);
}

static void
add_head_destroyed_listener(struct weston_head *head)
{
	/* We already have a destroy listener */
	if (weston_head_get_destroy_listener(head, handle_head_destroy))
		return;

	struct wl_listener *listener = zalloc(sizeof *listener);
	if (!listener)
		return;

	listener->notify = handle_head_destroy;
	weston_head_add_destroy_listener(head, listener);
}

static int
ivi_configure_windowed_output_from_config(struct ivi_output *output,
					  struct ivi_output_config *defaults)
{
	struct ivi_compositor *ivi = output->ivi;
	struct weston_config_section *section = output->config;
	int width;
	int height;
	int scale;

	if (section) {
		char *mode;

		weston_config_section_get_string(section, "mode", &mode, NULL);
		if (!mode || sscanf(mode, "%dx%d", &width, &height) != 2) {
			weston_log("Invalid mode for output %s. Using defaults.\n",
				   output->name);
			width = defaults->width;
			height = defaults->height;
		}
		free(mode);
	} else {
		width = defaults->width;
		height = defaults->height;
	}

	scale = defaults->scale;

	if (ivi->cmdline.width)
		width = ivi->cmdline.width;
	if (ivi->cmdline.height)
		height = ivi->cmdline.height;
	if (ivi->cmdline.scale)
		scale = ivi->cmdline.scale;

	weston_output_set_scale(output->output, scale);
	weston_output_set_transform(output->output, defaults->transform);

	if (ivi->window_api->output_set_size(output->output, width, height) < 0) {
		weston_log("Cannot configure output '%s' using weston_windowed_output_api.\n",
			   output->name);
		return -1;
	}


	weston_log("Configured windowed_output_api to %dx%d, scale %d\n",
		   width, height, scale);

	return 0;
}

static int
parse_transform(const char *transform, uint32_t *out)
{
	static const struct { const char *name; uint32_t token; } transforms[] = {
		{ "normal",             WL_OUTPUT_TRANSFORM_NORMAL },
		{ "rotate-90",          WL_OUTPUT_TRANSFORM_90 },
		{ "rotate-180",         WL_OUTPUT_TRANSFORM_180 },
		{ "rotate-270",         WL_OUTPUT_TRANSFORM_270 },
		{ "flipped",            WL_OUTPUT_TRANSFORM_FLIPPED },
		{ "flipped-rotate-90",  WL_OUTPUT_TRANSFORM_FLIPPED_90 },
		{ "flipped-rotate-180", WL_OUTPUT_TRANSFORM_FLIPPED_180 },
		{ "flipped-rotate-270", WL_OUTPUT_TRANSFORM_FLIPPED_270 },
	};

	for (size_t i = 0; i < ARRAY_LENGTH(transforms); i++)
		if (strcmp(transforms[i].name, transform) == 0) {
			*out = transforms[i].token;
			return 0;
		}

	*out = WL_OUTPUT_TRANSFORM_NORMAL;
	return -1;
}

int
parse_activation_area(const char *geometry, struct ivi_output *output)
{
	int n;
	unsigned width, height, x, y;

	n = sscanf(geometry, "%ux%u+%u,%u", &width, &height, &x, &y);
	if (n != 4) {
		return -1;
	}
	output->area_activation.width = width;
	output->area_activation.height = height;
	output->area_activation.x = x;
	output->area_activation.y = y;
	return 0;
}

static int
drm_configure_output(struct ivi_output *output)
{
	struct ivi_compositor *ivi = output->ivi;
	struct weston_config_section *section = output->config;
	int32_t scale = 1;
	uint32_t transform = WL_OUTPUT_TRANSFORM_NORMAL;
	enum weston_drm_backend_output_mode mode =
		WESTON_DRM_BACKEND_OUTPUT_PREFERRED;
	char *modeline = NULL;
	char *gbm_format = NULL;
	char *seat = NULL;

	if (section) {
		char *t;

		weston_config_section_get_int(section, "scale", &scale, 1);
		weston_config_section_get_string(section, "transform", &t, "normal");
		if (parse_transform(t, &transform) < 0)
			weston_log("Invalid transform \"%s\" for output %s\n",
				   t, output->name);
		weston_config_section_get_string(section, "activation-area", &t, "");
		if (parse_activation_area(t, output) < 0)
			weston_log("Invalid activation-area \"%s\" for output %s\n",
				   t, output->name);
		free(t);
	}

	weston_output_set_scale(output->output, scale);
	weston_output_set_transform(output->output, transform);

	if (section) {
		char *m;
		weston_config_section_get_string(section, "mode", &m, "preferred");

		/* This should have been handled earlier */
		assert(strcmp(m, "off") != 0);

		if (ivi->cmdline.use_current_mode || strcmp(m, "current") == 0) {
			mode = WESTON_DRM_BACKEND_OUTPUT_CURRENT;
		} else if (strcmp(m, "preferred") != 0) {
			modeline = m;
			m = NULL;
		}
		free(m);

		weston_config_section_get_string(section, "gbm-format",
						 &gbm_format, NULL);

		weston_config_section_get_string(section, "seat", &seat, "");
	}

	if (ivi->drm_api->set_mode(output->output, mode, modeline) < 0) {
		weston_log("Cannot configure output using weston_drm_output_api.\n");
		free(modeline);
		return -1;
	}
	free(modeline);

	ivi->drm_api->set_gbm_format(output->output, gbm_format);
	free(gbm_format);

	ivi->drm_api->set_seat(output->output, seat);
	free(seat);

	return 0;
}

/*
 * Reorgainizes the output's add array into two sections.
 * add[0..ret-1] are the heads that failed to get attached.
 * add[ret..add_len] are the heads that were successfully attached.
 *
 * The order between elements in each section is stable.
 */
static size_t
try_attach_heads(struct ivi_output *output)
{
	size_t fail_len = 0;

	for (size_t i = 1; i < output->add_len; i++) {
		if (weston_output_attach_head(output->output, output->add[i]) < 0) {
			struct weston_head *tmp = output->add[i];
			memmove(&output->add[fail_len + 1], output->add[fail_len],
				sizeof output->add[0] * (i - fail_len));
			output->add[fail_len++] = tmp;
		}
	}
	return fail_len;
}

/* Place output exactly to the right of the most recently enabled output.
 *
 * Historically, we haven't given much thought to output placement,
 * simply adding outputs in a horizontal line as they're enabled. This
 * function simply sets an output's x coordinate to the right of the
 * most recently enabled output, and its y to zero.
 *
 * If you're adding new calls to this function, you're also not giving
 * much thought to output placement, so please consider carefully if
 * it's really doing what you want.
 *
 * You especially don't want to use this for any code that won't
 * immediately enable the passed output.
 */
static void
weston_output_lazy_align(struct weston_output *output)
{
	struct weston_compositor *c;
	struct weston_output *peer;
	int next_x = 0;

	/* Put this output to the right of the most recently enabled output */
	c = output->compositor;
	if (!wl_list_empty(&c->output_list)) {
		peer = container_of(c->output_list.prev,
				struct weston_output, link);
		next_x = peer->pos.c.x + peer->width;
	}
	output->pos.c.x = next_x;
	output->pos.c.y = 0;
}


/*
 * Like try_attach_heads, this reorganizes the output's add array into a failed
 * and successful section.
 * i is the number of heads that already failed the previous step.
 */
static size_t
try_enable_output(struct ivi_output *output, size_t i)
{
	for (; i < output->add_len; ++i) {
		struct weston_head *head;

		weston_output_lazy_align(output->output);

		if (weston_output_enable(output->output) == 0)
			break;

		head = output->add[output->add_len - 1];
		memmove(&output->add[i + 1], &output->add[i],
			sizeof output->add[0] * (output->add_len - i));
		output->add[i] = head;

		weston_head_detach(head);
	}

	return i;
}

static int
try_attach_enable_heads(struct ivi_output *output)
{
	size_t fail_len;
	assert(!output->output->enabled);

	fail_len = try_attach_heads(output);

	if (drm_configure_output(output) < 0)
		return -1;

	fail_len = try_enable_output(output, fail_len);

	/* All heads failed to be attached */
	if (fail_len == output->add_len)
		return -1;

	/* For each successful head attached */
	for (size_t i = fail_len; i < output->add_len; ++i)
		add_head_destroyed_listener(output->add[i]);

	ivi_layout_restore(output->ivi, output);

	output->add_len = fail_len;
	return 0;
}

static int
process_output(struct ivi_output *output)
{
	if (output->output->enabled) {
		output->add_len = try_attach_heads(output);
		return output->add_len == 0 ? 0 : -1;
	}

	return try_attach_enable_heads(output);
}

static struct weston_head *
ivi_backend_iterate_heads(struct ivi_compositor *ivi, struct ivi_backend *wb,
		struct weston_head *iter)
{
	while ((iter = weston_compositor_iterate_heads(ivi->compositor, iter))) {
		if (iter->backend == wb->backend)
			break;
	}

	return iter;
}


static void
drm_head_disable(struct ivi_compositor *ivi, struct weston_head *head)
{
	struct weston_output *output;
	struct ivi_output *ivi_output;
	struct wl_listener *listener;

	output = weston_head_get_output(head);
	assert(output);

	listener = weston_output_get_destroy_listener(output,
						      handle_output_destroy);
	assert(listener);

	ivi_output = wl_container_of(listener, ivi_output, output_destroy);
	assert(ivi_output->output == output);

	weston_head_detach(head);
	if (count_heads(ivi_output->output) == 0) {
		if (ivi_output->output) {
			/* ivi_output->output destruction may be deferred in
			 * some cases (see drm_output_destroy()), so we need to
			 * forcibly trigger the destruction callback now, or
			 * otherwise would later access data that we are about
			 * to free
			 */
			struct weston_output *save = ivi_output->output;

			handle_output_destroy(&ivi_output->output_destroy, save);
			weston_output_destroy(save);
		}
	}
	wl_list_remove(&ivi_output->link);
	free(ivi_output->app_ids);
	free(ivi_output);
}

static struct weston_config_section *
find_controlling_output_config(struct weston_config *config,
			       const char *name)
{
	struct weston_config_section *section;
	char *same_as;
	int depth = 0;

	same_as = strdup(name);
	do {
		section = weston_config_get_section(config, "output",
						    "name", same_as);
		if (!section && depth > 0)
			weston_log("Configuration error: output section reffered"
				   "to by same-as=%s not found.\n", same_as);
		free(same_as);

		if (!section)
			return NULL;

		if (depth++ > 8) {
			weston_log("Configuration error: same-as nested too "
				   "deep for output '%s'.\n", name);
			return NULL;
		}

		weston_config_section_get_string(section, "same-as",
						 &same_as, NULL);
	} while (same_as);

	return section;
}

static void
drm_head_prepare_enable(struct ivi_compositor *ivi, struct ivi_backend *ivi_backend, struct weston_head *head)
{
	const char *name = weston_head_get_name(head);
	struct weston_config_section *section;
	struct ivi_output *output;
	char *output_name = NULL;


	section = find_controlling_output_config(ivi->config, name);
	if (section) {
		char *mode;

		weston_config_section_get_string(section, "mode", &mode, NULL);
		if (mode && strcmp(mode, "off") == 0) {
			free(mode);
			return;
		}
		free(mode);

		weston_config_section_get_string(section, "name",
						 &output_name, NULL);
	} else {
		output_name = strdup(name);
	}

	if (!output_name)
		return;

	output = ivi_ensure_output(ivi, ivi_backend, output_name, section, head);
	if (!output)
		return;

	if (output->add_len >= ARRAY_LENGTH(output->add))
		return;

	output->add[output->add_len++] = head;
}

static void
drm_heads_changed(struct wl_listener *listener, void *arg)
{
	struct weston_compositor *compositor = arg;
	struct weston_head *head = NULL;
	struct ivi_compositor *ivi = to_ivi_compositor(compositor);
	struct ivi_output *output;
	struct ivi_backend *ivi_backend =
		container_of(listener, struct ivi_backend, heads_changed);

	while ((head = ivi_backend_iterate_heads(ivi, ivi_backend, head))) {
		bool connected = weston_head_is_connected(head);
		bool enabled = weston_head_is_enabled(head);
		bool changed = weston_head_is_device_changed(head);
		bool non_desktop = weston_head_is_non_desktop(head);

		if (connected && !enabled && !non_desktop)
			drm_head_prepare_enable(ivi, ivi_backend, head);
		else if (!connected && enabled)
			drm_head_disable(ivi, head);
		else if (enabled && changed)
			weston_log("Detected a monitor change on head '%s', "
				   "not bothering to do anything about it.\n",
				   weston_head_get_name(head));

		weston_head_reset_device_changed(head);
	}

	wl_list_for_each(output, &ivi->outputs, link) {
		if (output->add_len == 0)
			continue;

		if (process_output(output) < 0) {
			output->add_len = 0;
			ivi->init_failed = true;
		}
	}
}

static void
simple_head_enable(struct ivi_compositor *ivi, struct ivi_backend *ivi_backend,
		   struct weston_head *head)
{
	struct ivi_output *output;
	struct weston_config_section *section;
	char *output_name = NULL;
	const char *name = weston_head_get_name(head);

	section = find_controlling_output_config(ivi->config, name);
	if (section) {
		char *mode;

		weston_config_section_get_string(section, "mode", &mode, NULL);
		if (mode && strcmp(mode, "off") == 0) {
			free(mode);
			return;
		}
		free(mode);

		weston_config_section_get_string(section, "name",
						 &output_name, NULL);
	} else {
		output_name = strdup(name);
	}

	if (!output_name)
		return;

	output = ivi_ensure_output(ivi, ivi_backend, output_name, section, head);
	if (!output) {
		weston_log("Failed to create output %s\n", output_name);
		return;
	}

	add_head_destroyed_listener(head);
}


static void
simple_head_disable(struct weston_head *head)
{
	struct weston_output *output;
	struct wl_listener *listener;

	listener = weston_head_get_destroy_listener(head, handle_head_destroy);
	wl_list_empty(&listener->link);

	output = weston_head_get_output(head);
	assert(output);
	weston_output_destroy(output);
}


static void
simple_heads_changed(struct wl_listener *listener, void *arg)
{
	struct weston_compositor *compositor = arg;
	struct ivi_compositor *ivi = to_ivi_compositor(compositor);
	struct weston_head *head = NULL;
	bool connected;
	bool enabled;
	bool changed;
	bool non_desktop;
	struct ivi_backend *ivi_backend =
		container_of(listener, struct ivi_backend, heads_changed);

	while ((head = ivi_backend_iterate_heads(ivi, ivi_backend, head))) {
		connected = weston_head_is_connected(head);
		enabled = weston_head_is_enabled(head);
		changed = weston_head_is_device_changed(head);
		non_desktop = weston_head_is_non_desktop(head);

		if (connected && !enabled && !non_desktop) {
			simple_head_enable(ivi, ivi_backend, head);
		} else if (!connected && enabled) {
			simple_head_disable(head);
		} else if (enabled && changed) {
			weston_log("Detected a monitor change on head '%s', "
					"not bothering to do anything about it.\n",
					weston_head_get_name(head));
		}
		weston_head_reset_device_changed(head);
	}
}

static int
load_drm_backend(struct ivi_compositor *ivi, int *argc, char *argv[],
		 enum weston_renderer_type renderer)
{
	struct weston_drm_backend_config config = {
		.base = {
			.struct_version = WESTON_DRM_BACKEND_CONFIG_VERSION,
			.struct_size = sizeof config,
		},
	};
	struct weston_config_section *section;
	int use_current_mode = 0;
	int ret = -1;
	bool force_pixman = false;
	bool use_shadow;
	bool without_input = false;
	struct ivi_backend *ivi_backend = NULL;
	char *drm_lease_name = NULL;

	const struct weston_option options[] = {
		{ WESTON_OPTION_STRING, "seat", 0, &config.seat_id },
		{ WESTON_OPTION_STRING, "drm-device", 0, &config.specific_device },
		{ WESTON_OPTION_STRING, "drm-lease", 0, &drm_lease_name },
		{ WESTON_OPTION_BOOLEAN, "current-mode", 0, &use_current_mode },
		{ WESTON_OPTION_BOOLEAN, "use-pixman", 0, &force_pixman },
		{ WESTON_OPTION_BOOLEAN, "continue-without-input", false, &without_input }
	};

	parse_options(options, ARRAY_LENGTH(options), argc, argv);

	if (force_pixman)
		config.renderer = WESTON_RENDERER_PIXMAN;
	else
		config.renderer = WESTON_RENDERER_AUTO;

	ivi->cmdline.use_current_mode = use_current_mode;

	section = weston_config_get_section(ivi->config, "core", NULL, NULL);
	weston_config_section_get_string(section, "gbm-format",
					 &config.gbm_format, NULL);
	weston_config_section_get_uint(section, "pageflip-timeout",
				       &config.pageflip_timeout, 0);
	weston_config_section_get_bool(section, "pixman-shadow", &use_shadow, 1);
	config.use_pixman_shadow = use_shadow;

	ret = setup_drm_lease(&ivi->drm_lease, &config, drm_lease_name);
	if (ret < 0)
		goto error;

	if (without_input)
		ivi->compositor->require_input = !without_input;

	ivi_backend = zalloc(sizeof(struct ivi_backend));
	ivi_backend->backend =
		weston_compositor_load_backend(ivi->compositor, WESTON_BACKEND_DRM,
					       &config.base);
	if (!ivi_backend->backend) {
		weston_log("Failed to load DRM backend\n");
		free(ivi_backend);
		return -1;
	}

	ivi_backend->heads_changed.notify = drm_heads_changed;
	weston_compositor_add_heads_changed_listener(ivi->compositor,
						     &ivi_backend->heads_changed);
	wl_list_insert(&ivi->backends, &ivi_backend->link);

	ivi->drm_api = weston_drm_output_get_api(ivi->compositor);
	if (!ivi->drm_api) {
		weston_log("Cannot use drm output api.\n");
		goto error;
	}

	free(drm_lease_name);

	return 0;

error:
	free(config.gbm_format);
	free(drm_lease_name);
	free(config.seat_id);
	return -1;
}

static void
windowed_parse_common_options(struct ivi_compositor *ivi, int *argc, char *argv[],
			      bool *force_pixman, bool *fullscreen, int *output_count)
{
	struct weston_config_section *section;
	bool pixman;
	int fs = 0;

	const struct weston_option options[] = {
		{ WESTON_OPTION_INTEGER, "width", 0, &ivi->cmdline.width },
		{ WESTON_OPTION_INTEGER, "height", 0, &ivi->cmdline.height },
		{ WESTON_OPTION_INTEGER, "scale", 0, &ivi->cmdline.scale },
		{ WESTON_OPTION_BOOLEAN, "use-pixman", 0, &force_pixman },
		{ WESTON_OPTION_BOOLEAN, "fullscreen", 0, &fs },
		{ WESTON_OPTION_INTEGER, "output-count", 0, output_count },
	};

	section = weston_config_get_section(ivi->config, "core", NULL, NULL);
	weston_config_section_get_bool(section, "use-pixman", &pixman, false);

	*output_count = 1;
	parse_options(options, ARRAY_LENGTH(options), argc, argv);
	*force_pixman = pixman;
	*fullscreen = fs;
}

static int
windowed_create_outputs(struct ivi_compositor *ivi, int output_count,
			struct weston_backend *backend,
			const char *match_prefix, const char *name_prefix)
{
	struct weston_config_section *section = NULL;
	const char *section_name;
	char *default_output = NULL;
	int i = 0;
	size_t match_len = strlen(match_prefix);

	while (weston_config_next_section(ivi->config, &section, &section_name)) {
		char *output_name;

		if (i >= output_count)
			break;

		if (strcmp(section_name, "output") != 0)
			continue;

		weston_config_section_get_string(section, "name", &output_name, NULL);
		if (output_name == NULL)
			continue;
		if (strncmp(output_name, match_prefix, match_len) != 0) {
			free(output_name);
			continue;
		}

		if (ivi->window_api->create_head(backend, output_name) < 0) {
			free(output_name);
			return -1;
		}

		free(output_name);
		++i;
	}

	for (; i < output_count; ++i) {
		if (asprintf(&default_output, "%s%d", name_prefix, i) < 0)
			return -1;

		if (ivi->window_api->create_head(backend, default_output) < 0) {
			free(default_output);
			return -1;
		}

		free(default_output);
	}

	return 0;
}


static int
wayland_backend_output_configure(struct weston_output *output)
{
	struct ivi_output *ivi_output = to_ivi_output(output);

	struct ivi_output_config defaults = {
		.width = WINDOWED_DEFAULT_WIDTH,
		.height = WINDOWED_DEFAULT_HEIGHT,
		.scale = 1,
		.transform = WL_OUTPUT_TRANSFORM_NORMAL
	};

	if (!ivi_output) {
		weston_log("Failed to configure and enable Wayland output. No ivi-output available!\n");
		return -1;
	}

	return ivi_configure_windowed_output_from_config(to_ivi_output(output), &defaults);
}

static int
load_wayland_backend(struct ivi_compositor *ivi, int *argc, char *argv[],
		     enum weston_renderer_type renderer)
{
	struct weston_wayland_backend_config config = {
		.base = {
			.struct_version = WESTON_WAYLAND_BACKEND_CONFIG_VERSION,
			.struct_size = sizeof config,
		},
	};
	struct weston_config_section *section;
	int sprawl = 0;
	int output_count;
	bool force_pixman = false;
	struct ivi_backend *ivi_backend = NULL;

	const struct weston_option options[] = {
		{ WESTON_OPTION_STRING, "display", 0, &config.display_name },
		{ WESTON_OPTION_STRING, "sprawl", 0, &sprawl },
	};

	windowed_parse_common_options(ivi, argc, argv, &force_pixman,
				      &config.fullscreen, &output_count);

	parse_options(options, ARRAY_LENGTH(options), argc, argv);
	config.sprawl = sprawl;

	section = weston_config_get_section(ivi->config, "shell", NULL, NULL);
	weston_config_section_get_string(section, "cursor-theme",
					 &config.cursor_theme, NULL);
	weston_config_section_get_int(section, "cursor-size",
				      &config.cursor_size, 32);

	ivi_backend = zalloc(sizeof(struct ivi_backend));
	ivi_backend->backend = weston_compositor_load_backend(ivi->compositor,
						      WESTON_BACKEND_WAYLAND,
						      &config.base);
	free(config.cursor_theme);
	free(config.display_name);

	if (!ivi_backend->backend) {
		weston_log("Failed to create Wayland backend!\n");
		free(ivi_backend);
		return -1;
	}

	ivi_backend->simple_output_configure = wayland_backend_output_configure;
	ivi_backend->heads_changed.notify = simple_heads_changed;
	weston_compositor_add_heads_changed_listener(ivi->compositor,
						     &ivi_backend->heads_changed);


	wl_list_insert(&ivi->backends, &ivi_backend->link);

	ivi->window_api = weston_windowed_output_get_api(ivi->compositor);

	/*
	 * We will just assume if load_backend() finished cleanly and
	 * windowed_output_api is not present that wayland backend is started
	 * with --sprawl or runs on fullscreen-shell. In this case, all values
	 * are hardcoded, so nothing can be configured; simply create and
	 * enable an output.
	 */
	if (ivi->window_api == NULL)
		return 0;

	return windowed_create_outputs(ivi, output_count, ivi_backend->backend, "WL", "wayland");
}

#ifdef HAVE_BACKEND_X11
static int
x11_backend_output_configure(struct weston_output *output)
{
	struct ivi_output *ivi_output = to_ivi_output(output);

	struct ivi_output_config defaults = {
		.width = WINDOWED_DEFAULT_WIDTH,
		.height = WINDOWED_DEFAULT_HEIGHT,
		.scale = 1,
		.transform = WL_OUTPUT_TRANSFORM_NORMAL
	};

	if (!ivi_output) {
		weston_log("Failed to configure and enable X11 output. No ivi-output available!\n");
		return -1;
	}


	return ivi_configure_windowed_output_from_config(ivi_output, &defaults);
}

static int
load_x11_backend(struct ivi_compositor *ivi, int *argc, char *argv[],
		 enum weston_renderer_type renderer)
{
	struct weston_x11_backend_config config = {
		.base = {
			.struct_version = WESTON_X11_BACKEND_CONFIG_VERSION,
			.struct_size = sizeof config,
		},
	};
	int no_input = 0;
	int output_count;
	bool force_pixman = false;
	struct ivi_backend *ivi_backend = NULL;

	const struct weston_option options[] = {
	       { WESTON_OPTION_BOOLEAN, "no-input", 0, &no_input },
	};

	windowed_parse_common_options(ivi, argc, argv, &force_pixman,
				      &config.fullscreen, &output_count);

	parse_options(options, ARRAY_LENGTH(options), argc, argv);
	if (force_pixman)
		config.renderer = WESTON_RENDERER_PIXMAN;
	else
		config.renderer = WESTON_RENDERER_AUTO;
	config.no_input = no_input;


	ivi_backend = zalloc(sizeof(struct ivi_backend));
	ivi_backend->backend = weston_compositor_load_backend(ivi->compositor,
						      WESTON_BACKEND_X11,
						      &config.base);
	if (!ivi_backend->backend) {
		weston_log("Failed to create X11 backend!\n");
		free(ivi_backend);
		return -1;
	}

	ivi_backend->simple_output_configure = x11_backend_output_configure;
	ivi_backend->heads_changed.notify = simple_heads_changed;
	weston_compositor_add_heads_changed_listener(ivi->compositor,
						     &ivi_backend->heads_changed);


	ivi->window_api = weston_windowed_output_get_api(ivi->compositor);
	if (!ivi->window_api) {
		weston_log("Cannot use weston_windowed_output_api.\n");
		return -1;
	}

	wl_list_insert(&ivi->backends, &ivi_backend->link);

	return windowed_create_outputs(ivi, output_count, ivi_backend->backend, "X", "screen");
}
#else
static int
load_x11_backend(struct ivi_compositor *ivi, int *argc, char **argv,
		 enum weston_renderer_type renderer)
{
	return -1;
}
#endif

#ifdef HAVE_BACKEND_RDP
static void
weston_rdp_backend_config_init(struct weston_rdp_backend_config *config)
{
        config->base.struct_version = WESTON_RDP_BACKEND_CONFIG_VERSION;
        config->base.struct_size = sizeof(struct weston_rdp_backend_config);

        config->renderer = WESTON_RENDERER_AUTO;
        config->bind_address = NULL;
        config->port = 3389;
        config->rdp_key = NULL;
        config->server_cert = NULL;
        config->server_key = NULL;
        config->env_socket = 0;
        config->external_listener_fd = -1;
        config->no_clients_resize = 0;
        config->force_no_compression = 0;
        config->remotefx_codec = true;
        config->refresh_rate = RDP_DEFAULT_FREQ;

}

static int
rdp_backend_output_configure(struct weston_output *output)
{
	struct ivi_compositor *ivi = to_ivi_compositor(output->compositor);
	struct ivi_output_config *parsed_options = ivi->parsed_options;
	const struct weston_rdp_output_api *api =
		weston_rdp_output_get_api(output->compositor);
	int width = 640;
	int height = 480;
	struct weston_config_section *section;
	uint32_t transform = WL_OUTPUT_TRANSFORM_NORMAL;
	char *transform_string;
	struct weston_mode new_mode = {};

	assert(parsed_options);

	if (!api) {
		weston_log("Cannot use weston_rdp_output_api.\n");
		return -1;
	}

	section = weston_config_get_section(ivi->config, "rdp", NULL, NULL);

	if (parsed_options->width)
		width = parsed_options->width;

	if (parsed_options->height)
		height = parsed_options->height;

	weston_output_set_scale(output, 1);

	weston_config_section_get_int(section, "width",
			&width, width);

	weston_config_section_get_int(section, "height",
			&height, height);

	if (parsed_options->transform)
		transform = parsed_options->transform;

	weston_config_section_get_string(section, "transform",
			&transform_string, "normal");

	if (parse_transform(transform_string, &transform) < 0) {
		weston_log("Invalid transform \"%s\" for output %s\n",
			   transform_string, output->name);
		return -1;
	}


	new_mode.width = width;
	new_mode.height = height;

	weston_log("Setting modeline to %dx%d\n", width, height);

	api->output_set_mode(output, &new_mode);
	weston_output_set_transform(output, transform);

	return 0;
}

static int
load_rdp_backend(struct ivi_compositor *ivi, int *argc, char **argv,
		 enum weston_renderer_type renderer)
{
	struct weston_rdp_backend_config config = {};
	struct weston_config_section *section;
	bool no_remotefx_codec = false;
	struct ivi_backend *ivi_backend = NULL;

	struct ivi_output_config *parsed_options = ivi_init_parsed_options(ivi->compositor);
	if (!parsed_options)
		return -1;

	weston_rdp_backend_config_init(&config);

	const struct weston_option rdp_options[] = {
		{ WESTON_OPTION_BOOLEAN, "env-socket", 0, &config.env_socket },
		{ WESTON_OPTION_INTEGER, "external-listener-fd", 0, &config.external_listener_fd },
		{ WESTON_OPTION_INTEGER, "width", 0, &parsed_options->width },
		{ WESTON_OPTION_INTEGER, "height", 0, &parsed_options->height },
		{ WESTON_OPTION_STRING,  "address", 0, &config.bind_address },
		{ WESTON_OPTION_INTEGER, "port", 0, &config.port },
		{ WESTON_OPTION_BOOLEAN, "no-clients-resize", 0, &config.no_clients_resize },
		{ WESTON_OPTION_STRING,  "rdp4-key", 0, &config.rdp_key },
		{ WESTON_OPTION_STRING,  "rdp-tls-cert", 0, &config.server_cert },
		{ WESTON_OPTION_STRING,  "rdp-tls-key", 0, &config.server_key },
		{ WESTON_OPTION_INTEGER, "scale", 0, &parsed_options->scale },
		{ WESTON_OPTION_BOOLEAN, "force-no-compression", 0, &config.force_no_compression },
		{ WESTON_OPTION_BOOLEAN, "no-remotefx-codec", 0, &no_remotefx_codec },
	};

	config.remotefx_codec = !no_remotefx_codec;
	config.renderer = renderer;

	section = weston_config_get_section(ivi->config, "rdp", NULL, NULL);

	weston_config_section_get_int(section, "refresh-rate",
			&config.refresh_rate, RDP_DEFAULT_FREQ);

	weston_config_section_get_string(section, "tls-cert",
			&config.server_cert, config.server_cert);

	weston_config_section_get_string(section, "tls-key",
			&config.server_key, config.server_key);


	parse_options(rdp_options, ARRAY_LENGTH(rdp_options), argc, argv);
	weston_log("No clients resize: %d\n", config.no_clients_resize);


	ivi_backend = zalloc(sizeof(struct ivi_backend));
	ivi_backend->backend = weston_compositor_load_backend(ivi->compositor,
					    WESTON_BACKEND_RDP, &config.base);
	if (!ivi_backend->backend) {
		weston_log("Failed to create RDP backend\n");
		free(ivi_backend);
		return -1;
	}

	ivi_backend->simple_output_configure = rdp_backend_output_configure;
	ivi_backend->heads_changed.notify = simple_heads_changed;
	weston_compositor_add_heads_changed_listener(ivi->compositor,
						     &ivi_backend->heads_changed);


	free(config.bind_address);
	free(config.rdp_key);
	free(config.server_cert);
	free(config.server_key);

	wl_list_insert(&ivi->backends, &ivi_backend->link);

	return 0;
}
#else
static int
load_rdp_backend(struct ivi_compositor *ivi, int *argc, char **argv,
		 enum weston_renderer_type renderer)
{
	return -1;
}
#endif

#ifdef HAVE_BACKEND_PIPEWIRE
static void
pipewire_backend_config_init(struct weston_pipewire_backend_config *config)
{
	config->base.struct_version = WESTON_PIPEWIRE_BACKEND_CONFIG_VERSION;
	config->base.struct_size = sizeof(struct weston_pipewire_backend_config);
}

static int
pipewire_backend_output_configure(struct weston_output *output)
{
	int default_width = 640;
	int default_height = 480;
	int width = 0;
	int height = 0;
	char *mode;
	struct ivi_compositor *ivi = to_ivi_compositor(output->compositor);
	struct ivi_output_config *parsed_options = ivi->parsed_options;

	const struct weston_pipewire_output_api *api =
		weston_pipewire_output_get_api(output->compositor);
	struct weston_config_section *section;
	char *gbm_format = NULL;
	uint32_t transform = WL_OUTPUT_TRANSFORM_NORMAL;
	int scale = 1;

	assert(parsed_options);

	if (!api) {
		weston_log("Cannot use weston_pipewire_output_api.\n");
		return -1;
	}

	section = weston_config_get_section(ivi->config,
					    "output", "name", output->name);

	weston_config_section_get_string(section, "mode", &mode, NULL);
	if (!mode || sscanf(mode, "%dx%d", &width, &height) != 2) {
		weston_log("Invalid mode for output %s. Using defaults.\n",
			   output->name);
		width = default_width;
		height = default_height;
	}

	if (parsed_options->width)
		width = parsed_options->width;

	if (parsed_options->height)
		height = parsed_options->height;

	weston_config_section_get_string(section, "gbm-format", &gbm_format, NULL);

	weston_output_set_scale(output, scale);
	weston_output_set_transform(output, transform);

	api->set_gbm_format(output, gbm_format);
	free(gbm_format);

	if (api->output_set_size(output, width, height) < 0) {
		weston_log("Cannot configure output \"%s\" using weston_pipewire_output_api.\n",
				output->name);
		return -1;
	}
	weston_log("pipewire_backend_output_configure.. Done\n");

	return 0;
}


static int
load_pipewire_backend(struct ivi_compositor *ivi, int *argc, char **argv,
		     enum weston_renderer_type renderer)
{
	struct weston_pipewire_backend_config config = {};
	struct weston_config_section *section;
	struct ivi_output_config *parsed_options =
		ivi_init_parsed_options(ivi->compositor);
	struct ivi_backend *ivi_backend = NULL;

	if (!parsed_options)
		return -1;

	pipewire_backend_config_init(&config);

	const struct weston_option pipewire_options[] = {
		{ WESTON_OPTION_INTEGER, "width", 0, &parsed_options->width },
		{ WESTON_OPTION_INTEGER, "height", 0, &parsed_options->height },
	};

	parse_options(pipewire_options, ARRAY_LENGTH(pipewire_options), argc, argv);

	config.renderer = renderer;

	section = weston_config_get_section(ivi->config, "core", NULL, NULL);
	weston_config_section_get_string(section, "gbm-format",
					 &config.gbm_format, NULL);

	section = weston_config_get_section(ivi->config, "pipewire", NULL, NULL);
	weston_config_section_get_int(section, "num-outputs",
				      &config.num_outputs, 1);


	ivi_backend = zalloc(sizeof(struct ivi_backend));
	ivi_backend->backend = weston_compositor_load_backend(ivi->compositor,
					    WESTON_BACKEND_PIPEWIRE, &config.base);
	if (!ivi_backend->backend) {
		weston_log("Failed to create PipeWire backend\n");
		free(ivi_backend);
		return -1;
	}

	ivi_backend->simple_output_configure = pipewire_backend_output_configure;
	ivi_backend->heads_changed.notify = simple_heads_changed;
	weston_compositor_add_heads_changed_listener(ivi->compositor,
						     &ivi_backend->heads_changed);

	wl_list_insert(&ivi->backends, &ivi_backend->link);

	return 0;
}
#else
static int
load_pipewire_backend(struct ivi_compositor *ivi, int *argc, char **argv,
		      enum weston_renderer_type renderer)
{
	return -1;
}
#endif

static int
load_backend(struct ivi_compositor *ivi, int *argc, char **argv,
	     const char *backend_name, const char *renderer_name)
{
	enum weston_compositor_backend backend;
	enum weston_renderer_type renderer;

	if (!get_backend_from_string(backend_name, &backend)) {
		weston_log("Error: unknown backend \"%s\"\n", backend_name);
		return -1;
	}

	if (!get_renderer_from_string(renderer_name, &renderer)) {
		weston_log("Error: unknown renderer \"%s\"\n", renderer_name);
		return -1;
	}

	switch (backend) {
	case WESTON_BACKEND_DRM:
		return load_drm_backend(ivi, argc, argv, renderer);
	case WESTON_BACKEND_RDP:
		return load_rdp_backend(ivi, argc, argv, renderer);
	case WESTON_BACKEND_PIPEWIRE:
		return load_pipewire_backend(ivi, argc, argv, renderer);
	case WESTON_BACKEND_WAYLAND:
		return load_wayland_backend(ivi, argc, argv, renderer);
	case WESTON_BACKEND_X11:
		return load_x11_backend(ivi, argc, argv, renderer);
	default:
		assert(!"unknown backend type in load_backend()");
	}

	return 0;
}

static int
load_backends(struct ivi_compositor *ivi, const char *backends,
	      int *argc, char **argv, const char *renderer)
{
	const char *p, *end;
	char buffer[256];

	if (backends == NULL)
		return 0;

	p = backends;
	while (*p) {
		end = strchrnul(p, ',');
		snprintf(buffer, sizeof buffer, "%.*s", (int) (end - p), p);

		if (load_backend(ivi, argc, argv, buffer, renderer) < 0)
			return -1;

		p = end;
		while (*p == ',')
			p++;
	}

	return 0;
}


static int
load_modules(struct ivi_compositor *ivi, const char *modules,
	     int *argc, char *argv[], bool *xwayland)
{
	const char *p, *end;
	char buffer[256];
	int (*module_init)(struct weston_compositor *wc, int argc, char *argv[]);

	if (modules == NULL)
		return 0;

	p = modules;
	while (*p) {
		end = strchrnul(p, ',');
		snprintf(buffer, sizeof buffer, "%.*s", (int) (end - p), p);

		if (strstr(buffer, "xwayland.so")) {
			*xwayland = true;
		} else if (strstr(buffer, "systemd-notify.so")) {
			weston_log("systemd-notify plug-in already loaded!\n");
		} else {
			module_init = weston_load_module(buffer, "wet_module_init", WESTON_MODULEDIR);
			if (!module_init)
				return -1;

			if (module_init(ivi->compositor, *argc, argv) < 0)
				return -1;

		}

		p = end;
		while (*p == ',')
			p++;
	}

	return 0;
}


static char *
choose_default_backend(void)
{
	char *backend = NULL;

	if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET"))
		backend = strdup("wayland-backend.so");
	else if (getenv("DISPLAY"))
		backend = strdup("x11-backend.so");
	else
		backend = strdup("drm-backend.so");

	return backend;
}

static int
compositor_init_config(struct ivi_compositor *ivi)
{
	struct xkb_rule_names xkb_names;
	struct weston_config_section *section;
	struct weston_compositor *compositor = ivi->compositor;
	struct weston_config *config = ivi->config;
	int repaint_msec;
	bool vt_switching;
	bool require_input;

	/* agl-compositor.ini [keyboard] */
	section = weston_config_get_section(config, "keyboard", NULL, NULL);
	weston_config_section_get_string(section, "keymap_rules",
					 (char **) &xkb_names.rules, NULL);
	weston_config_section_get_string(section, "keymap_model",
					 (char **) &xkb_names.model, NULL);
	weston_config_section_get_string(section, "keymap_layout",
					 (char **) &xkb_names.layout, NULL);
	weston_config_section_get_string(section, "keymap_variant",
					 (char **) &xkb_names.variant, NULL);
	weston_config_section_get_string(section, "keymap_options",
					 (char **) &xkb_names.options, NULL);

	if (weston_compositor_set_xkb_rule_names(compositor, &xkb_names) < 0)
		return -1;

	weston_config_section_get_int(section, "repeat-rate",
				      &compositor->kb_repeat_rate, 40);
	weston_config_section_get_int(section, "repeat-delay",
				      &compositor->kb_repeat_delay, 400);

	weston_config_section_get_bool(section, "vt-switching",
				       &vt_switching, false);
	compositor->vt_switching = vt_switching;

	/* agl-compositor.ini [core] */
	section = weston_config_get_section(config, "core", NULL, NULL);

	weston_config_section_get_bool(section, "disable-cursor",
				       &ivi->disable_cursor, false);
	weston_config_section_get_bool(section, "activate-by-default",
				       &ivi->activate_by_default, true);

	weston_config_section_get_bool(section, "require-input", &require_input, true);
	compositor->require_input = require_input;

	weston_config_section_get_int(section, "repaint-window", &repaint_msec,
				      compositor->repaint_msec);
	if (repaint_msec < -10 || repaint_msec > 1000) {
		weston_log("Invalid repaint_window value in config: %d\n",
			   repaint_msec);
	} else {
		compositor->repaint_msec = repaint_msec;
	}
	weston_log("Output repaint window is %d ms maximum.\n",
		   compositor->repaint_msec);

	return 0;
}

struct ivi_surface *
to_ivi_surface(struct weston_surface *surface)
{
	struct weston_desktop_surface *dsurface;

	dsurface = weston_surface_get_desktop_surface(surface);
	if (!dsurface)
		return NULL;

	return weston_desktop_surface_get_user_data(dsurface);
}

static void
activate_binding(struct weston_seat *seat,
		 struct weston_view *focus_view, uint32_t flags)
{
	struct weston_surface *focus_surface;
	struct weston_surface *main_surface;
	struct ivi_surface *ivi_surface;
	struct ivi_shell_seat *ivi_seat = get_ivi_shell_seat(seat);

	if (!focus_view)
		return;

	focus_surface = focus_view->surface;
	main_surface = weston_surface_get_main_surface(focus_surface);

	ivi_surface = to_ivi_surface(main_surface);
	if (!ivi_surface)
		return;

	if (ivi_seat)
		ivi_shell_activate_surface(ivi_surface, ivi_seat, flags);
}

static void
click_to_activate_binding(struct weston_pointer *pointer,
			  const struct timespec *time,
			  uint32_t button, void *data)
{
	if (pointer->grab != &pointer->default_grab)
		return;
	if (pointer->focus == NULL)
		return;

	activate_binding(pointer->seat, pointer->focus,
			 WESTON_ACTIVATE_FLAG_CLICKED);
}

static void
touch_to_activate_binding(struct weston_touch *touch,
			  const struct timespec *time,
			  void *data)
{
	if (touch->grab != &touch->default_grab)
		return;
	if (touch->focus == NULL)
		return;

	activate_binding(touch->seat, touch->focus,
			 WESTON_ACTIVATE_FLAG_NONE);
}

static void
add_bindings(struct weston_compositor *compositor)
{
	weston_compositor_add_button_binding(compositor, BTN_LEFT, 0,
					     click_to_activate_binding,
					     NULL);
	weston_compositor_add_button_binding(compositor, BTN_RIGHT, 0,
					     click_to_activate_binding,
					     NULL);
	weston_compositor_add_touch_binding(compositor, 0,
					    touch_to_activate_binding,
					    NULL);
}

static int
create_listening_socket(struct wl_display *display, const char *socket_name)
{
	if (socket_name) {
		if (wl_display_add_socket(display, socket_name)) {
			weston_log("fatal: failed to add socket: %s\n",
				   strerror(errno));
			return -1;
		}
	} else {
		socket_name = wl_display_add_socket_auto(display);
		if (!socket_name) {
			weston_log("fatal: failed to add socket: %s\n",
				   strerror(errno));
			return -1;
		}
	}

	setenv("WAYLAND_DISPLAY", socket_name, 1);

	return 0;
}

static bool
global_filter(const struct wl_client *client, const struct wl_global *global,
	      void *data)
{
	return true;
}

static int
load_config(struct weston_config **config, bool no_config,
	    const char *config_file)
{
	const char *file = "agl-compositor.ini";
	const char *full_path;

	if (config_file)
		file = config_file;

	if (!no_config)
		*config = weston_config_parse(file);

	if (*config) {
		full_path = weston_config_get_full_path(*config);

		weston_log("Using config file '%s'.\n", full_path);
		setenv(WESTON_CONFIG_FILE_ENV_VAR, full_path, 1);

		return 0;
	}

	if (config_file && !no_config) {
		weston_log("fatal: error opening or reading config file '%s'.\n",
			config_file);

		return -1;
	}

	weston_log("Starting with no config file.\n");
	setenv(WESTON_CONFIG_FILE_ENV_VAR, "", 1);

	return 0;
}

static FILE *logfile;

static char *
log_timestamp(char *buf, size_t len)
{
	struct timeval tv;
	struct tm *brokendown_time;
	char datestr[128];
	char timestr[128];

	gettimeofday(&tv, NULL);

	brokendown_time = localtime(&tv.tv_sec);
	if (brokendown_time == NULL) {
		snprintf(buf, len, "%s", "[(NULL)localtime] ");
		return buf;
	}

	memset(datestr, 0, sizeof(datestr));
	if (brokendown_time->tm_mday != cached_tm_mday) {
		strftime(datestr, sizeof(datestr), "Date: %Y-%m-%d %Z\n",
				brokendown_time);
		cached_tm_mday = brokendown_time->tm_mday;
	}

	strftime(timestr, sizeof(timestr), "%H:%M:%S", brokendown_time);
	/* if datestr is empty it prints only timestr*/
	snprintf(buf, len, "%s[%s.%03"PRIi64"]", datestr,
			timestr, (tv.tv_usec / 1000));

	return buf;
}

static void
custom_handler(const char *fmt, va_list arg)
{
	char timestr[512];

	weston_log_scope_printf(log_scope, "%s libwayland: ",
			log_timestamp(timestr, sizeof(timestr)));
	weston_log_scope_vprintf(log_scope, fmt, arg);
}

static void
log_file_open(const char *filename)
{
	wl_log_set_handler_server(custom_handler);

	if (filename)
		logfile = fopen(filename, "a");

	if (!logfile) {
		logfile = stderr;
	} else {
		os_fd_set_cloexec(fileno(logfile));
		setvbuf(logfile, NULL, _IOLBF, 256);
	}
}

static void
log_file_close(void)
{
	if (logfile && logfile != stderr)
		fclose(logfile);
	logfile = stderr;
}

static int
vlog(const char *fmt, va_list ap)
{
	const char *oom = "Out of memory";
	char timestr[128];
	int len = 0;
	char *str;

	if (weston_log_scope_is_enabled(log_scope)) {
		int len_va;
		char *xlog_timestamp = log_timestamp(timestr, sizeof(timestr));
		len_va = vasprintf(&str, fmt, ap);
		if (len_va >= 0) {
			len = weston_log_scope_printf(log_scope, "%s %s",
					xlog_timestamp, str);
			free(str);
		} else {
			len = weston_log_scope_printf(log_scope, "%s %s",
					xlog_timestamp, oom);
		}
	}

	return len;
}


static int
vlog_continue(const char *fmt, va_list ap)
{
	return weston_log_scope_vprintf(log_scope, fmt, ap);
}

static int
on_term_signal(int signo, void *data)
{
	struct wl_display *display = data;

	weston_log("caught signal %d\n", signo);
	wl_display_terminate(display);

	return 1;
}

static void
handle_exit(struct weston_compositor *compositor)
{
	wl_display_terminate(compositor->wl_display);
}

static void
usage(int error_code)
{
	FILE *out = error_code == EXIT_SUCCESS ? stdout : stderr;
	fprintf(out,
		"Usage: agl-compositor [OPTIONS]\n"
		"\n"
		"This is " PACKAGE_STRING ", the reference compositor for\n"
		"Automotive Grade Linux. " PACKAGE_STRING " supports multiple "
		"backends,\nand depending on which backend is in use different "
		"options will be accepted.\n"
		"\n"
		"Core options:\n"
		"\n"
		"  --version\t\tPrint agl-compositor version\n"
		"  -B, --backend=MODULE\tBackend module, one of\n"
			"\t\t\t\tdrm-backend.so\n"
			"\t\t\t\twayland-backend.so\n"
			"\t\t\t\tx11-backend.so\n"
			"\t\t\t\theadless-backend.so\n"
		"  -r, --renderer=NAME\tName of renderer to use: auto, gl, noop, pixman\n"
		"  -S, --socket=NAME\tName of socket to listen on\n"
		"  --log=FILE\t\tLog to the given file\n"
		"  -c, --config=FILE\tConfig file to load, defaults to agl-compositor.ini\n"
		"  --no-config\t\tDo not read agl-compositor.ini\n"
		"  --debug\t\tEnable debug extension(s)\n"
		"  -h, --help\t\tThis help message\n"
		"\n");
	exit(error_code);
}

static char *
copy_command_line(int argc, char * const argv[])
{
	FILE *fp;
	char *str = NULL;
	size_t size = 0;
	int i;

	fp = open_memstream(&str, &size);
	if (!fp)
		return NULL;

	fprintf(fp, "%s", argv[0]);
	for (i = 1; i < argc; i++)
		fprintf(fp, " %s", argv[i]);
	fclose(fp);

	return str;
}

#if !defined(BUILD_XWAYLAND)
void *
wet_load_xwayland(struct weston_compositor *comp)
{
	weston_log("Attempted to load xwayland library but compositor "
		   "was *not* built with xwayland support!\n");
	return NULL;
}
#endif

static void
weston_log_setup_scopes(struct weston_log_context *log_ctx,
			struct weston_log_subscriber *subscriber,
			const char *names)
{
	assert(log_ctx);
	assert(subscriber);

	char *tokenize = strdup(names);
	char *token = strtok(tokenize, ",");
	while (token) {
		weston_log_subscribe(log_ctx, subscriber, token);
		token = strtok(NULL, ",");
	}
	free(tokenize);
}

static void
weston_log_subscribe_to_scopes(struct weston_log_context *log_ctx,
                               struct weston_log_subscriber *logger,
                               const char *debug_scopes)
{
        if (logger && debug_scopes)
                weston_log_setup_scopes(log_ctx, logger, debug_scopes);
        else
                weston_log_subscribe(log_ctx, logger, "log");
}

WL_EXPORT
int wet_main(int argc, char *argv[], const struct weston_testsuite_data *test_data)
{
	struct ivi_compositor ivi = { 0 };
	char *cmdline;
	struct wl_display *display = NULL;
	struct wl_event_loop *loop;
	struct wl_event_source *signals[3] = { 0 };
	struct weston_config_section *section;
	/* Command line options */
	char *backends = NULL;
	char *socket_name = NULL;
	char *log = NULL;
	char *modules = NULL;
	char *option_modules = NULL;
	char *debug_scopes = NULL;
	int help = 0;
	int version = 0;
	int no_config = 0;
	int debug = 0;
	bool list_debug_scopes = false;
	char *config_file = NULL;
	struct weston_log_context *log_ctx = NULL;
	struct weston_log_subscriber *logger;
	int ret = EXIT_FAILURE;
	bool xwayland = false;
	bool no_black_curtain = false;
	struct sigaction action;
	char *renderer = NULL;
	struct wet_process *process, *process_tmp;

	const struct weston_option core_options[] = {
		{ WESTON_OPTION_STRING, "renderer", 'r', &renderer },
		{ WESTON_OPTION_STRING, "backend", 'B', &backends },
		{ WESTON_OPTION_STRING, "backends", 0, &backends },
		{ WESTON_OPTION_STRING, "socket", 'S', &socket_name },
		{ WESTON_OPTION_STRING, "log", 0, &log },
		{ WESTON_OPTION_BOOLEAN, "help", 'h', &help },
		{ WESTON_OPTION_BOOLEAN, "version", 0, &version },
		{ WESTON_OPTION_BOOLEAN, "no-config", 0, &no_config },
		{ WESTON_OPTION_BOOLEAN, "debug", 0, &debug },
		{ WESTON_OPTION_BOOLEAN, "no-black-curtain", false, &no_black_curtain },
		{ WESTON_OPTION_STRING, "config", 'c', &config_file },
		{ WESTON_OPTION_STRING, "modules", 0, &option_modules },
		{ WESTON_OPTION_STRING, "debug-scopes", 'l', &debug_scopes },
		{ WESTON_OPTION_STRING, "list-debug-scopes", 'L', &list_debug_scopes },
	};

	wl_list_init(&ivi.outputs);
	wl_list_init(&ivi.saved_outputs);
	wl_list_init(&ivi.surfaces);
	wl_list_init(&ivi.pending_surfaces);
	wl_list_init(&ivi.popup_pending_apps);
	wl_list_init(&ivi.fullscreen_pending_apps);
	wl_list_init(&ivi.split_pending_apps);
	wl_list_init(&ivi.remote_pending_apps);
	wl_list_init(&ivi.desktop_clients);
	wl_list_init(&ivi.backends);
	wl_list_init(&ivi.child_process_list);
	wl_list_init(&ivi.pending_apps);

	/* Prevent any clients we spawn getting our stdin */
	os_fd_set_cloexec(STDIN_FILENO);

	cmdline = copy_command_line(argc, argv);
	parse_options(core_options, ARRAY_LENGTH(core_options), &argc, argv);

	if (help)
		usage(EXIT_SUCCESS);

	if (version) {
		printf(PACKAGE_STRING "\n");
		ret = EXIT_SUCCESS;
		goto exit_signals;
	}

	log_ctx = weston_log_ctx_create();
	if (!log_ctx) {
		fprintf(stderr, "Failed to initialize weston debug framework.\n");
		goto exit_signals;
	}

        log_scope = weston_log_ctx_add_log_scope(log_ctx, "log",
						 "agl-compositor log\n",
						 NULL, NULL, NULL);

	log_file_open(log);
	weston_log_set_handler(vlog, vlog_continue);

	logger = weston_log_subscriber_create_log(logfile);
	weston_log_subscribe_to_scopes(log_ctx, logger, debug_scopes);

	weston_log("Command line: %s\n", cmdline);
	free(cmdline);

	if (load_config(&ivi.config, no_config, config_file) < 0)
		goto error_signals;
	section = weston_config_get_section(ivi.config, "core", NULL, NULL);
	if (!backends) {
		weston_config_section_get_string(section, "backend", &backends,
						 NULL);
		if (!backends)
			backends = choose_default_backend();
	}

	display = wl_display_create();
	loop = wl_display_get_event_loop(display);

	wl_display_set_global_filter(display,
				     global_filter, &ivi);

	/* Register signal handlers so we shut down cleanly */

	signals[0] = wl_event_loop_add_signal(loop, SIGTERM, on_term_signal,
					      display);
	signals[1] = wl_event_loop_add_signal(loop, SIGUSR2, on_term_signal,
					      display);
	signals[2] = wl_event_loop_add_signal(loop, SIGCHLD, sigchld_handler,
					      display);

	/* When debugging the compositor, if use wl_event_loop_add_signal() to
	 * catch SIGINT, the debugger can't catch it, and attempting to stop
	 * the compositor from within the debugger results in weston exiting
	 * cleanly.
	 *
	 * Instead, use the sigaction() function, which sets up the signal in a
	 * way that gdb can successfully catch, but have the handler for SIGINT
	 * send SIGUSR2 (xwayland uses SIGUSR1), which we catch via
	 * wl_event_loop_add_signal().
	 */
	action.sa_handler = sigint_helper;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);

	for (size_t i = 0; i < ARRAY_LENGTH(signals); ++i)
		if (!signals[i])
			goto error_signals;

	ivi.compositor = weston_compositor_create(display, log_ctx, &ivi, test_data);
	if (!ivi.compositor) {
		weston_log("fatal: failed to create compositor.\n");
		goto error_signals;
	}

	if (compositor_init_config(&ivi) < 0)
		goto error_compositor;

	ivi.compositor->multi_backend = backends && strchr(backends, ',');
	if (load_backends(&ivi, backends, &argc, argv, renderer) < 0) {
		weston_log("fatal: failed to create compositor backend\n");
		goto error_compositor;
	}

	if (weston_compositor_backends_loaded(ivi.compositor) < 0)
                goto error_compositor;

	weston_compositor_flush_heads_changed(ivi.compositor);

	if (ivi_desktop_init(&ivi) < 0)
		goto error_compositor;

	ivi_seat_init(&ivi);

	/* load additional modules */
	weston_config_section_get_string(section, "modules", &modules, "");
	if (load_modules(&ivi, modules, &argc, argv, &xwayland) < 0)
		goto error_compositor;

	if (load_modules(&ivi, option_modules, &argc, argv, &xwayland) < 0)
		goto error_compositor;

	if (!xwayland) {
		weston_config_section_get_bool(section, "xwayland", &xwayland,
					       false);
	}

	if (ivi_shell_init(&ivi) < 0)
		goto error_compositor;

	if (xwayland) {
		if (!wet_load_xwayland(ivi.compositor))
			goto error_compositor;
	}

	if (ivi_policy_init(&ivi) < 0)
		goto error_compositor;


	if (list_debug_scopes) {
		struct weston_log_scope *nscope = NULL;

		weston_log("Printing available debug scopes:\n");

		while ((nscope = weston_log_scopes_iterate(log_ctx, nscope))) {
			weston_log("\tscope name: %s, desc: %s",
					weston_log_scope_get_name(nscope),
					weston_log_scope_get_description(nscope));
		}

		weston_log("\n");

		goto error_compositor;
	}

	add_bindings(ivi.compositor);

	if (create_listening_socket(display, socket_name) < 0)
		goto error_compositor;

	if (!no_black_curtain) {
		weston_log("Installing black curtains\n");
		ivi_shell_init_black_fs(&ivi);
	}

	ivi.compositor->exit = handle_exit;

	weston_compositor_wake(ivi.compositor);

	ivi_shell_create_global(&ivi);

	ivi_launch_shell_client(&ivi, "shell-client",
				&ivi.shell_client.client);
	ivi_launch_shell_client(&ivi, "shell-client-ext",
				&ivi.shell_client_ext.client);

	if (debug) {
		weston_compositor_add_screenshot_authority(ivi.compositor,
					   &ivi.screenshot_auth,
					   screenshot_allow_all);
	}
	ivi_agl_systemd_notify(&ivi);

	wl_display_run(display);

	ret = ivi.compositor->exit_code;

	ivi_compositor_destroy_backends(&ivi);

	wl_display_destroy_clients(display);

error_compositor:
	free(backends);
	backends = NULL;
	free(modules);
	modules = NULL;

	cleanup_drm_lease(ivi.drm_lease);
	weston_compositor_destroy(ivi.compositor);

	weston_log_scope_destroy(log_scope);
	log_scope = NULL;

	weston_log_subscriber_destroy(logger);
	weston_log_ctx_destroy(log_ctx);

	ivi_policy_destroy(ivi.policy);

        wl_list_for_each_safe(process, process_tmp, &ivi.child_process_list, link)
                ivi_process_destroy(process, 0, false);

error_signals:
	for (size_t i = 0; i < ARRAY_LENGTH(signals); ++i)
		if (signals[i])
			wl_event_source_remove(signals[i]);

	wl_display_destroy(display);

	log_file_close();
	if (ivi.config)
		weston_config_destroy(ivi.config);

exit_signals:
	free(log);
	free(config_file);
	free(socket_name);
	free(option_modules);
	return ret;
}
