// The scene lab: the web's view of an atlas, built in Unreal from the atlas.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SharedPointer.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"

#include <memory>

#include "RpgSession.h"

#include "RpgSceneLabActor.generated.h"

class UProceduralMeshComponent;
class ACameraActor;

/// Fetches a session's atlas over the wire as a seated member, builds the
/// scene the web shows from it, shoots one frame and leaves. Spawned by the
/// `rpg.scene` console command; see RpgSceneLabActor.cpp.
UCLASS(NotBlueprintable)
class ARpgSceneLab final : public AActor
{
    GENERATED_BODY()

public:
    ARpgSceneLab();

    struct Target
    {
        FString Address;
        FString Session;
        FString Member;
        FString Player;
    };

    void ReadTarget(const Target& InTarget) { TargetToRead = InTarget; }

protected:
    void BeginPlay() override;

private:
    void FetchAtlas();
    void OnAtlas(bool Ok, const FString& Error, const std::shared_ptr<const rpgwire::Atlas>& Atlas);
    void Build(const struct FSceneAtlas& Atlas);
    void Shoot();
    void ShootAngled();
    void ShootAngledNow();
    void Leave();

    Target TargetToRead;
    int32 Built = 0;
    ACameraActor* TopCamera = nullptr;
    ACameraActor* AngleCamera = nullptr;
    FTimerHandle FetchTimer;
    FTimerHandle QuitTimer;
};
