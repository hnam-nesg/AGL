/*
 * Copyright © 2022 IGEL Co., Ltd.
 * Copyright © 2024 AISIN CORPORATION
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

#include "drm-lease.h"

#include <libweston/libweston.h>
#include <libweston/backend-drm.h>

#ifdef HAVE_DRM_LEASE
int setup_drm_lease(struct dlm_lease **drm_lease, struct weston_drm_backend_config *config, const char *drm_lease_name)
{
	if (drm_lease_name == NULL) {
		// drm lease option is not set. That is disabled.
		config->device_fd = -1;
		return 0;
	}

	int drm_fd = -1;
	struct dlm_lease *lease = dlm_get_lease(drm_lease_name);
	if (lease) {
		drm_fd = dlm_lease_fd(lease);
		if (drm_fd < 0)
		        dlm_release_lease(lease);
	}

	if (drm_fd < 0) {
		weston_log("Could not get DRM lease %s\n", drm_lease_name);
		cleanup_drm_lease(lease);
		config->device_fd = -1;
		return -1;
	}

	*drm_lease = lease;
	config->device_fd = drm_fd;

	return 0;
}

void cleanup_drm_lease(struct dlm_lease *lease)
{
	if (lease)
		dlm_release_lease(lease);
}
#else	//#ifdef HAVE_DRM_LEASE
int setup_drm_lease(struct dlm_lease **drm_lease, struct weston_drm_backend_config *config, const char *drm_lease_name)
{
	if (drm_lease_name != NULL) {
		weston_log("Set drm lease option but drm lease feature is disabled.\n");
		return -1;
	}

	return 0;
}

void cleanup_drm_lease(struct dlm_lease *lease)
{
	return;
}
#endif	//#ifdef HAVE_DRM_LEASE
