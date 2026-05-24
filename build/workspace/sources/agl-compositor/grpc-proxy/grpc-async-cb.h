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

#include <memory>

#define GRPC_CALLBACK_API_NONEXPERIMENTAL

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <mutex>
#include <condition_variable>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "shell.h"
#include "agl_shell.grpc.pb.h"

namespace {
       const char kDefaultGrpcServiceAddress[] = "127.0.0.1:14005";
}

class Lister : public grpc::ServerWriteReactor<::agl_shell_ipc::AppStateResponse> {
public:
	Lister(Shell *aglShell);
	void OnDone() override;
	void OnWriteDone(bool ok) override;
	void NextWrite(void);
	bool Writting(void);
private:
	Shell *m_shell;
	bool m_writting;
};

class GrpcServiceImpl final : public agl_shell_ipc::AglShellManagerService::CallbackService {
public:
	GrpcServiceImpl(Shell *aglShell) : m_aglShell(aglShell) {}
	void setAglShell(Shell *_aglShell) { m_aglShell = _aglShell; }

	grpc::ServerUnaryReactor *ActivateApp(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::ActivateRequest* request,
			::agl_shell_ipc::ActivateResponse* /*response*/) override;

	grpc::ServerUnaryReactor *DeactivateApp(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::DeactivateRequest* request,
			::agl_shell_ipc::DeactivateResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppFloat(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::FloatRequest* request,
			::agl_shell_ipc::FloatResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppSplit(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::SplitRequest* request,
			::agl_shell_ipc::SplitResponse* /*response*/) override;

	grpc::ServerUnaryReactor *GetOutputs(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::OutputRequest* /* request */,
			::agl_shell_ipc::ListOutputResponse* response) override;

	grpc::ServerUnaryReactor *SetAppNormal(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::NormalRequest* request,
			::agl_shell_ipc::NormalResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppFullscreen(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::FullscreenRequest* request,
			::agl_shell_ipc::FullscreenResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppOnOutput(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::AppOnOutputRequest* request,
			::agl_shell_ipc::AppOnOutputResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppPosition(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::AppPositionRequest* request,
			::agl_shell_ipc::AppPositionResponse* /*response*/) override;

	grpc::ServerUnaryReactor *SetAppScale(grpc::CallbackServerContext *context,
			const ::agl_shell_ipc::AppScaleRequest* request,
			::agl_shell_ipc::AppScaleResponse* /*response*/) override;

	grpc::ServerWriteReactor< ::agl_shell_ipc::AppStateResponse>* AppStatusState(
	      ::grpc::CallbackServerContext* /*context*/,
	      const ::agl_shell_ipc::AppStateRequest* /*request*/)  override;

private:
       Shell *m_aglShell = nullptr;
};
