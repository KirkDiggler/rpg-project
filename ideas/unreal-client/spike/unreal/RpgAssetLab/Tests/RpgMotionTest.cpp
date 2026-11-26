// RpgMotion's tests. The beat payloads are quoted from the record as the
// engine wrote it (spike/unreal/evidence-2026-11-26-verbs.log); where one is
// an assumption rather than an observation, the test says so.
#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "RpgMotion.h"

#include "Misc/AutomationTest.h"

namespace
{
    const TCHAR* const kMoved = TEXT("{\"beat\":\"moved\",\"member\":\"ue-walker\",\"position\":{\"x\":2,\"y\":1}}");
    const TCHAR* const kJoinedAt = TEXT("{\"beat\":\"joined_at\",\"cell\":{\"x\":2,\"y\":0},\"member\":\"guard-1\"}");
    // As written for the skeleton's swing at the fighter: keys kept, the
    // calculation's long body dropped.
    const TCHAR* const kMissed = TEXT("{\"actor\":\"ue-dummy\",\"against\":12,\"attack\":{\"damage_type\":\"piercing\","
        "\"name\":\"Shortsword\",\"ref\":\"dnd5e:weapons:shortsword\"},\"beat\":\"missed\",\"roll\":1,"
        "\"targets\":[\"ue-fighter\"],\"total\":5}");
    // ASSUMPTION, not yet observed: the engine's dice on the test server
    // always land on the low face. The name is the one record.proto gives
    // ("moved", "struck", "answered") and the shape follows `missed`.
    const TCHAR* const kStruck = TEXT("{\"actor\":\"ue-fighter\",\"against\":13,\"beat\":\"struck\",\"damage\":7,"
        "\"critical\":false,\"roll\":14,\"targets\":[\"ue-dummy\"],\"total\":18}");
    const TCHAR* const kTurnEnded = TEXT("{\"beat\":\"turn-ended\",\"member\":\"ue-fighter\",\"next\":\"ue-dummy\"}");

    bool IsNeighbour(const FIntPoint& a, const FIntPoint& b)
    {
        const int dq = b.X - a.X;
        const int dr = b.Y - a.Y;
        return (FMath::Abs(dq) + FMath::Abs(dr) + FMath::Abs(dq + dr)) / 2 == 1;
    }
} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionMovedTest, "RpgAssetLab.Motion.AMovedBeatWalksItsMemberFromTheOldCellToTheNew",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionMovedTest::RunTest(const FString& Parameters)
{
    TMap<FString, FIntPoint> cells;
    cells.Add(TEXT("ue-walker"), FIntPoint(1, 1));
    TArray<FMotionStep> steps;
    TestTrue(TEXT("a moved beat moves someone"), PlanMotion(TEXT("moved"), kMoved, cells, steps));
    TestEqual(TEXT("one step"), 1, steps.Num());
    if (steps.Num() == 1)
    {
        TestEqual(TEXT("the walker walks"), TEXT("ue-walker"), steps[0].member);
        TestEqual(TEXT("a walk"), FMotionStep::Kind::Walk, steps[0].kind);
        TestEqual(TEXT("from the cell it stood on"), FIntPoint(1, 1), steps[0].from);
        TestEqual(TEXT("to the cell the beat names"), FIntPoint(2, 1), steps[0].to);
    }
    TestEqual(TEXT("and now stands there"), FIntPoint(2, 1), cells.FindRef(TEXT("ue-walker")));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionJoinedTest, "RpgAssetLab.Motion.AJoinedBeatPlacesItsMemberAndMovesNobody",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionJoinedTest::RunTest(const FString& Parameters)
{
    TMap<FString, FIntPoint> cells;
    TArray<FMotionStep> steps;
    TestFalse(TEXT("joining is not motion"), PlanMotion(TEXT("joined_at"), kJoinedAt, cells, steps));
    TestEqual(TEXT("nothing to show"), 0, steps.Num());
    TestEqual(TEXT("but the figure has a cell"), FIntPoint(2, 0), cells.FindRef(TEXT("guard-1")));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionSwingTest, "RpgAssetLab.Motion.AMissedBeatMakesTheAttackerSwingAtItsTarget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionSwingTest::RunTest(const FString& Parameters)
{
    TMap<FString, FIntPoint> cells;
    TArray<FMotionStep> steps;
    TestTrue(TEXT("a missed beat is a swing"), PlanMotion(TEXT("missed"), kMissed, cells, steps));
    TestEqual(TEXT("one step"), 1, steps.Num());
    if (steps.Num() == 1)
    {
        TestEqual(TEXT("whoever rolled swings"), TEXT("ue-dummy"), steps[0].member);
        TestEqual(TEXT("a swing"), FMotionStep::Kind::Swing, steps[0].kind);
        TestEqual(TEXT("at the target"), TEXT("ue-fighter"), steps[0].target);
    }

    steps.Reset();
    TestTrue(TEXT("a struck beat is a swing too (assumed shape)"), PlanMotion(TEXT("struck"), kStruck, cells, steps));
    TestEqual(TEXT("one step"), 1, steps.Num());
    if (steps.Num() == 1)
    {
        TestEqual(TEXT("the attacker"), TEXT("ue-fighter"), steps[0].member);
        TestEqual(TEXT("a swing"), FMotionStep::Kind::Swing, steps[0].kind);
        TestEqual(TEXT("at the target"), TEXT("ue-dummy"), steps[0].target);
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionTurnTest, "RpgAssetLab.Motion.ATurnEndedBeatHandsTheTurnToNext",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionTurnTest::RunTest(const FString& Parameters)
{
    TMap<FString, FIntPoint> cells;
    TArray<FMotionStep> steps;
    TestTrue(TEXT("a turn-ended beat names the next member"), PlanMotion(TEXT("turn-ended"), kTurnEnded, cells, steps));
    TestEqual(TEXT("one step"), 1, steps.Num());
    if (steps.Num() == 1)
    {
        TestEqual(TEXT("the turn is now the dummy's"), TEXT("ue-dummy"), steps[0].member);
        TestEqual(TEXT("a turn"), FMotionStep::Kind::Turn, steps[0].kind);
    }
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionNothingTest, "RpgAssetLab.Motion.ABeatThatMovesNobodyPlansNothing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionNothingTest::RunTest(const FString& Parameters)
{
    const TCHAR* const beats[] = {TEXT("joined"), TEXT("tick"), TEXT("sighted"), TEXT("cells_revealed"), TEXT("bubble-formed")};
    for (const TCHAR* beat : beats)
    {
        TMap<FString, FIntPoint> cells;
        TArray<FMotionStep> steps;
        TestFalse(FString::Printf(TEXT("%s moves nobody"), beat), PlanMotion(beat, FString::Printf(TEXT("{\"beat\":\"%s\"}"), beat), cells, steps));
        TestEqual(FString::Printf(TEXT("%s plans nothing"), beat), 0, steps.Num());
    }
    TMap<FString, FIntPoint> cells;
    TArray<FMotionStep> steps;
    TestFalse(TEXT("a payload that is not JSON"), PlanMotion(TEXT("moved"), TEXT("{not json"), cells, steps));
    TestFalse(TEXT("a moved beat with no member"), PlanMotion(TEXT("moved"), TEXT("{\"beat\":\"moved\",\"position\":{\"x\":2,\"y\":1}}"), cells, steps));
    TestEqual(TEXT("nothing planned from either"), 0, steps.Num());
    TestEqual(TEXT("no figure placed"), 0, cells.Num());
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionHexLineOneStepTest, "RpgAssetLab.Motion.HexLineOfOneStepIsBothEnds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionHexLineOneStepTest::RunTest(const FString& Parameters)
{
    const TArray<FIntPoint> line = HexLine(FIntPoint(1, 1), FIntPoint(2, 1));
    TestEqual(TEXT("a cell and its neighbour"), 2, line.Num());
    if (line.Num() == 2)
    {
        TestEqual(TEXT("starts here"), FIntPoint(1, 1), line[0]);
        TestEqual(TEXT("ends there"), FIntPoint(2, 1), line[1]);
    }
    const TArray<FIntPoint> still = HexLine(FIntPoint(3, 4), FIntPoint(3, 4));
    TestEqual(TEXT("a line onto itself is the one cell"), 1, still.Num());
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMotionHexLineTest, "RpgAssetLab.Motion.HexLineIsContiguousAndEndsWhereAsked",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMotionHexLineTest::RunTest(const FString& Parameters)
{
    const FIntPoint from(-2, 16);
    const FIntPoint to(4, 9);
    const TArray<FIntPoint> line = HexLine(from, to);
    TestTrue(TEXT("a line"), line.Num() >= 2);
    if (line.Num() >= 2)
    {
        TestEqual(TEXT("starts where asked"), from, line[0]);
        TestEqual(TEXT("ends where asked"), to, line.Last());
        for (int32 i = 1; i < line.Num(); ++i)
        {
            TestTrue(FString::Printf(TEXT("cell %d follows cell %d adjacently"), i, i - 1), IsNeighbour(line[i - 1], line[i]));
        }
        // Hex distance from (-2,16) to (4,9): dq 6, dr -7, sum -1 → (6 + 7 + 1) / 2 = 7; the line is that many steps.
        TestEqual(TEXT("as many cells as the distance plus one"), 8, line.Num());
    }
    return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
