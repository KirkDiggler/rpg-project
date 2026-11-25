// RpgRecordStream: one subscription to a session's record, in plain C++.
//
// This class names no Unreal type. It is the seed of the client core the
// Unreal module wraps — the layer that stays testable without an editor, as
// the spike's headless client was. The wire types it uses are generated.
//
// The contract it follows (rpg-api-protos, dnd5e.api.session.v1alpha2
// RecordService): the stream catches up, then goes live; `lagged` is always
// the last message when it appears; `restarted` is always the first. A
// caller that is not seated names no member, which is the development-mode
// any-reader path and is refused outside development mode.
#pragma once

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include "HAL/Platform.h" // RPGWIRE_API: the one Unreal-ism, as on Session

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace rpgwire
{

// One message of the record stream, flattened.
struct RecordBeat
{
    enum class Item { kEvent, kLagged, kRestarted, kUnset };

    Item item = Item::kUnset;
    std::string id;            // ordering key, never a counter; a lagged message carries last_id here
    unsigned long long at = 0; // the world-clock reading in rounds
    std::string act;           // "<verb>:<id>" of the act that caused the beat; empty outside any act
    std::string beat;          // the encounter's beat name as written; "lagged", "restarted" or "unset" for the others
    std::string fields_json;   // the beat's fields, as the engine wrote them
};

// How a stream ended. code is a grpc status code; 1 is cancelled by Stop().
struct StreamEnd
{
    int code = 0;
    std::string message;
    unsigned long long events = 0;
};

class RPGWIRE_API RecordStream
{
public:
    struct Options
    {
        std::string address;   // host:port; the channel is insecure, as on a local or trusted network
        std::string session;
        std::string player_id; // the development scheme: the client sends "Dev <player id>"
        std::string member;    // the seat gate; empty names no seat (development mode only)
    };

    using OnBeat = std::function<void(const RecordBeat&)>;
    using OnEnded = std::function<void(const StreamEnd&)>;

    // Starts reading at once, on a thread of its own. on_beat and on_ended are
    // called on THAT thread; a host marshals to its own.
    RecordStream(Options options, OnBeat on_beat, OnEnded on_ended);
    ~RecordStream();

    RecordStream(const RecordStream&) = delete;
    RecordStream& operator=(const RecordStream&) = delete;

    // Cancels the call and returns once the reader thread has joined. Safe to
    // call more than once.
    void Stop();

private:
    void Run();

    Options options_;
    OnBeat on_beat_;
    OnEnded on_ended_;
    std::shared_ptr<grpc::Channel> channel_;
    grpc::ClientContext context_;
    std::thread reader_;
    std::atomic<bool> stopping_{false};
    std::atomic<unsigned long long> events_{0};
};

} // namespace rpgwire
