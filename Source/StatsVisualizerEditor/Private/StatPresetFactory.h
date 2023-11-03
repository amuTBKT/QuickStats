// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "StatPresetFactory.generated.h"

UCLASS(HideCategories = Object, MinimalAPI)
class UCustomStatPresetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
};
