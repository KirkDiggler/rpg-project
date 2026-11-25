#include "RpgSession.h"

#include <chrono>
#include <grpcpp/grpcpp.h>

#include "dnd5e/api/lobby/v1alpha1/service.grpc.pb.h"
#include "dnd5e/api/session/v1alpha1/service.grpc.pb.h"
#include "dnd5e/api/v1alpha1/character.grpc.pb.h"

namespace rpgwire
{

namespace v1 = dnd5e::api::session::v1alpha1;

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
