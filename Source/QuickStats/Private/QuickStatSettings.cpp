// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "QuickStatSettings.h"

void UQuickStatSettings::PostInitProperties()
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
		if (ensureMsgf(!Itr.Value.IsNull(), TEXT("[QuickStat] StatPreset(%s) is missing asset reference!"), *Itr.Key.ToString()))
		{
			ensureMsgf(Itr.Value.LoadSynchronous(), TEXT("[QuickStat] Asset:(%s) for StatPreset(%s) failed to load!"), *Itr.Value.ToString(), *Itr.Key.ToString());
		}
	}
#endif
}

FName UQuickStatSettings::GetCategoryName() const
{
	return FName(TEXT("Plugins"));
}

#if WITH_EDITOR
void UQuickStatSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}

	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	const FName PropertyName = PropertyThatChanged ? PropertyThatChanged->GetFName() : NAME_None;

	// make sure newly added presets are loaded
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuickStatSettings, StatPresets))
	{
		for (auto& Itr : StatPresets)
		{
			// newly added stats are always invalid, so no need to assert here
			if (!Itr.Value.IsNull())
			{
				ensureMsgf(Itr.Value.LoadSynchronous(), TEXT("[QuickStat] Asset:(%s) for StatPreset(%s) failed to load!"), *Itr.Key.ToString(), *Itr.Value.ToString());
			}
		}
	}
}
#endif

const UQuickStatPreset* UQuickStatSettings::GetPresetByName(FName PresetName) const
{
	if (StatPresets.Contains(PresetName))
	{
		return StatPresets[PresetName].Get();
	}
	return nullptr;
}
