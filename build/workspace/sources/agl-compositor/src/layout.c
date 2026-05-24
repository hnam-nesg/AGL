/*
 * Copyright © 2019, 2024 Collabora, Ltd.
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
#include "shared/helpers.h"

#include <assert.h>
#include <string.h>

#include <libweston/config-parser.h>
#include <libweston/libweston.h>
#include <libweston/desktop.h>

#define AGL_COMP_DEBUG

static const char *ivi_roles_as_string[] = {
	[IVI_SURFACE_ROLE_NONE]		= "NONE",
	[IVI_SURFACE_ROLE_BACKGROUND]	= "BACKGROUND",
	[IVI_SURFACE_ROLE_PANEL]	= "PANEL",
	[IVI_SURFACE_ROLE_DESKTOP]	= "DESKTOP",
	[IVI_SURFACE_ROLE_POPUP]	= "POPUP",
	[IVI_SURFACE_ROLE_SPLIT_H]	= "SPLIT_H",
	[IVI_SURFACE_ROLE_SPLIT_V]	= "SPLIT_V",
	[IVI_SURFACE_ROLE_FULLSCREEN]	= "FULLSCREEN",
	[IVI_SURFACE_ROLE_REMOTE]	= "REMOTE",
	[IVI_SURFACE_ROLE_TILE]		= "SPLIT",
};

bool
ivi_surf_in_hidden_layer(struct ivi_compositor *ivi, struct ivi_surface *surface);

static bool
is_assistant_overlay_app_id(const char *app_id)
{
	return app_id && strcmp(app_id, "assistant") == 0;
}

void
ivi_layout_save(struct ivi_compositor *ivi, struct ivi_output *output)
{
	struct ivi_output *new_output;
	ivi->need_ivi_output_relayout = true;

	new_output = zalloc(sizeof(*new_output));

	new_output->ivi = ivi;
	new_output->background = output->background;

	new_output->top = output->top;
	new_output->bottom = output->bottom;
	new_output->left = output->left;
	new_output->right = output->right;

	new_output->active = output->active;
	new_output->previous_active = output->previous_active;
	new_output->name = strdup(output->name);
	if (output->app_ids)
		new_output->app_ids = strdup(output->app_ids);

	new_output->area = output->area;
	new_output->area_saved = output->area_saved;
	new_output->area_activation = output->area_activation;

	weston_log("saving output layout for output %s\n", new_output->name);

	wl_list_insert(&ivi->saved_outputs, &new_output->link);
}

void
ivi_layout_restore(struct ivi_compositor *ivi, struct ivi_output *n_output)
{
	struct ivi_output *output = NULL;
	struct ivi_output *iter_output;

	if (!ivi->need_ivi_output_relayout)
		return;

	ivi->need_ivi_output_relayout = false;

	wl_list_for_each(iter_output, &ivi->saved_outputs, link) {
		if (strcmp(n_output->name, iter_output->name) == 0) {
			output = iter_output;
			break;
		}
	}

	if (!output)
		return;

	weston_log("restoring output layout for output %s\n", output->name);
	n_output->background = output->background;

	n_output->top = output->top;
	n_output->bottom = output->bottom;
	n_output->left = output->left;
	n_output->right = output->right;

	n_output->active = output->active;
	n_output->previous_active = output->previous_active;
	if (output->app_ids)
		n_output->app_ids = strdup(output->app_ids);

	n_output->area = output->area;
	n_output->area_saved = output->area_saved;
	n_output->area_activation = output->area_activation;

	free(output->app_ids);
	free(output->name);
	wl_list_remove(&output->link);
	free(output);
}

const char *
ivi_layout_get_surface_role_name(struct ivi_surface *surf)
{
	if (surf->role < 0 || surf->role >= ARRAY_LENGTH(ivi_roles_as_string))
		return " unknown surface role";

	return ivi_roles_as_string[surf->role];
}

static void
ivi_background_init(struct ivi_compositor *ivi, struct ivi_output *output)
{
	struct weston_output *woutput = output->output;
	struct ivi_surface *bg = output->background;
	struct weston_view *view;
	struct weston_surface *wsurface;

	if (!bg) {
		weston_log("WARNING: Output does not have a background\n");
		return;
	}

	wsurface = weston_desktop_surface_get_surface(bg->dsurface);
	assert(bg->role == IVI_SURFACE_ROLE_BACKGROUND);

	view = bg->view;
	weston_surface_map(wsurface);

	weston_view_set_output(view, woutput);
	weston_view_set_position(view, woutput->pos);

	weston_view_move_to_layer(view, &ivi->background.view_list);
	weston_log("(background) position view %p, x %f, y %f, on output %s\n", view,
			woutput->pos.c.x, woutput->pos.c.y, output->name);

}

static void
ivi_panel_init(struct ivi_compositor *ivi, struct ivi_output *output,
	       struct ivi_surface *panel)
{
	struct weston_output *woutput = output->output;
	struct weston_desktop_surface *dsurface;
	struct weston_view *view;
	struct weston_geometry geom;
	struct weston_coord_global pos = woutput->pos;
	struct weston_surface *wsurface;

	if (!panel)
		return;

	assert(panel->role == IVI_SURFACE_ROLE_PANEL);
	dsurface = panel->dsurface;
	view = panel->view;
	geom = weston_desktop_surface_get_geometry(dsurface);
	wsurface = weston_desktop_surface_get_surface(panel->dsurface);

	weston_log("(panel) geom.width %d, geom.height %d, geom.x %d, geom.y %d\n",
			geom.width, geom.height, geom.x, geom.y);

	switch (panel->panel.edge) {
	case AGL_SHELL_EDGE_TOP:
		output->area.y += geom.height;
		output->area.height -= geom.height;
		break;
	case AGL_SHELL_EDGE_BOTTOM:
		pos.c.y += woutput->height - geom.height;
		output->area.height -= geom.height;
		break;
	case AGL_SHELL_EDGE_LEFT:
		output->area.x += geom.width;
		output->area.width -= geom.width;
		break;
	case AGL_SHELL_EDGE_RIGHT:
		pos.c.x += woutput->width - geom.width;
		output->area.width -= geom.width;
		break;
	}

	pos.c.x -= geom.x;
	pos.c.y -= geom.y;

	weston_view_set_output(view, woutput);
	weston_view_set_position(view, pos);

	weston_surface_map(wsurface);
	weston_view_move_to_layer(view, &ivi->panel.view_list);

	weston_log("(panel) edge %d position view %p, x %f, y %f\n",
			panel->panel.edge, view, pos.c.x, pos.c.y);

	weston_log("panel type %d inited on output %s\n", panel->panel.edge,
			output->name);
}

/*
 * Initializes all static parts of the layout, i.e. the background and panels.
 */
void
ivi_layout_init(struct ivi_compositor *ivi, struct ivi_output *output)
{
	bool use_default_area = true;
	struct weston_config_section *section = output->config;
	char *t;

	weston_config_section_get_string(section, "activation-area", &t, NULL);
	if (t) {
		if (output->area_activation.width == 0 &&
		    output->area_activation.height == 0 &&
		    output->area_activation.x == 0 &&
		    output->area_activation.y == 0) {
			weston_log("WARNING: activation-area set in "
					"configuration file, but yet applied!\n");
			if (parse_activation_area(t, output) < 0)
				weston_log("Invalid activation-area \"%s\" for output %s\n",
					   t, output->name);
		} else {
			weston_log("WARNING: activation-area detected in ini file, "
					"but agl_shell override detected!\n");
			if (parse_activation_area(t, output) < 0)
				weston_log("Invalid activation-area \"%s\" for output %s\n",
					   t, output->name);
		}
	}
	free(t);

	ivi_background_init(ivi, output);

	if (output->area_activation.width ||
	    output->area_activation.height ||
	    output->area_activation.x ||
	    output->area_activation.y) {
		/* Sanity check target area is within output bounds */
		if ((output->area_activation.x + output->area_activation.width) < output->output->width ||
		    (output->area_activation.y + output->area_activation.height) < output->output->height) {
			weston_log("Using specified area for output %s, ignoring panels\n",
				   output->name);
			output->area.x = output->area_activation.x;
			output->area.y = output->area_activation.y;
			output->area.width = output->area_activation.width;
			output->area.height = output->area_activation.height;
			use_default_area = false;
		} else {
			weston_log("Invalid activation-area position for output %s, ignoring\n",
				   output->name);
		}
	}
	if (use_default_area) {
		output->area.x = 0;
		output->area.y = 0;
		output->area.width = output->output->width;
		output->area.height = output->output->height;

		ivi_panel_init(ivi, output, output->top);
		ivi_panel_init(ivi, output, output->bottom);
		ivi_panel_init(ivi, output, output->left);
		ivi_panel_init(ivi, output, output->right);
	}

	weston_compositor_schedule_repaint(ivi->compositor);

	weston_log("Usable area: %dx%d+%d,%d\n",
		   output->area.width, output->area.height,
		   output->area.x, output->area.y);
}

struct ivi_surface *
ivi_find_app(struct ivi_compositor *ivi, const char *app_id)
{
	struct ivi_surface *surf;
	const char *id;

	wl_list_for_each(surf, &ivi->surfaces, link) {
		id = weston_desktop_surface_get_app_id(surf->dsurface);
		if (id && strcmp(app_id, id) == 0)
			return surf;
	}

	return NULL;
}

static void
ivi_layout_activate_complete(struct ivi_output *output,
			     struct ivi_surface *surf)
{
	struct ivi_compositor *ivi = output->ivi;
	struct weston_output *woutput = output->output;
	struct weston_view *view = surf->view;
	struct weston_seat *wseat = get_ivi_shell_weston_first_seat(ivi);
	struct ivi_shell_seat *ivi_seat = get_ivi_shell_seat(wseat);
	const char *app_id = weston_desktop_surface_get_app_id(surf->dsurface);
	bool update_previous = true;
	struct weston_coord_global pos;

	if (output_has_black_curtain(output)) {
		if (!output->background) {
			weston_log("Found that we have no background surface "
				    "for output %s. Using black curtain as background\n",
				    output->output->name);

			struct weston_view *ev =
				output->fullscreen_view.fs->view;

			/* use the black curtain as background when we have
			 * none added by the shell client. */
			weston_view_move_to_layer(ev, &ivi->normal.view_list);
		} else {
			remove_black_curtain(output);
		}
	}


	weston_view_set_output(view, woutput);
	/* drop any previous masks set on this view */
	weston_view_set_mask_infinite(view);

	if (surf->role != IVI_SURFACE_ROLE_BACKGROUND) {
		pos.c.x = woutput->pos.c.x + output->area.x;
		pos.c.y = woutput->pos.c.y + output->area.y;
		weston_view_set_position(view, pos);
	}

	/* reset any previous orientation */
	if (surf->orientation != AGL_SHELL_TILE_ORIENTATION_NONE &&
	    !surf->sticky) {
		weston_log("%s() resetting itself to none orientation\n", __func__);
		surf->orientation = AGL_SHELL_TILE_ORIENTATION_NONE;
		weston_desktop_surface_set_orientation(surf->dsurface,
						       surf->orientation);
	}

	/* handle a movement from one output to another */
	if (surf->current_completed_output &&
	    surf->current_completed_output != output) {

		/* we're migrating the same surface but to another output */
		if (surf->current_completed_output->active == surf) {
			struct weston_view *ev =
				surf->current_completed_output->active->view;

			surf->current_completed_output->previous_active =
				surf->current_completed_output->active;
			surf->current_completed_output->active = NULL;

			weston_view_move_to_layer(ev, NULL);

			/* damage all possible outputs to avoid stale views */
			weston_compositor_damage_all(ivi->compositor);
		}
	}

	if (output->active) {
		/* keep the background surface mapped at all times */
		if (output->active->role != IVI_SURFACE_ROLE_BACKGROUND &&
		    !output->active->sticky && output->active != surf) {
			weston_surface_unmap(output->active->view->surface);
			weston_view_move_to_layer(output->active->view, NULL);
		}
	}

	if (output->previous_active && output->active) {
		const char *c_app_id =
			weston_desktop_surface_get_app_id(output->active->dsurface);

		/* if the currently activated app_id is the same as the one
		 * we're trying to complete activation with means we're
		 * operating on the same app_id so do update previous_active as
		 * it will overwrite it with the same value */
		if (c_app_id && app_id && !strcmp(c_app_id, app_id)) {
			update_previous = false;
		}
	}

	if (update_previous)
		output->previous_active = output->active;
	output->active = surf;
	surf->current_completed_output = output;

	if (!weston_surface_is_mapped(view->surface)) {
		weston_surface_map(view->surface);
		weston_view_move_to_layer(view, &ivi->normal.view_list);

		weston_log("Activation completed for app_id %s, role %s, output %s\n",
				app_id,
				ivi_layout_get_surface_role_name(surf), output->name);

		shell_send_app_state(ivi, app_id, AGL_SHELL_APP_STATE_ACTIVATED);

		if (ivi_seat)
			ivi_shell_activate_surface(surf, ivi_seat, WESTON_ACTIVATE_FLAG_NONE);
	} else {
		weston_view_update_transform(view);
	}


	/*
	 * the 'remote' role now makes use of this part so make sure we don't
	 * trip the enum such that we might end up with a modified output for
	 * 'remote' role
	 */
	if (surf->role == IVI_SURFACE_ROLE_DESKTOP) {
		if (surf->desktop.pending_output)
			surf->desktop.last_output = surf->desktop.pending_output;
		surf->desktop.pending_output = NULL;
	}
}

static bool
ivi_layout_find_output_with_app_id(const char *app_id, struct ivi_output *output)
{
	char *cur;
	size_t app_id_len;

	cur = output->app_ids;
	app_id_len = strlen(app_id);

	while ((cur = strstr(cur, app_id))) {
		if ((cur[app_id_len] == ',' || cur[app_id_len] == '\0') &&
	            (cur == output->app_ids || cur[-1] == ','))
			return true;
		cur++;
	}

	return false;
}

struct ivi_output *
ivi_layout_find_with_app_id(const char *app_id, struct ivi_compositor *ivi)
{
	struct ivi_output *out;

	if (!app_id)
		return NULL;

	wl_list_for_each(out, &ivi->outputs, link) {
		if (!out->app_ids)
			continue;

		if (ivi_layout_find_output_with_app_id(app_id, out))
			return out;
	}
	return NULL;
}

struct ivi_output *
ivi_layout_find_bg_output(struct ivi_compositor *ivi)
{
	struct ivi_output *out;

	wl_list_for_each(out, &ivi->outputs, link) {
		if (out->background &&
		    out->background->role == IVI_SURFACE_ROLE_BACKGROUND)
			return out;
	}

	return NULL;
}


static void
ivi_layout_add_to_hidden_layer(struct ivi_surface *surf,
			       struct ivi_output *ivi_output)
{
	struct weston_desktop_surface *dsurf = surf->dsurface;
	struct weston_view *ev = surf->view;
	struct ivi_compositor *ivi = surf->ivi;
	const char *app_id = weston_desktop_surface_get_app_id(dsurf);

	/*
	 * If the view isn't mapped, we put it onto the hidden layer so it will
	 * start receiving frame events, and will be able to act on our
	 * configure event.
	 */
	if (!weston_surface_is_mapped(ev->surface)) {
		weston_surface_map(ev->surface);
		weston_desktop_surface_set_maximized(dsurf, true);
		weston_desktop_surface_set_size(dsurf,
						ivi_output->area.width,
						ivi_output->area.height);

		weston_log("Setting app_id %s, role %s, set to maximized (%dx%d)\n",
			   app_id, ivi_layout_get_surface_role_name(surf),
			   ivi_output->area.width, ivi_output->area.height);

		surf->hidden_layer_output = ivi_output;

		weston_view_set_output(ev, ivi_output->output);
		weston_view_move_to_layer(ev, &ivi->hidden.view_list);

		weston_log("Placed app_id %s, type %s in hidden layer on output %s\n",
				app_id, ivi_layout_get_surface_role_name(surf),
				ivi_output->output->name);

		weston_compositor_schedule_repaint(ivi->compositor);
		return;
	}

	/* we might have another output to activate */
	if (surf->hidden_layer_output &&
	    surf->hidden_layer_output != ivi_output) {
		if (ivi_output->area.width != surf->hidden_layer_output->area.width ||
		    ivi_output->area.height != surf->hidden_layer_output->area.height) {
			weston_desktop_surface_set_maximized(dsurf, true);
			weston_desktop_surface_set_size(dsurf,
							ivi_output->area.width,
							ivi_output->area.height);
		}

		weston_log("Setting app_id %s, role %s, set to maximized (%dx%d)\n",
				app_id, ivi_layout_get_surface_role_name(surf),
				ivi_output->area.width, ivi_output->area.height);

		surf->hidden_layer_output = ivi_output;
		weston_view_set_output(ev, ivi_output->output);
		weston_view_move_to_layer(ev, &ivi->hidden.view_list);
		weston_log("Placed app_id %s, type %s in hidden layer on output %s\n",
				app_id, ivi_layout_get_surface_role_name(surf),
				ivi_output->output->name);
	}

	weston_compositor_schedule_repaint(ivi->compositor);
}

void
ivi_layout_remote_committed(struct ivi_surface *surf)
{
	struct weston_desktop_surface *dsurf = surf->dsurface;
	struct weston_geometry geom = weston_desktop_surface_get_geometry(dsurf);
	struct ivi_policy *policy = surf->ivi->policy;
	struct ivi_output *output;
	const char *app_id = weston_desktop_surface_get_app_id(dsurf);

	assert(surf->role == IVI_SURFACE_ROLE_REMOTE);

	output = surf->remote.output;

	if (policy && policy->api.surface_activate_by_default &&
	    !policy->api.surface_activate_by_default(surf, surf->ivi))
		return;

	/* we can only activate it again by using the protocol, but
	 * additionally the output is not reset when
	 * ivi_layout_activate_complete() terminates so we use the
	 * current active surface to avoid hitting this again and again
	 * */
	if (weston_surface_is_mapped(surf->view->surface) && output->active == surf)
		return;

	if (!surf->ivi->activate_by_default &&
	    !ivi_surf_in_hidden_layer(surf->ivi, surf)) {
		weston_log("Refusing to activate surface role %d, "
			    "app_id %s\n", surf->role, app_id);

		if (!weston_desktop_surface_get_maximized(dsurf) ||
		    geom.width != output->area.width ||
		    geom.height != output->area.height) {
			ivi_layout_add_to_hidden_layer(surf, output);
		}

		return;
	}

	if (!weston_desktop_surface_get_maximized(dsurf) ||
	    geom.width != output->area.width ||
	    geom.height != output->area.height)
		return;

	ivi_layout_activate_complete(output, surf);
}

void
ivi_layout_desktop_committed(struct ivi_surface *surf)
{
	struct weston_desktop_surface *dsurf = surf->dsurface;
	struct weston_geometry geom = weston_desktop_surface_get_geometry(dsurf);
	struct ivi_policy *policy = surf->ivi->policy;
	struct ivi_output *output;
	const char *app_id = weston_desktop_surface_get_app_id(dsurf);

	assert(surf->role == IVI_SURFACE_ROLE_DESKTOP);

	/*
	 * we can't make use here of the ivi_layout_get_output_from_surface()
	 * due to the fact that we'll always land here when a surface performs
	 * a commit and pending_output will not bet set. This works in tandem
	 * with 'mapped' at this point to avoid tripping over
	 * to a surface that continuously updates its content
	 */
	output = surf->desktop.pending_output;

	if (!output) {
		struct ivi_output *r_output;

		if (policy && policy->api.surface_activate_by_default &&
		    !policy->api.surface_activate_by_default(surf, surf->ivi))
			return;

		/* we can only activate it again by using the protocol */
		if (weston_surface_is_mapped(surf->view->surface))
			return;

		/* check first if there aren't any outputs being set */
		r_output = ivi_layout_find_with_app_id(app_id, surf->ivi);

		if (r_output) {
			struct weston_view *view = r_output->fullscreen_view.fs->view;
			if (weston_view_is_mapped(view))
				remove_black_curtain(r_output);
		}


		/* try finding an output with a background and use that */
		if (!r_output)
			r_output = ivi_layout_find_bg_output(surf->ivi);

		/* if we couldn't still find an output by this point, there's
		 * something wrong so we abort with a protocol error */
		if (!r_output) {
			wl_resource_post_error(surf->ivi->shell_client.resource,
					       AGL_SHELL_ERROR_INVALID_ARGUMENT,
					       "No valid output found to activate surface by default");
			return;
		}

		if (!surf->ivi->activate_by_default &&
		    (!surf->xwayland.is_set && !is_shell_surface_xwayland(surf))) {
			weston_log("Refusing to activate surface role %d, app_id %s, type %s\n",
					surf->role, app_id,
					is_shell_surface_xwayland(surf) ?
					"xwayland" : "regular");

			if (!weston_desktop_surface_get_maximized(dsurf) ||
			    geom.width != r_output->area.width ||
			    geom.height != r_output->area.height)
				ivi_layout_add_to_hidden_layer(surf, r_output);

			return;
		}

		/* use the output of the bg to activate the app on start-up by
		 * default */
		if (surf->view && r_output) {
			if (app_id && r_output) {
				weston_log("Surface with app_id %s, role %s activating by default\n",
					weston_desktop_surface_get_app_id(surf->dsurface),
					ivi_layout_get_surface_role_name(surf));
				ivi_layout_activate(r_output, app_id);
			} else if (!app_id) {
				/*
				 * applications not setting an app_id, or
				 * setting an app_id but at a later point in
				 * time, might fall-back here so give them a
				 * chance to receive the configure event and
				 * act upon it
				 */
				weston_log("Surface no app_id, role %s activating by default\n",
					ivi_layout_get_surface_role_name(surf));
				if (surf->xwayland.is_set || is_shell_surface_xwayland(surf)) {
					ivi_layout_activate_by_surf(r_output, surf);
					ivi_layout_activate_complete(r_output, surf);
				} else {
					ivi_layout_activate_by_surf(r_output, surf);
				}
			}
		}

		return;
	}

	if (!weston_desktop_surface_get_maximized(dsurf) ||
	    geom.width != output->area.width ||
	    geom.height != output->area.height)
		return;

	ivi_layout_activate_complete(output, surf);
}

void
ivi_layout_fullscreen_committed(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct ivi_policy *policy = ivi->policy;

	struct weston_desktop_surface *dsurface = surface->dsurface;
	struct weston_surface *wsurface =
		weston_desktop_surface_get_surface(dsurface);
	const char *app_id = weston_desktop_surface_get_app_id(dsurface);

	struct ivi_output *output = surface->split.output;
	struct weston_output *woutput = output->output;
	struct ivi_output *bg_output = ivi_layout_find_bg_output(ivi);

	struct weston_view *view = surface->view;
	struct weston_geometry geom =
		weston_desktop_surface_get_geometry(dsurface);

	struct weston_seat *wseat = get_ivi_shell_weston_first_seat(ivi);
	struct ivi_shell_seat *ivi_seat = get_ivi_shell_seat(wseat);

	bool is_fullscreen = weston_desktop_surface_get_fullscreen(dsurface);
	bool is_dim_same =
		geom.width == bg_output->output->width &&
		geom.height == bg_output->output->height;

	if (policy && policy->api.surface_activate_by_default &&
	    !policy->api.surface_activate_by_default(surface, surface->ivi))
		return;

	assert(surface->role == IVI_SURFACE_ROLE_FULLSCREEN);


	if (surface->state == FULLSCREEN && weston_view_is_mapped(view))
		return;

	/* if we still get here but we haven't resized so far, send configure
	 * events to do so */
	if (surface->state != RESIZING && (!is_fullscreen || !is_dim_same)) {
		struct ivi_output *bg_output =
			ivi_layout_find_bg_output(surface->ivi);

		weston_log("Placing fullscreen app_id %s, type %s in hidden layer\n",
				app_id, ivi_layout_get_surface_role_name(surface));
		weston_desktop_surface_set_fullscreen(dsurface, true);
		weston_desktop_surface_set_size(dsurface,
						bg_output->output->width,
						bg_output->output->height);

		surface->state = RESIZING;
		weston_view_set_output(view, output->output);
		weston_view_move_to_layer(view, &ivi->hidden.view_list);
		return;
	}

	/* eventually, we would set the surface fullscreen, but the client
	 * hasn't resized correctly by this point, so terminate connection */
	if (surface->state == RESIZING && is_fullscreen && !is_dim_same) {
		struct weston_desktop_client *desktop_client =
			weston_desktop_surface_get_client(dsurface);
		struct wl_client *client =
			weston_desktop_client_get_client(desktop_client);
		wl_client_post_implementation_error(client,
				"can not display surface due to invalid geometry."
				" Client should perform a geometry resize!");
		return;
	}

	/* this implies we resized correctly */
	if (!weston_view_is_mapped(view) || surface->state != FULLSCREEN) {
		surface->state = FULLSCREEN;

		weston_view_set_output(view, woutput);
		weston_view_set_position(view, woutput->pos);
		weston_surface_map(wsurface);
		weston_view_move_to_layer(view, &ivi->fullscreen.view_list);

		if (ivi_seat)
			ivi_shell_activate_surface(surface, ivi_seat, WESTON_ACTIVATE_FLAG_NONE);

		weston_log("Activation completed for app_id %s, role %s, "
			   "output %s\n", app_id,
			   ivi_layout_get_surface_role_name(surface),
			   output->name);

		weston_compositor_schedule_repaint(ivi->compositor);
	}
}

void
ivi_layout_desktop_resize(struct ivi_surface *surface,
			  struct weston_geometry area)
{
	struct weston_desktop_surface *dsurf = surface->dsurface;
	struct weston_view *view = surface->view;

	struct weston_coord_global pos;
	int width = area.width;
	int height = area.height;

	pos.c.x = area.x;
	pos.c.y = area.y;

	weston_desktop_surface_set_size(dsurf,
					width, height);

	weston_view_set_position(view, pos);

	weston_view_geometry_dirty(view);
	weston_surface_damage(view->surface);
}

void
ivi_layout_split_committed(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct ivi_policy *policy = ivi->policy;

	struct weston_desktop_surface *dsurface = surface->dsurface;
	struct weston_surface *wsurface =
		weston_desktop_surface_get_surface(dsurface);
	const char *app_id = weston_desktop_surface_get_app_id(dsurface);

	struct ivi_output *output = surface->split.output;
	struct weston_output *woutput = output->output;

	struct weston_seat *wseat = get_ivi_shell_weston_first_seat(ivi);
	struct ivi_shell_seat *ivi_seat = get_ivi_shell_seat(wseat);

	struct weston_view *view = surface->view;
	struct weston_geometry geom;

	struct weston_coord_global pos = woutput->pos;
	int width, height;

	if (policy && policy->api.surface_activate_by_default &&
	    !policy->api.surface_activate_by_default(surface, surface->ivi))
		return;

	if (weston_view_is_mapped(surface->view))
		return;

	geom = weston_desktop_surface_get_geometry(dsurface);

	assert(surface->role == IVI_SURFACE_ROLE_SPLIT_H ||
	       surface->role == IVI_SURFACE_ROLE_SPLIT_V);

	/* save the previous area in order to recover it back when if this kind
	 * of surface is being destroyed/removed */
	output->area_saved = output->area;

	switch (surface->role) {
	case IVI_SURFACE_ROLE_SPLIT_V:
		geom.width = (output->area.width / 2);

		pos.c.x += woutput->width - geom.width;
		output->area.width -= geom.width;

		width = woutput->width - pos.c.x;
		height = output->area.height;
		pos.c.y = output->area.y;

		break;
	case IVI_SURFACE_ROLE_SPLIT_H:
		geom.height = (output->area.height / 2);

		pos.c.y = output->area.y;
		output->area.y += geom.height;
		output->area.height -= geom.height;

		width = output->area.width;
		height = output->area.height;

		pos.c.x = output->area.x;

		break;
	default:
		assert(!"Invalid split orientation\n");
	}

	weston_desktop_surface_set_size(dsurface,
					width, height);

	/* resize the active surface first, output->area already contains
	 * correct area to resize to */
	if (output->active)
		ivi_layout_desktop_resize(output->active, output->area);

	weston_view_set_output(view, woutput);
	weston_view_set_position(view, pos);

	weston_surface_map(wsurface);
	weston_view_move_to_layer(view, &ivi->normal.view_list);

	if (ivi_seat)
		ivi_shell_activate_surface(surface, ivi_seat, WESTON_ACTIVATE_FLAG_NONE);
	weston_log("Activation completed for app_id %s, role %s, output %s\n",
			app_id, ivi_layout_get_surface_role_name(surface), output->name);
}

static void
ivi_compute_popup_position(const struct weston_output *output, struct weston_view *view,
			   int initial_x, int initial_y, double *new_x, double *new_y)
{
	*new_x = output->pos.c.x + initial_x;
	*new_y = output->pos.c.y + initial_y;
}


bool
ivi_surf_in_hidden_layer(struct ivi_compositor *ivi, struct ivi_surface *surface)
{
	struct weston_view *ev;

	wl_list_for_each(ev, &ivi->hidden.view_list.link, layer_link.link) {
		if (ev == surface->view)
			return true;
	}

	wl_list_for_each(ev, &ivi->fullscreen.view_list.link, layer_link.link) {
		if (ev == surface->view)
			return true;
	}

	return false;
}

void
ivi_layout_popup_committed(struct ivi_surface *surface)
{
	struct ivi_compositor *ivi = surface->ivi;
	struct ivi_policy *policy = ivi->policy;

	struct weston_desktop_surface *dsurface = surface->dsurface;
	struct weston_surface *wsurface =
		weston_desktop_surface_get_surface(dsurface);
	const char *app_id = weston_desktop_surface_get_app_id(dsurface);

	struct weston_coord_global pos;

	struct ivi_output *output = surface->popup.output;
	struct weston_output *woutput = output->output;

	struct weston_seat *wseat = get_ivi_shell_weston_first_seat(ivi);
	struct ivi_shell_seat *ivi_seat = get_ivi_shell_seat(wseat);

	struct weston_view *view = surface->view;

	if (policy && policy->api.surface_activate_by_default &&
	    !policy->api.surface_activate_by_default(surface, surface->ivi))
		return;

	if (weston_view_is_mapped(view) || surface->state == HIDDEN)
		return;

	assert(surface->role == IVI_SURFACE_ROLE_POPUP);

	/* remove it from hidden layer if present */
	if (ivi_surf_in_hidden_layer(ivi, surface)) {
		weston_view_move_to_layer(view, NULL);
	}

	weston_view_set_output(view, woutput);

	ivi_compute_popup_position(woutput, view,
				   surface->popup.x, surface->popup.y, &pos.c.x, &pos.c.y);
	weston_view_set_position(view, pos);
	weston_view_update_transform(view);

	/* only clip the pop-up dialog window if we have a valid
	 * width and height being passed on. Users might not want to have one
	 * set-up so only enfore it is really passed on. */
	if (surface->popup.bb.width > 0 && surface->popup.bb.height > 0)
		weston_view_set_mask(view, surface->popup.bb.x, surface->popup.bb.y,
				     surface->popup.bb.width, surface->popup.bb.height);

	weston_view_move_to_layer(view, &ivi->popup.view_list);

	if (ivi_seat && !is_assistant_overlay_app_id(app_id))
		ivi_shell_activate_surface(surface, ivi_seat, WESTON_ACTIVATE_FLAG_NONE);

	weston_surface_map(wsurface);
	weston_log("Activation completed for app_id %s, role %s, output %s\n",
			app_id, ivi_layout_get_surface_role_name(surface), output->name);
}

static void
ivi_layout_popup_re_add(struct ivi_surface *surface)
{
	assert(surface->role == IVI_SURFACE_ROLE_POPUP);
	struct weston_view *view = surface->view;

	if (weston_view_is_mapped(view)) {
		struct weston_desktop_surface *dsurface = surface->dsurface;
		struct weston_surface *wsurface =
			weston_desktop_surface_get_surface(dsurface);

		weston_view_move_to_layer(view, NULL);
		weston_surface_unmap(wsurface);
	}

	surface->state = NORMAL;
	ivi_layout_popup_committed(surface);
}

static void
ivi_layout_fullscreen_re_add(struct ivi_surface *surface)
{
	assert(surface->role == IVI_SURFACE_ROLE_FULLSCREEN);
	struct weston_view *view = surface->view;

	if (weston_view_is_mapped(view)) {
		struct weston_desktop_surface *dsurface = surface->dsurface;
		struct weston_surface *wsurface =
			weston_desktop_surface_get_surface(dsurface);

		weston_view_move_to_layer(view, NULL);
		weston_surface_unmap(wsurface);
	}

	surface->state = NORMAL;
	ivi_layout_fullscreen_committed(surface);
}

static bool
ivi_layout_surface_is_split_or_fullscreen(struct ivi_surface *surf)
{
	struct ivi_compositor *ivi = surf->ivi;
	struct ivi_surface *is;

	if (surf->role != IVI_SURFACE_ROLE_SPLIT_H &&
	    surf->role != IVI_SURFACE_ROLE_SPLIT_V &&
	    surf->role != IVI_SURFACE_ROLE_FULLSCREEN)
		return false;

	wl_list_for_each(is, &ivi->surfaces, link)
		if (is == surf)
			return true;

	return false;
}

void
ivi_layout_reset_split_surfaces(struct ivi_compositor *ivi)
{
	struct ivi_surface *ivisurf;
	struct ivi_surface *found_ivi_surf = NULL;
	bool found_split_surface = false;
	struct ivi_output *output = NULL;

	wl_list_for_each(ivisurf, &ivi->surfaces, link) {
		if (ivisurf->orientation != AGL_SHELL_TILE_ORIENTATION_NONE) {
			found_ivi_surf = ivisurf;
			found_split_surface = true;
			break;
		}
	}

	if (!found_split_surface)
		return;

	output = found_ivi_surf->current_completed_output;

	if (found_ivi_surf->sticky)
		return;

	if (output->previous_active && output->background != output->previous_active) {
		struct weston_view *ev = output->previous_active->view;
		struct weston_output *woutput = output->output;

		if (!weston_view_is_mapped(ev)) {
			weston_view_update_transform(ev);
		} else {
			weston_surface_map(ev->surface);
			weston_view_move_to_layer(ev, &ivi->normal.view_list);
		}

		weston_view_set_output(ev, woutput);

		_ivi_set_shell_surface_split(output->previous_active, NULL, 0,
					     AGL_SHELL_TILE_ORIENTATION_NONE, false, false);

		if (output->active == ivisurf) {
			output->active = output->previous_active;
		}
	}

	_ivi_set_shell_surface_split(ivisurf, NULL, 0,
				     AGL_SHELL_TILE_ORIENTATION_NONE, false, false);
}

void
ivi_layout_activate_by_surf(struct ivi_output *output, struct ivi_surface *surf)
{
	struct ivi_compositor *ivi = output->ivi;
	struct weston_desktop_surface *dsurf;
	struct weston_geometry geom;
	struct ivi_policy *policy = output->ivi->policy;

	dsurf = surf->dsurface;

	const char *app_id = weston_desktop_surface_get_app_id(dsurf);

	if (!surf)
		return;

	if (policy && policy->api.surface_activate &&
	    !policy->api.surface_activate(surf, surf->ivi)) {
		return;
	}

#ifdef AGL_COMP_DEBUG
	weston_log("Activating app_id %s, type %s, on output %s\n", app_id,
			ivi_layout_get_surface_role_name(surf), output->output->name);
#endif

	if (surf->role == IVI_SURFACE_ROLE_POPUP) {
		ivi_layout_popup_re_add(surf);
		return;
	}

	if (surf->role == IVI_SURFACE_ROLE_FULLSCREEN) {
		ivi_layout_fullscreen_re_add(surf);
		return;
	}
#if 0
	/* reset tile to desktop to allow to resize correctly */
	if (surf->role == IVI_SURFACE_ROLE_TILE && output->active == surf) {
		weston_log("%s() resetting tile role!\n", __func__);
		surf->role = IVI_SURFACE_ROLE_DESKTOP;
	}
#endif

	/* do not 're'-activate surfaces that are split or active */
	if (surf == output->active) {
		weston_log("Application %s is already active on output %s\n",
				app_id, output->output->name);
		return;
	}

	if (surf->sticky && surf->role == IVI_SURFACE_ROLE_TILE && output->active == surf) {
		weston_log("Application %s is already active on output %s (split role)\n",
				app_id, output->output->name);
		return;
	}

	if (ivi_layout_surface_is_split_or_fullscreen(surf)) {
		weston_log("Application %s is fullscreen or split on output %s\n",
				app_id, output->output->name);
		return;
	}

	// destroy any split types to allow correct re-activation
	ivi_layout_reset_split_surfaces(surf->ivi);

	if (surf->role == IVI_SURFACE_ROLE_REMOTE) {
		struct ivi_output *remote_output =
			ivi_layout_find_with_app_id(app_id, ivi);

		weston_log("Changed activation for app_id %s, type %s, on output %s\n", app_id,
				ivi_layout_get_surface_role_name(surf), output->output->name);

		/* if already active on a remote output do not
		 * attempt to activate it again */
		if (remote_output && remote_output->active == surf)
			return;
	}


	geom = weston_desktop_surface_get_geometry(dsurf);

	if (surf->role == IVI_SURFACE_ROLE_DESKTOP)
		surf->desktop.pending_output = output;
	if (weston_desktop_surface_get_maximized(dsurf) &&
	    geom.width == output->area.width &&
	    geom.height == output->area.height) {
		ivi_layout_activate_complete(output, surf);
		return;
	}

	/* the background surface is already "maximized" so we don't need to
	 * add to the hidden layer */
	if (surf->role == IVI_SURFACE_ROLE_BACKGROUND &&
	    output->active->role != IVI_SURFACE_ROLE_TILE) {
		ivi_layout_activate_complete(output, surf);
		return;
	}

	ivi_layout_add_to_hidden_layer(surf, output);
}

void
ivi_layout_activate(struct ivi_output *output, const char *app_id)
{
	struct ivi_surface *surf;
	struct ivi_compositor *ivi = output->ivi;

	if (!app_id)
		return;

	surf = ivi_find_app(ivi, app_id);
	if (!surf)
		return;

	ivi_layout_activate_by_surf(output, surf);
}

struct ivi_output *
ivi_layout_get_output_from_surface(struct ivi_surface *surf)
{
	struct ivi_output *ivi_output = NULL;

	switch (surf->role) {
	case IVI_SURFACE_ROLE_DESKTOP:
		if (surf->desktop.pending_output)
			ivi_output = surf->desktop.pending_output;
		else
			ivi_output = surf->desktop.last_output;
		break;
	case IVI_SURFACE_ROLE_POPUP:
		ivi_output = surf->popup.output;
		break;
	case IVI_SURFACE_ROLE_BACKGROUND:
		ivi_output = surf->bg.output;
		break;
	case IVI_SURFACE_ROLE_PANEL:
		ivi_output = surf->panel.output;
		break;
	case IVI_SURFACE_ROLE_FULLSCREEN:
		ivi_output = surf->fullscreen.output;
		break;
	case IVI_SURFACE_ROLE_SPLIT_H:
	case IVI_SURFACE_ROLE_SPLIT_V:
		ivi_output = surf->split.output;
		break;
	case IVI_SURFACE_ROLE_REMOTE:
		ivi_output = surf->remote.output;
		break;
	case IVI_SURFACE_ROLE_TILE:
		ivi_output = surf->current_completed_output;
		break;
	case IVI_SURFACE_ROLE_NONE:
	default:
		if (surf->view->output)
			return to_ivi_output(surf->view->output);
		break;
	}

	return ivi_output;
}

void
ivi_layout_deactivate(struct ivi_compositor *ivi, const char *app_id)
{
	struct ivi_surface *surf;
	struct ivi_output *ivi_output;
	struct ivi_policy *policy = ivi->policy;

	if (!app_id)
		return;

	surf = ivi_find_app(ivi, app_id);
	if (!surf)
		return;

	if (policy && policy->api.surface_deactivate &&
	    !policy->api.surface_deactivate(surf, surf->ivi)) {
		return;
	}

	ivi_output = ivi_layout_get_output_from_surface(surf);
	weston_log("Deactiving %s, role %s\n", app_id,
			ivi_layout_get_surface_role_name(surf));

	if (surf->role == IVI_SURFACE_ROLE_DESKTOP ||
	    surf->role == IVI_SURFACE_ROLE_REMOTE) {
		struct ivi_surface *previous_active;

		previous_active = ivi_output->previous_active;
		if (!previous_active) {
			/* we don't have a previous active it means we should
			 * display the bg */
			if (ivi_output->active) {
				struct weston_view *view;

				view = ivi_output->active->view;
				weston_view_unmap(view);
				weston_view_move_to_layer(view, NULL);
				ivi_output->active = NULL;
			}
		} else {
			struct weston_desktop_surface *dsurface;
			const char *previous_active_app_id;

			dsurface = previous_active->dsurface;
			previous_active_app_id =
				weston_desktop_surface_get_app_id(dsurface);
			ivi_layout_activate(ivi_output, previous_active_app_id);
		}
	} else if (surf->role == IVI_SURFACE_ROLE_POPUP ||
		   surf->role == IVI_SURFACE_ROLE_FULLSCREEN) {
		struct weston_view *view  = surf->view;

		surf->state = HIDDEN;
		weston_view_unmap(view);
		weston_view_move_to_layer(view, NULL);
	}

	shell_send_app_state(ivi, app_id, AGL_SHELL_APP_STATE_DEACTIVATED);
}
