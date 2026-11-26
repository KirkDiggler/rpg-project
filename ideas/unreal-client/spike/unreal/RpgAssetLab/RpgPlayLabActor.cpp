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
//   5. follows the record as the seated member, and places each figure where
//      its `joined_at` beat says it stood.
// Figures: the player's own is the bridged fighter; a monster is, for now, the
// bridged skeleton soldier whatever its reference says (the reference is
// logged, so the mapping to real models is the next step, not a guess).
#include "RpgPlayLabActor.h"
#include "CoreMinimal.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"

#include "Components/SkeletalMeshComponent.h"


#include "Animation/AnimSequence.h"
#include "Animation/SkeletalMeshActor.h"
#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
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
    constexpr float kFetchAfterSeconds = 2.0f;

    FVector CellCenter(double Q, double R)
    {
        return FVector(kSqrt3 * (Q + R / 2.0) * kCmPerScene, 1.5 * R * kCmPerScene, 0.0);
    }

    FString Take(const std::string& Text)
    {
        return FString(UTF8_TO_TCHAR(Text.c_str()));
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
    PrimaryActorTick.bCanEverTick = false;
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
}

void ARpgPlayLab::OnBeat(const rpgwire::RecordBeat& Beat)
{
    ++Beats;
    const double Since = FPlatformTime::Seconds() - StartedAt;
    if (Beat.beat == "joined_at")
    {
        // {"beat":"joined_at","cell":{"x":2,"y":0},"member":"guard-1"}
        TSharedPtr<FJsonObject> Fields;
        if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Take(Beat.fields_json)), Fields) && Fields.IsValid())
        {
            FString Who;
            double Q = 0.0;
            double R = 0.0;
            const TSharedPtr<FJsonObject>* Cell = nullptr;
            if (Fields->TryGetStringField(TEXT("member"), Who) && Fields->TryGetObjectField(TEXT("cell"), Cell))
            {
                (*Cell)->TryGetNumberField(TEXT("x"), Q);
                (*Cell)->TryGetNumberField(TEXT("y"), R);
                PlaceFigure(Who, static_cast<int32>(Q), static_cast<int32>(R));
            }
        }
    }
    if (Beats <= 12 || Beats % 100 == 0)
    {
        FString Fields = Take(Beat.fields_json);
        if (Fields.Len() > 150)
        {
            Fields = Fields.Left(150) + TEXT("...");
        }
        UE_LOG(LogRpgPlayLab, Display, TEXT("+%.1fs beat %d: %s"), Since, Beats, *Fields);
    }
}

void ARpgPlayLab::PlaceFigure(const FString& Who, int32 Q, int32 R)
{
    if (Figures.Contains(Who))
    {
        if (ASkeletalMeshActor* Figure = Figures[Who].Get())
        {
            Figure->SetActorLocation(CellCenter(Q, R));
        }
        return;
    }
    const FVector At = CellCenter(Q, R);
    const rpgwire::Member::Kind Kind = Kinds.FindRef(Who);
    UE_LOG(LogRpgPlayLab, Display, TEXT("  %s %s stood at (%d,%d)"), KindName(Kind), *Who, Q, R);
    Figures.Add(Who, SpawnFigure(Who, At));
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
}
