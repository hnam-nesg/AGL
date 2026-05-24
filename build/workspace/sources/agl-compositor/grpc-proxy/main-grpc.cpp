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

#include <cstdio>
#include <ctime>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

#include "shell.h"
#include "log.h"
#include "main-grpc.h"
#include "grpc-async-cb.h"

using namespace std::chrono_literals;

static int running = 1;

static void
agl_shell_bound_ok(void *data, struct agl_shell *agl_shell)
{
	(void) agl_shell;

	struct shell_data *sh = static_cast<struct shell_data *>(data);
	sh->wait_for_bound = false;

	LOG("bound_ok event!\n");
	sh->bound_ok = true;
}

static void
agl_shell_bound_fail(void *data, struct agl_shell *agl_shell)
{
	(void) agl_shell;

	struct shell_data *sh = static_cast<struct shell_data *>(data);
	sh->wait_for_bound = false;

	LOG("bound_fail event!\n");
	sh->bound_ok = false;
	sh->bound_fail = true;
}

static void
agl_shell_app_state(void *data, struct agl_shell *agl_shell,
		const char *app_id, uint32_t state)
{
	(void) agl_shell;
	struct shell_data *sh = static_cast<struct shell_data *>(data);
	LOG("got app_state event app_id %s,  state %d\n", app_id, state);

	if (sh->server_context_list.empty())
		return;

	::agl_shell_ipc::AppStateResponse app;

	sh->current_app_state.set_app_id(std::string(app_id));
	sh->current_app_state.set_state(state);

	auto start = sh->server_context_list.begin();
	while (start != sh->server_context_list.end()) {
		// hold on if we're still detecting another in-flight writting
		if (start->second->Writting()) {
			LOG("skip writing to lister %p\n", static_cast<void *>(start->second));
			continue;
		}

		LOG("writing to lister %p\n", static_cast<void *>(start->second));
		start->second->NextWrite();
		start++;
	}
}

static void
agl_shell_app_on_output(void *data, struct agl_shell *agl_shell,
		const char *app_id, const char *output_name)
{
	(void) agl_shell;
	(void) output_name;
	(void) data;
	(void) app_id;

	LOG("got app_on_output event app_id %s on output %s\n", app_id, output_name);
}


static const struct agl_shell_listener shell_listener = {
	agl_shell_bound_ok,
	agl_shell_bound_fail,
	agl_shell_app_state,
	agl_shell_app_on_output,
};

static void
agl_shell_ext_doas_done(void *data, struct agl_shell_ext *agl_shell_ext, uint32_t status)
{
	(void) agl_shell_ext;

	struct shell_data *sh = static_cast<struct shell_data *>(data);
	sh->wait_for_doas = false;

	if (status == AGL_SHELL_EXT_DOAS_SHELL_CLIENT_STATUS_SUCCESS) {
		LOG("got doas_ok true!\n");
		sh->doas_ok = true;
	}
}

static const struct agl_shell_ext_listener shell_ext_listener = {
        agl_shell_ext_doas_done,
};

static void
display_handle_geometry(void *data, struct wl_output *wl_output,
		int x, int y, int physical_width, int physical_height,
		int subpixel, const char *make, const char *model, int transform)
{
	(void) data;
	(void) wl_output;
	(void) x;
	(void) y;
	(void) physical_width;
	(void) physical_height;
	(void) subpixel;
	(void) make;
	(void) model;
	(void) transform;
}

static void
display_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		int width, int height, int refresh)
{
	(void) data;
	(void) wl_output;
	(void) flags;
	(void) width;
	(void) height;
	(void) refresh;
}

static void
display_handle_done(void *data, struct wl_output *wl_output)
{
	(void) data;
	(void) wl_output;
}

static void
display_handle_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
	(void) data;
	(void) wl_output;
	(void) factor;
}


static void
display_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
	(void) wl_output;

	struct window_output *woutput = static_cast<struct window_output *>(data);
	woutput->name = strdup(name);
}

static void
display_handle_description(void *data, struct wl_output *wl_output, const char *description)
{
	(void) data;
	(void) wl_output;
	(void) description;
}

static const struct wl_output_listener output_listener = {
	display_handle_geometry,
	display_handle_mode,
	display_handle_done,
	display_handle_scale,
	display_handle_name,
	display_handle_description,
};

static void
display_add_output(struct shell_data *sh, struct wl_registry *reg,
		   uint32_t id, uint32_t version)
{
	struct window_output *w_output;

	w_output = new struct window_output;
	w_output->shell_data = sh;

	w_output->output =
		static_cast<struct wl_output *>(wl_registry_bind(reg, id,
				&wl_output_interface,
				std::min(version, static_cast<uint32_t>(4))));

	wl_list_insert(&sh->output_list, &w_output->link);
	wl_output_add_listener(w_output->output, &output_listener, w_output);
}

static void
destroy_output(struct window_output *w_output)
{
	free(w_output->name);
	wl_list_remove(&w_output->link);
	free(w_output);
}

static void
global_add(void *data, struct wl_registry *reg, uint32_t id,
		const char *interface, uint32_t version)
{

	struct shell_data *sh = static_cast<struct shell_data *>(data);

	if (!sh)
		return;

	struct global_data gb;

	gb.version = version;
	gb.id = id;
	gb.interface_name = std::string(interface);
	sh->globals.push_back(gb);

	if (strcmp(interface, agl_shell_interface.name) == 0) {
		// nothing here, we're just going to bind a bit later after we
		// got doas_ok event
	} else if (strcmp(interface, "wl_output") == 0) {
                display_add_output(sh, reg, id, version);
        } else if (strcmp(interface, agl_shell_ext_interface.name) == 0) {
		sh->shell_ext =
			static_cast<struct agl_shell_ext *>(wl_registry_bind(reg, id,
				&agl_shell_ext_interface,
				std::min(static_cast<uint32_t>(1), version)));
		agl_shell_ext_add_listener(sh->shell_ext,
					   &shell_ext_listener, data);
	}
}

static void
global_remove(void *data, struct wl_registry *reg, uint32_t id)
{
	struct shell_data *sh = static_cast<struct shell_data *>(data);
	/* Don't care */
	(void) reg;
	(void) id;

	for (std::list<global_data>::iterator it = sh->globals.begin();
			it != sh->globals.end(); it = sh->globals.erase(it))
		;
}

static const struct wl_registry_listener registry_listener = {
	global_add,
	global_remove,
};


// we expect this client to be up & running *after* the shell client has
// already set-up panels/backgrounds.
//
// this means we need to wait for doas_done event with doas_shell_client_status
// set to sucess.
struct shell_data *
register_shell_ext(void)
{
	// try first to bind to agl_shell_ext
	int ret = 0;
	struct wl_registry *registry;

	struct shell_data *sh = new struct shell_data;

	sh->wl_display = wl_display_connect(NULL);
	if (!sh->wl_display) {
		goto err;
	}

	registry = wl_display_get_registry(sh->wl_display);

	sh->wait_for_bound = true;
	sh->wait_for_doas = true;

	sh->bound_fail = false;
	sh->bound_ok = false;

	sh->doas_ok = false;
	wl_list_init(&sh->output_list);

	wl_registry_add_listener(registry, &registry_listener, sh);
	wl_display_roundtrip(sh->wl_display);

	if (!sh->shell_ext) {
		LOG("agl_shell_ext interface was not found!\n");
		goto err;
	}

	do {
		// this should loop until we get back an doas_ok event
		agl_shell_ext_doas_shell_client(sh->shell_ext);

		while (ret !=- 1 && sh->wait_for_doas) {
			ret = wl_display_dispatch(sh->wl_display);

			if (sh->wait_for_doas)
				continue;
		}

		if (sh->doas_ok) {
			break;
		}

		std::this_thread::sleep_for(250ms);
		sh->wait_for_doas = true;
	} while (!sh->doas_ok);

	if (!sh->doas_ok) {
		LOG("agl_shell_ext: failed to get doas_ok status\n");
		goto err;
	}


	// search for the globals to get id and version
	for (std::list<global_data>::iterator it = sh->globals.begin();
			it != sh->globals.end(); it++) {
		if (it->interface_name == "agl_shell") {
			sh->shell =
				static_cast<struct agl_shell *>(wl_registry_bind(registry, it->id,
					&agl_shell_interface, std::min(static_cast<uint32_t>(11),
						it->version))
				);
			agl_shell_add_listener(sh->shell, &shell_listener, sh);
			break;
		}
	}

	if (!sh->shell) {
		LOG("agl_shell was not advertised!\n");
		goto err;
	}

	// wait to bound now
	while (ret != -1 && sh->wait_for_bound) {
		ret = wl_display_dispatch(sh->wl_display);

		if (sh->wait_for_bound)
			continue;
	}

	// at this point, we can't do anything about it
	if (!sh->bound_ok) {
		LOG("Failed to get bound_ok event!\n");
		goto err;
	}

	LOG("agl_shell/agl_shell_ext interfaces OK\n");
	return sh;
err:
	LOG("agl_shell/agl_shell_ext interfaces NOK\n");
	return NULL;
}

static void
destroy_shell_data(struct shell_data *sh)
{
        struct window_output *w_output, *w_output_next;

        wl_list_for_each_safe(w_output, w_output_next, &sh->output_list, link)
                destroy_output(w_output);

       for (std::list<global_data>::iterator it = sh->globals.begin();
		       it != sh->globals.end(); it = sh->globals.erase(it))
	       ;

        wl_display_flush(sh->wl_display);
        wl_display_disconnect(sh->wl_display);

	delete sh;
}

static void
start_grpc_server(std::shared_ptr<grpc::Server> server)
{
	LOG("gRPC server listening\n");
	server->Wait();
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	Shell *aglShell = nullptr;
	int ret = 0;

	// instantiante the grpc server
	std::string server_address(kDefaultGrpcServiceAddress);
	GrpcServiceImpl service{aglShell};

	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();

	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::shared_ptr<grpc::Server> server(builder.BuildAndStart());
	std::thread thread(start_grpc_server, server);

	// this blocks until we detect that another shell client started
	// running
	struct shell_data *sh = register_shell_ext();
	if (!sh) {
		LOG("Failed to get register ag_shell_ext\n");
		thread.join();
		exit(EXIT_FAILURE);
	}

	std::shared_ptr<struct agl_shell> agl_shell{sh->shell, agl_shell_destroy};
	aglShell = new Shell(agl_shell, sh);

	// now that we have aglShell, set it to the gRPC proxy as well
	service.setAglShell(aglShell);

	// serve wayland requests
	while (running && ret != -1) {
		ret = wl_display_dispatch(sh->wl_display);
	}

	thread.join();
	destroy_shell_data(sh);
	return 0;
}
