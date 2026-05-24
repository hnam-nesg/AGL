/*
 * Copyright © 2019 Collabora, Ltd.
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

#ifndef IVI_COMPOSITOR_H
#define IVI_COMPOSITOR_H

#include <stdbool.h>
#include "config.h"

#include <weston.h>
#include <libweston/backend-drm.h>
#include <libweston/libweston.h>
#include <libweston/windowed-output-api.h>
#include <libweston/desktop.h>

#include "remote.h"
#include "drm-lease.h"

#include "agl-shell-server-protocol.h"

struct ivi_compositor;

struct desktop_client {
	struct wl_resource *resource;
	struct ivi_compositor *ivi;
	struct wl_list link;	/* ivi_compositor::desktop_clients */
};

enum agl_shell_bound_status {
	BOUND_OK,
	BOUND_FAILED,
};

struct ivi_output_config {
	int width;
	int height;
	int32_t scale;
	uint32_t transform;
};

struct ivi_backend {
	struct weston_backend *backend;
	struct wl_listener heads_changed;
	int (*simple_output_configure)(struct weston_output *output);
	struct wl_list link;
};

struct ivi_compositor {
	struct weston_compositor *compositor;
	struct weston_config *config;
	struct ivi_output_config *parsed_options;


	bool init_failed;
	bool disable_cursor;
	bool activate_by_default;
	bool keep_pending_surfaces;

	struct wl_listener screenshot_auth;

	/*
	 * Options parsed from command line arugments.
	 * Overrides what is found in the config file.
	 */
	struct {
		/* drm */
		bool use_current_mode;
		/* wayland/x11 */
		int width;
		int height;
		int scale;
	} cmdline;
	const struct weston_windowed_output_api *window_api;
	const struct weston_drm_output_api *drm_api;

	struct wl_global *agl_shell;
	struct wl_global *agl_shell_ext;

	struct {
		struct wl_client *client;
		struct wl_resource *resource;

		/* this is for another agl-shell client, potentially used by
		 * the grpc-proxy */
		struct wl_resource *resource_ext;
		bool ready;
		enum agl_shell_bound_status status;
	} shell_client;

	struct {
		struct wl_client *client;
		struct wl_resource *resource;
		bool doas_requested;
		bool doas_requested_pending_bind;
		enum agl_shell_bound_status status;
	} shell_client_ext;

	struct wl_list desktop_clients;	/* desktop_client::link */

	struct wl_list outputs; /* ivi_output.link */
	struct wl_list saved_outputs; /* ivi_output.link */
	struct wl_list surfaces; /* ivi_surface.link */
	struct wl_list backends;

	struct weston_desktop *desktop;
	struct wl_listener seat_created_listener;
	struct ivi_policy *policy;

	struct wl_list pending_surfaces;
	struct wl_list popup_pending_apps;
	struct wl_list fullscreen_pending_apps;
	struct wl_list split_pending_apps;
	struct wl_list remote_pending_apps;

	struct wl_list pending_apps;	/** pending_app::link */

	struct wl_listener destroy_listener;
	struct wl_listener transform_listener;
	const struct weston_xwayland_surface_api *xwayland_surface_api;

	struct weston_layer hidden;
	struct weston_layer background;
	struct weston_layer normal;
	struct weston_layer panel;
	struct weston_layer popup;
	struct weston_layer fullscreen;

	bool need_ivi_output_relayout;
	struct wl_list child_process_list;
	struct dlm_lease *drm_lease;
};

struct ivi_surface;

enum ivi_output_type {
	OUTPUT_LOCAL,
	OUTPUT_REMOTE,
};

struct ivi_output {
	struct wl_list link; /* ivi_compositor.outputs */
	struct ivi_compositor *ivi;

	char *name;
	struct weston_config_section *config;
	struct weston_output *output;

	struct ivi_surface *background;
	/* Panels */
	struct ivi_surface *top;
	struct ivi_surface *bottom;
	struct ivi_surface *left;
	struct ivi_surface *right;

	/* for the black surface */
	struct fullscreen_view {
		struct ivi_surface *fs;
		struct wl_listener fs_destroy;
		struct weston_buffer_reference *buffer_ref;
	} fullscreen_view;

	struct wl_listener output_destroy;

	/*
	 * Usable area for normal clients, i.e. with panels removed.
	 * In output-coorrdinate space.
	 */
	struct weston_geometry area;
	struct weston_geometry area_saved;
	/*
	 * Potential user-specified non-default activation area
	 */
	struct weston_geometry area_activation;

	struct ivi_surface *active;
	struct ivi_surface *previous_active;

	/* Temporary: only used during configuration */
	size_t add_len;
	struct weston_head *add[8];

	char *app_ids;
	enum ivi_output_type type;
};

enum ivi_surface_role {
	IVI_SURFACE_ROLE_NONE,
	IVI_SURFACE_ROLE_DESKTOP,
	IVI_SURFACE_ROLE_BACKGROUND,
	IVI_SURFACE_ROLE_PANEL,
	IVI_SURFACE_ROLE_POPUP,
	IVI_SURFACE_ROLE_FULLSCREEN,
	IVI_SURFACE_ROLE_SPLIT_V,
	IVI_SURFACE_ROLE_SPLIT_H,
	IVI_SURFACE_ROLE_REMOTE,
	IVI_SURFACE_ROLE_TILE,
};

struct ivi_bounding_box {
	int x; int y;
	int width; int height;
};

struct pending_popup {
	struct ivi_output *ioutput;
	char *app_id;
	int x; int y;
	struct ivi_bounding_box bb;

	struct wl_list link;	/** ivi_compositor::popup_pending_surfaces */
};

struct pending_fullscreen {
	struct ivi_output *ioutput;
	char *app_id;
	struct wl_list link;	/** ivi_compositor::fullscreen_pending_apps */
};

struct pending_split {
	struct ivi_output *ioutput;
	char *app_id;
	uint32_t orientation;
	struct wl_list link;	/** ivi_compositor::split_pending_apps */
};

struct pending_remote {
	struct ivi_output *ioutput;
	char *app_id;
	struct wl_list link;    /** ivi_compositor::remote_pending_apps */
};

struct pending_app {
	struct ivi_output *ioutput;
	enum ivi_surface_role role;
	char *app_id;
	struct wl_list link;	/** ivi_compositor::pending_apps */
};

struct pending_app_tile {
	struct pending_app base;
	uint32_t orientation;
	uint32_t width;
	int32_t sticky;
};

struct ivi_desktop_surface {
	struct ivi_output *pending_output;
	struct ivi_output *last_output;
};

struct ivi_background_surface {
	struct ivi_output *output;
};

struct ivi_popup_surface {
	struct ivi_output *output;
	int x; int y; /* initial position */
	struct ivi_bounding_box bb;	/* bounding box */
};

struct ivi_fullscreen_surface {
	struct ivi_output *output;
};

struct ivi_split_surface {
	struct ivi_output *output;
	uint32_t orientation;
};

struct ivi_remote_surface {
	struct ivi_output *output;
};

struct ivi_panel_surface {
	struct ivi_output *output;
	enum agl_shell_edge edge;
};

enum ivi_surface_flags {
	IVI_SURFACE_PROP_MAP = (1 << 0),
	/* x, y, width, height */
	IVI_SURFACE_PROP_POSITION = (1 << 1),
};


struct ivi_surface {
	struct ivi_compositor *ivi;
	struct weston_desktop_surface *dsurface;
	struct weston_view *view;
	struct ivi_output *hidden_layer_output;
	struct ivi_output *current_completed_output;

	struct wl_list link;
	int focus_count;
	uint32_t orientation;
	int32_t sticky;

	struct {
		enum ivi_surface_flags flags;
		int32_t x, y;
		int32_t width, height;
	} pending;

	bool checked_pending;
	enum {
		NORMAL,
		RESIZING,
		FULLSCREEN,
		HIDDEN,
	} state;

	enum ivi_surface_role role;
	enum ivi_surface_role prev_role;
	union {
		struct ivi_desktop_surface desktop;
		struct ivi_background_surface bg;
		struct ivi_panel_surface panel;
		struct ivi_popup_surface popup;
		struct ivi_fullscreen_surface fullscreen;
		struct ivi_split_surface split;
		struct ivi_remote_surface remote;
	};

	struct {
		bool is_set;
		int32_t x;
		int32_t y;
	} xwayland;
};

struct ivi_shell_seat {
	struct weston_seat *seat;
	struct weston_surface *focused_surface;

	bool disable_cursor;
	bool new_caps_sent;

	struct wl_listener seat_destroy_listener;
	struct wl_listener caps_changed_listener;
	struct wl_listener keyboard_focus_listener;
	struct wl_listener pointer_focus_listener;
};

struct ivi_shell_client {
	struct wl_list link;
	char *command;
	bool require_ready;

	pid_t pid;
	struct wl_client *client;

	struct wl_listener client_destroy;
};

struct ivi_compositor *
to_ivi_compositor(struct weston_compositor *ec);

#ifdef HAVE_SYSTEMD
int
ivi_agl_systemd_notify(struct ivi_compositor *ivi);
#else
static int
ivi_agl_systemd_notify(struct ivi_compositor *ivi)
{
}
#endif

int
ivi_shell_init(struct ivi_compositor *ivi);

void
ivi_shell_init_black_fs(struct ivi_compositor *ivi);

int
ivi_shell_create_global(struct ivi_compositor *ivi);

int
ivi_launch_shell_client(struct ivi_compositor *ivi, const char *cmd_section, struct wl_client **client);

int
ivi_desktop_init(struct ivi_compositor *ivi);

struct ivi_shell_client *
ivi_shell_client_from_wl(struct wl_client *client);

struct ivi_output *
to_ivi_output(struct weston_output *o);

void
ivi_set_desktop_surface(struct ivi_surface *surface);

/*
 * removes the pending popup one
 */
void
ivi_check_pending_desktop_surface(struct ivi_surface *surface);

void
ivi_reflow_outputs(struct ivi_compositor *ivi);

struct ivi_surface *
to_ivi_surface(struct weston_surface *surface);

void
ivi_layout_set_mapped(struct ivi_surface *surface);

void
ivi_layout_set_position(struct ivi_surface *surface,
			int32_t x, int32_t y,
			int32_t width, int32_t height);

struct ivi_surface *
ivi_find_app(struct ivi_compositor *ivi, const char *app_id);

void
ivi_layout_commit(struct ivi_compositor *ivi);

void
ivi_layout_init(struct ivi_compositor *ivi, struct ivi_output *output);

void
ivi_layout_activate(struct ivi_output *output, const char *app_id);

void
ivi_layout_activate_by_surf(struct ivi_output *output, struct ivi_surface *surf);

void
ivi_layout_desktop_committed(struct ivi_surface *surf);
void
ivi_layout_remote_committed(struct ivi_surface *surf);

void
ivi_layout_popup_committed(struct ivi_surface *surface);

void
ivi_layout_fullscreen_committed(struct ivi_surface *surface);

void
ivi_layout_split_committed(struct ivi_surface *surface);

void
ivi_layout_deactivate(struct ivi_compositor *ivi, const char *app_id);

void
ivi_layout_desktop_resize(struct ivi_surface *surface,
			  struct weston_geometry area);

struct ivi_output *
ivi_layout_get_output_from_surface(struct ivi_surface *surf);

void
insert_black_curtain(struct ivi_output *output);

void
remove_black_curtain(struct ivi_output *output);

bool
output_has_black_curtain(struct ivi_output *output);

const char *
ivi_layout_get_surface_role_name(struct ivi_surface *surf);

void
ivi_set_pending_desktop_surface_remote(struct ivi_output *ioutput,
		const char *app_id);

struct ivi_output *
ivi_layout_find_with_app_id(const char *app_id, struct ivi_compositor *ivi);

void
shell_advertise_app_state(struct ivi_compositor *ivi, const char *app_id,
			  const char *data, uint32_t app_state);
void
ivi_screenshooter_create(struct ivi_compositor *ivi);

void
ivi_seat_init(struct ivi_compositor *ivi);

void
ivi_seat_reset_caps_sent(struct ivi_compositor *ivi);

void
ivi_check_pending_surface_desktop(struct ivi_surface *surface,
				  enum ivi_surface_role *role);

struct ivi_output *
ivi_layout_find_bg_output(struct ivi_compositor *ivi);
void
ivi_compositor_destroy_pending_surfaces(struct ivi_compositor *ivi);

void
ivi_shell_finalize(struct ivi_compositor *ivi);

struct ivi_surface *
get_ivi_shell_surface(struct weston_surface *surface);

struct ivi_shell_seat *
get_ivi_shell_seat(struct weston_seat *seat);

struct weston_seat *
get_ivi_shell_weston_first_seat(struct ivi_compositor *ivi);

void
ivi_shell_activate_surface(struct ivi_surface *ivi_surf,
                          struct ivi_shell_seat *ivi_seat,
                          uint32_t flags);
int
sigchld_handler(int signal_number, void *data);

void
shell_send_app_state(struct ivi_compositor *ivi, const char *app_id,
		     enum agl_shell_app_state state);
void
ivi_layout_restore(struct ivi_compositor *ivi, struct ivi_output *n_output);

void
ivi_layout_save(struct ivi_compositor *ivi, struct ivi_output *output);

struct weston_output *
get_default_output(struct weston_compositor *compositor);

struct weston_output *
get_focused_output(struct weston_compositor *compositor);

void
shell_send_app_on_output(struct ivi_compositor *ivi, const char *app_id,
			 const char *output_name);
bool
ivi_surface_count_one(struct ivi_output *ivi_output,
		      enum ivi_surface_role role);

int
parse_activation_area(const char *geometry, struct ivi_output *output);

bool
is_shell_surface_xwayland(struct ivi_surface *surf);

void
ivi_layout_reset_split_surfaces(struct ivi_compositor *ivi);

void
_ivi_set_shell_surface_split(struct ivi_surface *surface, struct ivi_output *output,
			     uint32_t orientation, uint32_t width, int32_t sticky,
			     bool to_activate);
struct ivi_output_config *
ivi_init_parsed_options(struct weston_compositor *compositor);

void
ivi_process_destroy(struct wet_process *process, int status, bool call_cleanup);

void
create_black_curtain_view(struct ivi_output *output);

#endif
