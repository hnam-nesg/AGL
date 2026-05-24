/*
 * Copyright © 2022 Collabora, Ltd.
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
#pragma once

#include <cstdio>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <wayland-client.h>

#define GRPC_CALLBACK_API_NONEXPERIMENTAL

#include "agl_shell.grpc.pb.h"

// forward declaration created in grpc-async-cb
class Lister;

struct global_data {
	uint32_t id;
	uint32_t version;
	std::string interface_name;
};

struct shell_data {
	struct wl_display *wl_display;
	struct agl_shell *shell;
	struct agl_shell_ext *shell_ext;

	bool wait_for_bound;
	bool wait_for_doas;

	bool bound_ok;
	bool bound_fail;
	bool doas_ok;

	uint32_t version;
	struct wl_list output_list;     /** window_output::link */

	::agl_shell_ipc::AppStateResponse current_app_state;
	std::list<std::pair<grpc::CallbackServerContext*, Lister *> > server_context_list;
	std::list<global_data> globals;
};

struct window_output {
	struct shell_data *shell_data;
	struct wl_output *output;
	char *name;
	struct wl_list link;    /** display::output_list */
};
