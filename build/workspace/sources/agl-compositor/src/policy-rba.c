/*
 * Copyright © 2020 Collabora, Ltd.
 * Copyright (c) 2020 DENSO CORPORATION.
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
#include "rba_adapter.h"

#include <string.h>

static bool
ivi_policy_rba_surface_create(struct ivi_surface *surf, void *user_data)
{
	return true;
}

static bool
ivi_policy_rba_surface_commmited(struct ivi_surface *surf, void *user_data)
{
	return true;
}

static bool
ivi_policy_rba_surface_activate(struct ivi_surface *surf, void *user_data)
{
	const char *app_id = NULL;
	app_id = weston_desktop_surface_get_app_id(surf->dsurface);
	if (app_id == NULL) {
		weston_log("app_id is NULL, surface activation failed.\n");
		return false;
	}
	return rba_adapter_arbitrate(app_id,surf->ivi);
}

static bool
ivi_policy_rba_surface_deactivate(struct ivi_surface *surf, void *user_data)
{
	return true;
}

static bool
ivi_policy_rba_surface_activate_default(struct ivi_surface *surf, void *user_data)
{
	return true;
}

static bool
ivi_policy_rba_surface_advertise_state_change(struct ivi_surface *surf, void *user_data)
{
	return true;
}

static bool
ivi_policy_rba_shell_bind_interface(void *client, void *interface)
{
	return rba_adapter_initialize();
}

static const struct ivi_policy_api policy_api = {
	.struct_size = sizeof(policy_api),
	.surface_create = ivi_policy_rba_surface_create,
	.surface_commited = ivi_policy_rba_surface_commmited,
	.surface_activate = ivi_policy_rba_surface_activate,
	.surface_deactivate = ivi_policy_rba_surface_deactivate,
	.surface_activate_by_default = ivi_policy_rba_surface_activate_default,
	.surface_advertise_state_change = ivi_policy_rba_surface_advertise_state_change,
	.shell_bind_interface = ivi_policy_rba_shell_bind_interface,
	.policy_rule_allow_to_add = NULL,
	.policy_rule_try_event = NULL,
};

int
ivi_policy_init(struct ivi_compositor *ivi)
{
	ivi->policy = ivi_policy_create(ivi, &policy_api, ivi);
	if (!ivi->policy)
		return -1;

	weston_log("Installing 'rba(Rule Base Arbitration)' policy engine\n");
	return 0;
}
