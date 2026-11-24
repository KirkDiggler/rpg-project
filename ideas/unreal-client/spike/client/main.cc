// record_client — Unreal-client spike, step 4. THROWAWAY.
//
// Reads a session's record stream from the Go engine with the protobuf /
// gRPC runtime TurboLink v1.4.2 links in UE 5.6 (protobuf 4.23.4, gRPC
// 1.57.0), through the production auth boundary (`authorization: Dev <id>`).
//
//   record_client <addr> <session> <player> [member] [seconds]
//
// A member names the seat gate (the caller must be seated as that member);
// without one, the read is the development-mode "any signed-in reader" path.
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>

#include "dnd5e/api/session/v1alpha1/service.grpc.pb.h"
#include "dnd5e/api/session/v1alpha2/record_service.grpc.pb.h"

namespace {

namespace v1 = dnd5e::api::session::v1alpha1;
namespace v2 = dnd5e::api::session::v1alpha2;

constexpr std::size_t kFieldsPreview = 200;

std::string preview(const std::string& s) {
  if (s.size() <= kFieldsPreview) return s;
  return s.substr(0, kFieldsPreview) + "...";
}

long long ms_since(std::chrono::steady_clock::time_point start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now() - start)
      .count();
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "usage: record_client <addr> <session> <player> [member] [seconds]\n";
    return 2;
  }
  const std::string addr = argv[1];
  const std::string session = argv[2];
  const std::string player = argv[3];
  const std::string member = argc > 4 && std::string(argv[4]) != "-" ? argv[4] : "";
  const int seconds = argc > 5 ? std::atoi(argv[5]) : 90;

  const auto started = std::chrono::steady_clock::now();
  std::cout << "connecting to " << addr << std::endl;
  auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());

  // 1) A unary verb through the real auth boundary, as the seated member:
  //    where does the host stand? `position` is a message field, so the
  //    generated code carries presence for it.
  if (!member.empty()) {
    auto session_stub = v1::SessionService::NewStub(channel);
    grpc::ClientContext where_ctx;
    where_ctx.AddMetadata("authorization", "Dev " + player);
    where_ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));
    v1::GetWhereRequest where_req;
    where_req.set_session(session);
    where_req.set_member(member);
    v1::GetWhereResponse where;
    const grpc::Status ws = session_stub->GetWhere(&where_ctx, where_req, &where);
    std::cout << "+" << ms_since(started) << "ms GetWhere ok=" << ws.ok()
              << " has_position=" << (ws.ok() && where.has_position());
    if (ws.ok() && where.has_position()) {
      std::cout << " position=(" << where.position().x() << ", " << where.position().y() << ")";
    } else {
      std::cout << " code=" << ws.error_code() << " msg=\"" << ws.error_message() << "\"";
    }
    std::cout << std::endl;
  }

  // 2) The record stream: catch up, then go live.
  auto record_stub = v2::RecordService::NewStub(channel);
  grpc::ClientContext stream_ctx;
  stream_ctx.AddMetadata("authorization", "Dev " + player);
  v2::StreamRecordRequest req;
  req.set_session(session);
  if (!member.empty()) req.set_member(member);
  std::unique_ptr<grpc::ClientReader<v2::StreamRecordResponse>> reader =
      record_stub->StreamRecord(&stream_ctx, req);

  std::thread timer([&stream_ctx, seconds] {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    stream_ctx.TryCancel();  // the stream stays open while the run lives
  });

  int events = 0, lagged = 0, restarted = 0;
  long long last_at = 0;
  v2::StreamRecordResponse resp;
  while (reader->Read(&resp)) {
    switch (resp.item_case()) {
      case v2::StreamRecordResponse::kEvent: {
        ++events;
        const auto& e = resp.event();
        std::cout << "+" << ms_since(started) << "ms event #" << events << " id=" << e.id()
                  << " at=" << e.at() << " act=[" << e.act() << "] beat=\"" << e.beat()
                  << "\" fields=" << preview(e.fields_json()) << "\n";
        last_at = ms_since(started);
        break;
      }
      case v2::StreamRecordResponse::kLagged:
        ++lagged;
        std::cout << "+" << ms_since(started) << "ms LAGGED last_id=" << resp.lagged().last_id()
                  << " (the server ends the stream after this)\n";
        break;
      case v2::StreamRecordResponse::kRestarted:
        ++restarted;
        std::cout << "+" << ms_since(started) << "ms RESTARTED (replace what you hold)\n";
        break;
      default:
        std::cout << "+" << ms_since(started) << "ms item unset\n";
        break;
    }
    std::cout.flush();
  }
  const grpc::Status finish = reader->Finish();
  timer.join();
  std::cout << "stream finished after " << ms_since(started) << "ms: code=" << finish.error_code()
            << " msg=\"" << finish.error_message() << "\"  events=" << events
            << " lagged=" << lagged << " restarted=" << restarted
            << " (last event at +" << last_at << "ms)\n";
  return finish.ok() || finish.error_code() == grpc::StatusCode::CANCELLED ? 0 : 1;
}
