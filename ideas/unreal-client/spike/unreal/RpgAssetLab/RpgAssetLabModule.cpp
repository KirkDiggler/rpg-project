// RpgAssetLab: the asset lab for the Unreal client's proof project.
//
// In a GAME run, `rpg.lab` builds one row of evidence in the current world,
// looks at it through a camera, takes a screenshot a few seconds later and
// quits, so the whole thing runs unattended and every animation has ticked:
//
//   UnrealEditor-Cmd <project> -game -RenderOffscreen -unattended -NoSound \
//     -windowed -ResX=1920 -ResY=1080 -ExecCmds="rpg.lab"
//
// Left to right: two characters that crossed the GLB -> FBX bridge, their
// idle playing; the raw Unreal-format copy of the one the fighter was built
// from; the bridged wall piece and rug; a raw prop from the pack.
#include "RpgAssetLabActor.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"


#include "Animation/AnimSequence.h"
#include "Animation/SkeletalMeshActor.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HighResScreenshot.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogRpgAssetLab, Log, All);

namespace
{
    // The left hand's bone, whatever the casing: the bridge taught that Unreal
    // capitalizes a mesh's bone names and not a track's.
    int32 LeftHandBone(const FReferenceSkeleton& Ref)
    {
        const int32 Exact = Ref.FindBoneIndex(TEXT("Hand_l"));
        if (Exact != INDEX_NONE)
        {
            return Exact;
        }
        for (int32 Bone = 0; Bone < Ref.GetNum(); ++Bone)
        {
            if (Ref.GetBoneName(Bone).ToString().Contains(TEXT("hand_l"), ESearchCase::IgnoreCase))
            {
                return Bone;
            }
        }
        return INDEX_NONE;
    }
}

namespace
{
    constexpr float SHOT_AFTER_SECONDS = 10.0f;
    constexpr float QUIT_AFTER_SHOT_SECONDS = 4.0f;

    template <class T>
    T* Asset(const TCHAR* Path)
    {
        T* Loaded = LoadObject<T>(nullptr, Path);
        if (!Loaded)
        {
            UE_LOG(LogRpgAssetLab, Error, TEXT("missing asset %s"), Path);
        }
        return Loaded;
    }
}

ARpgAssetLab::ARpgAssetLab()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARpgAssetLab::BeginPlay()
{
    Super::BeginPlay();
    Build();
    GetWorldTimerManager().SetTimer(ShotTimer, this, &ARpgAssetLab::Shoot, SHOT_AFTER_SECONDS, false);
}

namespace
{
    bool GNativeRow = false; // `rpg.lab native`: the stock soldier with clips from the animation packs
}

void ARpgAssetLab::Build()
{
    UWorld* World = GetWorld();

    if (GNativeRow)
    {
        BuildNativeRow(World);
        return;
    }

    auto PlaceStatic = [World](const TCHAR* Path, const FVector& At, float Yaw = 0.0f, float Scale = 1.0f)
    {
        AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(At, FRotator(0.0f, Yaw, 0.0f));
        Actor->SetMobility(EComponentMobility::Movable);
        Actor->SetActorScale3D(FVector(Scale));
        if (UStaticMesh* Mesh = Asset<UStaticMesh>(Path))
        {
            Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        }
    };

    // The row faces -Y, where the camera stands.
    auto PlaceCharacter = [this, World](const TCHAR* Path, const TCHAR* Idle, const FVector& At, const FString& Label)
    {
        ASkeletalMeshActor* Actor = World->SpawnActor<ASkeletalMeshActor>(At, FRotator(0.0f, 180.0f, 0.0f));
        USkeletalMeshComponent* Component = Actor->GetSkeletalMeshComponent();
        if (USkeletalMesh* Mesh = Asset<USkeletalMesh>(Path))
        {
            Component->SetSkeletalMeshAsset(Mesh);
        }
        if (UAnimSequence* Animation = Idle ? Asset<UAnimSequence>(Idle) : nullptr)
        {
            Component->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            Component->SetAnimation(Animation);
            Component->Play(true);
            Idles.Add(Component, Label);
        }
        Measured.Add(Component, Label);
    };

    PlaceStatic(TEXT("/Engine/BasicShapes/Plane.Plane"), FVector(0.0, 60.0, 0.0), 0.0f, 40.0f);

    ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0.0, 0.0, 500.0), FRotator(-48.0f, 32.0f, 0.0f));
    Sun->GetLightComponent()->SetIntensity(6.0f);
    if (UClass* SkySphereClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Engine/EngineSky/BP_Sky_Sphere.BP_Sky_Sphere_C")))
    {
        World->SpawnActor<AActor>(SkySphereClass, FVector::ZeroVector, FRotator::ZeroRotator);
    }
    ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.0, 0.0, 300.0), FRotator::ZeroRotator);
    Sky->GetLightComponent()->SetIntensity(1.0f);

    PlaceStatic(TEXT("/Game/WebBridge/Props/Crypt_Rug_01.Crypt_Rug_01"), FVector(0.0, 60.0, 0.2));
    PlaceStatic(TEXT("/Game/WebBridge/Env/Crypt_Wall_Base_01.Crypt_Wall_Base_01"), FVector(0.0, 260.0, 0.0), 180.0f);
    // First attempt (bone names left lower-case, no track found its bone) beside the
    // corrected fighter; the corrected soldier beside a copy of it with no clip,
    // because its rest pose looks like an idle and only a comparison tells.
    PlaceCharacter(TEXT("/Game/WebBridge/Characters/fighter.fighter"),
                   TEXT("/Game/WebBridge/Characters/fighterRoot_Idle_Relaxed.fighterRoot_Idle_Relaxed"),
                   FVector(-340.0, 60.0, 0.0), TEXT("fighter, first attempt"));
    PlaceCharacter(TEXT("/Game/WebBridgeCap/Characters/fighter.fighter"),
                   TEXT("/Game/WebBridgeCap/Characters/fighterRoot_Idle_Relaxed.fighterRoot_Idle_Relaxed"),
                   FVector(-170.0, 60.0, 0.0), TEXT("fighter, bones matched"));
    PlaceCharacter(TEXT("/Game/WebBridgeCap/Characters/skeleton-soldier-01.skeleton-soldier-01"),
                   nullptr, FVector(0.0, 60.0, 0.0), TEXT("soldier, no clip"));
    PlaceCharacter(TEXT("/Game/WebBridgeCap/Characters/skeleton-soldier-01.skeleton-soldier-01"),
                   TEXT("/Game/WebBridgeCap/Characters/skeleton-soldier-01Root_Idle_Relaxed.skeleton-soldier-01Root_Idle_Relaxed"),
                   FVector(170.0, 60.0, 0.0), TEXT("soldier, bones matched"));
    PlaceCharacter(TEXT("/Game/PolygonFantasyKingdom/Meshes/Chr/SK_Chr_Peasant_Male_01.SK_Chr_Peasant_Male_01"),
                   nullptr, FVector(340.0, 60.0, 0.0), TEXT("raw pack character"));
    PlaceStatic(TEXT("/Game/PolygonFantasyKingdom/Meshes/Props/SM_Prop_Barrel_01.SM_Prop_Barrel_01"), FVector(510.0, 30.0, 0.0));

    ACameraActor* Camera = World->SpawnActor<ACameraActor>(FVector(85.0, -930.0, 260.0), FRotator(-8.0f, 90.0f, 0.0f));
    Camera->GetCameraComponent()->SetFieldOfView(42.0f);
    // The map's default pawn draws a sphere where it spawns; it is not part of the evidence.
    for (TActorIterator<APawn> It(World); It; ++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
        for (UPrimitiveComponent* Primitive : Primitives)
        {
            Primitive->SetVisibility(false);
        }
    }
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
    {
        Controller->SetViewTarget(Camera);
    }
    UE_LOG(LogRpgAssetLab, Display, TEXT("lab built in %s; shot in %.0fs"), *World->GetMapName(), SHOT_AFTER_SECONDS);
}

// The native route, with its control: the pack's stock soldier with a clip
// imported from an animation pack onto the pack's own skeleton, the same
// soldier with no clip, and the soldier walking.
void ARpgAssetLab::BuildNativeRow(UWorld* World)
{
    ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0.0, 0.0, 500.0), FRotator(-48.0, 32.0, 0.0));
    Sun->GetLightComponent()->SetIntensity(6.0f);
    if (UClass* SkySphereClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Engine/EngineSky/BP_Sky_Sphere.BP_Sky_Sphere_C")))
    {
        World->SpawnActor<AActor>(SkySphereClass, FVector::ZeroVector, FRotator::ZeroRotator);
    }
    auto PlaceStatic = [World](const TCHAR* Path, const FVector& At, float Yaw = 0.0f, float Scale = 1.0f)
    {
        AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(At, FRotator(0.0, Yaw, 0.0));
        Actor->SetMobility(EComponentMobility::Movable);
        Actor->SetActorScale3D(FVector(Scale));
        if (UStaticMesh* Mesh = Asset<UStaticMesh>(Path))
        {
            Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        }
    };
    auto PlaceCharacter = [this, World](const TCHAR* Path, const TCHAR* Idle, const FVector& At, const FString& Label)
    {
        ASkeletalMeshActor* Actor = World->SpawnActor<ASkeletalMeshActor>(At, FRotator(0.0, 180.0, 0.0));
        USkeletalMeshComponent* Component = Actor->GetSkeletalMeshComponent();
        if (USkeletalMesh* Mesh = Asset<USkeletalMesh>(Path))
        {
            Component->SetSkeletalMeshAsset(Mesh);
        }
        if (UAnimSequence* Animation = Idle ? Asset<UAnimSequence>(Idle) : nullptr)
        {
            Component->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            Component->SetAnimation(Animation);
            Component->Play(true);
            Idles.Add(Component, Label);
        }
        Measured.Add(Component, Label);
    };
    PlaceStatic(TEXT("/Engine/BasicShapes/Plane.Plane"), FVector(0.0, 60.0, 0.0), 0.0f, 40.0);

    const TCHAR* Soldier = TEXT("/Game/PolygonFantasyKingdom/Meshes/Chr/SK_Chr_Soldier_Male_01.SK_Chr_Soldier_Male_01");
    // What the pack's skeleton calls its bones, against what the clips name.
    if (USkeletalMesh* SoldierMesh = Asset<USkeletalMesh>(Soldier))
    {
        const FReferenceSkeleton& Ref = SoldierMesh->GetRefSkeleton();
        FString Names;
        for (int32 Bone = 0; Bone < Ref.GetNum(); ++Bone)
        {
            Names += Ref.GetBoneName(Bone).ToString() + TEXT(" ");
        }
        UE_LOG(LogRpgAssetLab, Display, TEXT("BONES the pack soldier: %d bones: %s"), Ref.GetNum(), *Names);
    }
    PlaceCharacter(Soldier, TEXT("/Game/NativeAnim/A_POLY_IDL_Base_Masc.A_POLY_IDL_Base_Masc"), FVector(-170.0, 60.0, 0.0), TEXT("soldier, native idle"));
    PlaceCharacter(Soldier, nullptr, FVector(0.0, 60.0, 0.0), TEXT("soldier, no clip"));
    PlaceCharacter(Soldier, TEXT("/Game/NativeAnim/A_Walk_F_Masc.A_Walk_F_Masc"), FVector(170.0, 60.0, 0.0), TEXT("soldier, native walk"));

    ACameraActor* Camera = World->SpawnActor<ACameraActor>(FVector(0.0, -640.0, 230.0), FRotator(-9.0, 90.0, 0.0));
    Camera->GetCameraComponent()->SetFieldOfView(42.0f);
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
    {
        Controller->SetViewTarget(Camera);
    }
    for (TActorIterator<APawn> It(World); It; ++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
        for (UPrimitiveComponent* Primitive : Primitives)
        {
            Primitive->SetVisibility(false);
        }
    }
    UE_LOG(LogRpgAssetLab, Display, TEXT("native row built in %s"), *World->GetMapName());
}

void ARpgAssetLab::Shoot()
{
    for (const TPair<TWeakObjectPtr<USkeletalMeshComponent>, FString>& Idle : Idles)
    {
        const USkeletalMeshComponent* Component = Idle.Key.Get();
        UE_LOG(LogRpgAssetLab, Display, TEXT("LAB %-22s playing=%d position=%.2fs"),
            *Idle.Value, Component && Component->IsPlaying() ? 1 : 0,
            Component ? Component->GetPosition() : -1.0f);
        // A still cannot tell an idle from a rest pose that looks like it, and a
        // motion number means nothing without a control. So for every character,
        // clip or not: where the left hand points now, against where the skeleton's
        // rest pose would put it.
        const USkeletalMesh* Mesh = Component ? Component->GetSkeletalMeshAsset() : nullptr;
        if (!Mesh)
        {
            continue;
        }
        const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();
        const int32 Hand = LeftHandBone(Ref);
        if (Hand == INDEX_NONE)
        {
            UE_LOG(LogRpgAssetLab, Display, TEXT("LAB %-22s has no left-hand bone among %d bones"), *Idle.Value, Ref.GetNum());
            continue;
        }
        FQuat Rest = FQuat::Identity;
        for (int32 Bone = Hand; Bone != INDEX_NONE; Bone = Ref.GetParentIndex(Bone))
        {
            Rest = Ref.GetRefBonePose()[Bone].GetRotation() * Rest;
        }
        const FQuat Now = Component->GetBoneTransform(Hand).GetRotation();
        HandAtShot.Add(Idle.Value, Now);
        UE_LOG(LogRpgAssetLab, Display, TEXT("POSE %-22s left hand now=(%.0f,%.0f,%.0f) rest=(%.0f,%.0f,%.0f) apart=%.1f deg"),
            *Idle.Value, Now.Rotator().Pitch, Now.Rotator().Yaw, Now.Rotator().Roll,
            Rest.Rotator().Pitch, Rest.Rotator().Yaw, Rest.Rotator().Roll,
            FMath::RadiansToDegrees(Now.AngularDistance(Rest)));
    }
    // The control: the same soldier with no clip at all.
    for (const TPair<TWeakObjectPtr<USkeletalMeshComponent>, FString>& Pair : Measured)
    {
        const USkeletalMeshComponent* Component = Pair.Key.Get();
        const USkeletalMesh* Mesh = Component ? Component->GetSkeletalMeshAsset() : nullptr;
        if (!Mesh || HandAtShot.Contains(Pair.Value))
        {
            continue;
        }
        const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();
        const int32 Hand = LeftHandBone(Ref);
        if (Hand == INDEX_NONE)
        {
            continue;
        }
        FQuat Rest = FQuat::Identity;
        for (int32 Bone = Hand; Bone != INDEX_NONE; Bone = Ref.GetParentIndex(Bone))
        {
            Rest = Ref.GetRefBonePose()[Bone].GetRotation() * Rest;
        }
        const FQuat Now = Component->GetBoneTransform(Hand).GetRotation();
        HandAtShot.Add(Pair.Value, Now);
        UE_LOG(LogRpgAssetLab, Display, TEXT("POSE %-22s left hand now=(%.0f,%.0f,%.0f) rest=(%.0f,%.0f,%.0f) apart=%.1f deg"),
            *Pair.Value, Now.Rotator().Pitch, Now.Rotator().Yaw, Now.Rotator().Roll,
            Rest.Rotator().Pitch, Rest.Rotator().Yaw, Rest.Rotator().Roll,
            FMath::RadiansToDegrees(Now.AngularDistance(Rest)));
    }
    GetWorldTimerManager().SetTimer(MomentTimer, this, &ARpgAssetLab::ShootAfterAMoment, 0.88f, false);
}

void ARpgAssetLab::ShootAfterAMoment()
{
    for (const TPair<FString, FQuat>& Pair : HandAtShot)
    {
        const USkeletalMeshComponent* Component = nullptr;
        for (const TPair<TWeakObjectPtr<USkeletalMeshComponent>, FString>& Entry : Measured)
        {
            if (Entry.Value == Pair.Key)
            {
                Component = Entry.Key.Get();
            }
        }
        const USkeletalMesh* Mesh = Component ? Component->GetSkeletalMeshAsset() : nullptr;
        const int32 Hand = Mesh ? LeftHandBone(Mesh->GetRefSkeleton()) : INDEX_NONE;
        if (Hand != INDEX_NONE)
        {
            const float Moved = FMath::RadiansToDegrees(Pair.Value.AngularDistance(Component->GetBoneTransform(Hand).GetRotation()));
            UE_LOG(LogRpgAssetLab, Display, TEXT("MOTION %-22s left hand turned %.1f degrees in half a clip (0.88s)"), *Pair.Key, Moved);
        }
    }
    FScreenshotRequest::RequestScreenshot(TEXT("AssetLabGame"), false, false);
    GetWorldTimerManager().SetTimer(QuitTimer, this, &ARpgAssetLab::Leave, QUIT_AFTER_SHOT_SECONDS, false);
}

void ARpgAssetLab::Leave()
{
    FPlatformMisc::RequestExit(false);
}

namespace
{
    FAutoConsoleCommand GRpgLabCommand(
        TEXT("rpg.lab"),
        TEXT("Build the asset lab in the game world, take a screenshot, quit. Run with -game. `rpg.lab native`: the stock soldier with native clips."),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
            if (!World)
            {
                UE_LOG(LogRpgAssetLab, Error, TEXT("rpg.lab: no game world; start the editor with -game"));
                return;
            }
            GNativeRow = Args.Num() > 0 && Args[0].Equals(TEXT("native"), ESearchCase::IgnoreCase);
            World->SpawnActor<ARpgAssetLab>();
        }));
}

IMPLEMENT_MODULE(FDefaultModuleImpl, RpgAssetLab)
