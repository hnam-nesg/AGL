#pragma once
#include <cstdio>

#include <mutex>
#include <condition_variable>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "agl_shell.grpc.pb.h"
#include <chrono>

typedef void (*Callback)(agl_shell_ipc::AppStateResponse app_response);

class Reader : public grpc::ClientReadReactor<::agl_shell_ipc::AppStateResponse> {
public:
	Reader(agl_shell_ipc::AglShellManagerService::Stub *stub)
		: m_stub(stub)
	{
	}

	void AppStatusState(Callback callback)
	{
		::agl_shell_ipc::AppStateRequest request;

		// set up the callback
		m_callback = callback;
		m_stub->async()->AppStatusState(&m_context, &request, this);

		StartRead(&m_app_state);
		StartCall();
	}

	void OnReadDone(bool ok) override
	{
		if (ok) {
			m_callback(m_app_state);

			// blocks in StartRead() if the server doesn't send
			// antyhing
			StartRead(&m_app_state);
		}
	}

	void SetDone()
	{
		fprintf(stderr, "%s()\n", __func__);
		std::unique_lock<std::mutex> l(m_mutex);
		m_done = true;
	}

	void OnDone(const grpc::Status& s) override
	{
		fprintf(stderr, "%s()\n", __func__);
		std::unique_lock<std::mutex> l(m_mutex);

		m_status = s;

		fprintf(stderr, "%s() done\n", __func__);
		m_cv.notify_one();
	}

	grpc::Status Await()
	{
		std::unique_lock<std::mutex> l(m_mutex);

		m_cv.wait(l, [this] { return m_done; });

		return std::move(m_status);
	}
private:
	grpc::ClientContext m_context;
	::agl_shell_ipc::AppStateResponse m_app_state;
	agl_shell_ipc::AglShellManagerService::Stub *m_stub;

	Callback m_callback;


	std::mutex m_mutex;
	std::condition_variable m_cv;
	grpc::Status m_status;
	bool m_done = false;
};

class GrpcClient {
public:
	GrpcClient(bool wait_for_ready);
	bool ActivateApp(const std::string& app_id, const std::string& output_name);
	bool DeactivateApp(const std::string& app_id);
	bool SetAppFloat(const std::string& app_id, int32_t x_pos, int32_t y_pos);
	bool SetAppFullscreen(const std::string& app_id);
	bool SetAppOnOutput(const std::string& app_id, const std::string& output);
	bool SetAppNormal(const std::string& app_id);
	bool SetAppPosition(const std::string& app_id, int32_t x, int32_t y);
	bool SetAppScale(const std::string& app_id, int32_t width, int32_t height);
	bool SetAppSplit(const std::string& app_id, uint32_t orientation,
			 int32_t width, int32_t sticky, const std::string& output_name);
	std::vector<std::string> GetOutputs();
	void GetAppState();
	void AppStatusState(Callback callback);
	grpc::Status Wait();

private:
	Reader *reader;
	bool wait_for_ready_{false};
	std::chrono::milliseconds deadline_;
	std::unique_ptr<agl_shell_ipc::AglShellManagerService::Stub> m_stub;
};

