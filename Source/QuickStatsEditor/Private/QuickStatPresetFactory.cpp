// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "QuickStatPresetFactory.h"
#include "QuickStatSettings.h"

UQuickStatPresetFactory::UQuickStatPresetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	SupportedClass = UQuickStatPreset::StaticClass();
}

UObject* UQuickStatPresetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UQuickStatPreset>(InParent, Class, Name, Flags);
}

FText UQuickStatPresetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("QuickStats Preset"));
}
