// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "CodeStatDefinitionCustomization.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FStatsVisualizerEditorModule"

class FStatsVisualizerEditorModule : public IModuleInterface
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

IMPLEMENT_MODULE(FStatsVisualizerEditorModule, StatsVisualizerEditor)