// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

using UnrealBuildTool;

public class QuickStatsEditor : ModuleRules
{
	public QuickStatsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.NoPCHs;

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
				"QuickStats",
			}
		);
	}
}
