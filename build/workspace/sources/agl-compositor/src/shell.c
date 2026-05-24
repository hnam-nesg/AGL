/*
 * Copyright © 2019, 2022, 2024 Collabora, Ltd.
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
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libweston/libweston.h>
#include <libweston/config-parser.h>

#include <weston.h>
#include "shared/os-compatibility.h"
#include "shared/helpers.h"
#include "shared/process-util.h"

#include "agl-shell-server-protocol.h"

static uint32_t
reverse_orientation(uint32_t orientation);

const char *
split_orientation_to_string(uint32_t orientation);

static bool
is_assistant_overlay_app_id(const char *app_id)
{
	return app_id && strcmp(app_id, "assistant") == 0;
}

void
ivi_set_desktop_surface(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	assert(surface->role == IVI_SURFACE_ROLE_NONE);

	surface->role = IVI_SURFACE_ROLE_DESKTOP;
	wl_list_insert(&ivi->surfaces, &surface->link);
}

void
ivi_set_desktop_surface_popup(struct ivi_surface *surface)
{
       struct ivi_compositor *ivi = surface->ivi;
       assert(surface->role == IVI_SURFACE_ROLE_NONE);

       surface->role = IVI_SURFACE_ROLE_POPUP;
       wl_list_insert(&ivi->surfaces, &surface->link);
}

void
ivi_set_desktop_surface_fullscreen(struct ivi_surface *surface)
{
       struct ivi_compositor *ivi = surface->ivi;
       assert(surface->role == IVI_SURFACE_ROLE_NONE);

       surface->role = IVI_SURFACE_ROLE_FULLSCREEN;
       wl_list_insert(&ivi->surfaces, &surface->link);
}

void
ivi_set_desktop_surface_remote(struct ivi_surface *surface)
{
       struct ivi_compositor *ivi = surface->ivi;
       struct weston_view *view;
       struct ivi_output *output = surface->remote.output;

       assert(surface->role == IVI_SURFACE_ROLE_NONE);

       /* remote type are the same as desktop just that client can tell
        * the compositor to start on another output */
       surface->role = IVI_SURFACE_ROLE_REMOTE;

       /* if thew black surface view is mapped on the mean we need
        * to remove it in order to start showing the 'remote' surface
        * just being added */
       if (output->fullscreen_view.fs) {
               view = output->fullscreen_view.fs->view;
               if (weston_view_is_mapped(view))
                       remove_black_curtain(output);
       }

       wl_list_insert(&ivi->surfaces, &surface->link);
}

void
ivi_set_background_surface(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	assert(surface->role == IVI_SURFACE_ROLE_BACKGROUND);

	wl_list_insert(&ivi->surfaces, &surface->link);
}

static struct pending_popup *
ivi_ensure_popup(struct ivi_output *ioutput, int x, int y, int bx, int by,
		 int width, int height, const char *app_id)
{
	struct pending_popup *p_popup = zalloc(sizeof(*p_popup));
	size_t len_app_id = strlen(app_id);

	if (!p_popup)
		return NULL;
	p_popup->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_popup->app_id) {
		free(p_popup);
		return NULL;
	}
	memcpy(p_popup->app_id, app_id, len_app_id);
	p_popup->ioutput = ioutput;
	p_popup->x = x;
	p_popup->y = y;

	p_popup->bb.x = bx;
	p_popup->bb.y = by;
	p_popup->bb.width = width;
	p_popup->bb.height = height;

	return p_popup;
}

static void
ivi_update_popup(struct ivi_output *ioutput, int x, int y, int bx, int by,
		 int width, int height, const char *app_id, struct pending_popup *p_popup)
{
	size_t len_app_id = strlen(app_id);

	wl_list_remove(&p_popup->link);
	wl_list_init(&p_popup->link);

	memset(p_popup->app_id, 0, strlen(app_id) + 1);
	free(p_popup->app_id);

	p_popup->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_popup->app_id)
		return;
	memcpy(p_popup->app_id, app_id, len_app_id);

	p_popup->ioutput = ioutput;
	p_popup->x = x;
	p_popup->y = y;

	p_popup->bb.x = bx;
	p_popup->bb.y = by;
	p_popup->bb.width = width;
	p_popup->bb.height = height;
}

static struct pending_fullscreen *
ivi_ensure_fullscreen(struct ivi_output *ioutput, const char *app_id)
{
	struct pending_fullscreen *p_fullscreen = zalloc(sizeof(*p_fullscreen));
	size_t len_app_id = strlen(app_id);

	if (!p_fullscreen)
		return NULL;
	p_fullscreen->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_fullscreen->app_id) {
		free(p_fullscreen);
		return NULL;
	}
	memcpy(p_fullscreen->app_id, app_id, len_app_id);

	p_fullscreen->ioutput = ioutput;
	return p_fullscreen;
}

static void
ivi_update_fullscreen(struct ivi_output *ioutput, const char *app_id,
		      struct pending_fullscreen *p_fullscreen)
{
	size_t len_app_id = strlen(app_id);

	wl_list_remove(&p_fullscreen->link);
	wl_list_init(&p_fullscreen->link);

	memset(p_fullscreen->app_id, 0, strlen(app_id) + 1);
	free(p_fullscreen->app_id);

	p_fullscreen->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_fullscreen->app_id)
		return;
	memcpy(p_fullscreen->app_id, app_id, len_app_id);

	p_fullscreen->ioutput = ioutput;
}

static struct pending_remote *
ivi_ensure_remote(struct ivi_output *ioutput, const char *app_id)
{
	struct pending_remote *p_remote = zalloc(sizeof(*p_remote));
	size_t len_app_id = strlen(app_id);

	if (!p_remote)
		return NULL;
	p_remote->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_remote->app_id) {
		free(p_remote);
		return NULL;
	}
	memcpy(p_remote->app_id, app_id, len_app_id);

	p_remote->ioutput = ioutput;
	return p_remote;
}

static void
ivi_update_remote(struct ivi_output *ioutput, const char *app_id,
		      struct pending_remote *p_remote)
{
	size_t len_app_id = strlen(app_id);

	wl_list_remove(&p_remote->link);
	wl_list_init(&p_remote->link);

	memset(p_remote->app_id, 0, strlen(app_id) + 1);
	free(p_remote->app_id);

	p_remote->app_id = zalloc(sizeof(char) * (len_app_id + 1));
	if (!p_remote->app_id)
		return;
	memcpy(p_remote->app_id, app_id, len_app_id);

	p_remote->ioutput = ioutput;
}

static void
ivi_set_pending_desktop_surface_popup(struct ivi_output *ioutput, int x, int y, int bx,
				      int by, int width, int height, const char *app_id)
{
	struct ivi_compositor *ivi = ioutput->ivi;
	struct pending_popup *p_popup = NULL;
	struct pending_popup *popup;

	wl_list_for_each(popup, &ivi->popup_pending_apps, link)
		if (!strcmp(app_id, popup->app_id))
			p_popup = popup;

	if (!p_popup)
		p_popup = ivi_ensure_popup(ioutput, x, y, bx, by, width, height, app_id);
	else
		ivi_update_popup(ioutput, x, y, bx, by, width, height, app_id, p_popup);
	if (!p_popup)
		return;

	wl_list_insert(&ivi->popup_pending_apps, &p_popup->link);
}

static void
ivi_set_pending_desktop_surface_fullscreen(struct ivi_output *ioutput,
					   const char *app_id)
{
	struct ivi_compositor *ivi = ioutput->ivi;
	struct pending_fullscreen *p_fullscreen = NULL;
	struct pending_fullscreen *fullscreen;

	wl_list_for_each(fullscreen, &ivi->fullscreen_pending_apps, link)
		if (!strcmp(app_id, fullscreen->app_id))
			p_fullscreen = fullscreen;

	if (!p_fullscreen)
		p_fullscreen = ivi_ensure_fullscreen(ioutput, app_id);
	else
		ivi_update_fullscreen(ioutput, app_id, p_fullscreen);

	if (!p_fullscreen)
		return;
	wl_list_insert(&ivi->fullscreen_pending_apps, &p_fullscreen->link);
}

void
ivi_set_pending_desktop_surface_remote(struct ivi_output *ioutput,
		const char *app_id)
{
	struct ivi_compositor *ivi = ioutput->ivi;
	struct pending_remote *remote;
	struct pending_remote *p_remote = NULL;

	wl_list_for_each(remote, &ivi->remote_pending_apps, link)
		if (!strcmp(app_id, remote->app_id))
			p_remote = remote;

	if (!p_remote)
		p_remote = ivi_ensure_remote(ioutput, app_id);
	else
		ivi_update_remote(ioutput, app_id, p_remote);
	if (!p_remote)
		return;

	wl_list_insert(&ivi->remote_pending_apps, &p_remote->link);
}


static void
ivi_remove_pending_desktop_surface_split(struct pending_split *split)
{
	free(split->app_id);
	wl_list_remove(&split->link);
	free(split);
}

static void
ivi_remove_pending_desktop_surface_fullscreen(struct pending_fullscreen *fs)
{
	free(fs->app_id);
	wl_list_remove(&fs->link);
	free(fs);
}

static void
ivi_remove_pending_desktop_surface_popup(struct pending_popup *p_popup)
{
	free(p_popup->app_id);
	wl_list_remove(&p_popup->link);
	free(p_popup);
}

static void
ivi_remove_pending_desktop_surface_remote(struct pending_remote *remote)
{
	free(remote->app_id);
	wl_list_remove(&remote->link);
	free(remote);
}

void
ivi_check_pending_surface_desktop(struct ivi_surface *surface,
				  enum ivi_surface_role *role)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct wl_list *role_pending_list;
	struct pending_popup *p_popup;
	struct pending_split *p_split;
	struct pending_fullscreen *p_fullscreen;
	struct pending_remote *p_remote;
	const char *app_id =
		weston_desktop_surface_get_app_id(surface->dsurface);

	if (is_assistant_overlay_app_id(app_id)) {
		*role = IVI_SURFACE_ROLE_POPUP;
		return;
	}

	role_pending_list = &ivi->popup_pending_apps;
	wl_list_for_each(p_popup, role_pending_list, link) {
		if (app_id && !strcmp(app_id, p_popup->app_id)) {
			*role = IVI_SURFACE_ROLE_POPUP;
			return;
		}
	}

	role_pending_list = &ivi->split_pending_apps;
	wl_list_for_each(p_split, role_pending_list, link) {
		if (app_id && !strcmp(app_id, p_split->app_id)) {
			*role = IVI_SURFACE_ROLE_SPLIT_V;
			return;
		}
	}

	role_pending_list = &ivi->fullscreen_pending_apps;
	wl_list_for_each(p_fullscreen, role_pending_list, link) {
		if (app_id && !strcmp(app_id, p_fullscreen->app_id)) {
			*role = IVI_SURFACE_ROLE_FULLSCREEN;
			return;
		}
	}

	role_pending_list = &ivi->remote_pending_apps;
	wl_list_for_each(p_remote, role_pending_list, link) {
		if (app_id && !strcmp(app_id, p_remote->app_id)) {
			*role = IVI_SURFACE_ROLE_REMOTE;
			return;
		}
	}

	/* else, we are a regular desktop surface */
	*role = IVI_SURFACE_ROLE_DESKTOP;
}

struct pending_app *
ivi_check_pending_app_type(struct ivi_surface *surface, enum ivi_surface_role role)
{
	struct pending_app *papp;
	const char *app_id = NULL;

	app_id = weston_desktop_surface_get_app_id(surface->dsurface);
	if (!app_id)
		return NULL;

	wl_list_for_each(papp, &surface->ivi->pending_apps, link) {
		if (strcmp(app_id, papp->app_id) == 0 && papp->role == role)
			return papp;
	}

	return NULL;
}

static bool
ivi_compositor_keep_pending_surfaces(struct ivi_surface *surface)
{
	return surface->ivi->keep_pending_surfaces;
}

static bool
ivi_check_pending_desktop_surface_popup(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct pending_popup *p_popup, *next_p_popup;
	const char *_app_id =
		weston_desktop_surface_get_app_id(surface->dsurface);

	if (is_assistant_overlay_app_id(_app_id)) {
		struct weston_output *woutput = get_focused_output(ivi->compositor);
		struct ivi_output *output;

		if (!woutput)
			woutput = get_default_output(ivi->compositor);
		if (!woutput)
			return false;

		output = to_ivi_output(woutput);
		if (!output)
			return false;

		surface->popup.output = output;
		surface->popup.x = 0;
		surface->popup.y = 0;
		surface->popup.bb.x = 0;
		surface->popup.bb.y = 0;
		surface->popup.bb.width = 0;
		surface->popup.bb.height = 0;
		return true;
	}

	if (wl_list_empty(&ivi->popup_pending_apps) || !_app_id)
		return false;

	wl_list_for_each_safe(p_popup, next_p_popup,
			&ivi->popup_pending_apps, link) {
		if (!strcmp(_app_id, p_popup->app_id)) {
			surface->popup.output = p_popup->ioutput;
			surface->popup.x = p_popup->x;
			surface->popup.y = p_popup->y;

			surface->popup.bb.x = p_popup->bb.x;
			surface->popup.bb.y = p_popup->bb.y;
			surface->popup.bb.width = p_popup->bb.width;
			surface->popup.bb.height = p_popup->bb.height;

			if (!ivi_compositor_keep_pending_surfaces(surface))
				ivi_remove_pending_desktop_surface_popup(p_popup);
			return true;
		}
	}

	return false;
}

static bool
ivi_check_pending_desktop_surface_fullscreen(struct ivi_surface *surface)
{
	struct pending_fullscreen *fs_surf, *next_fs_surf;
	struct ivi_compositor *ivi = surface->ivi;
	const char *_app_id =
		weston_desktop_surface_get_app_id(surface->dsurface);

	if (wl_list_empty(&ivi->fullscreen_pending_apps) || !_app_id)
		return false;

	wl_list_for_each_safe(fs_surf, next_fs_surf,
			&ivi->fullscreen_pending_apps, link) {
		if (!strcmp(_app_id, fs_surf->app_id)) {
			surface->fullscreen.output = fs_surf->ioutput;
			if (!ivi_compositor_keep_pending_surfaces(surface))
				ivi_remove_pending_desktop_surface_fullscreen(fs_surf);
			return true;
		}
	}

	return false;
}

static bool
ivi_check_pending_desktop_surface_remote(struct ivi_surface *surface)
{
	struct pending_remote *remote_surf, *next_remote_surf;
	struct ivi_compositor *ivi = surface->ivi;
	const char *_app_id =
		weston_desktop_surface_get_app_id(surface->dsurface);

	if (wl_list_empty(&ivi->remote_pending_apps) || !_app_id)
		return false;

	wl_list_for_each_safe(remote_surf, next_remote_surf,
			&ivi->remote_pending_apps, link) {
		if (!strcmp(_app_id, remote_surf->app_id)) {
			surface->remote.output = remote_surf->ioutput;
			if (!ivi_compositor_keep_pending_surfaces(surface))
				ivi_remove_pending_desktop_surface_remote(remote_surf);
			return true;
		}
	}

	return false;
}

void
ivi_check_pending_desktop_surface(struct ivi_surface *surface)
{
	bool ret = false;

	ret = ivi_check_pending_desktop_surface_popup(surface);
	if (ret) {
		ivi_set_desktop_surface_popup(surface);
		ivi_layout_popup_committed(surface);
		return;
	}

	ret = ivi_check_pending_desktop_surface_fullscreen(surface);
	if (ret) {
		ivi_set_desktop_surface_fullscreen(surface);
		ivi_layout_fullscreen_committed(surface);
		return;
	}

	ret = ivi_check_pending_desktop_surface_remote(surface);
	if (ret) {
		ivi_set_desktop_surface_remote(surface);
		ivi_layout_remote_committed(surface);
		return;
	}

	/* new way of doing it */
	struct pending_app *papp =
		ivi_check_pending_app_type(surface, IVI_SURFACE_ROLE_TILE);
	if (papp) {
		struct pending_app_tile *papp_tile =
			container_of(papp, struct pending_app_tile, base);

		// handle the currently active surface
		if (papp->ioutput->active) {
			int width_prev_app = 0;

			if (papp_tile->width > 0) {
				if (papp_tile->orientation == AGL_SHELL_TILE_ORIENTATION_TOP ||
				    papp_tile->orientation == AGL_SHELL_TILE_ORIENTATION_BOTTOM)
					width_prev_app = papp->ioutput->area.height - papp_tile->width;
				else
					width_prev_app = papp->ioutput->area.width - papp_tile->width;
			}
			_ivi_set_shell_surface_split(papp->ioutput->active, NULL,
					reverse_orientation(papp_tile->orientation),
					width_prev_app, false, false);
		}

		surface->role = IVI_SURFACE_ROLE_TILE;
		surface->current_completed_output = papp->ioutput;
		wl_list_insert(&surface->ivi->surfaces, &surface->link);

		_ivi_set_shell_surface_split(surface, papp->ioutput,
					     papp_tile->orientation, papp_tile->width,
					     papp_tile->sticky, true);

		/* remove it from pending */
		wl_list_remove(&papp->link);
		free(papp->app_id);
		free(papp);

		return;
	}

	/* if we end up here means we have a regular desktop app and
	 * try to activate it */
	ivi_set_desktop_surface(surface);
	ivi_layout_desktop_committed(surface);
}

void
ivi_shell_init_black_fs(struct ivi_compositor *ivi)
{
	struct ivi_output *out;

	wl_list_for_each(out, &ivi->outputs, link) {
		create_black_curtain_view(out);
		insert_black_curtain(out);
	}
}

int
ivi_shell_init(struct ivi_compositor *ivi)
{
	weston_layer_init(&ivi->hidden, ivi->compositor);
	weston_layer_init(&ivi->background, ivi->compositor);
	weston_layer_init(&ivi->normal, ivi->compositor);
	weston_layer_init(&ivi->panel, ivi->compositor);
	weston_layer_init(&ivi->popup, ivi->compositor);
	weston_layer_init(&ivi->fullscreen, ivi->compositor);

	weston_layer_set_position(&ivi->hidden,
				  WESTON_LAYER_POSITION_HIDDEN);
	weston_layer_set_position(&ivi->background,
				  WESTON_LAYER_POSITION_BACKGROUND);
	weston_layer_set_position(&ivi->normal,
				  WESTON_LAYER_POSITION_NORMAL);
	weston_layer_set_position(&ivi->panel,
				  WESTON_LAYER_POSITION_UI);
	weston_layer_set_position(&ivi->popup,
				  WESTON_LAYER_POSITION_TOP_UI);
	weston_layer_set_position(&ivi->fullscreen,
				  WESTON_LAYER_POSITION_FULLSCREEN);

	return 0;
}


static void
ivi_surf_destroy(struct ivi_surface *surf)
{
	struct weston_surface *wsurface = surf->view->surface;

	if (weston_surface_is_mapped(wsurface)) {
		weston_desktop_surface_unlink_view(surf->view);
		weston_view_destroy(surf->view);
	}

	wl_list_remove(&surf->link);
	free(surf);
}

static void
ivi_shell_destroy_views_on_layer(struct weston_layer *layer)
{
	struct weston_view *view, *view_next;

	wl_list_for_each_safe(view, view_next, &layer->view_list.link, layer_link.link) {
		struct ivi_surface *ivi_surf =
			get_ivi_shell_surface(view->surface);
		if (ivi_surf)
			ivi_surf_destroy(ivi_surf);
	}
}

void
ivi_shell_finalize(struct ivi_compositor *ivi)
{
	struct ivi_output *output;

	ivi_shell_destroy_views_on_layer(&ivi->hidden);
	weston_layer_fini(&ivi->hidden);

	ivi_shell_destroy_views_on_layer(&ivi->background);
	weston_layer_fini(&ivi->background);

	ivi_shell_destroy_views_on_layer(&ivi->normal);
	weston_layer_fini(&ivi->normal);

	ivi_shell_destroy_views_on_layer(&ivi->panel);
	weston_layer_fini(&ivi->panel);

	ivi_shell_destroy_views_on_layer(&ivi->popup);
	weston_layer_fini(&ivi->popup);

	wl_list_for_each(output, &ivi->outputs, link) {
		if (output->fullscreen_view.fs &&
		    output->fullscreen_view.fs->view) {
			weston_surface_unref(output->fullscreen_view.fs->view->surface);
			output->fullscreen_view.fs->view = NULL;
		}
	}
	weston_layer_fini(&ivi->fullscreen);
}

void
ivi_shell_advertise_xdg_surfaces(struct ivi_compositor *ivi, struct wl_resource *resource)
{
	struct ivi_surface *surface;

	wl_list_for_each(surface, &ivi->surfaces, link) {
		const char *app_id =
			weston_desktop_surface_get_app_id(surface->dsurface);
		if (app_id == NULL) {
			weston_log("WARNING app_is is null, unable to advertise\n");
			return;
		}
	}
}

static struct wl_client *
client_launch(struct weston_compositor *compositor,
		     struct wet_process *proc,
		     const char *path,
		     wet_process_cleanup_func_t cleanup)
{
	struct wl_client *client = NULL;
	struct custom_env child_env;
	struct fdstr wayland_socket;
	const char *fail_cloexec = "Couldn't unset CLOEXEC on client socket";
	const char *fail_seteuid = "Couldn't call seteuid";
	char *fail_exec;
	char * const *argp;
	char * const *envp;
	sigset_t allsigs;
	pid_t pid;
	bool ret;
	struct ivi_compositor *ivi;
	size_t written __attribute__((unused));

	weston_log("launching '%s'\n", path);
	str_printf(&fail_exec, "Error: Couldn't launch client '%s'\n", path);

	custom_env_init_from_environ(&child_env);
	custom_env_add_from_exec_string(&child_env, path);

	if (os_socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0,
				  wayland_socket.fds) < 0) {
		weston_log("client_launch: "
			   "socketpair failed while launching '%s': %s\n",
			   path, strerror(errno));
		custom_env_fini(&child_env);
		return NULL;
	}
	fdstr_update_str1(&wayland_socket);
	custom_env_set_env_var(&child_env, "WAYLAND_SOCKET",
			       wayland_socket.str1);

	argp = custom_env_get_argp(&child_env);
	envp = custom_env_get_envp(&child_env);

	pid = fork();
	switch (pid) {
	case 0:
		/* Put the client in a new session so it won't catch signals
		 * intended for the parent. Sharing a session can be
		 * confusing when launching weston under gdb, as the ctrl-c
		 * intended for gdb will pass to the child, and weston
		 * will cleanly shut down when the child exits.
		 */
		setsid();

		/* do not give our signal mask to the new process */
		sigfillset(&allsigs);
		sigprocmask(SIG_UNBLOCK, &allsigs, NULL);

		/* Launch clients as the user. Do not launch clients with wrong euid. */
		if (seteuid(getuid()) == -1) {
			written = write(STDERR_FILENO, fail_seteuid, strlen(fail_seteuid));
			_exit(EXIT_FAILURE);
		}

		ret = fdstr_clear_cloexec_fd1(&wayland_socket);
		if (!ret) {
			written = write(STDERR_FILENO, fail_cloexec, strlen(fail_cloexec));
			_exit(EXIT_FAILURE);
		}

		execve(argp[0], argp, envp);

		if (fail_exec)
			written = write(STDERR_FILENO, fail_exec, strlen(fail_exec));
		_exit(EXIT_FAILURE);

	default:
		close(wayland_socket.fds[1]);
		ivi = weston_compositor_get_user_data(compositor);
		client = wl_client_create(compositor->wl_display,
					  wayland_socket.fds[0]);
		if (!client) {
			custom_env_fini(&child_env);
			close(wayland_socket.fds[0]);
			free(fail_exec);
			weston_log("client_launch: "
				"wl_client_create failed while launching '%s'.\n",
				path);
			return NULL;
		}

		proc->pid = pid;
		proc->cleanup = cleanup;
		wl_list_insert(&ivi->child_process_list, &proc->link);
		break;

	case -1:
		fdstr_close_all(&wayland_socket);
		weston_log("client_launch: "
			   "fork failed while launching '%s': %s\n", path,
			   strerror(errno));
		break;
	}

	custom_env_fini(&child_env);
	free(fail_exec);

	return client;
}

struct process_info {
	struct wet_process proc;
	char *path;
};

int
sigchld_handler(int signal_number, void *data)
{
	struct wet_process *p;
	struct ivi_compositor *ivi = data;
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		wl_list_for_each(p, &ivi->child_process_list, link) {
			if (p->pid == pid)
				break;
		}

		if (&p->link == &ivi->child_process_list) {
			weston_log("unknown child process exited\n");
			continue;
		}

		wl_list_remove(&p->link);
		wl_list_init(&p->link);
		p->cleanup(p, status, NULL);
	}

	if (pid < 0 && errno != ECHILD)
		weston_log("waitpid error %s\n", strerror(errno));

	return 1;
}


static void
process_handle_sigchld(struct wet_process *process, int status, void *data)
{
	struct process_info *pinfo =
		container_of(process, struct process_info, proc);

	/*
	 * There are no guarantees whether this runs before or after
	 * the wl_client destructor.
	 */

	if (WIFEXITED(status)) {
		weston_log("%s exited with status %d\n", pinfo->path,
			   WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		weston_log("%s died on signal %d\n", pinfo->path,
			   WTERMSIG(status));
	} else {
		weston_log("%s disappeared\n", pinfo->path);
	}

	free(pinfo->path);
	free(pinfo);
}

int
ivi_launch_shell_client(struct ivi_compositor *ivi, const char *cmd_section,
			struct wl_client **client)
{
	struct process_info *pinfo;
	struct weston_config_section *section;
	char *command = NULL;

	section = weston_config_get_section(ivi->config, cmd_section, NULL, NULL);
	if (section)
		weston_config_section_get_string(section, "command", &command, NULL);

	if (!command)
		return -1;

	pinfo = zalloc(sizeof *pinfo);
	if (!pinfo)
		return -1;

	pinfo->path = strdup(command);
	if (!pinfo->path)
		goto out_free;

	*client = client_launch(ivi->compositor, &pinfo->proc, command, process_handle_sigchld);
	if (!*client)
		goto out_str;

	return 0;

out_str:
	free(pinfo->path);

out_free:
	free(pinfo);

	return -1;
}

static void
destroy_black_curtain_view(struct wl_listener *listener, void *data)
{
	struct fullscreen_view *fs =
		wl_container_of(listener, fs, fs_destroy);


	if (fs && fs->fs) {
		wl_list_remove(&fs->fs_destroy.link);
		free(fs->fs);
	}
}


int
curtain_get_label(struct weston_surface *surface, char *buf, size_t len)
{
	return snprintf(buf, len, "%s", "black curtain");
}

static void
curtain_surface_committed(struct weston_surface *es, struct weston_coord_surface new_origin)
{

}

void
create_black_curtain_view(struct ivi_output *output)
{
	struct weston_surface *surface = NULL;
	struct weston_view *view;
	struct ivi_compositor *ivi = output->ivi;
	struct weston_compositor *ec = ivi->compositor;
	struct weston_output *woutput = output->output;
	struct weston_buffer_reference *buffer_ref;

	if (!woutput)
		return;

	surface = weston_surface_create(ec);
	if (!surface)
		return;

	view = weston_view_create(surface);
	if (!view)
		goto err_surface;

	buffer_ref = weston_buffer_create_solid_rgba(ec, 0.035, 0.045, 0.055, 1.0);
	if (buffer_ref == NULL)
		goto err_view;

	surface->committed = curtain_surface_committed;
	surface->committed_private = NULL;
	weston_surface_set_size(surface, woutput->width, woutput->height);

	weston_surface_attach_solid(surface, buffer_ref,
				    woutput->width, woutput->height);

	weston_surface_set_label_func(surface, curtain_get_label);
	weston_view_set_position(view, woutput->pos);

	output->fullscreen_view.fs = zalloc(sizeof(struct ivi_surface));
	if (!output->fullscreen_view.fs)
		goto err_view;

	output->fullscreen_view.fs->view = view;
	output->fullscreen_view.buffer_ref = buffer_ref;

	output->fullscreen_view.fs_destroy.notify =
		destroy_black_curtain_view;
	wl_signal_add(&woutput->destroy_signal,
		      &output->fullscreen_view.fs_destroy);

	return;

err_view:
	weston_view_destroy(view);
err_surface:
	weston_surface_unref(surface);
}

bool
output_has_black_curtain(struct ivi_output *output)
{
	return (output->fullscreen_view.fs &&
		output->fullscreen_view.fs->view &&
		weston_surface_is_mapped(output->fullscreen_view.fs->view->surface));
}

void
remove_black_curtain(struct ivi_output *output)
{
	struct weston_view *view;
	struct weston_surface *wsurface;

	if ((!output &&
	    !output->fullscreen_view.fs &&
	    !output->fullscreen_view.fs->view) ||
	    !output->fullscreen_view.fs) {
		weston_log("Output %s doesn't have a surface installed!\n", output->name);
		return;
	}

	view = output->fullscreen_view.fs->view;
	wsurface = view->surface;
	assert(weston_surface_is_mapped(view->surface));

	weston_surface_unmap(wsurface);

	weston_view_move_to_layer(view, NULL);
	weston_log("Removed black curtain from output %s\n", output->output->name);
}

void
insert_black_curtain(struct ivi_output *output)
{
	struct weston_view *view;
	struct weston_surface *wsurface;

	if ((!output &&
	    !output->fullscreen_view.fs &&
	    !output->fullscreen_view.fs->view) ||
	    !output->output || !output->fullscreen_view.fs) {
		weston_log("Output %s doesn't have a surface installed!\n", output->name);
		return;
	}

	view = output->fullscreen_view.fs->view;
	wsurface = view->surface;
	if (weston_surface_is_mapped(wsurface))
		return;

	weston_surface_map(wsurface);
	weston_view_move_to_layer(view, &output->ivi->fullscreen.view_list);
	weston_log("Added black curtain to output %s\n", output->output->name);
}

void
shell_send_app_state(struct ivi_compositor *ivi, const char *app_id,
		     enum agl_shell_app_state state)
{
	if (app_id && wl_resource_get_version(ivi->shell_client.resource) >=
	    AGL_SHELL_APP_STATE_SINCE_VERSION) {

		agl_shell_send_app_state(ivi->shell_client.resource,
					 app_id, state);

		if (ivi->shell_client.resource_ext)
			agl_shell_send_app_state(ivi->shell_client.resource_ext,
						 app_id, state);
	}
}

void
shell_send_app_on_output(struct ivi_compositor *ivi, const char *app_id,
			 const char *output_name)
{
	if (app_id && ivi->shell_client.resource &&
	    wl_resource_get_version(ivi->shell_client.resource) >=
	    AGL_SHELL_APP_ON_OUTPUT_SINCE_VERSION) {

		agl_shell_send_app_on_output(ivi->shell_client.resource,
					 app_id, output_name);

		if (ivi->shell_client.resource_ext)
			agl_shell_send_app_on_output(ivi->shell_client.resource_ext,
						 app_id, output_name);
	}
}

static void
shell_ready(struct wl_client *client, struct wl_resource *shell_res)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct ivi_output *output;
	struct ivi_surface *surface, *tmp;

	if (wl_resource_get_version(shell_res) >=
	    AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	    ivi->shell_client.status == BOUND_FAILED) {
		wl_resource_post_error(shell_res,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "agl_shell (ready quest) has already "
				       "been bound. Check out bound_fail event");
		return;
	}

	/* Init already finished. Do nothing */
	if (ivi->shell_client.ready)
		return;


	ivi->shell_client.ready = true;

	wl_list_for_each(output, &ivi->outputs, link) {
		if (output->background &&
		    output->background->role == IVI_SURFACE_ROLE_BACKGROUND) {
			/* track the background surface role as a "regular"
			 * surface so we can activate it */
			ivi_set_background_surface(output->background);
			if (output_has_black_curtain(output))
				remove_black_curtain(output);
		}

		ivi_layout_init(ivi, output);
	}

	wl_list_for_each_safe(surface, tmp, &ivi->pending_surfaces, link) {
		const char *app_id;

		wl_list_remove(&surface->link);
		wl_list_init(&surface->link);
		ivi_check_pending_desktop_surface(surface);
		surface->checked_pending = true;
		app_id = weston_desktop_surface_get_app_id(surface->dsurface);

		shell_send_app_state(ivi, app_id, AGL_SHELL_APP_STATE_STARTED);
	}
}

static void
shell_set_background(struct wl_client *client,
		     struct wl_resource *shell_res,
		     struct wl_resource *surface_res,
		     struct wl_resource *output_res)
{
	struct weston_head *head = weston_head_from_resource(output_res);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *output = to_ivi_output(woutput);
	struct weston_surface *wsurface = wl_resource_get_user_data(surface_res);
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct weston_desktop_surface *dsurface;
	struct ivi_surface *surface;

	if ((wl_resource_get_version(shell_res) >=
	    AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	    ivi->shell_client.status == BOUND_FAILED) ||
	    ivi->shell_client.resource_ext == shell_res) {
		wl_resource_post_error(shell_res,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "agl_shell (set_background) has already "
				       "been bound. Check out bound_fail event");
		return;
	}

	dsurface = weston_surface_get_desktop_surface(wsurface);
	if (!dsurface) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_INVALID_ARGUMENT,
				       "surface must be a desktop surface");
		return;
	}

	surface = weston_desktop_surface_get_user_data(dsurface);
	if (surface->role != IVI_SURFACE_ROLE_NONE) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_INVALID_ARGUMENT,
				       "surface already has another ivi role");
		return;
	}

	if (output->background) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_BACKGROUND_EXISTS,
				       "output already has background");
		return;
	}

	surface->checked_pending = true;
	surface->role = IVI_SURFACE_ROLE_BACKGROUND;
	surface->bg.output = output;
	wl_list_remove(&surface->link);
	wl_list_init(&surface->link);

	output->background = surface;

	weston_desktop_surface_set_maximized(dsurface, true);
	weston_desktop_surface_set_size(dsurface,
					output->output->width,
					output->output->height);
}

static void
shell_set_panel(struct wl_client *client,
		struct wl_resource *shell_res,
		struct wl_resource *surface_res,
		struct wl_resource *output_res,
		uint32_t edge)
{
	struct weston_head *head = weston_head_from_resource(output_res);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *output = to_ivi_output(woutput);
	struct weston_surface *wsurface = wl_resource_get_user_data(surface_res);
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct weston_desktop_surface *dsurface;
	struct ivi_surface *surface;
	struct ivi_surface **member;
	int32_t width = 0, height = 0;

	if ((wl_resource_get_version(shell_res) >=
	     AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	     ivi->shell_client.status == BOUND_FAILED) ||
	    ivi->shell_client.resource_ext == shell_res) {
		wl_resource_post_error(shell_res,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "agl_shell (set_panel) has already been bound. "
				       "Check out bound_fail event");
		return;
	}

	dsurface = weston_surface_get_desktop_surface(wsurface);
	if (!dsurface) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_INVALID_ARGUMENT,
				       "surface must be a desktop surface");
		return;
	}

	surface = weston_desktop_surface_get_user_data(dsurface);
	if (surface->role != IVI_SURFACE_ROLE_NONE) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_INVALID_ARGUMENT,
				       "surface already has another ivi role");
		return;
	}

	switch (edge) {
	case AGL_SHELL_EDGE_TOP:
		member = &output->top;
		break;
	case AGL_SHELL_EDGE_BOTTOM:
		member = &output->bottom;
		break;
	case AGL_SHELL_EDGE_LEFT:
		member = &output->left;
		break;
	case AGL_SHELL_EDGE_RIGHT:
		member = &output->right;
		break;
	default:
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_INVALID_ARGUMENT,
				       "invalid edge for panel");
		return;
	}

	if (*member) {
		wl_resource_post_error(shell_res,
				       AGL_SHELL_ERROR_BACKGROUND_EXISTS,
				       "output already has panel on this edge");
		return;
	}

	surface->checked_pending = true;
	surface->role = IVI_SURFACE_ROLE_PANEL;
	surface->panel.output = output;
	surface->panel.edge = edge;
	wl_list_remove(&surface->link);
	wl_list_init(&surface->link);

	*member = surface;

	switch (surface->panel.edge) {
	case AGL_SHELL_EDGE_TOP:
	case AGL_SHELL_EDGE_BOTTOM:
		width = woutput->width;
		break;
	case AGL_SHELL_EDGE_LEFT:
	case AGL_SHELL_EDGE_RIGHT:
		height = woutput->height;
		break;
	}

	weston_desktop_surface_set_size(dsurface, width, height);
}

static void
shell_activate_app(struct wl_client *client,
		   struct wl_resource *shell_res,
		   const char *app_id,
		   struct wl_resource *output_res)
{
	struct weston_head *head;
	struct weston_output *woutput;
	struct ivi_compositor *ivi;
	struct ivi_output *output;

	head = weston_head_from_resource(output_res);
	if (!head) {
		weston_log("Invalid output to activate '%s' on\n", app_id);
		return;
	}

	woutput = weston_head_get_output(head);
	ivi = wl_resource_get_user_data(shell_res);
	output = to_ivi_output(woutput);

	if (wl_resource_get_version(shell_res) >=
	    AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	    ivi->shell_client.status == BOUND_FAILED) {
		wl_resource_post_error(shell_res,
				       WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "agl_shell has already been bound. "
				       "Check out bound_fail event");
		return;
	}

	ivi_layout_activate(output, app_id);
}

static void
shell_deactivate_app(struct wl_client *client, struct wl_resource *shell_res,
			 const char *app_id)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);

	ivi_layout_deactivate(ivi, app_id);
}

static void
shell_set_app_float(struct wl_client *client, struct wl_resource *shell_res,
		   const char *app_id, int32_t x_pos, int32_t y_pos)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct weston_output *output = get_focused_output(ivi->compositor);
	struct ivi_output *ivi_output;
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);

	if (!output)
		output = get_default_output(ivi->compositor);

	ivi_output = to_ivi_output(output);

	/* verify if already mapped as desktop role, regular, unmap it, so we
	 * can make it float */
	if (surf && surf->role == IVI_SURFACE_ROLE_DESKTOP) {
		struct weston_view *ev = surf->view;
		struct weston_desktop_surface *dsurf = surf->dsurface;
		struct ivi_bounding_box bb = {};
		const char *prev_activate_app_id = NULL;

		if (surf != ivi_output->active) {
			weston_log("Detected that app_id %s is not active but "
				   "continue to allowing setting it up as floating\n", app_id);
		}

		if (ivi_output->previous_active)
			prev_activate_app_id =
				weston_desktop_surface_get_app_id(ivi_output->previous_active->dsurface);

		if (surf == ivi_output->active && prev_activate_app_id &&
		    strcmp(app_id, prev_activate_app_id)) {
			ivi_layout_deactivate(ivi, app_id);
		} else {
			weston_view_move_to_layer(ev, NULL);
		}

		surf->hidden_layer_output = ivi_output;

		/* set attributes */
		surf->popup.output = ivi_output;

		surf->popup.x = x_pos;
		surf->popup.y = y_pos;
		surf->popup.bb = bb;


		/* change the role */
		surf->role = IVI_SURFACE_ROLE_NONE;

		wl_list_remove(&surf->link);
		wl_list_init(&surf->link);

		ivi_set_desktop_surface_popup(surf);

		weston_desktop_surface_set_maximized(dsurf, false);
		weston_desktop_surface_set_size(dsurf, 0, 0);

		/* add to hidden layer */
		weston_view_move_to_layer(ev, &ivi->hidden.view_list);
		weston_compositor_schedule_repaint(ivi->compositor);

	} else if (!surf || (surf && surf->role != IVI_SURFACE_ROLE_POPUP)) {
		ivi_set_pending_desktop_surface_popup(ivi_output, x_pos, y_pos, 
						      0, 0, 0, 0, app_id);
	}
}

static void
shell_set_app_fullscreen(struct wl_client *client,
			 struct wl_resource *shell_res, const char *app_id)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct weston_output *output = get_focused_output(ivi->compositor);
	struct ivi_output *ivi_output;
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);

	if (!output)
		output = get_default_output(ivi->compositor);

	ivi_output = to_ivi_output(output);

	if (surf && surf->role == IVI_SURFACE_ROLE_DESKTOP) {
		struct weston_view *ev = surf->view;
		struct weston_desktop_surface *dsurf = surf->dsurface;

		if (surf != ivi_output->active)
			return;

		surf->hidden_layer_output = ivi_output;

		/* set attributes */
		surf->fullscreen.output = ivi_output;

		/* change the role */
		surf->role = IVI_SURFACE_ROLE_NONE;

		wl_list_remove(&surf->link);
		wl_list_init(&surf->link);

		ivi_set_desktop_surface_fullscreen(surf);

		weston_desktop_surface_set_maximized(dsurf, false);
		weston_desktop_surface_set_fullscreen(dsurf, true);
		weston_desktop_surface_set_size(dsurf,
						ivi_output->output->width,
						ivi_output->output->height);

		/* add to hidden layer */
		weston_view_move_to_layer(ev, &ivi->hidden.view_list);
		weston_compositor_schedule_repaint(ivi->compositor);
	} else if (!surf || (surf && surf->role != IVI_SURFACE_ROLE_FULLSCREEN)) {
		ivi_set_pending_desktop_surface_fullscreen(ivi_output, app_id);
	}
}


static void
shell_set_app_normal(struct wl_client *client, struct wl_resource *shell_res,
		     const char *app_id)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(shell_res);
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);
	struct weston_output *output = get_focused_output(ivi->compositor);
	struct ivi_output *ivi_output;
	struct weston_desktop_surface *dsurf;
	struct weston_geometry area = {};


	if (!surf || (surf && surf->role == IVI_SURFACE_ROLE_DESKTOP))
		return;

	if (!output)
		output = get_default_output(ivi->compositor);

	dsurf = surf->dsurface;
	ivi_output = to_ivi_output(output);

	/* change the role */
	surf->role = IVI_SURFACE_ROLE_NONE;
	surf->state = NORMAL;
	surf->desktop.pending_output = ivi_output;

	wl_list_remove(&surf->link);
	wl_list_init(&surf->link);

	ivi_set_desktop_surface(surf);

	if (ivi_output->area_activation.width ||
	    ivi_output->area_activation.height)
		area = ivi_output->area_activation;
	else
		area = ivi_output->area;

	if (weston_desktop_surface_get_fullscreen(dsurf))
		weston_desktop_surface_set_fullscreen(dsurf, false);

	weston_desktop_surface_set_maximized(dsurf, true);
	weston_desktop_surface_set_size(dsurf, area.width, area.height);

	/* add to hidden layer */
	weston_view_move_to_layer(surf->view, &ivi->hidden.view_list);
	weston_compositor_schedule_repaint(ivi->compositor);

}

static void
shell_destroy(struct wl_client *client, struct wl_resource *res)
{
	struct 	ivi_compositor *ivi = wl_resource_get_user_data(res);

	/* reset status in case bind_fail was sent */
	if (wl_resource_get_version(res) >= AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	    ivi->shell_client.status == BOUND_FAILED)
		ivi->shell_client.status = BOUND_OK;
}

static void
shell_set_activate_region(struct wl_client *client, struct wl_resource *res,
                         struct wl_resource *output, int x, int y,
                         int width, int height)
{
       struct ivi_compositor *ivi = wl_resource_get_user_data(res);
       struct weston_head *head = weston_head_from_resource(output);
       struct weston_output *woutput = weston_head_get_output(head);
       struct ivi_output *ioutput = to_ivi_output(woutput);

       struct weston_geometry area = { .x = x, .y = y,
                                       .width = width,
                                       .height = height };
       if (ivi->shell_client.ready)
               return;

       ioutput->area_activation = area;
}

static void
shell_set_app_output(struct wl_client *client, struct wl_resource *res,
		    const char *app_id, struct wl_resource *output)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(res);
	struct weston_head *head = weston_head_from_resource(output);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *ioutput = to_ivi_output(woutput);
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);
	struct ivi_output *desktop_last_output;
	struct ivi_output *current_completed_output;

	if (!app_id || !ioutput)
		return;

	/* handle the case we're not mapped at all */
	if (!surf) {
		ivi_set_pending_desktop_surface_remote(ioutput, app_id);
		shell_send_app_on_output(ivi, app_id, woutput->name);
		return;
	}

	desktop_last_output = surf->desktop.last_output;
	current_completed_output = surf->current_completed_output;

	if (surf->remote.output)
		surf->hidden_layer_output = surf->remote.output;
	else
		surf->hidden_layer_output = desktop_last_output;
	assert(surf->hidden_layer_output);

	if (ivi_surface_count_one(current_completed_output, IVI_SURFACE_ROLE_REMOTE) ||
	    ivi_surface_count_one(current_completed_output, IVI_SURFACE_ROLE_DESKTOP)) {
		if (!current_completed_output->background)
			insert_black_curtain(current_completed_output);
	} else {
		ivi_layout_deactivate(ivi, app_id);
	}

	/* update the remote output */
	surf->remote.output = ioutput;

	if (surf->role != IVI_SURFACE_ROLE_REMOTE) {
		wl_list_remove(&surf->link);
		wl_list_init(&surf->link);

		surf->role = IVI_SURFACE_ROLE_NONE;
		ivi_set_desktop_surface_remote(surf);
	}

	shell_send_app_on_output(ivi, app_id, woutput->name);
}

static void
shell_set_app_position(struct wl_client *client, struct wl_resource *res,
		    const char *app_id, int32_t x, int32_t y)
{
	struct weston_coord_global pos;
	struct ivi_compositor *ivi = wl_resource_get_user_data(res);
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);

	if (!surf || !app_id || surf->role != IVI_SURFACE_ROLE_POPUP)
		return;

	pos.c.x = x;
	pos.c.y = y;
	weston_view_set_position(surf->view, pos);
	weston_compositor_schedule_repaint(ivi->compositor);
}

static void
shell_set_app_scale(struct wl_client *client, struct wl_resource *res,
		    const char *app_id, int32_t width, int32_t height)
{

	struct ivi_compositor *ivi = wl_resource_get_user_data(res);
	struct ivi_surface *surf = ivi_find_app(ivi, app_id);

	if (!surf || !app_id || surf->role != IVI_SURFACE_ROLE_POPUP)
		return;

	weston_desktop_surface_set_size(surf->dsurface, width, height);
	weston_compositor_schedule_repaint(ivi->compositor);
}

static void
_ivi_set_pending_desktop_surface_split(struct wl_resource *output,
				       const char *app_id, uint32_t orientation)
{
	weston_log("%s() added split surface for app_id '%s' with "
		"orientation %d to pending\n", __func__, app_id, orientation);

	struct weston_head *head = weston_head_from_resource(output);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *ivi_output = to_ivi_output(woutput);

	struct pending_app_tile *app_tile = zalloc(sizeof(*app_tile));

	app_tile->base.app_id = strdup(app_id);
	app_tile->base.ioutput = ivi_output;
	app_tile->base.role = IVI_SURFACE_ROLE_TILE;

	app_tile->orientation = orientation;
	wl_list_insert(&ivi_output->ivi->pending_apps, &app_tile->base.link);
}


const char *
split_orientation_to_string(uint32_t orientation)
{
	switch (orientation) {
	case AGL_SHELL_TILE_ORIENTATION_LEFT:
		return "tile_left";
	break;
	case AGL_SHELL_TILE_ORIENTATION_RIGHT:
		return "tile_right";
	break;
	case AGL_SHELL_TILE_ORIENTATION_TOP:
		return "title_top";
	break;
	case AGL_SHELL_TILE_ORIENTATION_BOTTOM:
		return "tile_bottom";
	break;
	default:
		return "none";
	}
}

static uint32_t
reverse_orientation(uint32_t orientation)
{

	switch (orientation) {
	case AGL_SHELL_TILE_ORIENTATION_LEFT:
		return AGL_SHELL_TILE_ORIENTATION_RIGHT;
	break;
	case AGL_SHELL_TILE_ORIENTATION_RIGHT:
		return AGL_SHELL_TILE_ORIENTATION_LEFT;
	break;
	case AGL_SHELL_TILE_ORIENTATION_TOP:
		return AGL_SHELL_TILE_ORIENTATION_BOTTOM;
	break;
	case AGL_SHELL_TILE_ORIENTATION_BOTTOM:
		return AGL_SHELL_TILE_ORIENTATION_TOP;
	break;
	default:
		return AGL_SHELL_TILE_ORIENTATION_NONE;
	}
}

void
_ivi_set_shell_surface_split(struct ivi_surface *surface, struct ivi_output *ioutput,
			     uint32_t orientation, uint32_t width, int32_t sticky,
			     bool to_activate)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct weston_view *ev = surface->view;
	struct weston_geometry geom = {};
	struct ivi_output *output = NULL;
	struct weston_coord_global pos;

	int new_width, new_height;
	int x, y;

	geom = weston_desktop_surface_get_geometry(surface->dsurface);
	output = ivi_layout_get_output_from_surface(surface);

	if (!output)
		output = ioutput;

	// if we don't supply a width we automatically default to doing a half
	// split, each window taking half of the current output
	switch (orientation) {
	case AGL_SHELL_TILE_ORIENTATION_LEFT:
	case AGL_SHELL_TILE_ORIENTATION_RIGHT:
		if (width == 0) {
			new_width = output->area.width / 2;
			new_height = output->area.height;
		} else {
			new_width = width;
			new_height = output->area.height;
		}
	break;
	case AGL_SHELL_TILE_ORIENTATION_TOP:
	case AGL_SHELL_TILE_ORIENTATION_BOTTOM:
		if (width == 0) {
			new_width = output->area.width;
			new_height = output->area.height / 2;
		} else {
			new_width = output->area.width;
			new_height = width;
		}
	break;
	case AGL_SHELL_TILE_ORIENTATION_NONE:
		/* use the current_completed_output because the sticky window
		 * might have changed the output area */
		new_width = surface->current_completed_output->area_activation.width;
		new_height = surface->current_completed_output->area_activation.height;

		if (new_width != output->area.width)
			output->area.width = new_width;

		if (new_height != output->area.height)
			output->area.height = new_height;

		weston_log("Adjusting activation area "
			   "to %dX%d\n", output->area.width, output->area.height);

		if (surface->sticky) {
			surface->sticky = 0;
			weston_log("Resetting sticky window\n");
		}

		surface->role = surface->prev_role;
		weston_log("Resetting tile role to previous role\n");
	break;
	default:
		/* nothing */
		assert(!"Invalid orientation passed");

	}

	x = output->area.x - geom.x;
	y = output->area.y - geom.y;

	if (orientation == AGL_SHELL_TILE_ORIENTATION_RIGHT)
		x += output->area.width - new_width;
	else if (orientation == AGL_SHELL_TILE_ORIENTATION_BOTTOM)
		y += output->area.height - new_height;


	if (to_activate) {
		struct ivi_shell_seat *ivi_seat = NULL;
		struct weston_seat *wseat = get_ivi_shell_weston_first_seat(ivi);

		if (wseat)
			ivi_seat = get_ivi_shell_seat(wseat);

		if (!weston_view_is_mapped(ev))
			weston_view_update_transform(ev);


		// mark view as mapped
		weston_surface_map(ev->surface);

		// update older/new active surface
		output->previous_active = output->active;
		output->active = surface;

		// add to the layer and inflict damage
		weston_view_set_output(ev, output->output);
		weston_view_move_to_layer(ev, &ivi->normal.view_list);

		// handle input / keyboard
		if (ivi_seat)
			ivi_shell_activate_surface(surface, ivi_seat, WESTON_ACTIVATE_FLAG_NONE);
	}

	pos.c.x = x;
	pos.c.y = y;

	weston_view_set_position(surface->view, pos);
	weston_desktop_surface_set_size(surface->dsurface, new_width, new_height);
	weston_desktop_surface_set_orientation(surface->dsurface, orientation);
	surface->orientation = orientation;

	weston_compositor_schedule_repaint(ivi->compositor);

	if (sticky)
		surface->sticky = sticky;

	if (orientation != AGL_SHELL_TILE_ORIENTATION_NONE) {
		surface->prev_role = surface->role;
		surface->role = IVI_SURFACE_ROLE_TILE;
		weston_log("Found split orientation different that none, "
			   "setting surface role to orientation tile\n");
	}

	if (surface->sticky) {
		if (orientation == AGL_SHELL_TILE_ORIENTATION_LEFT ||
		    orientation == AGL_SHELL_TILE_ORIENTATION_RIGHT)
			output->area.width -= new_width;

		if (orientation == AGL_SHELL_TILE_ORIENTATION_TOP ||
		    orientation == AGL_SHELL_TILE_ORIENTATION_BOTTOM)
			output->area.height -= new_height;

		weston_log("Found sticky window, adjusting activation area "
			   "to %dX%d\n", output->area.width, output->area.height);
	}
}

static int
shell_ivi_surf_count_split_surfaces(struct ivi_compositor *ivi)
{
	int count = 0;
	struct ivi_surface *surf;

	wl_list_for_each(surf, &ivi->surfaces, link) {
		if (surf->orientation > AGL_SHELL_TILE_ORIENTATION_NONE)
			count++;
	}

	return count;
}


static
void shell_set_app_split(struct wl_client *client, struct wl_resource *res,
			 const char *app_id, uint32_t orientation, int32_t width,
			 int32_t sticky, struct wl_resource *output_res)
{
	struct ivi_surface *surf;
	struct ivi_compositor *ivi = wl_resource_get_user_data(res);

	struct weston_head *head = weston_head_from_resource(output_res);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *output = to_ivi_output(woutput);

	if (!app_id)
		return;

	if (shell_ivi_surf_count_split_surfaces(ivi) > 2) {
		weston_log("Found more than two split surfaces in tile orientation.\n");
		return;
	}

	/* add it as pending until */
	surf = ivi_find_app(ivi, app_id);
	if (!surf) {
		_ivi_set_pending_desktop_surface_split(output_res, app_id, orientation);
		return;
	}

	if (output->previous_active && output->background != output->previous_active) {
		int width_prev_app = 0;
		struct weston_view *ev = output->previous_active->view;
		const char *prev_app_id = 
			weston_desktop_surface_get_app_id(output->previous_active->dsurface);

		if (orientation != AGL_SHELL_TILE_ORIENTATION_NONE)  {
			if (!weston_view_is_mapped(ev))
				weston_view_update_transform(ev);

			weston_surface_map(ev->surface);

			weston_view_set_output(ev, woutput);
			weston_view_move_to_layer(ev, &ivi->normal.view_list);
		} else {
			weston_surface_unmap(ev->surface);
			weston_view_move_to_layer(ev, NULL);
		}

		/* a 0 width means we have no explicit width set-up */
		if (width > 0) {
			if (orientation == AGL_SHELL_TILE_ORIENTATION_TOP ||
			    orientation == AGL_SHELL_TILE_ORIENTATION_BOTTOM)
				width_prev_app = output->area.height - width;
			else
				width_prev_app = output->area.width - width;
		}

		weston_log("Setting previous application '%s' to orientation '%s' width %d, not sticky\n",
			   prev_app_id, split_orientation_to_string(reverse_orientation(orientation)),
			   width_prev_app);

		_ivi_set_shell_surface_split(output->previous_active, NULL,
					     reverse_orientation(orientation),
					     width_prev_app, false, false);

		if (orientation == AGL_SHELL_TILE_ORIENTATION_NONE &&
		    output->active == surf) {
			output->active = output->previous_active;
		}
	}

	weston_log("Setting application '%s' to orientation '%s' width %d, %s\n",
		   app_id, split_orientation_to_string(orientation),
		   width, sticky == 1 ? "sticky" : "not sticky");
	_ivi_set_shell_surface_split(surf, NULL, orientation, width, sticky, false);
}

static void
shell_ext_destroy(struct wl_client *client, struct wl_resource *res)
{
	struct 	ivi_compositor *ivi = wl_resource_get_user_data(res);

	ivi->shell_client_ext.doas_requested = false;
	wl_resource_destroy(res);
}

static void
shell_ext_doas(struct wl_client *client, struct wl_resource *res)
{
	struct 	ivi_compositor *ivi = wl_resource_get_user_data(res);

	ivi->shell_client_ext.doas_requested = true;

	if (ivi->shell_client_ext.resource && ivi->shell_client.resource) {
		ivi->shell_client_ext.doas_requested_pending_bind = true;

		agl_shell_ext_send_doas_done(ivi->shell_client_ext.resource,
					     AGL_SHELL_EXT_DOAS_SHELL_CLIENT_STATUS_SUCCESS);
	} else {
		agl_shell_ext_send_doas_done(ivi->shell_client_ext.resource,
					     AGL_SHELL_EXT_DOAS_SHELL_CLIENT_STATUS_FAILED);
	}

}

static const struct agl_shell_interface agl_shell_implementation = {
	.ready = shell_ready,
	.set_background = shell_set_background,
	.set_panel = shell_set_panel,
	.activate_app = shell_activate_app,
	.destroy = shell_destroy,
	.set_activate_region = shell_set_activate_region,
	.deactivate_app = shell_deactivate_app,
	.set_app_float = shell_set_app_float,
	.set_app_normal = shell_set_app_normal,
	.set_app_fullscreen = shell_set_app_fullscreen,
	.set_app_output = shell_set_app_output,
	.set_app_position = shell_set_app_position,
	.set_app_scale = shell_set_app_scale,
	.set_app_split = shell_set_app_split,
};

static const struct agl_shell_ext_interface agl_shell_ext_implementation = {
	.destroy = shell_ext_destroy,
	.doas_shell_client = shell_ext_doas,
};

void
ivi_compositor_destroy_pending_surfaces(struct ivi_compositor *ivi)
{
	struct pending_popup *p_popup, *next_p_popup;
	struct pending_split *split_surf, *next_split_surf;
	struct pending_fullscreen *fs_surf, *next_fs_surf;
	struct pending_remote *remote_surf, *next_remote_surf;

	wl_list_for_each_safe(p_popup, next_p_popup,
			      &ivi->popup_pending_apps, link)
		ivi_remove_pending_desktop_surface_popup(p_popup);

	wl_list_for_each_safe(split_surf, next_split_surf,
			      &ivi->split_pending_apps, link)
		ivi_remove_pending_desktop_surface_split(split_surf);

	wl_list_for_each_safe(fs_surf, next_fs_surf,
			      &ivi->fullscreen_pending_apps, link)
		ivi_remove_pending_desktop_surface_fullscreen(fs_surf);

	wl_list_for_each_safe(remote_surf, next_remote_surf,
			      &ivi->remote_pending_apps, link)
		ivi_remove_pending_desktop_surface_remote(remote_surf);
}

static void
unbind_agl_shell(struct wl_resource *resource)
{
	struct ivi_compositor *ivi;
	struct ivi_output *output;
	struct ivi_surface *surf, *surf_tmp;

	ivi = wl_resource_get_user_data(resource);

	/* reset status to allow other clients issue legit requests */
	if (wl_resource_get_version(resource) >=
	    AGL_SHELL_BOUND_OK_SINCE_VERSION &&
	    ivi->shell_client.status == BOUND_FAILED) {
		ivi->shell_client.status = BOUND_OK;
		return;
	}

	wl_list_for_each(output, &ivi->outputs, link) {

		/* reset the active surf if there's one present */
		if (output->active) {
			struct weston_geometry area = {};

			weston_surface_unmap(output->active->view->surface);
			weston_view_move_to_layer(output->active->view, NULL);

			output->active = NULL;
			output->area_activation = area;
		}

		insert_black_curtain(output);
	}

	wl_list_for_each_safe(surf, surf_tmp, &ivi->surfaces, link) {
		wl_list_remove(&surf->link);
		wl_list_init(&surf->link);
	}

	wl_list_for_each_safe(surf, surf_tmp, &ivi->pending_surfaces, link) {
		wl_list_remove(&surf->link);
		wl_list_init(&surf->link);
	}

	wl_list_init(&ivi->surfaces);
	wl_list_init(&ivi->pending_surfaces);

	ivi->shell_client.ready = false;
	ivi->shell_client.resource = NULL;
	ivi->shell_client.resource_ext = NULL;
	ivi->shell_client.client = NULL;
}

static void
unbind_agl_shell_ext(struct wl_resource *resource)
{
	struct ivi_compositor *ivi = wl_resource_get_user_data(resource);

	ivi->shell_client_ext.resource = NULL;
	ivi->shell_client.resource_ext = NULL;
	ivi->shell_client_ext.doas_requested = false;
}

static void
bind_agl_shell(struct wl_client *client,
	       void *data, uint32_t version, uint32_t id)
{
	struct ivi_compositor *ivi = data;
	struct wl_resource *resource;
	struct ivi_policy *policy;
	void *interface;

	policy = ivi->policy;
	interface = (void *) &agl_shell_interface;
	if (policy && policy->api.shell_bind_interface &&
	    !policy->api.shell_bind_interface(client, interface)) {
		wl_client_post_implementation_error(client,
				       "client not authorized to use agl_shell");
		return;
	}

	resource = wl_resource_create(client, &agl_shell_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	if (ivi->shell_client.resource && wl_resource_get_version(resource) == 1) {
		wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
				"agl_shell has already been bound (version 1)");
		wl_resource_destroy(resource);
		return;
	}

	// for agl_shell client proxy
	if (ivi->shell_client.resource &&
	    ivi->shell_client_ext.doas_requested_pending_bind) {

		ivi->shell_client_ext.doas_requested_pending_bind = false;

		// if there's one connected
		if (ivi->shell_client.resource_ext) {
			ivi->shell_client_ext.status = BOUND_FAILED;
			agl_shell_send_bound_fail(ivi->shell_client.resource_ext);
			wl_resource_destroy(resource);
			return;
		}

		wl_resource_set_implementation(resource, &agl_shell_implementation,
					       ivi, NULL);
		ivi->shell_client.resource_ext = resource;
		ivi->shell_client_ext.status = BOUND_OK;
		agl_shell_send_bound_ok(ivi->shell_client.resource_ext);
		return;
	}


	// if we already have an agl_shell client
	if (ivi->shell_client.resource) {
		agl_shell_send_bound_fail(resource);
		ivi->shell_client.status = BOUND_FAILED;
		wl_resource_destroy(resource);
		return;
	}

	// default agl_shell client
	wl_resource_set_implementation(resource, &agl_shell_implementation,
				       ivi, unbind_agl_shell);
	ivi->shell_client.resource = resource;

	if (wl_resource_get_version(resource) >=
	    AGL_SHELL_BOUND_OK_SINCE_VERSION) {
		/* if we land here we'll have BOUND_OK by default,
		   but still do the assignment */
		ivi->shell_client.status = BOUND_OK;
		agl_shell_send_bound_ok(ivi->shell_client.resource);
	}
}

static void
bind_agl_shell_ext(struct wl_client *client,
		   void *data, uint32_t version, uint32_t id)
{
	struct ivi_compositor *ivi = data;
	struct wl_resource *resource;

	resource = wl_resource_create(client, &agl_shell_ext_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	if (ivi->shell_client_ext.resource) {
		wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
				       "agl_shell_ext has already been bound");
		return;
	}

	wl_resource_set_implementation(resource, &agl_shell_ext_implementation,
				       ivi, unbind_agl_shell_ext);
	ivi->shell_client_ext.resource = resource;
}

int
ivi_shell_create_global(struct ivi_compositor *ivi)
{
	ivi->agl_shell = wl_global_create(ivi->compositor->wl_display,
					  &agl_shell_interface, 11,
					  ivi, bind_agl_shell);
	if (!ivi->agl_shell) {
		weston_log("Failed to create wayland global.\n");
		return -1;
	}

	ivi->agl_shell_ext = wl_global_create(ivi->compositor->wl_display,
					      &agl_shell_ext_interface, 1,
					      ivi, bind_agl_shell_ext);
	if (!ivi->agl_shell_ext) {
		weston_log("Failed to create agl_shell_ext global.\n");
		return -1;
	}

	return 0;
}
