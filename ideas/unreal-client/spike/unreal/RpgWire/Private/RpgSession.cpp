#include "RpgSession.h"

#include <chrono>
#include <grpcpp/grpcpp.h>

#include "dnd5e/api/lobby/v1alpha1/service.grpc.pb.h"
#include "dnd5e/api/session/v1alpha1/service.grpc.pb.h"
#include "dnd5e/api/session/v1alpha2/record_service.grpc.pb.h"
#include "dnd5e/api/v1alpha1/character.grpc.pb.h"

namespace rpgwire
{

namespace v1 = dnd5e::api::session::v1alpha1;
namespace v2 = dnd5e::api::session::v1alpha2;

namespace
{
    // The engine's words, with the call's name in front.
    bool Refused(const grpc::Status& status, const char* call, std::string* error)
    {
        if (error)
        {
            *error = std::string(call) + ": " + status.error_message();
        }
        return false;
    }

    Atlas::Cell CellOf(const v1::Position& position)
    {
        return Atlas::Cell(static_cast<int>(position.x()), static_cast<int>(position.y()));
    }
} // namespace

Session::Session(Options options)
    : options_(std::move(options))
    , channel_(grpc::CreateChannel(options_.address, grpc::InsecureChannelCredentials()))
{
}

std::unique_ptr<grpc::ClientContext> Session::Context() const
{
    std::unique_ptr<grpc::ClientContext> context(new grpc::ClientContext());
    context->AddMetadata("authorization", "Dev " + options_.player_id);
    context->set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));
    return context;
}

bool Session::Atlas(const std::string& session, const std::string& member, struct Atlas* out, std::string* error)
{
    v1::GetAtlasResponse wire;
    if (!AtlasWire(session, member, &wire, error))
    {
        return false;
    }
    ReadAtlas(wire, *out);
    return true;
}

bool Session::AtlasWire(const std::string& session, const std::string& member,
                        v1::GetAtlasResponse* out, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::GetAtlasRequest request;
    request.set_session(session);
    request.set_member(member);
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->GetAtlas(context.get(), request, out);
    if (!status.ok())
    {
        if (error)
        {
            *error = status.error_message();
        }
        return false;
    }
    return true;
}

bool Session::Characters(std::vector<Character>* out, std::string* error)
{
    namespace character = dnd5e::api::v1alpha1;
    std::unique_ptr<character::CharacterService::Stub> stub = character::CharacterService::NewStub(channel_);
    character::ListCharactersRequest request;
    character::ListCharactersResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->ListCharacters(context.get(), request, &response);
    if (!status.ok())
    {
        if (error)
        {
            *error = status.error_message();
        }
        return false;
    }
    out->clear();
    for (const auto& wire : response.characters())
    {
        Character value;
        value.id = wire.id();
        value.name = wire.name();
        value.level = wire.level();
        out->push_back(value);
    }
    return true;
}

bool Session::StartGame(const std::string& character_id, std::string* session, std::string* error)
{
    namespace lobby = dnd5e::api::lobby::v1alpha1;
    std::unique_ptr<lobby::LobbyService::Stub> stub = lobby::LobbyService::NewStub(channel_);

    lobby::CreateLobbyRequest create;
    create.set_campaign_id("default-campaign"); // the web's dev campaign (LobbyFlow.tsx)
    create.set_character_id(character_id);
    lobby::CreateLobbyResponse created;
    {
        std::unique_ptr<grpc::ClientContext> context = Context();
        const grpc::Status status = stub->CreateLobby(context.get(), create, &created);
        if (!status.ok())
        {
            if (error)
            {
                *error = "CreateLobby: " + status.error_message();
            }
            return false;
        }
    }

    lobby::SetReadyRequest ready;
    ready.set_lobby_id(created.lobby_id());
    ready.set_ready(true);
    lobby::SetReadyResponse readied;
    {
        std::unique_ptr<grpc::ClientContext> context = Context();
        const grpc::Status status = stub->SetReady(context.get(), ready, &readied);
        if (!status.ok())
        {
            if (error)
            {
                *error = "SetReady: " + status.error_message();
            }
            return false;
        }
    }

    lobby::StartEncounterRequest start;
    start.set_lobby_id(created.lobby_id());
    start.set_dungeon_key(""); // the default dungeon, as the web's lobby sends it
    lobby::StartEncounterResponse started;
    grpc::Status status;
    {
        std::unique_ptr<grpc::ClientContext> context = Context();
        status = stub->StartEncounter(context.get(), start, &started);
    }
    std::string recovery;
    if (!status.ok() && status.error_message().find("seated in session \"") != std::string::npos)
    {
        // A character sits in one run at a time, and the refusal names that run:
        //   ... is seated in session "<id>" ...
        // Leave it, as the character (the Exit verb: "the client just says
        // leave"), then start this lobby. One attempt.
        const std::string message = status.error_message();
        const size_t first = message.find("seated in session \"") + 19;
        const size_t last = message.find('\"', first);
        if (last != std::string::npos && last > first)
        {
            const std::string earlier = message.substr(first, last - first);
            v1::ExitRequest exit;
            exit.set_session(earlier);
            exit.set_member(character_id);
            v1::ExitResponse left;
            std::unique_ptr<grpc::ClientContext> exitContext = Context();
            std::unique_ptr<v1::SessionService::Stub> sessions = v1::SessionService::NewStub(channel_);
            const grpc::Status exited = sessions->Exit(exitContext.get(), exit, &left);
            recovery = " (left the earlier run " + earlier + (exited.ok() ? ")" : ": " + exited.error_message() + ")");
            if (exited.ok())
            {
                lobby::StartEncounterResponse retried;
                std::unique_ptr<grpc::ClientContext> retryContext = Context();
                status = stub->StartEncounter(retryContext.get(), start, &retried);
                started = retried;
            }
        }
    }
    if (!status.ok())
    {
        if (error)
        {
            *error = "StartEncounter: " + status.error_message() + recovery;
        }
        return false;
    }
    if (session)
    {
        *session = started.encounter_id();
    }
    return true;
}

bool Session::Afford(const std::string& session, const std::string& member, Afforded* out, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::AffordRequest request;
    request.set_session(session);
    request.set_member(member);
    v1::AffordResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->Afford(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "Afford", error);
    }
    out->clock = static_cast<int>(response.clock());
    out->declarations.clear();
    for (const v1::Declaration& declaration : response.declarations())
    {
        Declaration entry;
        entry.verb = static_cast<int>(declaration.verb());
        entry.slot = static_cast<int>(declaration.slot());
        entry.available = declaration.available();
        entry.id = declaration.id();
        entry.why = declaration.has_why() ? declaration.why().text() : std::string();
        out->declarations.push_back(entry);
    }
    return true;
}

bool Session::Move(const std::string& session, const std::string& member, const std::vector<Cell>& path,
                   const std::string& declaration_id, Walk* out, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::MoveRequest request;
    request.set_session(session);
    request.set_member(member);
    for (const Cell& cell : path)
    {
        v1::Position* position = request.add_path();
        position->set_x(cell.first);
        position->set_y(cell.second);
    }
    request.set_declaration_id(declaration_id);
    v1::MoveResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->Move(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "Move", error);
    }
    out->steps.clear();
    for (const v1::Step& step : response.steps())
    {
        if (step.has_position())
        {
            out->steps.push_back(CellOf(step.position()));
        }
    }
    out->joined_combat = response.joined_combat();
    out->continues = response.continues();
    out->stop_reason = response.stop_reason();
    return true;
}

bool Session::Attack(const std::string& session, const std::string& attacker, const std::string& target,
                     const std::string& declaration_id, Swing* out, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::AttackRequest request;
    request.set_session(session);
    request.set_attacker(attacker);
    request.set_target(target);
    request.set_declaration_id(declaration_id);
    v1::AttackResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->Attack(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "Attack", error);
    }
    out->roll = response.roll();
    out->total = response.total();
    out->against = response.against();
    out->hit = response.hit();
    out->critical = response.critical();
    out->damage = response.damage();
    return true;
}

bool Session::EndTurn(const std::string& session, const std::string& member, const std::string& declaration_id,
                      std::string* next, bool* round_wrapped, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::EndTurnRequest request;
    request.set_session(session);
    request.set_member(member);
    request.set_declaration_id(declaration_id);
    v1::EndTurnResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->EndTurn(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "EndTurn", error);
    }
    *next = response.next();
    *round_wrapped = response.round_wrapped();
    return true;
}

bool Session::Exit(const std::string& session, const std::string& member, std::string* error)
{
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    v1::ExitRequest request;
    request.set_session(session);
    request.set_member(member);
    v1::ExitResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->Exit(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "Exit", error);
    }
    return true;
}

bool Session::Record(const std::string& session, const std::string& member, const std::string& after_id, int limit,
                     std::vector<RecordBeat>* out, std::string* error)
{
    std::unique_ptr<v2::RecordService::Stub> stub = v2::RecordService::NewStub(channel_);
    v2::GetRecordRequest request;
    request.set_session(session);
    request.set_member(member);
    request.set_after_id(after_id);
    request.set_limit(limit);
    v2::GetRecordResponse response;
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->GetRecord(context.get(), request, &response);
    if (!status.ok())
    {
        return Refused(status, "GetRecord", error);
    }
    out->clear();
    for (const v2::RecordEvent& event : response.events())
    {
        RecordBeat beat;
        beat.item = RecordBeat::Item::kEvent;
        beat.id = event.id();
        beat.at = event.at();
        beat.act = event.act();
        beat.beat = event.beat();
        beat.fields_json = event.fields_json();
        out->push_back(beat);
    }
    return true;
}

bool Session::Roster(const std::string& session, const std::string& member, std::vector<Member>* out, std::string* error)
{
    v1::GetRosterRequest request;
    request.set_session(session);
    request.set_member(member);
    v1::GetRosterResponse response;
    std::unique_ptr<v1::SessionService::Stub> stub = v1::SessionService::NewStub(channel_);
    std::unique_ptr<grpc::ClientContext> context = Context();
    const grpc::Status status = stub->GetRoster(context.get(), request, &response);
    if (!status.ok())
    {
        if (error)
        {
            *error = status.error_message();
        }
        return false;
    }
    out->clear();
    for (const auto& wire : response.members())
    {
        Member value;
        value.id = wire.id();
        switch (wire.kind())
        {
            case v1::MEMBER_KIND_PLAYER: value.kind = Member::Kind::Player; break;
            case v1::MEMBER_KIND_MONSTER: value.kind = Member::Kind::Monster; break;
            case v1::MEMBER_KIND_WORLD: value.kind = Member::Kind::World; break;
            default: value.kind = Member::Kind::Unspecified; break;
        }
        value.name = wire.name();
        value.monster_ref = wire.monster_ref();
        value.class_ref = wire.class_ref();
        out->push_back(value);
    }
    return true;
}

namespace
{
    int CellCoordinate(double Value)
    {
        return static_cast<int>(Value);
    }
}

void ReadAtlas(const v1::GetAtlasResponse& wire, Atlas& out)
{
    out = Atlas();
    out.dungeon_key = wire.dungeon_key();
    for (const auto& cell : wire.cells())
    {
        out.cells.emplace_back(CellCoordinate(cell.x()), CellCoordinate(cell.y()));
    }
    for (const auto& region : wire.regions())
    {
        Atlas::Region value;
        value.id = region.id();
        value.archetype = region.archetype();
        for (const auto& cell : region.cells())
        {
            value.cells.emplace_back(CellCoordinate(cell.x()), CellCoordinate(cell.y()));
        }
        out.regions.push_back(value);
    }
    for (const auto& wall : wire.structural_walls())
    {
        Atlas::Authored& value = out.authored[wall.id()];
        value.height = wall.height();
        value.elevation = wall.elevation();
        value.openings = wall.openings_size();
    }
    for (const auto& door : wire.structural_doors())
    {
        Atlas::Authored& value = out.authored[door.id()];
        value.height = door.height();
        value.elevation = door.elevation();
        value.door = true;
    }
    for (const auto& placed : wire.placed())
    {
        Atlas::Placed value;
        value.id = placed.id();
        value.width = placed.placement().width();
        value.depth = placed.placement().depth();
        value.origin_x = placed.placement().origin().x();
        value.origin_y = placed.placement().origin().y();
        value.facing = placed.placement().facing();
        out.placed.push_back(value);
    }
    if (wire.has_start())
    {
        out.start = Atlas::Cell(CellCoordinate(wire.start().at().x()), CellCoordinate(wire.start().at().y()));
    }
}

} // namespace rpgwire
