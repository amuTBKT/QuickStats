// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if STATS

#include "ConsoleSettings.h"

class FCanvas;
class FViewport;
class FCommonViewportClient;

class FCustomStatsRenderer
{
public:
	static void RegisterStatPresets();
	static void UnregisterStatPresets();

	// disable all the stats and enable these
	static void SetPresets_Command(const TArray<FName>& PresetNames);

	// add this preset to the enabled list
	static void EnablePresets_Command(const TArray<FName>& PresetNames);

	// remove this preset from the enabled list
	static void DisablePresets_Command(const TArray<FName>& PresetNames);

private:
	// engine stats rendering
	static int32 OnRenderStats(UWorld* World, FViewport* Viewport, FCanvas* Canvas, int32 X, int32 Y, const FVector* ViewLocation, const FRotator* ViewRotation);
	static bool OnToggleStats(UWorld* World, FCommonViewportClient* ViewportClient, const TCHAR* Stream);
	static void PopulateAutoCompletePresetNames(TArray<FAutoCompleteCommand>& AutoCompleteList);

	// helpers
	static void SetEnabledPresets(TArray<FName> NewPresets);
	static void EnableStatGroup(FName StatGroupName);
	static void DisableStatGroup(FName StatGroupName);

private:
	static const FName StatsVisualizerPresetsName;
	static const FName StatsVisualizerPresetsCategory;
	static const FText StatsVisualizerPresetsDescription;

	static bool IsRenderingStats;
	static TArray<FName> EnabledPresets;
};

#endif //#if STATS