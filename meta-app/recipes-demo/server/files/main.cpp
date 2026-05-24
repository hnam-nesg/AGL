#include <grpcpp/grpcpp.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ui_state.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using uistate::EventType;
using uistate::GetStateReply;
using uistate::GetStateRequest;
using uistate::SetStateReply;
using uistate::SetStateRequest;
using uistate::SubscribeStateRequest;
using uistate::UiElementState;
using uistate::UiStateEvent;
using uistate::UiStateService;

namespace {

int64_t NowMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch())
        .count();
}

struct StoredState {
    UiElementState state;
    uint64_t version = 0;
    int64_t timestamp_ms = 0;
};

struct Subscriber {
    std::mutex mu;
    std::condition_variable cv;
    std::deque<UiStateEvent> queue;
    bool active = true;
};

UiStateEvent MakeEvent(EventType type, const StoredState &stored)
{
    UiStateEvent event;
    event.set_type(type);
    *event.mutable_state() = stored.state;
    event.set_version(stored.version);
    event.set_timestamp_ms(stored.timestamp_ms);
    return event;
}

class UiStateServiceImpl final : public UiStateService::Service
{
public:
    Status SetState(ServerContext *context,
                    const SetStateRequest *request,
                    SetStateReply *reply) override
    {
        (void)context;

        if (!request->has_state()) {
            reply->set_ok(false);
            reply->set_message("missing state");
            reply->set_version(0);
            return Status::OK;
        }

        const UiElementState &incoming = request->state();
        if (incoming.target_id().empty()) {
            reply->set_ok(false);
            reply->set_message("target_id is empty");
            reply->set_version(0);
            return Status::OK;
        }

        UiStateEvent updated_event;

        {
            std::lock_guard<std::mutex> lock(state_mu_);

            StoredState &entry = states_[incoming.target_id()];
            entry.state = incoming;
            entry.version = ++global_version_;
            entry.timestamp_ms = NowMs();

            updated_event = MakeEvent(EventType::UPDATED, entry);

            reply->set_ok(true);
            reply->set_message("state updated");
            reply->set_version(entry.version);
        }

        BroadcastEvent(updated_event);
        return Status::OK;
    }

    Status GetState(ServerContext *context,
                    const GetStateRequest *request,
                    GetStateReply *reply) override
    {
        (void)context;

        if (request->target_id().empty()) {
            reply->set_found(false);
            reply->set_version(0);
            return Status::OK;
        }

        std::lock_guard<std::mutex> lock(state_mu_);

        auto it = states_.find(request->target_id());
        if (it == states_.end()) {
            reply->set_found(false);
            reply->set_version(0);
            return Status::OK;
        }

        reply->set_found(true);
        *reply->mutable_state() = it->second.state;
        reply->set_version(it->second.version);
        return Status::OK;
    }

    Status SubscribeState(ServerContext *context,
                          const SubscribeStateRequest *request,
                          ServerWriter<UiStateEvent> *writer) override
    {
        auto subscriber = std::make_shared<Subscriber>();

        RegisterSubscriber(subscriber);

        if (request->send_snapshot()) {
            std::vector<UiStateEvent> snapshots;

            {
                std::lock_guard<std::mutex> lock(state_mu_);
                snapshots.reserve(states_.size());

                for (const auto &kv : states_) {
                    snapshots.push_back(MakeEvent(EventType::SNAPSHOT, kv.second));
                }
            }

            {
                std::lock_guard<std::mutex> lock(subscriber->mu);
                for (const auto &ev : snapshots) {
                    subscriber->queue.push_back(ev);
                }
            }
            subscriber->cv.notify_one();
        }

        while (!context->IsCancelled()) {
            UiStateEvent event;
            bool has_event = false;

            {
                std::unique_lock<std::mutex> lock(subscriber->mu);

                subscriber->cv.wait_for(
                    lock,
                    std::chrono::milliseconds(200),
                    [&]() {
                        return !subscriber->queue.empty() ||
                               !subscriber->active ||
                               context->IsCancelled();
                    });

                if (context->IsCancelled() || !subscriber->active) {
                    break;
                }

                if (!subscriber->queue.empty()) {
                    event = std::move(subscriber->queue.front());
                    subscriber->queue.pop_front();
                    has_event = true;
                }
            }

            if (!has_event) {
                continue;
            }

            if (!writer->Write(event)) {
                break;
            }
        }

        UnregisterSubscriber(subscriber);
        return Status::OK;
    }

private:
    void RegisterSubscriber(const std::shared_ptr<Subscriber> &subscriber)
    {
        std::lock_guard<std::mutex> lock(subscribers_mu_);
        subscribers_.push_back(subscriber);
    }

    void UnregisterSubscriber(const std::shared_ptr<Subscriber> &subscriber)
    {
        {
            std::lock_guard<std::mutex> lock(subscribers_mu_);
            auto it = std::remove(subscribers_.begin(), subscribers_.end(), subscriber);
            subscribers_.erase(it, subscribers_.end());
        }

        {
            std::lock_guard<std::mutex> lock(subscriber->mu);
            subscriber->active = false;
        }
        subscriber->cv.notify_all();
    }

    void BroadcastEvent(const UiStateEvent &event)
    {
        std::vector<std::shared_ptr<Subscriber>> snapshot_subscribers;

        {
            std::lock_guard<std::mutex> lock(subscribers_mu_);
            snapshot_subscribers = subscribers_;
        }

        for (const auto &sub : snapshot_subscribers) {
            {
                std::lock_guard<std::mutex> lock(sub->mu);
                if (!sub->active) {
                    continue;
                }
                sub->queue.push_back(event);
            }
            sub->cv.notify_one();
        }
    }

private:
    std::mutex state_mu_;
    std::unordered_map<std::string, StoredState> states_;
    uint64_t global_version_ = 0;

    std::mutex subscribers_mu_;
    std::vector<std::shared_ptr<Subscriber>> subscribers_;
};

void RunServer(const std::string &server_address)
{
    UiStateServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    if (!server) {
        throw std::runtime_error("failed to start gRPC server");
    }

    std::cout << "UiStateService server listening at: "
              << server_address << std::endl;

    server->Wait();
}

} // namespace

int main(int argc, char **argv)
{
    try {
        std::string server_address = "127.0.0.1:50051";
        if (argc > 1 && argv[1] != nullptr && std::string(argv[1]).size() > 0) {
            server_address = argv[1];
        }

        RunServer(server_address);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}