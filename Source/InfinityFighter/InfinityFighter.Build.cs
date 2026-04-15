// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InfinityFighter : ModuleRules
{
	public InfinityFighter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core", 
	        "CoreUObject", 
	        "Engine", 
	        "InputCore", 
	        "EnhancedInput",
	        "GameplayTags",
	        "Niagara", 
	        "AIModule",
	        "UMG",
	        "Slate",
	        "SlateCore",
	        "NavigationSystem"
        });

        // AI/Navigation runtime dependencies used by GameMode/AI spawning
        PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTasks" });

        // Android: libc++_shared 명시적 링크 (UE5.6 NDK r27 vtable 링크 누락 버그 우회)
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicSystemLibraries.Add("c++_shared");
        }

		PrivateIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(ModuleDirectory);
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
