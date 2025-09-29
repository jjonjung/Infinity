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
        PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTasks", "NavigationSystem", "AugmentedReality", "AugmentedReality", "AugmentedReality", "AugmentedReality", "AugmentedReality", "AugmentedReality" });

		PrivateIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(ModuleDirectory);
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
