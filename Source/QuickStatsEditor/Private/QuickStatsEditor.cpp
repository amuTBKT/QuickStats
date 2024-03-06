// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "StatCustomization.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FQuickStatsEditorModule"

class FQuickStatsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomPropertyTypeLayout(FCodeStatDefinition::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCodeStatDefinitionCustomization::MakeInstance));
	}

	virtual void ShutdownModule() override
	{
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQuickStatsEditorModule, QuickStatsEditor)
