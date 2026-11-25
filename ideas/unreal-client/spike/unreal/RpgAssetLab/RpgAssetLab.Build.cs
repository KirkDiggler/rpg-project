// RpgAssetLab: the proof project's labs — the asset row (RpgAssetLabModule.cpp)
// and the scene built from the atlas (RpgSceneLabActor.cpp).
using UnrealBuildTool;

public class RpgAssetLab : ModuleRules
{
    public RpgAssetLab(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = false;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "RpgWire", "ProceduralMeshComponent" });
    }
}
