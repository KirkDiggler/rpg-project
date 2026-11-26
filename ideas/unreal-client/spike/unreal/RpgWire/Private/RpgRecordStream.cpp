#include "RpgRecordStream.h"

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>

#include "dnd5e/api/session/v1alpha2/record_service.grpc.pb.h"

namespace rpgwire
{

RecordStream::RecordStream(Options options, OnBeat on_beat, OnEnded on_ended)
    : options_(std::move(options))
    , on_beat_(std::move(on_beat))
    , on_ended_(std::move(on_ended))
    , channel_(grpc::CreateChannel(options_.address, grpc::InsecureChannelCredentials()))
{
    reader_ = std::thread([this] { Run(); });
}

RecordStream::~RecordStream()
{
    Stop();
}

void RecordStream::Stop()
{
    bool expected = false;
    if (stopping_.compare_exchange_strong(expected, true))
    {
        context_.TryCancel();
    }
    if (reader_.joinable())
    {
        reader_.join();
    }
}

void RecordStream::Run()
{
    namespace v2 = dnd5e::api::session::v1alpha2;

    std::unique_ptr<v2::RecordService::Stub> stub = v2::RecordService::NewStub(channel_);
    context_.AddMetadata("authorization", "Dev " + options_.player_id);

    v2::StreamRecordRequest request;
    request.set_session(options_.session);
    if (!options_.member.empty())
    {
        request.set_member(options_.member);
    }

    std::unique_ptr<grpc::ClientReader<v2::StreamRecordResponse>> reader(stub->StreamRecord(&context_, request));

    v2::StreamRecordResponse response;
    while (reader->Read(&response))
    {
        RecordBeat beat;
        switch (response.item_case())
        {
            case v2::StreamRecordResponse::kEvent:
            {
                const v2::RecordEvent& event = response.event();
                beat.item = RecordBeat::Item::kEvent;
                beat.id = event.id();
                beat.at = event.at();
                beat.act = event.act();
                beat.beat = event.beat();
                beat.fields_json = event.fields_json();
                break;
            }
            case v2::StreamRecordResponse::kLagged:
                beat.item = RecordBeat::Item::kLagged;
                beat.id = response.lagged().last_id();
                beat.beat = "lagged";
                break;
            case v2::StreamRecordResponse::kRestarted:
                beat.item = RecordBeat::Item::kRestarted;
                beat.beat = "restarted";
                break;
            default:
                beat.item = RecordBeat::Item::kUnset;
                beat.beat = "unset";
                break;
        }
        events_.fetch_add(1);
        on_beat_(beat);
    }

    const grpc::Status status = reader->Finish();
    StreamEnd end;
    end.code = static_cast<int>(status.error_code());
    end.message = status.error_message();
    end.events = events_.load();
    if (on_ended_)
    {
        on_ended_(end);
    }
}

} // namespace rpgwire
