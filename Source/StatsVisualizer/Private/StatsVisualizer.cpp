// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "CustomStatsRenderer.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FStatsVisualizerModule"

class FStatsVisualizerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
#if STATS
		FCoreDelegates::OnPostEngineInit.AddStatic(&FCustomStatsRenderer::RegisterStatPresets);
#endif
	}

	virtual void ShutdownModule() override
	{
#if STATS
		FCustomStatsRenderer::UnregisterStatPresets();
#endif
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStatsVisualizerModule, StatsVisualizer)