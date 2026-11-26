// The scene lab: what the web shows for an atlas, built in Unreal from the
// atlas the engine sends. In a game run:
//
//   UnrealEditor-Cmd <project> -game -RenderOffscreen -unattended -NoSound \
//     -windowed -ResX=1920 -ResY=1080 -ExecCmds="rpg.scene <addr> <session> <player> <member>"
//
// The atlas is fetched over the wire as the seated member, so it is the atlas
// as that member knows it, exactly what the web would draw for them.
//
// In the EDITOR (no -game), or with a fifth argument `stay`, the scene is
// built into the editor's own world, the viewport is placed at the angled
// camera, and nothing is shot or closed: hold the right mouse button in the
// viewport and fly with W A S D.
//
// The frame is the web's (rpg-dnd5e-web atlasToScene3D.ts, structuralLayout.ts):
//   - pointy-top hexes, circumradius one scene unit; a cell (q, r) sits at
//     x = sqrt(3) * (q + r/2), y = 1.5 * r;
//   - authored walls and doors are in FEET, one hex being five feet flat to
//     flat, so a foot is sqrt(3)/5 scene units;
//   - one scene unit is one metre here. Unreal's X is the web's x, Unreal's Y
//     is the web's z (so the picture is the web's seen from its default
//     azimuth, mirrored left to right; cells and walls go through the same
//     mapping, so they agree with each other either way).
#include "RpgSceneLabActor.h"
#include "CoreMinimal.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Engine/GameViewportClient.h"

#include "Components/SkeletalMeshComponent.h"


#include "Async/Async.h"
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
#include "Landscape.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HighResScreenshot.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "RpgSession.h"

#include <memory>

DEFINE_LOG_CATEGORY_STATIC(LogRpgSceneLab, Log, All);

namespace
{
    constexpr double kSqrt3 = 1.7320508075688772;
    constexpr double kFeetPerHex = 5.0;
    constexpr double kScenePerFoot = kSqrt3 / kFeetPerHex;
    constexpr double kCmPerScene = 100.0;
    constexpr float kFetchAfterSeconds = 3.0f;
    constexpr float kShotAfterSeconds = 6.0f;
    constexpr float kQuitAfterShotSeconds = 4.0f;

    FVector CellCenter(double Q, double R)
    {
        return FVector(kSqrt3 * (Q + R / 2.0) * kCmPerScene, 1.5 * R * kCmPerScene, 0.0);
    }

    FVector FeetPoint(double X, double Y, double Z)
    {
        return FVector(X * kScenePerFoot * kCmPerScene, Y * kScenePerFoot * kCmPerScene, Z * kScenePerFoot * kCmPerScene);
    }

    double Feet(double Value)
    {
        return Value * kScenePerFoot * kCmPerScene;
    }

    FString Take(const std::string& Text)
    {
        return FString(UTF8_TO_TCHAR(Text.c_str()));
    }

    FLinearColor GroundColorFor(const FString& Archetype)
    {
        // The web tints a tile by its region's archetype. Two shades until the
        // web's ground materials are ported.
        return Archetype == TEXT("crypt") ? FLinearColor(0.36f, 0.37f, 0.41f) : FLinearColor(0.46f, 0.40f, 0.30f);
    }

} // namespace

/// The atlas as Unreal values, converted from rpgwire::Atlas on the game thread.
struct FSceneAtlas
{
    struct FRegion
    {
        FString Id;
        FString Archetype;
        TSet<TPair<int32, int32>> Cells;
    };
    struct FPlaced
    {
        FString Id;
        double Width = 0.0;
        double Depth = 0.0;
        double OriginX = 0.0;
        double OriginY = 0.0;
        double Facing = 0.0;
    };
    struct FAuthored
    {
        double Height = 0.0;
        double Elevation = 0.0;
        int32 Openings = 0;
        bool Door = false;
    };

    TArray<TPair<int32, int32>> Cells;
    TArray<FRegion> Regions;
    TMap<FString, FAuthored> Authored;
    TArray<FPlaced> Placed;
    TPair<int32, int32> Start{0, 0};
    FString DungeonKey;
};

ARpgSceneLab::ARpgSceneLab()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ARpgSceneLab::BeginPlay()
{
    Super::BeginPlay();
    Start();
}

void ARpgSceneLab::Start()
{
    if (Started)
    {
        return;
    }
    Started = true;
    FetchAtlas(); // at once: a timer would wait for the editor to tick, and it does not while its window is hidden
}

void ARpgSceneLab::FetchAtlas()
{
    TWeakObjectPtr<ARpgSceneLab> WeakThis(this);
    const Target TargetCopy = TargetToRead;
    UE_LOG(LogRpgSceneLab, Display, TEXT("fetching the atlas of %s as %s"), *TargetCopy.Session, *TargetCopy.Member);
    Async(EAsyncExecution::ThreadPool, [WeakThis, TargetCopy]()
    {
        rpgwire::Session::Options Options;
        Options.address = TCHAR_TO_UTF8(*TargetCopy.Address);
        Options.player_id = TCHAR_TO_UTF8(*TargetCopy.Player);
        rpgwire::Session Session(Options);

        std::shared_ptr<rpgwire::Atlas> Atlas = std::make_shared<rpgwire::Atlas>();
        std::string Error;
        const bool Ok = Session.Atlas(TCHAR_TO_UTF8(*TargetCopy.Session), TCHAR_TO_UTF8(*TargetCopy.Member), Atlas.get(), &Error);
        const FString ErrorText = Take(Error);
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Ok, ErrorText, Atlas]()
        {
            if (ARpgSceneLab* Self = WeakThis.Get())
            {
                Self->OnAtlas(Ok, ErrorText, Atlas);
            }
        });
    });
}

void ARpgSceneLab::OnAtlas(bool Ok, const FString& Error, const std::shared_ptr<const rpgwire::Atlas>& Wire)
{
    if (!Ok)
    {
        UE_LOG(LogRpgSceneLab, Error, TEXT("GetAtlas failed: %s"), *Error);
        return;
    }
    // Plain values in, Unreal values out; nothing here meets a wire type.
    FSceneAtlas Atlas;
    Atlas.DungeonKey = Take(Wire->dungeon_key);
    for (const rpgwire::Atlas::Cell& Cell : Wire->cells)
    {
        Atlas.Cells.Emplace(Cell.first, Cell.second);
    }
    for (const rpgwire::Atlas::Region& Region : Wire->regions)
    {
        FSceneAtlas::FRegion Value;
        Value.Id = Take(Region.id);
        Value.Archetype = Take(Region.archetype);
        for (const rpgwire::Atlas::Cell& Cell : Region.cells)
        {
            Value.Cells.Add(TPair<int32, int32>(Cell.first, Cell.second));
        }
        Atlas.Regions.Add(MoveTemp(Value));
    }
    for (const auto& Entry : Wire->authored)
    {
        FSceneAtlas::FAuthored Value;
        Value.Height = Entry.second.height;
        Value.Elevation = Entry.second.elevation;
        Value.Openings = Entry.second.openings;
        Value.Door = Entry.second.door;
        Atlas.Authored.Add(Take(Entry.first), Value);
    }
    for (const rpgwire::Atlas::Placed& Placed : Wire->placed)
    {
        FSceneAtlas::FPlaced Value;
        Value.Id = Take(Placed.id);
        Value.Width = Placed.width;
        Value.Depth = Placed.depth;
        Value.OriginX = Placed.origin_x;
        Value.OriginY = Placed.origin_y;
        Value.Facing = Placed.facing;
        Atlas.Placed.Add(Value);
    }
    Atlas.Start = TPair<int32, int32>(Wire->start.first, Wire->start.second);

    UE_LOG(LogRpgSceneLab, Display, TEXT("atlas of %s: %d cells, %d regions, %d authored walls and doors, %d placed, start (%d,%d)"),
        *Atlas.DungeonKey, Atlas.Cells.Num(), Atlas.Regions.Num(), Atlas.Authored.Num(), Atlas.Placed.Num(),
        Atlas.Start.Key, Atlas.Start.Value);
    Build(Atlas);
    if (Stay)
    {
        return;
    }
    GetWorldTimerManager().SetTimer(QuitTimer, this, &ARpgSceneLab::Shoot, kShotAfterSeconds, false);
}

void ARpgSceneLab::Build(const FSceneAtlas& Atlas)
{
    UWorld* World = GetWorld();

    // The default map's floor and landscape would compete with the hexes and
    // z-fight them at the same height. Hide every static shape that was here
    // before the build; sky and lights stay.
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
        for (UPrimitiveComponent* Primitive : Primitives)
        {
            Primitive->SetVisibility(false);
        }
    }
    for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
        for (UPrimitiveComponent* Primitive : Primitives)
        {
            Primitive->SetVisibility(false);
        }
    }

    ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0.0, 0.0, 500.0), FRotator(-48.0, 32.0, 0.0));
    Sun->GetLightComponent()->SetIntensity(6.0f);
    if (UClass* SkySphereClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Engine/EngineSky/BP_Sky_Sphere.BP_Sky_Sphere_C")))
    {
        World->SpawnActor<AActor>(SkySphereClass, FVector::ZeroVector, FRotator::ZeroRotator);
    }
    ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.0, 0.0, 300.0), FRotator::ZeroRotator);
    Sky->GetLightComponent()->SetIntensity(1.0f);

    // ---- the floor: one hex per revealed cell, tinted by its region ----
    TMap<TPair<int32, int32>, FLinearColor> CellTints;
    for (const FSceneAtlas::FRegion& Region : Atlas.Regions)
    {
        const FLinearColor Tint = GroundColorFor(Region.Archetype);
        for (const TPair<int32, int32>& Cell : Region.Cells)
        {
            CellTints.Add(Cell, Tint);
        }
    }
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;
    FBox CellBox(ForceInit);
    for (const TPair<int32, int32>& Cell : Atlas.Cells)
    {
        const FVector Center = CellCenter(Cell.Key, Cell.Value);
        CellBox += Center;
        const FColor Tint = CellTints.FindRef(Cell).ToFColor(false);
        const int32 Base = Vertices.Num();
        for (int32 Corner = 0; Corner < 6; ++Corner)
        {
            const double Angle = FMath::DegreesToRadians(30.0 + 60.0 * Corner);
            Vertices.Add(Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0) * kCmPerScene + FVector(0.0, 0.0, 1.0));
            Normals.Add(FVector::UpVector);
            UVs.Add(FVector2D::ZeroVector);
            Colors.Add(Tint);
            Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
        }
        // Both windings, so the face shows whichever side is front.
        for (int32 Corner = 1; Corner < 5; ++Corner)
        {
            Triangles.Append({Base, Base + Corner, Base + Corner + 1});
            Triangles.Append({Base, Base + Corner + 1, Base + Corner});
        }
    }
    UProceduralMeshComponent* Floor = NewObject<UProceduralMeshComponent>(this);
    Floor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    Floor->RegisterComponent();
    if (UMaterial* VertexColor = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/VertexColorMaterial.VertexColorMaterial")))
    {
        Floor->SetMaterial(0, VertexColor);
    }
    Floor->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
    Built += Atlas.Cells.Num();

    // ---- the authored walls and doors: each placed span a box ----
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterial* ShapeMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    const FLinearColor Stone(0.55f, 0.54f, 0.50f);
    const FLinearColor Wood(0.30f, 0.19f, 0.10f);
    int32 Skipped = 0;
    for (const FSceneAtlas::FPlaced& Placed : Atlas.Placed)
    {
        // "wall/<id>/span/<n>" is one span of the wall <id>; anything else names a wall, a door or a prop.
        FString Parent = Placed.Id;
        bool Span = false;
        if (Placed.Id.StartsWith(TEXT("wall/")))
        {
            Parent = Placed.Id.Mid(5);
            const int32 Cut = Parent.Find(TEXT("/span/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
            if (Cut != INDEX_NONE)
            {
                Parent = Parent.Left(Cut);
                Span = true;
            }
        }
        const FSceneAtlas::FAuthored* Info = Atlas.Authored.Find(Parent);
        if (!Info && !Atlas.DungeonKey.IsEmpty())
        {
            Info = Atlas.Authored.Find(Atlas.DungeonKey / Parent); // a door is placed under its bare id
        }
        if (!Info)
        {
            ++Skipped; // a prop; the manifest that names its model is the next leg
            continue;
        }
        // A wall with openings is drawn by its spans; one without, by itself.
        if (!Info->Door && (Span != (Info->Openings > 0)))
        {
            continue;
        }
        const FVector Center = FeetPoint(Placed.OriginX, Placed.OriginY, Info->Elevation + Info->Height / 2.0);
        AStaticMeshActor* Box = World->SpawnActor<AStaticMeshActor>(Center, FRotator(0.0, Placed.Facing, 0.0));
        Box->SetMobility(EComponentMobility::Movable);
        Box->SetActorScale3D(FVector(Feet(Placed.Depth), Feet(Placed.Width), Feet(Info->Height)) / 100.0);
        Box->GetStaticMeshComponent()->SetStaticMesh(Cube);
        if (ShapeMaterial)
        {
            UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(ShapeMaterial, this);
            Tint->SetVectorParameterValue(TEXT("Color"), Info->Door ? Wood : Stone);
            Box->GetStaticMeshComponent()->SetMaterial(0, Tint);
        }
        ++Built;
    }
    UE_LOG(LogRpgSceneLab, Display, TEXT("built %d floor cells; %d wall and door boxes; %d placements skipped (props)"), Atlas.Cells.Num(), Built - Atlas.Cells.Num(), Skipped);

    // ---- the member at the start cell, idle ----
    const FVector StartAt = CellCenter(Atlas.Start.Key, Atlas.Start.Value);
    ASkeletalMeshActor* Member = World->SpawnActor<ASkeletalMeshActor>(StartAt, FRotator(0.0, 180.0, 0.0));
    if (USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/WebBridgeCap/Characters/fighter.fighter")))
    {
        Member->GetSkeletalMeshComponent()->SetSkeletalMeshAsset(Mesh);
    }
    if (UAnimSequence* Idle = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/WebBridgeCap/Characters/fighterRoot_Idle_Relaxed.fighterRoot_Idle_Relaxed")))
    {
        Member->GetSkeletalMeshComponent()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        Member->GetSkeletalMeshComponent()->SetAnimation(Idle);
        Member->GetSkeletalMeshComponent()->Play(true);
    }

    // ---- the cameras: one straight down to check the shape, one at the web's kind of angle ----
    // Unreal's field of view is horizontal: at 16:9 a 42-degree lens sees 42 degrees across and
    // about 24 down the frame. Both half-angles set the distance, with a margin.
    const FBox Revealed = CellBox.ExpandBy(kCmPerScene);
    const FVector Middle = Revealed.GetCenter();
    const double Width = FMath::Max(Revealed.GetSize().X, 400.0);
    const double Depth = FMath::Max(Revealed.GetSize().Y, 400.0);
    const double HalfHorizontal = FMath::DegreesToRadians(42.0 / 2.0);
    const double HalfVertical = FMath::Atan(FMath::Tan(HalfHorizontal) / (16.0 / 9.0));
    const double Margin = 400.0;
    const double Distance = FMath::Max((Width / 2.0 + Margin) / FMath::Tan(HalfHorizontal),
                                       (Depth / 2.0 + Margin) / FMath::Tan(HalfVertical));
    // Rolled 180: Unreal is left-handed, so a camera looking straight down with north up shows
    // east on the left. South up shows east on the right, which is the web's top view (its x
    // right, its z down the screen).
    TopCamera = World->SpawnActor<ACameraActor>(Middle + FVector(0.0, 0.0, Distance), FRotator(-90.0, 90.0, 180.0));
    TopCamera->GetCameraComponent()->SetFieldOfView(42.0f);
    // Pitched 60 degrees, from the south; the view axis meets the ground at the middle.
    const double Pitch = FMath::DegreesToRadians(60.0);
    AngleCamera = World->SpawnActor<ACameraActor>(Middle + FVector(0.0, -Distance * FMath::Cos(Pitch), Distance * FMath::Sin(Pitch)), FRotator(-60.0, 90.0, 0.0));
    AngleCamera->GetCameraComponent()->SetFieldOfView(42.0f);
    UE_LOG(LogRpgSceneLab, Display, TEXT("revealed cells span %.0f x %.0f cm around %s; cameras %.0f cm out; start cell (%d,%d)"),
        Width - 200.0, Depth - 200.0, *Middle.ToCompactString(), Distance, Atlas.Start.Key, Atlas.Start.Value);
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
    {
        Controller->SetViewTarget(TopCamera);
    }
    if (Stay)
    {
        // Put the editor's viewport where the angled camera is; from there it flies freely.
        if (GEditor && GEditor->GetActiveViewport())
        {
            if (FLevelEditorViewportClient* Viewport = static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
            {
                Viewport->SetViewLocation(AngleCamera->GetActorLocation());
                Viewport->SetViewRotation(AngleCamera->GetActorRotation());
                Viewport->FOVAngle = 42.0f;
                Viewport->Invalidate(false, false);
            }
        }
        return;
    }
    // The map's default pawn draws a sphere where it spawns; it is not part of the evidence.
    for (TActorIterator<APawn> It(World); It; ++It)
    {
        TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
        for (UPrimitiveComponent* Primitive : Primitives)
        {
            Primitive->SetVisibility(false);
        }
    }
}

void ARpgSceneLab::Shoot()
{
    FScreenshotRequest::RequestScreenshot(TEXT("RpgSceneTop"), false, false); // one request at a time: a second would replace the first
    GetWorldTimerManager().SetTimer(QuitTimer, this, &ARpgSceneLab::ShootAngled, 2.5f, false);
}

void ARpgSceneLab::ShootAngled()
{
    if (APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
    {
        Controller->SetViewTarget(AngleCamera);
    }
    GetWorldTimerManager().SetTimer(QuitTimer, this, &ARpgSceneLab::ShootAngledNow, 0.5f, false);
}

void ARpgSceneLab::ShootAngledNow()
{
    FScreenshotRequest::RequestScreenshot(TEXT("RpgSceneAngle"), false, false);
    GetWorldTimerManager().SetTimer(QuitTimer, this, &ARpgSceneLab::Leave, kQuitAfterShotSeconds, false);
}

void ARpgSceneLab::Leave()
{
    FPlatformMisc::RequestExit(false);
}

namespace
{
    FAutoConsoleCommand GRpgSceneCommand(
        TEXT("rpg.scene"),
        TEXT("Build the web's view of a session's atlas. rpg.scene <addr> <session> <player> <member> [stay]. In the editor it builds into the editor world and stays open."),
        FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
        {
            // A game world when run with -game; otherwise the editor's own world, to look around in.
            UWorld* World = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWorld() : nullptr;
            const bool bEditor = World == nullptr;
            if (!World && GEditor)
            {
                World = GEditor->GetEditorWorldContext().World();
            }
            if (!World)
            {
                UE_LOG(LogRpgSceneLab, Error, TEXT("rpg.scene: no world to build in"));
                return;
            }
            if (Args.Num() < 4)
            {
                UE_LOG(LogRpgSceneLab, Error, TEXT("usage: rpg.scene <addr> <session> <player> <member> [stay]"));
                return;
            }
            ARpgSceneLab* Lab = World->SpawnActor<ARpgSceneLab>();
            ARpgSceneLab::Target TargetToRead;
            TargetToRead.Address = Args[0];
            TargetToRead.Session = Args[1];
            TargetToRead.Player = Args[2];
            TargetToRead.Member = Args[3];
            Lab->SetStay(bEditor || (Args.Num() > 4 && Args[4].Equals(TEXT("stay"), ESearchCase::IgnoreCase)));
            UE_LOG(LogRpgSceneLab, Display, TEXT("building in the %s world%s"), bEditor ? TEXT("editor") : TEXT("game"),
                Lab->StaysOpen() ? TEXT("; staying open") : TEXT(""));
            Lab->ReadTarget(TargetToRead);
            Lab->Start();
        }));
}
