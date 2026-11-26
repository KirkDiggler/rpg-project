// The core's verbs, proven against the engine's own server.
//
// These are integration tests: they speak gRPC to the server the spike's
// engine-side test serves (ideas/unreal-client/spike/go-side), which
// pre-launches one run per geometry these tests need. They run with
//
//   UnrealEditor-Cmd <project> -ExecCmds="Automation RunTests RpgWire"
//     -TestExit="Automation Test Queue Empty" -nullrhi -unattended -NoSound
//
// with RPGWIRE_TEST_ADDR set to that server and RPGWIRE_TEST_INFO to the
// file it wrote. A test fails, naming the variable, when either is unset: a
// skipped test would read as a pass.
#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "RpgSession.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

#include <cstdlib>
#include <map>

namespace
{
    using rpgwire::Afforded;
    using rpgwire::Atlas;
    using rpgwire::Declaration;
    using rpgwire::Session;
    using rpgwire::Swing;
    using rpgwire::Walk;

    constexpr int kVerbAttack = 1;   // dnd5e.api.session.v1alpha1.Verb
    constexpr int kVerbMove = 2;
    constexpr int kVerbEndTurn = 3;
    constexpr int kClockWorld = 1;   // dnd5e.api.session.v1alpha1.ClockKind
    constexpr int kClockTurn = 2;

    struct Fixture
    {
        std::string address;
        std::map<std::string, std::string> info;

        bool Load(FAutomationTestBase& test)
        {
            address = TCHAR_TO_UTF8(*FPlatformMisc::GetEnvironmentVariable(TEXT("RPGWIRE_TEST_ADDR")));
            const FString infoPath = FPlatformMisc::GetEnvironmentVariable(TEXT("RPGWIRE_TEST_INFO"));
            if (address.empty() || infoPath.IsEmpty())
            {
                test.AddError(TEXT("RPGWIRE_TEST_ADDR and RPGWIRE_TEST_INFO must name the engine's test server and its info file"));
                return false;
            }
            FString text;
            if (!FFileHelper::LoadFileToString(text, *infoPath))
            {
                test.AddError(FString::Printf(TEXT("cannot read the info file %s"), *infoPath));
                return false;
            }
            TSharedPtr<FJsonObject> json;
            if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(text), json) || !json.IsValid())
            {
                test.AddError(TEXT("the info file is not JSON"));
                return false;
            }
            for (const auto& pair : json->Values)
            {
                info[TCHAR_TO_UTF8(*pair.Key)] = TCHAR_TO_UTF8(*pair.Value->AsString());
            }
            return true;
        }

        Session SignIn(const std::string& player) const
        {
            Session::Options options;
            options.address = address;
            options.player_id = player;
            return Session(options);
        }

        std::string Need(FAutomationTestBase& test, const char* key) const
        {
            const auto found = info.find(key);
            if (found == info.end())
            {
                test.AddError(FString::Printf(TEXT("the info file has no \"%s\""), UTF8_TO_TCHAR(key)));
                return std::string();
            }
            return found->second;
        }
    };

    FString Take(const std::string& text)
    {
        return FString(UTF8_TO_TCHAR(text.c_str()));
    }

    /// The declarations as one readable line per entry; what a refused or
    /// unavailable verb says is the evidence, so nothing is summarised away.
    void LogDeclarations(const Afforded& afforded)
    {
        for (const Declaration& declaration : afforded.declarations)
        {
            UE_LOG(LogTemp, Display, TEXT("  AFFORD verb %d slot %d available %d id \"%s\"%s%s"),
                declaration.verb, declaration.slot, declaration.available ? 1 : 0, *Take(declaration.id),
                declaration.why.empty() ? TEXT("") : TEXT(" why: "), *Take(declaration.why));
        }
    }

    std::string DeclarationFor(const Afforded& afforded, int verb)
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

    /// A neighbour of `cell` that is on the atlas's floor, in a fixed order.
    Atlas::Cell NeighbourOnTheFloor(const Atlas& atlas, const Atlas::Cell& cell)
    {
        const Atlas::Cell candidates[6] = {
            {cell.first + 1, cell.second}, {cell.first - 1, cell.second}, {cell.first, cell.second + 1},
            {cell.first, cell.second - 1}, {cell.first + 1, cell.second - 1}, {cell.first - 1, cell.second + 1}};
        for (const Atlas::Cell& candidate : candidates)
        {
            for (const Atlas::Cell& known : atlas.cells)
            {
                if (known == candidate)
                {
                    return candidate;
                }
            }
        }
        return Atlas::Cell(cell.first, cell.second);
    }

    /// Where a member stood when it joined, from the record: the beat carries
    // {"member": …, "cell": {"x": …, "y": …}}.
    bool JoinedAt(const std::vector<rpgwire::RecordBeat>& record, const std::string& member, Atlas::Cell* out)
    {
        for (const rpgwire::RecordBeat& beat : record)
        {
            if (beat.beat != "joined_at")
            {
                continue;
            }
            TSharedPtr<FJsonObject> fields;
            if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Take(beat.fields_json)), fields) && fields.IsValid())
            {
                FString who;
                const TSharedPtr<FJsonObject>* cell = nullptr;
                if (fields->TryGetStringField(TEXT("member"), who) && who == Take(member) && fields->TryGetObjectField(TEXT("cell"), cell))
                {
                    double q = 0.0;
                    double r = 0.0;
                    (*cell)->TryGetNumberField(TEXT("x"), q);
                    (*cell)->TryGetNumberField(TEXT("y"), r);
                    *out = Atlas::Cell(static_cast<int>(q), static_cast<int>(r));
                    return true;
                }
            }
        }
        return false;
    }
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionAffordTest, "RpgWire.Session.AffordListsDeclarationsWithIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionAffordTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this))
    {
        return false;
    }
    Session session = fixture.SignIn(fixture.Need(*this, "fighter_host"));
    const std::string run = fixture.Need(*this, "tomb");
    const std::string member = fixture.Need(*this, "fighter");

    Afforded afforded;
    std::string error;
    if (!TestTrue(TEXT("Afford succeeds"), session.Afford(run, member, &afforded, &error))) return false;
    TestEqual(TEXT("a fight at launch: the turn clock"), 2, afforded.clock);
    TestTrue(TEXT("at least one declaration"), afforded.declarations.size() >= 1);
    LogDeclarations(afforded);
    for (const Declaration& declaration : afforded.declarations)
    {
        if (declaration.available)
        {
            TestFalse(*FString::Printf(TEXT("an available verb (%d) carries an id"), declaration.verb), declaration.id.empty());
        }
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionAttackAdjacentTest, "RpgWire.Session.AttackOnAnAdjacentMonsterIsResolvedByTheEngine",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionAttackAdjacentTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this)) return false;
    Session session = fixture.SignIn(fixture.Need(*this, "fighter_host"));
    const std::string run = fixture.Need(*this, "tomb");
    const std::string member = fixture.Need(*this, "fighter");
    const std::string target = fixture.Need(*this, "dummy");

    Afforded afforded;
    std::string error;
    if (!TestTrue(TEXT("Afford succeeds"), session.Afford(run, member, &afforded, &error))) return false;
    const std::string declaration = DeclarationFor(afforded, kVerbAttack);
    if (!TestFalse(TEXT("an available Attack declaration"), declaration.empty())) { LogDeclarations(afforded); return false; }

    Swing swing;
    if (!TestTrue(*Take(error), session.Attack(run, member, target, declaration, &swing, &error))) return false;
    UE_LOG(LogTemp, Display, TEXT("  ATTACK roll %d total %d against %d hit %d critical %d damage %d"),
        swing.roll, swing.total, swing.against, swing.hit, swing.critical, swing.damage);
    TestTrue(TEXT("the engine names the armour class rolled against"), swing.against > 0);
    TestTrue(TEXT("a d20 was rolled by the engine, not here"), swing.roll >= 1 && swing.roll <= 20);
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionAttackFarTest, "RpgWire.Session.AttackOnAFarTargetIsRefused",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionAttackFarTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this)) return false;
    Session session = fixture.SignIn(fixture.Need(*this, "archer_host"));
    const std::string run = fixture.Need(*this, "range");
    const std::string member = fixture.Need(*this, "archer");
    const std::string target = fixture.Need(*this, "far");

    Afforded afforded;
    std::string error;
    if (!TestTrue(TEXT("Afford succeeds"), session.Afford(run, member, &afforded, &error))) return false;
    const std::string declaration = DeclarationFor(afforded, kVerbAttack);
    if (declaration.empty())
    {
        // The engine may refuse in Afford itself: the declaration is withheld.
        LogDeclarations(afforded);
        return TestTrue(TEXT("no available Attack declaration: the engine refuses in Afford"), true);
    }
    Swing swing;
    if (TestFalse(TEXT("a target out of reach is refused"), session.Attack(run, member, target, declaration, &swing, &error)))
    {
        UE_LOG(LogTemp, Display, TEXT("  REFUSED attack %s -> %s: \"%s\""), *Take(member), *Take(target), *Take(error));
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionEndTurnTest, "RpgWire.Session.EndTurnHandsTheTurnOn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionEndTurnTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this)) return false;
    Session session = fixture.SignIn(fixture.Need(*this, "fighter_host"));
    const std::string run = fixture.Need(*this, "tomb");
    const std::string member = fixture.Need(*this, "fighter");

    Afforded afforded;
    std::string error;
    if (!TestTrue(TEXT("Afford succeeds"), session.Afford(run, member, &afforded, &error))) return false;
    const std::string declaration = DeclarationFor(afforded, kVerbEndTurn);
    if (!TestFalse(TEXT("an available End Turn declaration"), declaration.empty())) { LogDeclarations(afforded); return false; }

    std::string next;
    bool roundWrapped = false;
    if (!TestTrue(*Take(error), session.EndTurn(run, member, declaration, &next, &roundWrapped, &error))) return false;
    UE_LOG(LogTemp, Display, TEXT("  END TURN %s -> next \"%s\", round wrapped %d"), *Take(member), *Take(next), roundWrapped);
    TestFalse(TEXT("the engine names who acts next"), next.empty());
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionMoveTest, "RpgWire.Session.MoveHonoursTheClockAffordReports",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionMoveTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this)) return false;

    // The walker, alone: the world clock, no declaration.
    {
        Session session = fixture.SignIn(fixture.Need(*this, "walker_host"));
        const std::string run = fixture.Need(*this, "walk");
        const std::string member = fixture.Need(*this, "walker");
        Afforded afforded;
        std::string error;
        Atlas atlas;
        std::vector<rpgwire::RecordBeat> record;
        if (!TestTrue(*Take(error), session.Afford(run, member, &afforded, &error))) return false;
        if (!TestTrue(*Take(error), session.Atlas(run, member, &atlas, &error))) return false;
        if (!TestTrue(*Take(error), session.Record(run, member, "", 0, &record, &error))) return false;
        LogDeclarations(afforded);
        TestEqual(TEXT("alone on the board: the world clock"), kClockWorld, afforded.clock);
        Atlas::Cell here;
        if (!TestTrue(TEXT("the record says where the walker joined"), JoinedAt(record, member, &here))) return false;
        const Atlas::Cell there = NeighbourOnTheFloor(atlas, here);
        if (!TestTrue(TEXT("a neighbouring cell on the floor"), there != here)) return false;

        Walk out;
        if (!TestTrue(*Take(error), session.Move(run, member, {there}, "", &out, &error))) return false;
        TestTrue(TEXT("a step was taken"), out.steps.size() >= 1);
        if (out.steps.size() >= 1)
        {
            TestTrue(TEXT("the first step landed on the cell asked for"), out.steps[0] == there);
        }
        UE_LOG(LogTemp, Display, TEXT("  MOVE (%d,%d) -> (%d,%d): %d step(s), joined combat %d, continues %d, stop \"%s\""),
            here.first, here.second, there.first, there.second, static_cast<int32>(out.steps.size()),
            out.joined_combat, out.continues, *Take(out.stop_reason));
        Walk back;
        if (!TestTrue(*Take(error), session.Move(run, member, {here}, "", &back, &error))) return false;
        TestEqual(TEXT("and back: the walker ends where it began"), 1, static_cast<int32>(back.steps.size()));
    }

    // Alice in the castle: whichever clock Afford reports, Move honours it.
    {
        Session session = fixture.SignIn(fixture.Need(*this, "castle_host"));
        const std::string run = fixture.Need(*this, "castle");
        const std::string member = fixture.Need(*this, "castle_member");
        Afforded afforded;
        std::string error;
        Atlas atlas;
        std::vector<rpgwire::RecordBeat> record;
        if (!TestTrue(*Take(error), session.Afford(run, member, &afforded, &error))) return false;
        if (!TestTrue(*Take(error), session.Atlas(run, member, &atlas, &error))) return false;
        if (!TestTrue(*Take(error), session.Record(run, member, "", 0, &record, &error))) return false;
        LogDeclarations(afforded);
        UE_LOG(LogTemp, Display, TEXT("  the castle runs on clock %d (%s)"), afforded.clock,
            afforded.clock == kClockTurn ? TEXT("turn") : afforded.clock == kClockWorld ? TEXT("world") : TEXT("other"));
        Atlas::Cell here;
        if (!TestTrue(TEXT("the record says where alice joined"), JoinedAt(record, member, &here))) return false;
        const Atlas::Cell there = NeighbourOnTheFloor(atlas, here);
        if (!TestTrue(TEXT("a neighbouring cell on the floor"), there != here)) return false;

        if (afforded.clock == kClockTurn)
        {
            Walk refused;
            if (TestFalse(TEXT("on the turn clock, a Move with no declaration is refused"), session.Move(run, member, {there}, "", &refused, &error)))
            {
                UE_LOG(LogTemp, Display, TEXT("  REFUSED move without a declaration: \"%s\""), *Take(error));
            }
            const std::string declaration = DeclarationFor(afforded, kVerbMove);
            if (!TestFalse(TEXT("an available Move declaration"), declaration.empty())) { LogDeclarations(afforded); return false; }
            Walk out;
            if (!TestTrue(*Take(error), session.Move(run, member, {there}, declaration, &out, &error))) return false;
            TestEqual(TEXT("one step with the declaration"), 1, static_cast<int32>(out.steps.size()));
            Walk back;
            if (!TestTrue(*Take(error), session.Move(run, member, {here}, declaration, &back, &error))) return false;
            TestEqual(TEXT("and back"), 1, static_cast<int32>(back.steps.size()));
        }
        else
        {
            Walk out;
            if (!TestTrue(*Take(error), session.Move(run, member, {there}, "", &out, &error))) return false;
            TestEqual(TEXT("one step, no declaration asked"), 1, static_cast<int32>(out.steps.size()));
            Walk back;
            if (!TestTrue(*Take(error), session.Move(run, member, {here}, "", &back, &error))) return false;
            TestEqual(TEXT("and back"), 1, static_cast<int32>(back.steps.size()));
        }
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionRecordTest, "RpgWire.Session.RecordHoldsTheBeatsOfThoseActs",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSessionRecordTest::RunTest(const FString& Parameters)
{
    Fixture fixture;
    if (!fixture.Load(*this)) return false;
    Session session = fixture.SignIn(fixture.Need(*this, "fighter_host"));
    const std::string run = fixture.Need(*this, "tomb");
    const std::string member = fixture.Need(*this, "fighter");

    std::vector<rpgwire::RecordBeat> record;
    std::string error;
    if (!TestTrue(*Take(error), session.Record(run, member, "", 0, &record, &error))) return false;
    TestTrue(TEXT("the record holds beats"), record.size() >= 1);

    // Every beat an act caused, whole: these lines are the fixtures the motion
    // planner is written against (round-verbs.md, Task 2).
    int acts = 0;
    int attacks = 0;
    for (const rpgwire::RecordBeat& beat : record)
    {
        if (beat.act.empty())
        {
            continue;
        }
        ++acts;
        if (beat.act.rfind("attack:", 0) == 0)
        {
            ++attacks;
        }
        UE_LOG(LogTemp, Display, TEXT("  RECORD id %s act %s beat %s %s"), *Take(beat.id), *Take(beat.act),
            *Take(beat.beat), *Take(beat.fields_json));
    }
    FString names;
    for (const rpgwire::RecordBeat& beat : record)
    {
        names += Take(beat.beat) + TEXT(" ");
    }
    UE_LOG(LogTemp, Display, TEXT("  RECORD %d beats, %d caused by an act, %d of them an attack's; beat names: %s"),
        static_cast<int32>(record.size()), acts, attacks, *names);
    TestTrue(TEXT("the record holds the attack this suite made before this test"), attacks >= 1);

    // The walks this suite made, in the two runs that were walked: the `moved`
    // beats are the other fixture the motion planner is written against.
    const char* const walkedRuns[2][3] = {{"walk", "walker", "walker_host"}, {"castle", "castle_member", "castle_host"}};
    for (const char* const* walkedRun : walkedRuns)
    {
        Session who = fixture.SignIn(fixture.Need(*this, walkedRun[2]));
        std::vector<rpgwire::RecordBeat> walked;
        if (!TestTrue(TEXT("the walked run's record reads"), who.Record(fixture.Need(*this, walkedRun[0]), fixture.Need(*this, walkedRun[1]), "", 0, &walked, &error))) return false;
        for (const rpgwire::RecordBeat& step : walked)
        {
            if (step.beat == "moved" || step.act.rfind("move", 0) == 0 || step.act.rfind("walk", 0) == 0)
            {
                UE_LOG(LogTemp, Display, TEXT("  WALKED [%s] %s %s"), *Take(step.act), *Take(step.beat), *Take(step.fields_json));
            }
        }
    }
    return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
