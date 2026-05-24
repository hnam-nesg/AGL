#ifndef DRM_LEASE_H
#define DRM_LEASE_H

#include "config.h"

#ifdef HAVE_DRM_LEASE
#include <dlmclient.h>
#else
struct dlm_lease;
#endif	//#ifdef HAVE_DRM_LEASE
struct weston_drm_backend_config;

int setup_drm_lease(struct dlm_lease **drm_lease, struct weston_drm_backend_config *config, const char *drm_lease_name);
void cleanup_drm_lease(struct dlm_lease *drm_lease);
#endif /* DRM_LEASE_H */
