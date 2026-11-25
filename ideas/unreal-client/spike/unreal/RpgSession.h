// RpgSession: one signed-in connection to the engine, in plain C++.
//
// Like RpgRecordStream, it names no Unreal type. It owns a channel and the
// credential every call carries, and makes blocking calls; a host runs them
// off its own thread and marshals the result back.
#pragma once

// RPGWIRE_API is the module's export mark. It is the one Unreal-ism this
// header carries: a host in another module calls Session, and a symbol that is
// not marked does not cross a module boundary in Unreal.
#include "HAL/Platform.h"

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dnd5e::api::session::v1alpha1
{
class GetAtlasResponse;
}

namespace rpgwire
{

struct Atlas;

class RPGWIRE_API Session
{
public:
    struct Options
    {
        std::string address;   // host:port; the channel is insecure, as on a local or trusted network
        std::string player_id; // the development scheme: every call carries "Dev <player id>"
    };

    explicit Session(Options options);

    // Blocking. Returns false and fills `error` when the call does not succeed.
    // The atlas as this member knows it, in plain values (see `Atlas` below).
    bool Atlas(const std::string& session, const std::string& member, struct Atlas* out, std::string* error);

    // The same call with the wire type left visible; for this module's own use.
    bool AtlasWire(const std::string& session, const std::string& member,
                   dnd5e::api::session::v1alpha1::GetAtlasResponse* out, std::string* error);

private:
    std::unique_ptr<grpc::ClientContext> Context() const;

    Options options_;
    std::shared_ptr<grpc::Channel> channel_;
};

/// An atlas in plain values, read once from the wire on the thread that fetched
/// it. Detached from the wire types on purpose: the generated types belong to
/// this module alone, because their symbols do not cross a module boundary in
/// Unreal, so a host that holds them is a host that fails to load.
struct Atlas
{
    using Cell = std::pair<int, int>; // axial (q, r)

    struct Region
    {
        std::string id;
        std::string archetype;
        std::vector<Cell> cells;
    };

    /// A wall or a door as the atlas authors it, in feet.
    struct Authored
    {
        double height = 0.0;
        double elevation = 0.0;
        int openings = 0;
        bool door = false;
    };

    /// One placed footprint, in feet: `depth` runs along `facing` (degrees), `width` across it.
    struct Placed
    {
        std::string id;
        double width = 0.0;
        double depth = 0.0;
        double origin_x = 0.0;
        double origin_y = 0.0;
        double facing = 0.0;
    };

    std::string dungeon_key;
    std::vector<Cell> cells;       // the cells this member knows
    std::vector<Region> regions;
    std::map<std::string, Authored> authored; // walls and doors, by id
    std::vector<Placed> placed;    // walls, their spans, doors and props
    Cell start{0, 0};
};

/// Fills `out` from what `Atlas` fetched.
RPGWIRE_API void ReadAtlas(const dnd5e::api::session::v1alpha1::GetAtlasResponse& wire, Atlas& out);

} // namespace rpgwire
