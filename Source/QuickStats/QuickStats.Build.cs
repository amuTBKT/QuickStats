// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

using UnrealBuildTool;

public class QuickStats : ModuleRules
{
	public QuickStats(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"EngineSettings"
			}
		);
	}
}
