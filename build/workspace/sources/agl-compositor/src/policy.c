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

#include <string.h>
#include <libweston/zalloc.h>
#include <assert.h>

#include "shared/helpers.h"
#include "ivi-compositor.h"

#include "policy.h"

static void
ivi_policy_remove_state_event(struct state_event *st_ev)
{
	free(st_ev->name);
	wl_list_remove(&st_ev->link);
	free(st_ev);
}

static void
ivi_policy_destroy_state_event(struct wl_list *list)
{
	struct state_event *st_ev, *tmp_st_ev;
	wl_list_for_each_safe(st_ev, tmp_st_ev, list, link)
		ivi_policy_remove_state_event(st_ev);
}

static struct state_event *
ivi_policy_state_event_create(uint32_t val, const char *value)
{
	struct state_event *ev_st = zalloc(sizeof(*ev_st));
	size_t value_len = strlen(value);

	if (!ev_st)
		return NULL;
	ev_st->value = val;
	ev_st->name = zalloc(sizeof(char) * value_len + 1);
	if (!ev_st->name) {
		free(ev_st);
		return NULL;
	}
	memcpy(ev_st->name, value, value_len);

	return ev_st;
}

void
ivi_policy_add_state(struct ivi_policy *policy, uint32_t state, const char *value)
{
	struct state_event *ev_st;
	if (!policy)
		return;

	ev_st = ivi_policy_state_event_create(state, value);
	if (!ev_st)
		return;
	wl_list_insert(&policy->states, &ev_st->link);
}

void
ivi_policy_add_event(struct ivi_policy *policy, uint32_t ev, const char *value)
{
	struct state_event *ev_st;
	if (!policy)
		return;

	ev_st = ivi_policy_state_event_create(ev, value);
	if (!ev_st)
		return;
	wl_list_insert(&policy->events, &ev_st->link);
}

static void
ivi_policy_add_default_states(struct ivi_policy *policy)
{
	const char *default_states[] = { "invalid", "start", "stop", "reverse" };
	if (!policy)
		return;

	for (uint32_t i = 0; i < ARRAY_LENGTH(default_states); i ++) {
		struct state_event *ev_st =
			ivi_policy_state_event_create(i, default_states[i]);
		if (!ev_st)
			return;
		wl_list_insert(&policy->states, &ev_st->link);
	}
}

static void
ivi_policy_add_default_events(struct ivi_policy *policy)
{
	const char *default_events[] = { "show", "hide" };
	if (!policy)
		return;

	for (uint32_t i = 0; i < ARRAY_LENGTH(default_events); i ++) {
		struct state_event *ev_st =
			ivi_policy_state_event_create(i, default_events[i]);
		if (!ev_st)
			return;
		wl_list_insert(&policy->events, &ev_st->link);
	}
}

static void
ivi_policy_try_event(struct ivi_a_policy *a_policy)
{
	struct ivi_policy *policy = a_policy->policy;

	if (policy->api.policy_rule_try_event)
	    return policy->api.policy_rule_try_event(a_policy);
}

static int
ivi_policy_try_event_timeout(void *user_data)
{
	struct ivi_a_policy *a_policy = user_data;
	ivi_policy_try_event(a_policy);
	return 0;
}

static void
ivi_policy_setup_event_timeout(struct ivi_policy *ivi_policy,
			       struct ivi_a_policy *a_policy)
{
	struct ivi_compositor *ivi = ivi_policy->ivi;
	struct wl_display *wl_display = ivi->compositor->wl_display;
	struct wl_event_loop *loop = wl_display_get_event_loop(wl_display);

	a_policy->timer = wl_event_loop_add_timer(loop,
						  ivi_policy_try_event_timeout,
						  a_policy);

	wl_event_source_timer_update(a_policy->timer, a_policy->timeout);
}

static void
ivi_policy_check_policies(struct wl_listener *listener, void *data)
{
	struct ivi_a_policy *a_policy;
	struct ivi_policy *ivi_policy =
		wl_container_of(listener, ivi_policy, listener_check_policies);

	ivi_policy->state_change_in_progress = true;
	wl_list_for_each(a_policy, &ivi_policy->policies, link) {
		if (ivi_policy->current_state == a_policy->state) {
			/* check the timeout first to see if there's a timeout */
			if (a_policy->timeout > 0)
				ivi_policy_setup_event_timeout(ivi_policy,
							       a_policy);
			else
				ivi_policy_try_event(a_policy);
		}
	}

	ivi_policy->previous_state = ivi_policy->current_state;
	ivi_policy->state_change_in_progress = false;
}


struct ivi_policy *
ivi_policy_create(struct ivi_compositor *ivi,
                  const struct ivi_policy_api *api, void *user_data)
{
	struct ivi_policy *policy = zalloc(sizeof(*policy));

	if (!policy)
		return NULL;
	policy->user_data = user_data;
	policy->ivi = ivi;
	policy->state_change_in_progress = false;

	policy->api.struct_size =
		MIN(sizeof(struct ivi_policy_api), api->struct_size);
	/* install the hooks */
	memcpy(&policy->api, api, policy->api.struct_size);

	/* to trigger a check for policies use */
	wl_signal_init(&policy->signal_state_change);

	policy->listener_check_policies.notify = ivi_policy_check_policies;
	wl_signal_add(&policy->signal_state_change,
		      &policy->listener_check_policies);

	policy->current_state = AGL_SHELL_POLICY_STATE_INVALID;
	policy->previous_state = AGL_SHELL_POLICY_STATE_INVALID;

	/* policy rules */
	wl_list_init(&policy->policies);

	wl_list_init(&policy->events);
	wl_list_init(&policy->states);

	/* add the default states and enums */
	ivi_policy_add_default_states(policy);
	ivi_policy_add_default_events(policy);

	return policy;
}

void
ivi_policy_destroy(struct ivi_policy *ivi_policy)
{
	struct ivi_a_policy *a_policy, *a_policy_tmp;

	if (!ivi_policy)
		return;

	wl_list_for_each_safe(a_policy, a_policy_tmp,
			      &ivi_policy->policies, link) {
		free(a_policy->app_id);
		wl_list_remove(&a_policy->link);
		free(a_policy);
	}

	ivi_policy_destroy_state_event(&ivi_policy->states);
	ivi_policy_destroy_state_event(&ivi_policy->events);

	free(ivi_policy);
}


/* verifies if the state is one has been added */
static bool
ivi_policy_state_is_known(uint32_t state, struct ivi_policy *policy)
{
	struct state_event *ev_st;

	wl_list_for_each(ev_st, &policy->states, link) {
		if (ev_st->value == state) {
			return true;
		}
	}
	return false;
}

/*
 * The generic way would be the following:
 *
 * - 'car' is in 'state' ->
 *   	{ do 'event' for app 'app_id' at 'timeout' time if same state as 'car_state' }
 *
 * a 0 timeout means, immediately, a timeout > 0, means to install timer an
 * execute when timeout expires
 *
 * The following happens:
 * 'car' changes its state -> verify what policy needs to be run
 * 'car' in same state -> no action
 *
 */
int
ivi_policy_add(struct ivi_policy *policy, const char *app_id, uint32_t state,
	       uint32_t event, uint32_t timeout, struct wl_resource *output_res)
{
	size_t app_id_len;
	struct weston_head *head = weston_head_from_resource(output_res);
	struct weston_output *woutput = weston_head_get_output(head);
	struct ivi_output *output = to_ivi_output(woutput);
	struct ivi_a_policy *a_policy;

	if (!policy) {
		weston_log("Failed to retrieve policy!\n");
		return -1;
	}

	if (policy->state_change_in_progress)
		return -1;

	/* we should be allow to do this in the first place, only if the
	 * hooks allows us to  */
	if (policy->api.policy_rule_allow_to_add &&
	    !policy->api.policy_rule_allow_to_add(policy))
		return -1;

	if (!ivi_policy_state_is_known(state, policy))
		return -1;

	a_policy = zalloc(sizeof(*a_policy));
	if (!a_policy)
		return -1;

	app_id_len = strlen(app_id);
	a_policy->app_id = zalloc(sizeof(char) * app_id_len + 1);
	if (!a_policy->app_id) {
		free(a_policy);
		return -1;
	}
	memcpy(a_policy->app_id, app_id, app_id_len);

	a_policy->state = state;
	a_policy->event = event;
	a_policy->timeout = timeout;
	a_policy->output = output;
	a_policy->policy = policy;

	wl_list_insert(&policy->policies, &a_policy->link);

	return 0;
}

/* we start with 'invalid' state, so a initial state to even 'stop' should
 * trigger a check of policies
 */
int
ivi_policy_state_change(struct ivi_policy *policy, uint32_t state)
{
	bool found_state = false;
	if (!policy) {
		weston_log("Failed to retrieve policy!\n");
		return -1;
	}

	if (policy->current_state == state) {
		return -1;
	}

	/* if we don't know the state, make sure it is first added */
	found_state = ivi_policy_state_is_known(state, policy);
	if (!found_state) {
		return -1;
	}

	/* current_state is actually the new state */
	policy->current_state = state;

	/* signal that we need to check the current policies */
	wl_signal_emit(&policy->signal_state_change, policy);

	return 0;
}
