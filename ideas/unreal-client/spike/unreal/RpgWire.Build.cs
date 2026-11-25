// RpgWire: the record client, in one module and two directories.
//
//   Private/RpgRecordStream.*   the stream, in plain C++ that names no Unreal type
//   Private/pb/                 the contract, compiled: generated protobuf and gRPC C++
//   everything else             the thin Unreal face: the game-thread marshal and the console entry
//
// One module, because a module boundary is a binary boundary here: Unreal
// exports only what a module's API macro marks, and generated protobuf and
// gRPC classes carry no such mark, so a symbol that crosses from the
// generated code into another module is not there when it is called. The
// directories keep the seam that matters: the stream can be compiled and
// tested without the editor, as the spike's headless client was.
//
// The libraries are protobuf 4.23.4 and gRPC 1.57.0 — what TurboLink v1.4.2
// ships for UE 5.6, built with clang 16, libc++, no exceptions, no RTTI.
// None of TurboLink's own code is used. Unreal compiles with no RTTI by
// default, so only that is stated again, to be explicit about the match.
using System;
using System.IO;
using UnrealBuildTool;

public class RpgWire : ModuleRules
{
    public RpgWire(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;          // the generated corpus is 900,000 lines: one file per action
        bEnableExceptions = false;
        bUseRTTI = false;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject" });
        PrivateDependencyModuleNames.AddRange(new[] { "Engine" });

        string thirdParty = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty"));

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "pb"));
        foreach (string lib in new[] { "protobuf", "grpc", "re2", "abseil" })
        {
            PublicSystemIncludePaths.Add(Path.Combine(thirdParty, lib, "include"));
        }

        PrivateDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PrivateDefinitions.Add("NOMINMAX");
        PrivateDefinitions.Add("WIN32_LEAN_AND_MEAN");

        // Debug libraries only for a Debug build; every other configuration,
        // Development included, links the Release libraries.
        string config = Target.Configuration == UnrealTargetConfiguration.Debug ? "Debug" : "Release";

        bool linux = Target.Platform == UnrealTargetPlatform.Linux;
        if (linux || Target.Platform == UnrealTargetPlatform.Win64)
        {
            string platformDir = linux ? "linux" : "win64";
            string pattern = linux ? "lib*.a" : "*.lib";
            foreach (string libDir in new[] { "grpc", "protobuf", "re2", "abseil" })
            {
                string dir = Path.Combine(thirdParty, libDir, "lib", platformDir, config);
                if (!Directory.Exists(dir))
                {
                    throw new BuildException("RpgWire: no libraries vendored for " + platformDir + "/" + config + " at " + dir);
                }
                string[] files = Directory.GetFiles(dir, pattern);
                Array.Sort(files, StringComparer.Ordinal);
                PublicAdditionalLibraries.AddRange(files);
            }
            // gRPC's TLS and compression come from the engine, as in TurboLink's own module.
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
        }
        else
        {
            throw new BuildException("RpgWire: no libraries are vendored for platform " + Target.Platform);
        }
    }
}
