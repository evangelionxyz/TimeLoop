// Copyright (c) 2026 Team Ficus

using UnrealBuildTool;

public class TimeLoop : ModuleRules
{
	public TimeLoop(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
		]);

		PublicIncludePaths.AddRange([
			"TimeLoop",
			"TimeLoop/Actor",
			"TimeLoop/Components",
			"TimeLoop/Core",
			"TimeLoop/UI",
		]);

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
