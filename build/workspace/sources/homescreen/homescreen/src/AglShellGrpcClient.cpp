//include stuff here
#include <cstdio>

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "AglShellGrpcClient.h"
#include "agl_shell.grpc.pb.h"
#include "hmi-debug.h"

using namespace std::chrono;

namespace {
	const char kDefaultGrpcServiceAddress[] = "127.0.0.1:14005";
}

GrpcClient::GrpcClient()
{
	m_channel = grpc::CreateChannel(kDefaultGrpcServiceAddress,
			grpc::InsecureChannelCredentials());

	// init the stub here
	m_stub = agl_shell_ipc::AglShellManagerService::NewStub(m_channel);
	reader = new Reader(m_stub.get());
}

void
GrpcClient::WaitForConnected(int wait_time_ms, int tries_timeout)
{
	struct timespec ts;
	grpc_connectivity_state state;
	int try_ = 0;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec = 0;
	ts.tv_nsec = 500 * 1000 * 1000; // 500ms

	while (((state = m_channel->GetState(true)) != GRPC_CHANNEL_READY) && 
		try_++ < tries_timeout) {
		HMI_DEBUG("HomesScreen", "waiting for channel state to be ready, current state %d", state);
		nanosleep(&ts, NULL);
	}

}


bool
GrpcClient::ActivateApp(const std::string& app_id, const std::string& output_name)
{
	agl_shell_ipc::ActivateRequest request;

	request.set_app_id(app_id);
	request.set_output_name(output_name);

	grpc::ClientContext context;
	::agl_shell_ipc::ActivateResponse reply;

	grpc::Status status = m_stub->ActivateApp(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::DeactivateApp(const std::string& app_id)
{
	agl_shell_ipc::DeactivateRequest request;

	request.set_app_id(app_id);

	grpc::ClientContext context;
	::agl_shell_ipc::DeactivateResponse reply;

	grpc::Status status = m_stub->DeactivateApp(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppFloat(const std::string& app_id, int32_t x_pos, int32_t y_pos)
{
	agl_shell_ipc::FloatRequest request;

	request.set_app_id(app_id);

	request.set_x_pos(x_pos);
	request.set_y_pos(y_pos);

	grpc::ClientContext context;
	::agl_shell_ipc::FloatResponse reply;

	grpc::Status status = m_stub->SetAppFloat(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppNormal(const std::string& app_id)
{
	agl_shell_ipc::NormalRequest request;

	request.set_app_id(app_id);

	grpc::ClientContext context;
	::agl_shell_ipc::NormalResponse reply;

	grpc::Status status = m_stub->SetAppNormal(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppFullscreen(const std::string& app_id)
{
	agl_shell_ipc::FullscreenRequest request;

	request.set_app_id(app_id);

	grpc::ClientContext context;
	::agl_shell_ipc::FullscreenResponse reply;

	grpc::Status status = m_stub->SetAppFullscreen(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppOnOutput(const std::string& app_id, const std::string &output)
{
	agl_shell_ipc::AppOnOutputRequest request;

	request.set_app_id(app_id);
	request.set_output(output);

	grpc::ClientContext context;
	::agl_shell_ipc::AppOnOutputResponse reply;

	grpc::Status status = m_stub->SetAppOnOutput(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppPosition(const std::string& app_id, int32_t x, int32_t y)
{
	agl_shell_ipc::AppPositionRequest request;

	request.set_app_id(app_id);
	request.set_x(x);
	request.set_y(y);

	grpc::ClientContext context;
	::agl_shell_ipc::AppPositionResponse reply;

	grpc::Status status = m_stub->SetAppPosition(&context, request, &reply);
	return status.ok();
}

bool
GrpcClient::SetAppScale(const std::string& app_id, int32_t width, int32_t height)
{
	agl_shell_ipc::AppScaleRequest request;

	request.set_app_id(app_id);
	request.set_width(width);
	request.set_height(height);

	grpc::ClientContext context;
	::agl_shell_ipc::AppScaleResponse reply;

	grpc::Status status = m_stub->SetAppScale(&context, request, &reply);
	return status.ok();
}


grpc::Status
GrpcClient::Wait(void)
{
	return reader->Await();
}

void
GrpcClient::AppStatusState(Callback callback, void *data)
{
	reader->AppStatusState(callback, data);
}

std::vector<std::string>
GrpcClient::GetOutputs()
{
	grpc::ClientContext context;
	std::vector<std::string> v;

	::agl_shell_ipc::OutputRequest request;
	::agl_shell_ipc::ListOutputResponse response;

	grpc::Status status = m_stub->GetOutputs(&context, request, &response);
	if (!status.ok())
		return std::vector<std::string>();

	for (int i = 0; i < response.outputs_size(); i++) {
		::agl_shell_ipc::OutputResponse rresponse = response.outputs(i);
		v.push_back(rresponse.name());
	}

	return v;
}
