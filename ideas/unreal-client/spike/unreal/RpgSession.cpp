#include "RpgSession.h"

#include <chrono>
#include <grpcpp/grpcpp.h>

#include "dnd5e/api/session/v1alpha1/service.grpc.pb.h"

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
