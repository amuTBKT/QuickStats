// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

using UnrealBuildTool;

public class StatsVisualizerEditor : ModuleRules
{
	public StatsVisualizerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"InputCore",
				"Engine",
				"UnrealEd",
				"StatsVisualizer",
			}
		);
	}
}
