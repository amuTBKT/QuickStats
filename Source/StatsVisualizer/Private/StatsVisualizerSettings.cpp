// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "StatsVisualizerSettings.h"

void UStatsVisualizerSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	if (IsTemplate())
	{
		ImportConsoleVariableValues();
	}
#endif

#if STATS
	// load preset data assets
	for (auto& Itr : StatPresets)
	{
		if (ensureMsgf(!Itr.Value.IsNull(), TEXT("[StatsVisualizer] StatPreset(%s) is missing asset reference!"), *Itr.Key.ToString()))
		{
			ensureMsgf(Itr.Value.LoadSynchronous(), TEXT("[StatsVisualizer] Asset:(%s) for StatPreset(%s) failed to load!"), *Itr.Value.ToString(), *Itr.Key.ToString());
		}
	}
#endif
}

FName UStatsVisualizerSettings::GetCategoryName() const
{
	return FName(TEXT("Plugins"));
}

#if WITH_EDITOR
void UStatsVisualizerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}

	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	const FName PropertyName = PropertyThatChanged ? PropertyThatChanged->GetFName() : NAME_None;

	// make sure newly added presets are loaded
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UStatsVisualizerSettings, StatPresets))
	{
		for (auto& Itr : StatPresets)
		{
			// newly added stats are always invalid, so no need to assert here
			if (!Itr.Value.IsNull())
			{
				ensureMsgf(Itr.Value.LoadSynchronous(), TEXT("[StatsVisualizer] Asset:(%s) for StatPreset(%s) failed to load!"), *Itr.Key.ToString(), *Itr.Value.ToString());
			}
		}
	}
}
#endif

const UCustomStatPreset* UStatsVisualizerSettings::GetPresetByName(FName PresetName) const
{
	if (StatPresets.Contains(PresetName))
	{
		return StatPresets[PresetName].Get();
	}
	return nullptr;
}