// The play lab: a real game, started from Unreal, on the real stack.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"

#include <memory>
#include <string>
#include <vector>

#include "RpgMotion.h"
#include "RpgRecordStream.h"
#include "RpgSession.h"

#include "RpgPlayLabActor.generated.h"

class ASkeletalMeshActor;
class UAnimSequence;

/// Signs in as a sandbox player, starts a game through the lobby the way the
/// web does, builds the atlas scene, and follows the record: each figure
/// stands where the record says it joined and moves where the record says it
/// moved. Verbs go out through `rpg.verb`; what they come to is only ever
/// read back from the record, so a monster's walk and the player's look the
/// same. Spawned by the `rpg.play` console command; see the .cpp.
UCLASS(NotBlueprintable)
class ARpgPlayLab final : public AActor
{
    GENERATED_BODY()

public:
    ARpgPlayLab();

    struct Target
    {
        FString Address;
        FString Player;
    };

    void ReadTarget(const Target& InTarget) { TargetToPlay = InTarget; }

    /// Stay open for looking around (the editor's own world): no frame, no exit.
    void SetStay(bool InStay) { Stay = InStay; }

    /// Begins the flow. An actor spawned into the editor's world never begins
    /// play, so the command calls this once the target is set.
    void Start();

    /// Queues a verb to send: "afford"; "move <q> <r>"; "attack <target>";
    /// "end". Sent once the game has started (at once, if it has), in order,
    /// a pause apart so each one's motion can be watched.
    void QueueVerb(const FString& Verb, const FString& A = FString(), const FString& B = FString());

    /// Saves a frame under Saved/Screenshots, named `<Name>.png`.
    void TakeFrame(const FString& Name);

    virtual void Tick(float DeltaSeconds) override;

private:
    void Fetch();
    void OnGameStarted(bool Ok, const FString& Error, const FString& CharacterId, const FString& Session,
                       const std::vector<rpgwire::Member>& Roster);
    void OnBeat(const rpgwire::RecordBeat& Beat);
    void SendVerb(const FString& Verb, const FString& A, const FString& B);
    void PlaceFigure(const FString& Member, int32 Q, int32 R);
    ASkeletalMeshActor* SpawnFigure(const FString& Member, const FVector& At);

    // Motion: steps wait per figure; one plays at a time for each.
    void BeginStep(const FString& Member, const FMotionStep& Step, double Now);
    void Pump(double Now);
    void MoveMarkerTo(const FString& WhoseTurn);
    void AdoptPlayerController();

    struct FVerb
    {
        FString Verb;
        FString A;
        FString B;
        double NotBefore = 0.0;
    };

    struct FMotion
    {
        FMotionStep Step;
        double StartedAt = 0.0;
        FVector From = FVector::ZeroVector;
        FVector To = FVector::ZeroVector;
        double Seconds = 0.0;
    };

    Target TargetToPlay;
    bool Stay = false;
    bool Started = false;
    FString Player;
    FString Session;
    FString Member;          // this player's character, seated in the session
    TMap<FString, rpgwire::Member::Kind> Kinds;
    TMap<FString, TWeakObjectPtr<ASkeletalMeshActor>> Figures;
    TMap<FString, FIntPoint> Cells;                         // as the record says
    TArray<FVerb> Verbs;                                    // waiting to be sent
    TMap<FString, TArray<FMotionStep>> Waiting;             // per figure
    TMap<FString, FMotion> Playing;                         // per figure, at most one
    TWeakObjectPtr<AActor> TurnMarker;
    TWeakObjectPtr<AActor> Viewed;                               // the controller already looking through the camera
    std::unique_ptr<rpgwire::RecordStream> Record;
    double StartedAt = 0.0;
    int32 Beats = 0;
    int32 MotionSteps = 0;
    int32 Frames = 0;
    int32 BeatsSinceVerb = -1;                             // -1 until the first verb goes out
    TArray<TPair<FString, double>> FramesDue;                 // a frame to take when its moment comes
};
