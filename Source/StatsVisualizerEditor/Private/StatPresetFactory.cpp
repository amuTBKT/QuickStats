// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "StatPresetFactory.h"
#include "StatsVisualizerSettings.h"

UCustomStatPresetFactory::UCustomStatPresetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	SupportedClass = UCustomStatPreset::StaticClass();
}

UObject* UCustomStatPresetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCustomStatPreset>(InParent, Class, Name, Flags);
}

FText UCustomStatPresetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Stat Preset"));
}
