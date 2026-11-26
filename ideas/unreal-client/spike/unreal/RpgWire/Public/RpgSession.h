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
#include "RpgRecordStream.h" // RecordBeat, for the record read back

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

/// A cell on the hex board, axial (q, r).
using Cell = std::pair<int, int>;

/// One thing a member's turn lets it do right now, as Afford reports it.
struct Declaration
{
    std::string id;       // the selector a verb carries; empty when unavailable
    int verb = 0;         // dnd5e.api.session.v1alpha1.Verb: 1 attack, 2 move, 3 end turn
    int slot = 0;
    bool available = false;
    std::string why;      // the engine's words when it is not
};

/// What Afford said: which kind of time the member is in, and what it may do.
struct Afforded
{
    int clock = 0; // dnd5e.api.session.v1alpha1.ClockKind: 1 world, 2 turn
    std::vector<Declaration> declarations;
};

/// A walk the engine accepted.
struct Walk
{
    std::vector<Cell> steps;
    bool joined_combat = false;
    bool continues = false;
    std::string stop_reason;
};

/// A swing the engine resolved.
struct Swing
{
    int roll = 0;
    int total = 0;
    int against = 0;
    bool hit = false;
    bool critical = false;
    int damage = 0;
};

/// The selector a verb carries for `verb` (1 attack, 2 move, 3 end turn), when
/// Afford says it is available; empty otherwise.
inline std::string DeclarationFor(const Afforded& afforded, int verb)
{
    for (const Declaration& declaration : afforded.declarations)
    {
        if (declaration.verb == verb && declaration.available && !declaration.id.empty())
        {
            return declaration.id;
        }
    }
    return std::string();
}

/// One of the player's characters.
struct Character
{
    std::string id;
    std::string name;
    int level = 0;
};

/// One member of a session's roster.
struct Member
{
    enum class Kind { Unspecified = 0, Player = 1, Monster = 2, World = 3 };

    std::string id;
    Kind kind = Kind::Unspecified;
    std::string name;
    std::string monster_ref; // for a monster: the creature, as a reference
    std::string class_ref;   // for a player: the class, as a reference
};

class RPGWIRE_API Session
{
public:
    struct Options
    {
        std::string address;   // host:port; the channel is insecure, as on a local or trusted network
        std::string player_id; // the development scheme: every call carries "Dev <player id>"
    };

    explicit Session(Options options);

    // Every call below blocks, and returns false with `error` filled when it
    // does not succeed. Run them off the game thread.

    // The signed-in player's characters.
    bool Characters(std::vector<Character>* out, std::string* error);

    // Starts a game the way the web's lobby does: create a lobby with the
    // character, ready, start the default dungeon. `session` is the encounter
    // the party is seated in; `member` is the character's id in it.
    bool StartGame(const std::string& character_id, std::string* session, std::string* error);

    // Who is in the session, as this member knows them. Positions are not in
    // it on purpose: the record's `joined_at` beats say where each stood.
    bool Roster(const std::string& session, const std::string& member, std::vector<Member>* out, std::string* error);

    // The atlas as this member knows it, in plain values (see `Atlas` below).
    bool Atlas(const std::string& session, const std::string& member, struct Atlas* out, std::string* error);

    // The same call with the wire type left visible; for this module's own use.
    bool AtlasWire(const std::string& session, const std::string& member,
                   dnd5e::api::session::v1alpha1::GetAtlasResponse* out, std::string* error);

    // What the member may do right now, and which kind of time it is in.
    bool Afford(const std::string& session, const std::string& member, Afforded* out, std::string* error);

    // Walks `path` cell by cell. `path` names the cells to step onto, never
    // the one the member stands on (the engine refuses that). `declaration_id`
    // is the one Afford gave for Move on the turn clock; empty on the world
    // clock. The engine may stop early (sight, the turn's budget, an
    // obstacle): `steps` is what was walked.
    bool Move(const std::string& session, const std::string& member, const std::vector<Cell>& path,
              const std::string& declaration_id, Walk* out, std::string* error);

    // Swings at `target`; `declaration_id` as for Move, and never empty.
    bool Attack(const std::string& session, const std::string& attacker, const std::string& target,
                const std::string& declaration_id, Swing* out, std::string* error);

    // Ends the member's turn; `next` is who acts after it. A driven turn in
    // between runs inside the call, and reaches the client through the record.
    bool EndTurn(const std::string& session, const std::string& member, const std::string& declaration_id,
                 std::string* next, bool* round_wrapped, std::string* error);

    // Leaves the session, freeing the seat.
    bool Exit(const std::string& session, const std::string& member, std::string* error);

    // The record as written so far, oldest first, from after `after_id` (empty:
    // from the start); at most `limit` beats (0: the engine's own page).
    bool Record(const std::string& session, const std::string& member, const std::string& after_id, int limit,
                std::vector<RecordBeat>* out, std::string* error);

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
    using Cell = rpgwire::Cell;

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
