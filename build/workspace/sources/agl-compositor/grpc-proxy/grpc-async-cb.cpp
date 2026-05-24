/*
 * Copyright © 2022, 2023 Collabora, Ltd.
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

#define GRPC_CALLBACK_API_NONEXPERIMENTAL

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "log.h"
#include "agl_shell.grpc.pb.h"
#include "grpc-async-cb.h"

Lister::Lister(Shell *shell) : m_shell(shell)
{
	// don't call NextWrite() just yet we do it explicitly when getting
	// the events from the compositor
	m_writting = false;
}

void
Lister::OnDone()
{
	delete this;
}

void Lister::OnWriteDone(bool ok)
{
	LOG("got ok %d\n", ok);
	if (ok) {
		LOG("done writting %d\n", m_writting);
		m_writting = false;
	}
}

void
Lister::NextWrite(void)
{
	if (m_writting) {
		LOG(">>>>> still in writting\n");
		return;
	}
	m_writting = true;
	StartWrite(&m_shell->m_shell_data->current_app_state);
}

bool
Lister::Writting(void)
{
	return m_writting;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::ActivateApp(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::ActivateRequest* request,
                            ::agl_shell_ipc::ActivateResponse* /*response*/)
{
	LOG("activating app %s on output %s\n", request->app_id().c_str(),
						request->output_name().c_str());
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}

	m_aglShell->ActivateApp(request->app_id(), request->output_name());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::DeactivateApp(grpc::CallbackServerContext *context,
                              const ::agl_shell_ipc::DeactivateRequest* request,
                              ::agl_shell_ipc::DeactivateResponse* /*response*/)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}

	m_aglShell->DeactivateApp(request->app_id());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppFloat(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::FloatRequest* request,
                            ::agl_shell_ipc::FloatResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppFloat(request->app_id(),
				request->x_pos(), request->y_pos());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppNormal(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::NormalRequest* request,
                            ::agl_shell_ipc::NormalResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppNormal(request->app_id());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppFullscreen(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::FullscreenRequest* request,
                            ::agl_shell_ipc::FullscreenResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppFullscreen(request->app_id());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppOnOutput(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::AppOnOutputRequest* request,
                            ::agl_shell_ipc::AppOnOutputResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppOnOutput(request->app_id(), request->output());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppPosition(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::AppPositionRequest* request,
                            ::agl_shell_ipc::AppPositionResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppPosition(request->app_id(), request->x(), request->y());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppScale(grpc::CallbackServerContext *context,
                            const ::agl_shell_ipc::AppScaleRequest* request,
                            ::agl_shell_ipc::AppScaleResponse* /* response */)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppScale(request->app_id(),
				request->width(), request->height());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::SetAppSplit(grpc::CallbackServerContext *context,
           const ::agl_shell_ipc::SplitRequest* request,
           ::agl_shell_ipc::SplitResponse* /*response*/)
{
	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}
	m_aglShell->SetAppSplit(request->app_id(), request->tile_orientation(),
				request->width(), request->sticky(),
				request->output_name());

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerUnaryReactor *
GrpcServiceImpl::GetOutputs(grpc::CallbackServerContext *context,
	   const ::agl_shell_ipc::OutputRequest* /* request */,
	   ::agl_shell_ipc::ListOutputResponse* response)
{
	struct window_output *output;

	if (!m_aglShell) {
		LOG("m_aglShell not set-up\n");
		return nullptr;
	}

	struct wl_list *list = &m_aglShell->m_shell_data->output_list;
	wl_list_for_each(output, list, link) {
		auto m_output = response->add_outputs();
		m_output->set_name(output->name);
	}

	grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}

grpc::ServerWriteReactor<::agl_shell_ipc::AppStateResponse>*
GrpcServiceImpl::AppStatusState(grpc::CallbackServerContext* context,
				 const ::agl_shell_ipc::AppStateRequest* /*request */)
{


	if (!m_aglShell) {
		LOG(">> m_aglShell not set-up\n");
		return nullptr;
	}

	Lister *n = new Lister(m_aglShell);
	m_aglShell->m_shell_data->server_context_list.push_back(std::pair(context, n));
	LOG("added lister %p\n", static_cast<void *>(n));

	// just return a Lister to keep the channel open
	return n;
}
