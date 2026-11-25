// The asset lab actor: builds the lab when play begins, shoots later, quits after.
#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include "RpgAssetLabActor.generated.h"

/// Builds the asset lab in the game world at play, takes one screenshot and
/// leaves. Spawned by the `rpg.lab` console command; see RpgAssetLabModule.cpp.
UCLASS(NotBlueprintable)
class ARpgAssetLab final : public AActor
{
    GENERATED_BODY()

public:
    ARpgAssetLab();

protected:
    void BeginPlay() override;

private:
    void Build();
    void Shoot();
    void ShootAfterAMoment();
    void Leave();

    TMap<TWeakObjectPtr<USkeletalMeshComponent>, FString> Idles;
    TMap<TWeakObjectPtr<USkeletalMeshComponent>, FString> Measured;
    FTimerHandle ShotTimer;
    FTimerHandle MomentTimer;
    FTimerHandle QuitTimer;
    TMap<FString, FQuat> HandAtShot;
};
