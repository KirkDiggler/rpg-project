// RpgAssetLab: the proof project's asset lab (see Private/RpgAssetLabModule.cpp).
using UnrealBuildTool;

public class RpgAssetLab : ModuleRules
{
    public RpgAssetLab(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = false;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
    }
}
