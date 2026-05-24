/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2016 Giulio Camuffo
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

#include "config.h"
#include "ivi-compositor.h"

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <libweston/libweston.h>
#include <weston.h>
#include <libweston/xwayland-api.h>
#include "shared/helpers.h"
#include "shared/process-util.h"
#include "shared/xalloc.h"

#ifdef HAVE_XWAYLAND_LISTENFD
#  define LISTEN_STR "-listenfd"
#else
#  define LISTEN_STR "-listen"
#endif

struct wet_xwayland {
	struct weston_compositor *compositor;
	const struct weston_xwayland_api *api;
	struct weston_xwayland *xwayland;
	struct wl_event_source *display_fd_source;
	int wm_fd;
	struct wet_process *process;
};

static int
handle_display_fd(int fd, uint32_t mask, void *data)
{
	struct wet_xwayland *wxw = data;
	char buf[64];
	ssize_t n;

	/* xwayland exited before being ready, don't finish initialization,
	 * the process watcher will cleanup */
	if (!(mask & WL_EVENT_READABLE))
		goto out;

	/* Xwayland writes to the pipe twice, so if we close it too early
	 * it's possible the second write will fail and Xwayland shuts down.
	 * Make sure we read until end of line marker to avoid this. */
	n = read(fd, buf, sizeof buf);
	if (n < 0 && errno != EAGAIN) {
		weston_log("read from Xwayland display_fd failed: %s\n",
				strerror(errno));
		goto out;
	}
	/* Returning 1 here means recheck and call us again if required. */
	if (n <= 0 || (n > 0 && buf[n - 1] != '\n'))
		return 1;

	wxw->api->xserver_loaded(wxw->xwayland, wxw->wm_fd);

out:
	wl_event_source_remove(wxw->display_fd_source);
	close(fd);

	return 0;
}


static void
xserver_cleanup(struct wet_process *process, int status, void *data)
{
	struct wet_xwayland *wxw = data;

	/* We only have one Xwayland process active, so make sure it's the
	 * right one */
	assert(process == wxw->process);

	wxw->api->xserver_exited(wxw->xwayland);
	wxw->process = NULL;
}
static void
cleanup_for_child_process() {
	sigset_t allsigs;

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
}

static struct wet_process *
client_launch(struct weston_compositor *compositor,
		  struct custom_env *child_env,
		  int *no_cloexec_fds,
		  size_t num_no_cloexec_fds,
		  wet_process_cleanup_func_t cleanup,
		  void *cleanup_data)
{
	struct ivi_compositor *ivi = to_ivi_compositor(compositor);
	struct wet_process *proc = NULL;
	const char *fail_cloexec = "Couldn't unset CLOEXEC on child FDs";
	const char *fail_seteuid = "Couldn't call seteuid";
	char *fail_exec;
	char * const *argp;
	char * const *envp;
	pid_t pid;
	int err;
	size_t i;
	size_t written __attribute__((unused));

	argp = custom_env_get_argp(child_env);
	envp = custom_env_get_envp(child_env);

	weston_log("launching '%s'\n", argp[0]);
	str_printf(&fail_exec, "Error: Couldn't launch client '%s'\n", argp[0]);

	pid = fork();
	switch (pid) {
	case 0:
		cleanup_for_child_process();

		/* Launch clients as the user. Do not launch clients with wrong euid. */
		if (seteuid(getuid()) == -1) {
			written = write(STDERR_FILENO, fail_seteuid,
					strlen(fail_seteuid));
			_exit(EXIT_FAILURE);
		}

		for (i = 0; i < num_no_cloexec_fds; i++) {
			err = os_fd_clear_cloexec(no_cloexec_fds[i]);
			if (err < 0) {
				written = write(STDERR_FILENO, fail_cloexec,
						strlen(fail_cloexec));
				_exit(EXIT_FAILURE);
			}
		}

		execve(argp[0], argp, envp);

		if (fail_exec)
			written = write(STDERR_FILENO, fail_exec,
					strlen(fail_exec));
		_exit(EXIT_FAILURE);

	default:
		proc = xzalloc(sizeof(*proc));
		proc->pid = pid;
		proc->cleanup = cleanup;
		proc->cleanup_data = cleanup_data;
		proc->path = strdup(argp[0]);
		wl_list_insert(&ivi->child_process_list, &proc->link);
		break;

	case -1:
		weston_log("weston_client_launch: "
			   "fork failed while launching '%s': %s\n", argp[0],
			   strerror(errno));
		break;
	}

	custom_env_fini(child_env);
	free(fail_exec);
	return proc;
}

static struct weston_config *
ivi_get_config(struct weston_compositor *ec)
{
        struct ivi_compositor *ivi = to_ivi_compositor(ec);

        return ivi->config;
}


static struct wl_client *
spawn_xserver(void *user_data, const char *display, int abstract_fd, int unix_fd)
{
	struct wet_xwayland *wxw = user_data;
	struct fdstr wayland_socket = FDSTR_INIT;
	struct fdstr x11_abstract_socket = FDSTR_INIT;
	struct fdstr x11_unix_socket = FDSTR_INIT;
	struct fdstr x11_wm_socket = FDSTR_INIT;
	struct fdstr display_pipe = FDSTR_INIT;

	char *xserver = NULL;
	struct weston_config *config = ivi_get_config(wxw->compositor);
	struct weston_config_section *section;
	struct wl_client *client;
	struct wl_event_loop *loop;
	struct custom_env child_env;
	int no_cloexec_fds[5];
	size_t num_no_cloexec_fds = 0;
	size_t written __attribute__ ((unused));

	if (os_socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0, wayland_socket.fds) < 0) {
		weston_log("wl connection socketpair failed\n");
		goto err;
	}
	fdstr_update_str1(&wayland_socket);
	no_cloexec_fds[num_no_cloexec_fds++] = wayland_socket.fds[1];

	if (os_socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0, x11_wm_socket.fds) < 0) {
		weston_log("X wm connection socketpair failed\n");
		goto err;
	}
	fdstr_update_str1(&x11_wm_socket);
	no_cloexec_fds[num_no_cloexec_fds++] = x11_wm_socket.fds[1];

	if (pipe2(display_pipe.fds, O_CLOEXEC) < 0) {
		weston_log("pipe creation for displayfd failed\n");
		goto err;
	}
	fdstr_update_str1(&display_pipe);
	no_cloexec_fds[num_no_cloexec_fds++] = display_pipe.fds[1];

	fdstr_set_fd1(&x11_abstract_socket, abstract_fd);
	no_cloexec_fds[num_no_cloexec_fds++] = abstract_fd;

	fdstr_set_fd1(&x11_unix_socket, unix_fd);
	no_cloexec_fds[num_no_cloexec_fds++] = unix_fd;

	assert(num_no_cloexec_fds <= ARRAY_LENGTH(no_cloexec_fds));

	section = weston_config_get_section(config, "xwayland", NULL, NULL);
	weston_config_section_get_string(section, "path",
					 &xserver, XSERVER_PATH);
	custom_env_init_from_environ(&child_env);
	custom_env_set_env_var(&child_env, "WAYLAND_SOCKET", wayland_socket.str1);

	custom_env_add_arg(&child_env, xserver);
	custom_env_add_arg(&child_env, display);
	custom_env_add_arg(&child_env, "-rootless");
	custom_env_add_arg(&child_env, LISTEN_STR);
	custom_env_add_arg(&child_env, x11_abstract_socket.str1);
	custom_env_add_arg(&child_env, LISTEN_STR);
	custom_env_add_arg(&child_env, x11_unix_socket.str1);
	custom_env_add_arg(&child_env, "-displayfd");
	custom_env_add_arg(&child_env, display_pipe.str1);
	custom_env_add_arg(&child_env, "-wm");
	custom_env_add_arg(&child_env, x11_wm_socket.str1);
	custom_env_add_arg(&child_env, "-terminate");

	wxw->process = client_launch(wxw->compositor, &child_env,
				     no_cloexec_fds, num_no_cloexec_fds,
				     xserver_cleanup, wxw);
	if (!wxw->process) {
		weston_log("Couldn't start Xwayland\n");
		goto err;
	}

	client = wl_client_create(wxw->compositor->wl_display,
				  wayland_socket.fds[0]);
	if (!client) {
		weston_log("Couldn't create client for Xwayland\n");
		goto err_proc;
	}

	wxw->wm_fd = x11_wm_socket.fds[0];

	/* Now we can no longer fail, close the child end of our sockets */
	close(wayland_socket.fds[1]);
	close(x11_wm_socket.fds[1]);
	close(display_pipe.fds[1]);

	/* During initialization the X server will round trip
	 * and block on the wayland compositor, so avoid making
	 * blocking requests (like xcb_connect_to_fd) until
	 * it's done with that. */
	loop = wl_display_get_event_loop(wxw->compositor->wl_display);
	wxw->display_fd_source =
		wl_event_loop_add_fd(loop, display_pipe.fds[0],
				     WL_EVENT_READABLE,
				     handle_display_fd, wxw);

	free(xserver);

	return client;


err_proc:
	wl_list_remove(&wxw->process->link);
err:
	free(xserver);
	fdstr_close_all(&display_pipe);
	fdstr_close_all(&x11_wm_socket);
	fdstr_close_all(&wayland_socket);
	free(wxw->process);
	return NULL;
}

void
wet_xwayland_destroy(struct weston_compositor *compositor, void *data)
{
	struct wet_xwayland *wxw = data;

	/* Calling this will call the process cleanup, in turn cleaning up the
	 * client and the core Xwayland state */
	if (wxw->process)
		ivi_process_destroy(wxw->process, 0, true);

	free(wxw);
}

void *
wet_load_xwayland(struct weston_compositor *compositor)
{
	const struct weston_xwayland_api *api;
	struct weston_xwayland *xwayland;
	struct wet_xwayland *wxw;

	if (weston_compositor_load_xwayland(compositor) < 0)
		return NULL;

	api = weston_xwayland_get_api(compositor);
	if (!api) {
		weston_log("Failed to get the xwayland module API.\n");
		return NULL;
	}

	xwayland = api->get(compositor);
	if (!xwayland) {
		weston_log("Failed to get the xwayland object.\n");
		return NULL;
	}

	wxw = zalloc(sizeof *wxw);
	if (!wxw)
		return NULL;

	wxw->compositor = compositor;
	wxw->api = api;
	wxw->xwayland = xwayland;
	if (api->listen(xwayland, wxw, spawn_xserver) < 0)
		return NULL;

	return wxw;
}
