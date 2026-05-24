/*
 * Copyright © 2020 Collabora, Ltd.
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

#ifndef POLICY_H
#define POLICY_H

#include "ivi-compositor.h"

/* default state, invalid should at least be in order
 * to signal states */
#ifndef AGL_SHELL_POLICY_STATE_INVALID
#define AGL_SHELL_POLICY_STATE_INVALID 0
#endif

/* default events */
#ifndef AGL_SHELL_POLICY_EVENT_SHOW
#define AGL_SHELL_POLICY_EVENT_SHOW 0
#endif

#ifndef AGL_SHELL_POLICY_EVENT_HIDE
#define AGL_SHELL_POLICY_EVENT_HIDE 1
#endif

struct ivi_policy;

struct state_event {
	uint32_t value;
	char *name;
	struct wl_list link;	/* ivi_policy::states or ivi_policy::events */
};

struct ivi_a_policy {
	struct ivi_policy *policy;

	char *app_id;
	uint32_t state;
	uint32_t event;
	uint32_t timeout;
	struct ivi_output *output;
	struct wl_event_source *timer;	/* for policies that have a timeout */

	struct wl_list link;	/* ivi_policy::ivi_policies */
};

struct ivi_policy_api {
	size_t struct_size;

	bool (*surface_create)(struct ivi_surface *surf, void *user_data);
	bool (*surface_commited)(struct ivi_surface *surf, void *user_data);
	bool (*surface_activate)(struct ivi_surface *surf, void *user_data);
	bool (*surface_deactivate)(struct ivi_surface *surf, void *user_data);

	bool (*surface_activate_by_default)(struct ivi_surface *surf, void *user_data);
	bool (*surface_advertise_state_change)(struct ivi_surface *surf, void *user_data);

	bool (*shell_bind_interface)(void *client, void *interface);

	/** see also ivi_policy_add(). If set this will be executed before
	 * adding a new policy rule  */
	bool (*policy_rule_allow_to_add)(void *user_data);

	/** this callback will be executed when a there's a policy state change */
	void (*policy_rule_try_event)(struct ivi_a_policy *a_policy);
};

struct ivi_policy {
	struct ivi_compositor *ivi;
	/* user-defined hooks */
	struct ivi_policy_api api;
	void *user_data;

	/* represents the policy rules */
	struct wl_list policies;	/* ivi_a_policy::link */

	/* no state update chnage is being done as long as we have the same
	 * state */
	uint32_t current_state;
	uint32_t previous_state;

	/* guards against current in change in progress */
	bool state_change_in_progress;

	/* additional states which can be verified in
	 * ivi_policy_api::policy_rule_try_event() */
	struct wl_list states;	/* state_event::link */
	struct wl_list events;	/* state_event::link */

	/* necessary to for signaling the state change */
	struct wl_listener listener_check_policies;
	struct wl_signal signal_state_change;
};


/** Initialize the policy setup
 *
 * Policy engine should call ivi_policy_create() with its own ivi_policy_api
 * setup.
 */
struct ivi_policy *
ivi_policy_create(struct ivi_compositor *compositor,
                  const struct ivi_policy_api *api, void *user_data);

/** Destroys the policy setup
 *
 */
void
ivi_policy_destroy(struct ivi_policy *ivi_policy);

/** Add a policy rule.
 *
 * ivi_policy_api::policy_rule_allow_to_add() can be used to limit adding
 * policy rules.
 *
 * Returns 0 in case of success, or -1 in case of failure.
 *
 */
int
ivi_policy_add(struct ivi_policy *policy, const char *app_id, uint32_t state,
	       uint32_t event, uint32_t timeout, struct wl_resource *output_res);

/** Trigger a state change. This should be called **each time** there is a need
 * to apply the policy rules.
 *
 * Returns 0 in case of success, or -1 in case of failure.
 */
int
ivi_policy_state_change(struct ivi_policy *policy, uint32_t state);


/** Add a new state. The state can be verified in ivi_policy_api::policy_rule_try_event()
 *
 */
void
ivi_policy_add_state(struct ivi_policy *policy, uint32_t state, const char *value);

/** Add a new event. The event can be verified in ivi_policy_api::policy_rule_try_event()
 *
 */
void
ivi_policy_add_event(struct ivi_policy *policy, uint32_t state, const char *value);

/** Initialize the policy. Not implemented.
 *
 * Should be implemented by the policy engine. A single policy engine can be used
 * at one time.
 *
 * Returns 0 in case of success, or -1 in case of failure.
 */
int
ivi_policy_init(struct ivi_compositor *ivi);

#endif
