// The play lab: a real game, started from Unreal, on the real stack.
//
//   UnrealEditor-Cmd <project> -game ... -ExecCmds="rpg.play <api-host:port> <player>"
//   (in the editor: the same command; it stays open to look around)
//
// What it does, in the order the web does it:
//   1. lists the signed-in player's characters and takes the first;
//   2. creates a lobby with that character, readies, starts the default dungeon;
//   3. reads the roster of the session the party was seated in;
//   4. builds the atlas scene (through the scene lab, which fetches the atlas);
//   5. follows the record as the seated member: each figure stands where its
//      `joined_at` beat says, and moves as its later beats say.
//
// Verbs go out with `rpg.verb` and come back only through the record, so the
// player's walk and a monster's are read by the same code (design.md, intent
// in, record out).
// Figures: the player's own is the bridged fighter; anything else is, for now,
// the bridged skeleton soldier whatever its reference says (the reference is
// logged, so the mapping to real models is the next step, not a guess).
#include "RpgPlayLabActor.h"

#include "CoreMinimal.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"

#include "Animation/AnimSequence.h"
#include "Animation/SkeletalMeshActor.h"
#include "Async/Async.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "RpgSceneLabActor.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include <cmath>

DEFINE_LOG_CATEGORY_STATIC(LogRpgPlayLab, Log, All);

namespace
{
    constexpr double kSqrt3 = 1.7320508075688772;
    constexpr double kCmPerScene = 100.0;
    constexpr double kWalkSeconds = 0.40;      // one cell
    constexpr double kSwingSeconds = 0.50;     // turn, lunge, back
    constexpr double kLungeCm = 40.0;
    constexpr double kMarkerHeightCm = 230.0;
    constexpr double kFacingOffsetDegrees = 0.0; // a figure's model faces +X at yaw 0
    constexpr double kFirstVerbAfterSeconds = 3.0;
    constexpr double kVerbGapSeconds = 2.5;
    constexpr int kVerbAttack = 1;             // dnd5e.api.session.v1alpha1.Verb
    constexpr int kVerbMove = 2;
    constexpr int kVerbEndTurn = 3;
    constexpr int kClockTurn = 2;              // dnd5e.api.session.v1alpha1.ClockKind

    bool gFramesOn = true; // rpg.frames 0: no screenshots, for a clean frame-time measurement

    FVector CellCenter(double Q, double R)
    {
        return FVector(kSqrt3 * (Q + R / 2.0) * kCmPerScene, 1.5 * R * kCmPerScene, 0.0);
    }

    FString Take(const std::string& Text)
    {
        return FString(UTF8_TO_TCHAR(Text.c_str()));
    }

    FString Cell(const FIntPoint& QR)
    {
        return FString::Printf(TEXT("(%d,%d)"), QR.X, QR.Y);
    }

    FRotator Facing(const FVector& Direction)
    {
        return FRotator(0.0, FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)) + kFacingOffsetDegrees, 0.0);
    }

    const TCHAR* KindName(rpgwire::Member::Kind Kind)
    {
        switch (Kind)
        {
            case rpgwire::Member::Kind::Player: return TEXT("player");
            case rpgwire::Member::Kind::Monster: return TEXT("monster");
            case rpgwire::Member::Kind::World: return TEXT("world");
            default: return TEXT("unspecified");
        }
    }
} // namespace

ARpgPlayLab::ARpgPlayLab()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ARpgPlayLab::Start()
{
    if (Started)
    {
        return;
    }
    Started = true;
    // At once, not on a timer: the editor stops ticking when its window is not
    // on screen, and a timer would wait for that. The fetch runs off the game
    // thread; what needs the game thread completes when the window is shown.
    Fetch();
}

void ARpgPlayLab::Fetch()
{
    TWeakObjectPtr<ARpgPlayLab> WeakThis(this);
    const Target TargetCopy = TargetToPlay;
    UE_LOG(LogRpgPlayLab, Display, TEXT("signing in as %s at %s"), *TargetCopy.Player, *TargetCopy.Address);
    Async(EAsyncExecution::ThreadPool, [WeakThis, TargetCopy]()
    {
        rpgwire::Session::Options Options;
        Options.address = TCHAR_TO_UTF8(*TargetCopy.Address);
        Options.player_id = TCHAR_TO_UTF8(*TargetCopy.Player);
        rpgwire::Session Session(Options);

        std::string Error;
        std::vector<rpgwire::Character> Characters;
        std::string SessionId;
        std::vector<rpgwire::Member> Roster;
        bool Ok = Session.Characters(&Characters, &Error);
        if (Ok && Characters.empty())
        {
            Ok = false;
            Error = "this player has no character; is the sandbox seed in this stack?";
        }
        if (Ok)
        {
            Ok = Session.StartGame(Characters.front().id, &SessionId, &Error);
        }
        if (Ok)
        {
            Ok = Session.Roster(SessionId, Characters.front().id, &Roster, &Error);
        }
        const FString CharacterId = Ok ? Take(Characters.front().id) : FString();
        const FString ErrorText = Take(Error);
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Ok, ErrorText, CharacterId, SessionId, Roster]()
        {
            if (ARpgPlayLab* Self = WeakThis.Get())
            {
                Self->OnGameStarted(Ok, ErrorText, CharacterId, Take(SessionId), Roster);
            }
        });
    });
}

void ARpgPlayLab::OnGameStarted(bool Ok, const FString& Error, const FString& CharacterId, const FString& InSession,
                                const std::vector<rpgwire::Member>& Roster)
{
    if (!Ok)
    {
        UE_LOG(LogRpgPlayLab, Error, TEXT("could not start a game: %s"), *Error);
        return;
    }
    Player = TargetToPlay.Player;
    Session = InSession;
    Member = CharacterId;
    StartedAt = FPlatformTime::Seconds();
    UE_LOG(LogRpgPlayLab, Display, TEXT("game started: player %s, character %s, session %s, %d in the roster"),
        *Player, *Member, *Session, static_cast<int32>(Roster.size()));
    for (const rpgwire::Member& Entry : Roster)
    {
        Kinds.Add(Take(Entry.id), Entry.kind);
        FString Detail;
        if (!Entry.monster_ref.empty())
        {
            Detail += FString::Printf(TEXT("monster %s "), *Take(Entry.monster_ref));
        }
        if (!Entry.class_ref.empty())
        {
            Detail += FString::Printf(TEXT("class %s"), *Take(Entry.class_ref));
        }
        UE_LOG(LogRpgPlayLab, Display, TEXT("  roster: %s  %s  %s"), *Take(Entry.id), KindName(Entry.kind), *Detail);
    }

    // The scene the atlas describes, built by the scene lab as this member.
    UWorld* World = GetWorld();
    ARpgSceneLab* Scene = World->SpawnActor<ARpgSceneLab>();
    ARpgSceneLab::Target AtlasTarget;
    AtlasTarget.Address = TargetToPlay.Address;
    AtlasTarget.Session = Session;
    AtlasTarget.Player = Player;
    AtlasTarget.Member = Member;
    Scene->ReadTarget(AtlasTarget);
    Scene->SetStay(true);
    Scene->Start();

    // The record, as the seated member: where everyone stood, then everything that happens.
    rpgwire::RecordStream::Options Options;
    Options.address = TCHAR_TO_UTF8(*TargetToPlay.Address);
    Options.session = TCHAR_TO_UTF8(*Session);
    Options.member = TCHAR_TO_UTF8(*Member);
    Options.player_id = TCHAR_TO_UTF8(*Player);
    Record = std::make_unique<rpgwire::RecordStream>(Options,
        [WeakThis = TWeakObjectPtr<ARpgPlayLab>(this)](const rpgwire::RecordBeat& Beat)
        {
            AsyncTask(ENamedThreads::GameThread, [WeakThis, Beat]()
            {
                if (ARpgPlayLab* Self = WeakThis.Get())
                {
                    Self->OnBeat(Beat);
                }
            });
        },
        [WeakThis = TWeakObjectPtr<ARpgPlayLab>(this)](const rpgwire::StreamEnd& End)
        {
            const int32 Code = End.code;
            const FString Message = Take(End.message);
            const unsigned long long Events = End.events;
            AsyncTask(ENamedThreads::GameThread, [WeakThis, Code, Message, Events]()
            {
                if (ARpgPlayLab* Self = WeakThis.Get())
                {
                    UE_LOG(LogRpgPlayLab, Display, TEXT("the record ended: code %d, %s, %llu events"), Code, *Message, Events);
                    Self->Record.reset();
                }
            });
        });
    UE_LOG(LogRpgPlayLab, Display, TEXT("following the record as %s"), *Member);

    // The idle window: the first seconds of a game standing still.
    Open(Idle, TEXT("idle"), FPlatformTime::Seconds() + 0.5, 2.0);

    // The turn marker's shape, loaded now: the first `turn-ended` beat is no
    // time to start loading a package.
    LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    // Verbs asked for before the game existed go now, in order, a pause apart.
    const double Now = FPlatformTime::Seconds();
    for (int32 i = 0; i < Verbs.Num(); ++i)
    {
        Verbs[i].NotBefore = Now + kFirstVerbAfterSeconds + i * kVerbGapSeconds;
    }
    if (Verbs.Num() > 0)
    {
        UE_LOG(LogRpgPlayLab, Display, TEXT("%d verb(s) queued; the first goes in %.1fs"), Verbs.Num(), kFirstVerbAfterSeconds);
    }
}

void ARpgPlayLab::OnBeat(const rpgwire::RecordBeat& Beat)
{
    ++Beats;
    ++BeatsThisTick;
    const double Since = FPlatformTime::Seconds() - StartedAt;
    const FString Fields = Take(Beat.fields_json);

    TArray<FMotionStep> steps;
    PlanMotion(Take(Beat.beat), Fields, Cells, steps);
    if (Beat.beat == "joined_at")
    {
        // {"beat":"joined_at","cell":{"x":2,"y":0},"member":"guard-1"}: the figure begins here.
        TSharedPtr<FJsonObject> json;
        FString who;
        if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Fields), json) && json.IsValid() &&
            json->TryGetStringField(TEXT("member"), who))
        {
            if (const FIntPoint* At = Cells.Find(who))
            {
                PlaceFigure(who, At->X, At->Y);
            }
        }
    }
    for (const FMotionStep& step : steps)
    {
        Waiting.FindOrAdd(step.member).Add(step);
    }

    // The record's opening, and then everything that follows a verb: the
    // evidence of what the engine did with it, beat by beat.
    const bool bAfterAVerb = BeatsSinceVerb >= 0;
    if (Beats <= 12 || (bAfterAVerb && BeatsSinceVerb <= 150))
    {
        const FString act = Beat.act.empty() ? FString() : FString::Printf(TEXT(" [%s]"), *Take(Beat.act).Left(14));
        UE_LOG(LogRpgPlayLab, Display, TEXT("+%.1fs beat %d%s %s%s"), Since, Beats, *act,
            *Fields.Left(170), Fields.Len() > 170 ? TEXT("...") : TEXT(""));
    }
    if (bAfterAVerb)
    {
        ++BeatsSinceVerb;
    }
    if (Beats % 25 == 0)
    {
        UE_LOG(LogRpgPlayLab, Display, TEXT("BEATS handled %d, motion steps %d, frames %d"), Beats, MotionSteps, Frames);
    }
}

void ARpgPlayLab::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const double now = FPlatformTime::Seconds();
    AdoptPlayerController();
    Pump(now);
    Sample(Idle, TEXT("idle"), DeltaSeconds, now);
    Sample(Burst, TEXT("burst"), DeltaSeconds, now);
    BeatsThisTick = 0;
    FramesThisTick = 0;
}

void ARpgPlayLab::Open(FWindow& Window, const TCHAR* Name, double Now, double Seconds)
{
    Window = FWindow();
    Window.Open = true;
    Window.Since = Now;
    Window.Seconds = Seconds;
    UE_LOG(LogRpgPlayLab, Display, TEXT("FRAMES %s window opens"), Name);
}

void ARpgPlayLab::Sample(FWindow& Window, const TCHAR* Name, double DeltaSeconds, double Now)
{
    if (!Window.Open || Now < Window.Since)
    {
        return;
    }
    Window.Deltas.Add(DeltaSeconds * 1000.0);
    Window.Beats += BeatsThisTick;
    if (Window.BeatsInFirstFrame == 0 && BeatsThisTick > 0)
    {
        Window.BeatsInFirstFrame = BeatsThisTick;
    }
    Window.FramesShot += FramesThisTick;
    if (Now - Window.Since >= Window.Seconds)
    {
        Close(Window, Name);
    }
}

void ARpgPlayLab::Close(FWindow& Window, const TCHAR* Name)
{
    if (!Window.Open)
    {
        return;
    }
    LogWindow(Name, Window);
    Window = FWindow();
}

void ARpgPlayLab::LogWindow(const TCHAR* Name, const FWindow& Window)
{
    if (Window.Deltas.Num() == 0)
    {
        UE_LOG(LogRpgPlayLab, Display, TEXT("FRAMES %s: no frames sampled"), Name);
        return;
    }
    TArray<double> sorted = Window.Deltas;
    sorted.Sort();
    double sum = 0.0;
    for (const double delta : sorted)
    {
        sum += delta;
    }
    const int32 n = sorted.Num();
    const double p99 = sorted[FMath::Clamp(static_cast<int32>(0.99 * (n - 1)), 0, n - 1)];
    const double p50 = sorted[n / 2];
    UE_LOG(LogRpgPlayLab, Display, TEXT("FRAMES %s: %.1fs, %d frame(s), mean %.2f ms, p50 %.2f ms, p99 %.2f ms, max %.2f ms; %d beat(s), %d of them in one frame; %d frame(s) shot"),
        Name, sum / 1000.0, n, sum / n, p50, p99, sorted[n - 1], Window.Beats, Window.BeatsInFirstFrame, Window.FramesShot);
}

void ARpgPlayLab::AdoptPlayerController()
{
    // In a game, the controller may not exist when the scene is built (a
    // command-line `rpg.play` runs before the game mode begins play), so the
    // view is claimed here, once, as soon as there is one to claim it with.
    UWorld* world = GetWorld();
    if (!world || !world->IsGameWorld())
    {
        return;
    }
    APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
    if (!controller || controller == Viewed.Get())
    {
        return;
    }
    AActor* camera = nullptr;
    for (TActorIterator<ARpgSceneLab> it(world); it && !camera; ++it)
    {
        camera = it->Camera();
    }
    if (!camera)
    {
        return; // the scene is not built yet; the next tick asks again
    }
    Viewed = controller;
    controller->SetViewTarget(camera);
    controller->bShowMouseCursor = true;
    controller->DefaultMouseCursor = EMouseCursor::Crosshairs;
    FInputModeGameAndUI input;
    input.SetHideCursorDuringCapture(false);
    controller->SetInputMode(input);
    if (APawn* pawn = controller->GetPawn())
    {
        pawn->SetActorHiddenInGame(true); // the map's default pawn is not part of the scene
    }
    UE_LOG(LogRpgPlayLab, Display, TEXT("VIEW the player controller looks through the scene's camera; the cursor is shown"));
}

void ARpgPlayLab::Pump(double Now)
{
    // Frames whose moment has come.
    for (int32 i = FramesDue.Num() - 1; i >= 0; --i)
    {
        if (FramesDue[i].Value <= Now)
        {
            const FString name = FramesDue[i].Key;
            FramesDue.RemoveAt(i);
            TakeFrame(name);
        }
    }

    // Verbs, in order, when due.
    while (Verbs.Num() > 0 && Verbs[0].NotBefore <= Now)
    {
        const FVerb due = Verbs[0];
        Verbs.RemoveAt(0);
        if (BeatsSinceVerb < 0)
        {
            BeatsSinceVerb = 0;
        }
        // A swing or an ended turn can run other members' turns inside the
        // call: the record of it arrives at once.
        if (due.Verb == TEXT("attack") || due.Verb == TEXT("end"))
        {
            Close(Burst, TEXT("burst"));
            Open(Burst, TEXT("burst"), Now, 3.0);
        }
        SendVerb(due.Verb, due.A, due.B);
    }

    // The step each figure is on.
    for (auto It = Playing.CreateIterator(); It; ++It)
    {
        FMotion& motion = It.Value();
        ASkeletalMeshActor* figure = Figures.FindRef(It.Key()).Get();
        if (!figure)
        {
            It.RemoveCurrent();
            continue;
        }
        const double t = motion.Seconds > 0.0 ? FMath::Clamp((Now - motion.StartedAt) / motion.Seconds, 0.0, 1.0) : 1.0;
        if (motion.Step.kind == FMotionStep::Kind::Walk)
        {
            figure->SetActorLocation(FMath::Lerp(motion.From, motion.To, t));
            const FVector direction = motion.To - motion.From;
            if (!direction.IsNearlyZero())
            {
                figure->SetActorRotation(Facing(direction));
            }
        }
        else
        {
            // A swing: the figure already faces its target; it leans out and back.
            figure->SetActorLocation(motion.From + motion.To * (FMath::Sin(t * PI) * kLungeCm));
        }
        if (t >= 1.0)
        {
            const double took = Now - motion.StartedAt;
            figure->SetActorLocation(motion.Step.kind == FMotionStep::Kind::Walk ? motion.To : motion.From);
            UE_LOG(LogRpgPlayLab, Display, TEXT("MOTION %s %s after %.2fs at %s"), *It.Key(),
                motion.Step.kind == FMotionStep::Kind::Walk ? TEXT("arrived") : TEXT("settled"), took,
                *figure->GetActorLocation().ToCompactString());
            if (motion.Step.kind == FMotionStep::Kind::Walk && It.Key() == Member)
            {
                TakeFrame(FString::Printf(TEXT("%s-arrived-%d"), *It.Key(), ++Frames));
            }
            It.RemoveCurrent();
        }
    }

    // A waiting step begins for a figure with none playing.
    for (auto It = Waiting.CreateIterator(); It; ++It)
    {
        if (It.Value().Num() == 0 || Playing.Contains(It.Key()))
        {
            continue;
        }
        const FMotionStep step = It.Value()[0];
        It.Value().RemoveAt(0);
        const FString who = It.Key();
        if (It.Value().Num() == 0)
        {
            It.RemoveCurrent();
        }
        BeginStep(who, step, Now);
    }
}

void ARpgPlayLab::BeginStep(const FString& Who, const FMotionStep& Step, double Now)
{
    ++MotionSteps;
    TWeakObjectPtr<ASkeletalMeshActor>& slot = Figures.FindOrAdd(Who);
    if (!slot.IsValid())
    {
        if (const FIntPoint* at = Cells.Find(Who))
        {
            PlaceFigure(Who, at->X, at->Y);
        }
    }

    if (Step.kind == FMotionStep::Kind::Turn)
    {
        UE_LOG(LogRpgPlayLab, Display, TEXT("MARKER the turn is now %s's"), *Who);
        MoveMarkerTo(Who);
        // A moment later: the shape is loaded at game start, and the frame is
        // worth taking only once the marker has settled above the figure.
        FramesDue.Emplace(FString::Printf(TEXT("turn-%d"), ++Frames), Now + 0.6);
        return;
    }

    ASkeletalMeshActor* figure = slot.Get();
    if (!figure)
    {
        UE_LOG(LogRpgPlayLab, Error, TEXT("MOTION %s has no figure to move"), *Who);
        return;
    }
    FMotion motion;
    motion.Step = Step;
    motion.StartedAt = Now;
    motion.From = figure->GetActorLocation();
    if (Step.kind == FMotionStep::Kind::Walk)
    {
        motion.To = CellCenter(Step.to.X, Step.to.Y);
        motion.Seconds = kWalkSeconds;
        UE_LOG(LogRpgPlayLab, Display, TEXT("MOTION %s walks %s -> %s over %.2fs, from %s"), *Who, *Cell(Step.from),
            *Cell(Step.to), kWalkSeconds, *motion.From.ToCompactString());
        // A frame costs the frame it is taken in, so only the player's own steps are shot.
        if (Who == Member)
        {
            TakeFrame(FString::Printf(TEXT("%s-walk-%d"), *Who, ++Frames));
        }
    }
    else
    {
        TWeakObjectPtr<ASkeletalMeshActor> target = Figures.FindRef(Step.target);
        FVector toward = target.IsValid() ? target->GetActorLocation() - motion.From : FVector::ZeroVector;
        toward.Z = 0.0;
        motion.To = toward.IsNearlyZero() ? figure->GetActorForwardVector() : toward.GetSafeNormal();
        motion.Seconds = kSwingSeconds;
        figure->SetActorRotation(Facing(motion.To));
        UE_LOG(LogRpgPlayLab, Display, TEXT("MOTION %s swings at %s over %.2fs: faces %s, lunges %.0f cm"), *Who,
            *Step.target, kSwingSeconds, *motion.To.ToCompactString(), kLungeCm);
        TakeFrame(FString::Printf(TEXT("%s-swing-%d"), *Who, ++Frames));
    }
    Playing.Add(Who, motion);
}

void ARpgPlayLab::MoveMarkerTo(const FString& WhoseTurn)
{
    UWorld* world = GetWorld();
    if (!world)
    {
        return;
    }
    if (!TurnMarker.IsValid())
    {
        // As the scene lab builds its walls: mobility on the actor, the shape,
        // and a tinted basic material, so it renders like they do.
        AStaticMeshActor* marker = world->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
        marker->SetMobility(EComponentMobility::Movable);
        marker->SetActorScale3D(FVector(0.9, 0.9, 0.02));
        UStaticMeshComponent* component = marker->GetStaticMeshComponent();
        component->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
        component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (UMaterial* shape = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
        {
            UMaterialInstanceDynamic* tint = UMaterialInstanceDynamic::Create(shape, this);
            tint->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.85f, 0.1f));
            component->SetMaterial(0, tint);
        }
        TurnMarker = marker;
    }
    AActor* marker = TurnMarker.Get();
    TWeakObjectPtr<ASkeletalMeshActor> figure = Figures.FindRef(WhoseTurn);
    const FIntPoint* cell = Cells.Find(WhoseTurn);
    if (!marker || (!figure.IsValid() && !cell))
    {
        return;
    }
    FVector at = figure.IsValid() ? figure->GetActorLocation() : CellCenter(cell->X, cell->Y);
    at.Z = kMarkerHeightCm;
    marker->SetActorLocation(at);
}

void ARpgPlayLab::PlaceFigure(const FString& Who, int32 Q, int32 R)
{
    if (ASkeletalMeshActor* figure = Figures.FindRef(Who).Get())
    {
        figure->SetActorLocation(CellCenter(Q, R));
        return;
    }
    const FVector at = CellCenter(Q, R);
    const rpgwire::Member::Kind kind = Kinds.FindRef(Who);
    UE_LOG(LogRpgPlayLab, Display, TEXT("  %s %s stood at (%d,%d)"), KindName(kind), *Who, Q, R);
    Figures.Add(Who, SpawnFigure(Who, at));
}

void ARpgPlayLab::QueueVerb(const FString& Verb, const FString& A, const FString& B)
{
    FVerb queued;
    queued.Verb = Verb;
    queued.A = A;
    queued.B = B;
    queued.NotBefore = FPlatformTime::Seconds() + 0.05 + Verbs.Num() * kVerbGapSeconds;
    Verbs.Add(queued);
    UE_LOG(LogRpgPlayLab, Display, TEXT("VERB queued: %s %s %s%s"), *Verb, *A, *B, Started ? TEXT("") : TEXT(" (waits for the game)"));
}

void ARpgPlayLab::SendVerb(const FString& Verb, const FString& A, const FString& B)
{
    // The path is named here, on the game thread, where the figures' cells are.
    const FIntPoint here = Cells.FindRef(Member);
    const FIntPoint there(A.IsEmpty() ? 0 : FCString::Atoi(*A), B.IsEmpty() ? 0 : FCString::Atoi(*B));
    const TArray<FIntPoint> line = HexLine(here, there);
    const Target target = TargetToPlay;
    const FString session = Session;
    const FString member = Member;
    const FString who = Member;

    Async(EAsyncExecution::ThreadPool, [target, session, member, who, Verb, A, B, here, there, line]()
    {
        rpgwire::Session::Options options;
        options.address = TCHAR_TO_UTF8(*target.Address);
        options.player_id = TCHAR_TO_UTF8(*target.Player);
        rpgwire::Session session_(options);
        const std::string run = TCHAR_TO_UTF8(*session);
        const std::string me = TCHAR_TO_UTF8(*member);

        std::string error;
        rpgwire::Afforded afforded;
        FString report;
        if (!session_.Afford(run, me, &afforded, &error))
        {
            report = FString::Printf(TEXT("VERB %s %s %s: Afford refused: \"%s\""), *Verb, *A, *B, *Take(error));
        }
        else if (Verb == TEXT("afford"))
        {
            report = FString::Printf(TEXT("VERB afford: clock %d, %d declaration(s)"), afforded.clock,
                static_cast<int32>(afforded.declarations.size()));
            for (const rpgwire::Declaration& declaration : afforded.declarations)
            {
                report += FString::Printf(TEXT("\n  AFFORD verb %d slot %d available %d%s%s"), declaration.verb, declaration.slot,
                    declaration.available ? 1 : 0, declaration.why.empty() ? TEXT("") : TEXT(" why: "), *Take(declaration.why));
            }
        }
        else if (Verb == TEXT("move"))
        {
            const bool turnClock = afforded.clock == kClockTurn;
            const std::string declaration = turnClock ? rpgwire::DeclarationFor(afforded, kVerbMove) : std::string();
            if (turnClock && declaration.empty())
            {
                report = FString::Printf(TEXT("VERB move %s: refused in Afford on the turn clock, no Move to declare"), *Cell(there));
            }
            else
            {
                // The path names the cells to step onto: the engine refuses one
                // that begins on the cell the member stands on.
                std::vector<rpgwire::Cell> path;
                for (int32 i = line.Num() > 1 ? 1 : 0; i < line.Num(); ++i)
                {
                    path.emplace_back(line[i].X, line[i].Y);
                }
                rpgwire::Walk walk;
                if (session_.Move(run, me, path, declaration, &walk, &error))
                {
                    report = FString::Printf(TEXT("VERB move %s -> %s: %d of %d cell(s) walked, stop reason: %s%s%s%s"), *Cell(here),
                        *Cell(there), static_cast<int32>(walk.steps.size()), static_cast<int32>(path.size()),
                        walk.stop_reason.empty() ? TEXT("none") : *Take(walk.stop_reason),
                        walk.joined_combat ? TEXT(", joined combat") : TEXT(""),
                        walk.continues ? TEXT(", the path continues") : TEXT(""), turnClock ? TEXT(" (turn clock, declared)") : TEXT(" (world clock)"));
                }
                else
                {
                    report = FString::Printf(TEXT("VERB move %s -> %s: refused: \"%s\""), *Cell(here), *Cell(there), *Take(error));
                }
            }
        }
        else if (Verb == TEXT("attack"))
        {
            const std::string declaration = rpgwire::DeclarationFor(afforded, kVerbAttack);
            if (declaration.empty())
            {
                report = TEXT("VERB attack: refused in Afford: no available Attack");
                for (const rpgwire::Declaration& row : afforded.declarations)
                {
                    if (row.verb == kVerbAttack)
                    {
                        report += FString::Printf(TEXT(" (verb %d: %s)"), row.verb, *Take(row.why));
                    }
                }
            }
            else
            {
                rpgwire::Swing swing;
                if (session_.Attack(run, me, TCHAR_TO_UTF8(*A), declaration, &swing, &error))
                {
                    report = FString::Printf(TEXT("VERB attack %s: roll %d, total %d against %d, %s%s, damage %d"), *A, swing.roll,
                        swing.total, swing.against, swing.hit ? TEXT("hit") : TEXT("missed"), swing.critical ? TEXT(", critical") : TEXT(""),
                        swing.damage);
                }
                else
                {
                    report = FString::Printf(TEXT("VERB attack %s: refused: \"%s\""), *A, *Take(error));
                }
            }
        }
        else if (Verb == TEXT("end"))
        {
            const std::string declaration = rpgwire::DeclarationFor(afforded, kVerbEndTurn);
            if (declaration.empty())
            {
                report = FString::Printf(TEXT("VERB end: refused in Afford on clock %d: no available End Turn"), afforded.clock);
            }
            else
            {
                std::string next;
                bool wrapped = false;
                if (session_.EndTurn(run, me, declaration, &next, &wrapped, &error))
                {
                    report = FString::Printf(TEXT("VERB end: the turn is now %s's, round wrapped %d"), *Take(next), wrapped);
                }
                else
                {
                    report = FString::Printf(TEXT("VERB end: refused: \"%s\""), *Take(error));
                }
            }
        }
        else
        {
            report = FString::Printf(TEXT("VERB %s: not a verb this lab knows (afford, move <q> <r>, attack <target>, end)"), *Verb);
        }

        AsyncTask(ENamedThreads::GameThread, [report]()
        {
            UE_LOG(LogRpgPlayLab, Display, TEXT("%s"), *report);
        });
    });
}

void ARpgPlayLab::TakeFrame(const FString& Name)
{
    if (!gFramesOn)
    {
        return;
    }
    FScreenshotRequest::RequestScreenshot(Name + TEXT(".png"), false, false);
    ++FramesThisTick;
    UE_LOG(LogRpgPlayLab, Display, TEXT("FRAME %s"), *Name);
    // Where each figure is in that frame, in its pixels, so the picture can be
    // read by position rather than by eye.
    APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
    UWorld* world = GetWorld();
    if (!controller || !world || !world->IsGameWorld())
    {
        return;
    }
    const double now = FPlatformTime::Seconds();
    for (const auto& pair : Figures)
    {
        const ASkeletalMeshActor* figure = pair.Value.Get();
        if (!figure)
        {
            continue;
        }
        FVector2D screen = FVector2D::ZeroVector;
        if (controller->ProjectWorldLocationToScreen(figure->GetActorLocation() + FVector(0.0, 0.0, 90.0), screen))
        {
            const FIntPoint* cell = Cells.Find(pair.Key);
            UE_LOG(LogRpgPlayLab, Display, TEXT("  AT %s %s moving %d -> pixel (%.0f, %.0f), world %s"), *pair.Key,
                cell ? *Cell(*cell) : TEXT("(?)"), Playing.Contains(pair.Key) ? 1 : 0, screen.X, screen.Y,
                *figure->GetActorLocation().ToCompactString());
        }
    }
    if (TurnMarker.IsValid())
    {
        FVector2D screen = FVector2D::ZeroVector;
        controller->ProjectWorldLocationToScreen(TurnMarker->GetActorLocation(), screen);
        UE_LOG(LogRpgPlayLab, Display, TEXT("  AT marker -> pixel (%.0f, %.0f)"), screen.X, screen.Y);
    }
    UE_LOG(LogRpgPlayLab, Display, TEXT("  AT frame taken %.3fs"), now);
}

ASkeletalMeshActor* SpawnFigureImpl(UWorld* World, const FVector& At, const TCHAR* MeshPath, const TCHAR* IdlePath)
{
    ASkeletalMeshActor* Figure = World->SpawnActor<ASkeletalMeshActor>(At, FRotator(0.0, 180.0, 0.0));
    USkeletalMeshComponent* Component = Figure->GetSkeletalMeshComponent();
    if (USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, MeshPath))
    {
        Component->SetSkeletalMeshAsset(Mesh);
    }
    if (UAnimSequence* Idle = LoadObject<UAnimSequence>(nullptr, IdlePath))
    {
        Component->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        Component->SetAnimation(Idle);
        Component->Play(true);
        Component->SetUpdateAnimationInEditor(true);
    }
    return Figure;
}

ASkeletalMeshActor* ARpgPlayLab::SpawnFigure(const FString& Who, const FVector& At)
{
    // Until references map to models: the player's own character is the bridged
    // fighter, anything else the bridged skeleton soldier.
    if (Who == Member)
    {
        return SpawnFigureImpl(GetWorld(), At,
            TEXT("/Game/WebBridgeCap/Characters/fighter.fighter"),
            TEXT("/Game/WebBridgeCap/Characters/fighterRoot_Idle_Relaxed.fighterRoot_Idle_Relaxed"));
    }
    return SpawnFigureImpl(GetWorld(), At,
        TEXT("/Game/WebBridgeCap/Characters/skeleton-soldier-01.skeleton-soldier-01"),
        TEXT("/Game/WebBridgeCap/Characters/skeleton-soldier-01Root_Idle_Relaxed.skeleton-soldier-01Root_Idle_Relaxed"));
}

namespace
{
    ARpgPlayLab* ThePlayLab(UWorld* World)
    {
        ARpgPlayLab* found = nullptr;
        for (TActorIterator<ARpgPlayLab> it(World); it && !found; ++it)
        {
            found = *it;
        }
        return found;
    }

    FAutoConsoleCommand GRpgPlayCommand(
        TEXT("rpg.play"),
        TEXT("Start a real game as a sandbox player, build its scene, follow its record. rpg.play <api-host:port> <player>"),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
            const bool bEditor = World == nullptr;
            if (!World && GEditor)
            {
                World = GEditor->GetEditorWorldContext().World();
            }
            if (!World)
            {
                UE_LOG(LogRpgPlayLab, Error, TEXT("rpg.play: no world to build in"));
                return;
            }
            if (Args.Num() < 2)
            {
                UE_LOG(LogRpgPlayLab, Error, TEXT("usage: rpg.play <api-host:port> <player>"));
                return;
            }
            ARpgPlayLab* Lab = World->SpawnActor<ARpgPlayLab>();
            ARpgPlayLab::Target TargetToPlay;
            TargetToPlay.Address = Args[0];
            TargetToPlay.Player = Args[1];
            Lab->SetStay(bEditor);
            Lab->ReadTarget(TargetToPlay);
            Lab->Start();
            UE_LOG(LogRpgPlayLab, Display, TEXT("playing in the %s world"), bEditor ? TEXT("editor") : TEXT("game"));
        }));

    FAutoConsoleCommand GRpgVerbCommand(
        TEXT("rpg.verb"),
        TEXT("Send a verb as the seated character: rpg.verb afford | move <q> <r> | attack <target> | end. What it comes to is read from the record."),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
            if (!World && GEditor)
            {
                World = GEditor->GetEditorWorldContext().World();
            }
            ARpgPlayLab* Lab = ThePlayLab(World);
            if (!Lab)
            {
                UE_LOG(LogRpgPlayLab, Error, TEXT("rpg.verb: no game yet; rpg.play <api-host:port> <player> first"));
                return;
            }
            if (Args.Num() < 1)
            {
                UE_LOG(LogRpgPlayLab, Error, TEXT("usage: rpg.verb afford | move <q> <r> | attack <target> | end"));
                return;
            }
            Lab->QueueVerb(Args[0], Args.Num() > 1 ? Args[1] : FString(), Args.Num() > 2 ? Args[2] : FString());
        }));

    FAutoConsoleCommand GRpgFramesCommand(
        TEXT("rpg.frames"),
        TEXT("rpg.frames 0|1: whether the lab takes evidence frames (a frame costs the frame it is taken in)."),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            if (Args.Num() >= 1)
            {
                gFramesOn = FCString::Atoi(*Args[0]) != 0;
            }
            UE_LOG(LogRpgPlayLab, Display, TEXT("frames %s"), gFramesOn ? TEXT("on") : TEXT("off"));
        }));

    FAutoConsoleCommand GRpgFrameCommand(
        TEXT("rpg.frame"),
        TEXT("Save a frame of the game as <name>.png under Saved/Screenshots."),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
            if (!World && GEditor)
            {
                World = GEditor->GetEditorWorldContext().World();
            }
            ARpgPlayLab* Lab = ThePlayLab(World);
            if (!Lab || Args.Num() < 1)
            {
                UE_LOG(LogRpgPlayLab, Error, TEXT("usage: rpg.frame <name>, after rpg.play"));
                return;
            }
            Lab->TakeFrame(Args[0]);
        }));
}
