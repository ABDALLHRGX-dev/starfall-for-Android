using System;
using System.IO;
using UnrealBuildTool;

public class Starfall : ModuleRules
{
    public Starfall(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableExceptions = true;
        bUseRTTI = true;
        
        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public")
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                ModuleDirectory
            }
        );
        
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "Engine"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "HTTP"
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDefinitions.Add("STARFALL_ANDROID=1");

            PublicSystemLibraries.Add("z");
            PublicSystemLibraries.Add("dl");
        }
    }
}
