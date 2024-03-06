// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "QuickStatsRenderer.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FStatsVisualizerModule"

class FQuickStatsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
#if STATS
		FCoreDelegates::OnPostEngineInit.AddStatic(&FQuickStatsRenderer::RegisterStatPresets);
#endif
	}

	virtual void ShutdownModule() override
	{
#if STATS
		FQuickStatsRenderer::UnregisterStatPresets();
#endif
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuickStatsModule, QuickStats)
