// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

using UnrealBuildTool;

public class StatsVisualizer : ModuleRules
{
	public StatsVisualizer(ReadOnlyTargetRules Target) : base(Target)
	{
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
