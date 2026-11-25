// The play lab: a real game, started from Unreal, on the real stack.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"

#include <memory>
#include <string>
#include <vector>

#include "RpgRecordStream.h"
#include "RpgSession.h"

#include "RpgPlayLabActor.generated.h"

class ASkeletalMeshActor;
class UAnimSequence;

/// Signs in as a sandbox player, starts a game through the lobby the way the
/// web does, builds the atlas scene, and places each figure where the record
/// says it stood. Spawned by the `rpg.play` console command; see the .cpp.
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

private:
    void Fetch();
    void OnGameStarted(bool Ok, const FString& Error, const FString& CharacterId, const FString& Session,
                       const std::vector<rpgwire::Member>& Roster);
    void OnBeat(const rpgwire::RecordBeat& Beat);
    void PlaceFigure(const FString& Member, int32 Q, int32 R);
    ASkeletalMeshActor* SpawnFigure(const FString& Member, const FVector& At);

    Target TargetToPlay;
    bool Stay = false;
    bool Started = false;
    FString Player;
    FString Session;
    FString Member;          // this player's character, seated in the session
    TMap<FString, rpgwire::Member::Kind> Kinds;
    TMap<FString, TWeakObjectPtr<ASkeletalMeshActor>> Figures;
    std::unique_ptr<rpgwire::RecordStream> Record;
    double StartedAt = 0.0;
    int32 Beats = 0;
    FTimerHandle FetchTimer;
};
