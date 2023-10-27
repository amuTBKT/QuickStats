// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "CustomStatsRenderer.h"

#if STATS

#include "StatsVisualizerSettings.h"
#include "String/ParseTokens.h"
#include "Stats/StatsData.h"

#include "Misc/CommandLine.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

bool			FCustomStatsRenderer::IsRenderingStats = false;
TArray<FName>	FCustomStatsRenderer::EnabledPresets = {};

const FName		FCustomStatsRenderer::StatsVisualizerPresetsName = FName(TEXT("STAT_Presets"));
const FName		FCustomStatsRenderer::StatsVisualizerPresetsCategory = FName(TEXT("STATCAT_StatsVisualizerPresets"));
const FText		FCustomStatsRenderer::StatsVisualizerPresetsDescription = FText::FromString(FString(TEXT("Visualizer for custom stats.")));

static TAutoConsoleVariable<FString> CVarEnabledPresets(
	TEXT("stats.Presets"),
	TEXT(""),
	TEXT("Presets to enable on boot, can be overriden by -statpresets"),
	ECVF_ReadOnly);

static FAutoConsoleCommand SetPresetCommand(
	TEXT("stats.SetPresets"),
	TEXT("Set active stat presets.\n"),
	FConsoleCommandWithArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args)
		{
			TArray<FName> Presets;
			for (const FString& PresetName : Args)
			{
				Presets.Add(FName(PresetName));
			}

			if (Presets.Num() > 0)
			{
				FCustomStatsRenderer::SetPresets_Command(Presets);
			}
		}
	)
);

static FAutoConsoleCommand EnablePresetCommand(
	TEXT("stats.EnablePresets"),
	TEXT("Enable stat presets.\n"),
	FConsoleCommandWithArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args)
		{
			TArray<FName> Presets;
			for (const FString& PresetName : Args)
			{
				Presets.Add(FName(PresetName));
			}

			if (Presets.Num() > 0)
			{
				FCustomStatsRenderer::EnablePresets_Command(Presets);
			}
		}
	)
);

static FAutoConsoleCommand DisablePresetCommand(
	TEXT("stats.DisablePresets"),
	TEXT("Disable stat presets.\n"),
	FConsoleCommandWithArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args)
		{
			TArray<FName> Presets;
			for (const FString& PresetName : Args)
			{
				Presets.Add(FName(PresetName));
			}

			if (Presets.Num() > 0)
			{
				FCustomStatsRenderer::DisablePresets_Command(Presets);
			}
		}
	)
);

void FCustomStatsRenderer::RegisterStatPresets()
{
	checkf(GEngine, TEXT("GEngine is not valid, the stat visualizer won't be functional!"));
	if (GEngine)
	{
		UEngine::FEngineStatRender RenderFunc = UEngine::FEngineStatRender::CreateStatic(&FCustomStatsRenderer::OnRenderStats);
		UEngine::FEngineStatToggle ToggleFunc = UEngine::FEngineStatToggle::CreateStatic(&FCustomStatsRenderer::OnToggleStats);
		GEngine->AddEngineStat(StatsVisualizerPresetsName, StatsVisualizerPresetsCategory, StatsVisualizerPresetsDescription, RenderFunc, ToggleFunc, false);
	}

	UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&FCustomStatsRenderer::PopulateAutoCompletePresetNames);

	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();
	check(Settings);

	// check commandline for enabled presets
	FString RequestedPresets = TEXT("");
	if (!FParse::Value(FCommandLine::Get(), TEXT("-statpresets="), RequestedPresets, false))
	{
		// check console variable
		RequestedPresets = CVarEnabledPresets.GetValueOnAnyThread();
	}

	if (!RequestedPresets.IsEmpty())
	{
		UE::String::ParseTokens(RequestedPresets, TEXT(","),
			[&](FStringView Token)
			{
				const FName PresetName(Token);

				if (Settings->StatPresets.Find(PresetName))
				{
					EnabledPresets.AddUnique(PresetName);
				}
				else if (PresetName != NAME_None) //None can be used to disable all preset.
				{
					UE_LOG(LogTemp, Warning, TEXT("Preset(%s) is not defined in StatVisualizerSettings!"), *PresetName.ToString());
				}
			}
		);
	}

#if 0
	// take the first preset if it's still empty
	if (EnabledPresets.Num() == 0)
	{
		TSet<FName> AvailablePresetNames;
		Settings->StatPresets.GetKeys(AvailablePresetNames);
		for (FName PresetName : AvailablePresetNames)
		{
			EnabledPresets.Add(PresetName);
			break;
		}
	}
#endif
}

void FCustomStatsRenderer::UnregisterStatPresets()
{
	if (GEngine)
	{
		GEngine->RemoveEngineStat(StatsVisualizerPresetsName);
	}
}

void FCustomStatsRenderer::PopulateAutoCompletePresetNames(TArray<FAutoCompleteCommand>& AutoCompleteList)
{
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();

	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();
	check(Settings);

	// disable all presets
	{
		FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
		AutoCompleteCommand.Command = TEXT("stats.SetPresets None");
		AutoCompleteCommand.Desc = TEXT("Disable all presets");
		AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
	}
	{
		FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
		AutoCompleteCommand.Command = TEXT("stats.DisablePresets All");
		AutoCompleteCommand.Desc = TEXT("Disable all presets");
		AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
	}

	// presets from config
	for (const auto& Itr : Settings->StatPresets)
	{
		const FString PresetName = Itr.Key.ToString();

		// set preset
		{
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
			AutoCompleteCommand.Command = *FString::Printf(TEXT("stats.SetPresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Set this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}

		// enable preset
		{
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
			AutoCompleteCommand.Command = *FString::Printf(TEXT("stats.EnablePresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Enable this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}

		// disable preset
		{
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
			AutoCompleteCommand.Command = *FString::Printf(TEXT("stats.DisablePresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Disable this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}
	}
}

int32 FCustomStatsRenderer::OnRenderStats(UWorld* World, FViewport* Viewport, FCanvas* Canvas, int32 X, int32 Y, const FVector* ViewLocation, const FRotator* ViewRotation)
{
	if (GAreScreenMessagesEnabled && IsRenderingStats)
	{
		const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();		
		const float ViewportOffsetX = Settings->ViewportOffsetX;
		const float ViewportOffsetY = Settings->ViewportOffsetY;
		const int32 ColumnSpacing = Settings->ColumnSpacing;
		const int32 StatDescriptionMaxLength = Settings->StatDescriptionMaxLength;
		const FLinearColor& BackgroundColor = Settings->BackgroundColor;
		const bool ShowPresetNames = Settings->ShowPresetNames;

		const UFont* Font = GEngine->GetLargeFont();
		const int32 RowHeight = FMath::TruncToInt(Font->GetMaxCharHeight() * 1.1f);

		X += ViewportOffsetX;
		Y += ViewportOffsetY;

		auto ShortenName = [&](const FString& LongName)
		{
			if (LongName.Len() > StatDescriptionMaxLength)
			{
				return FString(TEXT("...")) + LongName.Right(StatDescriptionMaxLength);
			}
			return LongName;
		};

		auto CalculateStatColor = [](double StatValue, double StatBudget)
		{
			FColor Color = FColor::Green;
			
			if (StatBudget > 0.)
			{
				if (StatValue > StatBudget)
				{
					Color = FColor::Red;
				}
				else if (StatValue > StatBudget * 0.75)
				{
					Color = FColor::Yellow;
				}
			}

			return Color;
		};

		int32 NumStatsToRender = 0;
		for (FName PresetName : EnabledPresets)
		{
			if (const UCustomStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
			{
				NumStatsToRender += StatPreset->StatsToDisplay.Num();
			}
		}

		if (NumStatsToRender > 0)
		{
			if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
			{
				const int32 NumRowsToDraw = NumStatsToRender + (ShowPresetNames ? EnabledPresets.Num() : 0);

				// padding and size are sort of magic numbers :^)
				const int32 UniformPadding = 8;
				const int32 PresetScopePadding = ShowPresetNames ? 8 : 0;
				const int32 StatValueTextWidth = 64;
				const int32 Width = ColumnSpacing + StatValueTextWidth + UniformPadding + PresetScopePadding;
				const int32 Height = RowHeight * NumRowsToDraw + 2 * UniformPadding;
				Canvas->DrawTile(X - UniformPadding, Y - UniformPadding, Width, Height, 0.f, 0.f, 1.f, 1.f, BackgroundColor);

				// lookup for counter stats
				TMap<FName, const FComplexStatMessage*> StatNameToCounterStats;
				for (const auto& Group : StatsData->ActiveStatGroups)
				{
					for (const FComplexStatMessage& CounterStatMessage : Group.CountersAggregate)
					{
						const FName StatName = CounterStatMessage.GetShortName();
						StatNameToCounterStats.Add(StatName, &CounterStatMessage);
					}
				}

				const FCustomStatEvaluationContext EvalulationContext{ StatsData->NameToStatMap, StatNameToCounterStats };

				for (FName PresetName : EnabledPresets)
				{
					if (ShowPresetNames)
					{
						Canvas->DrawShadowedString(X, Y, *PresetName.ToString(), Font, FColor::Green);
						Y += RowHeight;
					}

					if (const UCustomStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
					{
						const TArray<FCustomStat>& StatsToDisplay = StatPreset->StatsToDisplay;
						for (const FCustomStat& Stat : StatsToDisplay)
						{
							// default values for invalid stat
							const FString StatDescStr = ShortenName(Stat.StatDescription);
							FString StatValueStr = TEXT("N/A");
							FColor StatColor = FColor::Magenta;

							double StatValue;
							if (Stat.StatExpression && Stat.StatExpression->Evaluate(EvalulationContext, StatValue))
							{
								StatValueStr = FString::Printf(TEXT("%0.2f"), StatValue);
								StatColor = CalculateStatColor(StatValue, Stat.Budget);
							}

							Canvas->DrawShadowedString(X + PresetScopePadding, Y, *StatDescStr, Font, StatColor);
							Canvas->DrawShadowedString(X + PresetScopePadding + ColumnSpacing, Y, *StatValueStr, Font, StatColor);
							Y += RowHeight;
						}

					}
				}
			}
		}
		else
		{
			Canvas->DrawShadowedString(X, Y, TEXT("No preset selected!"), Font, FColor::Red);
		}
	}

	return Y;
}

bool FCustomStatsRenderer::OnToggleStats(UWorld* World, FCommonViewportClient* ViewportClient, const TCHAR* Stream)
{
	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();

	IsRenderingStats = !IsRenderingStats;
	
	TSet<FName> StatGroupsToToggle;
	for (FName PresetName : EnabledPresets)
	{
		if (const UCustomStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
		{
			for (const FCustomStat& Stat : StatPreset->StatsToDisplay)
			{
				if (Stat.StatExpression)
				{
					StatGroupsToToggle.Append(Stat.StatExpression->GetRequiredStatGroupNames());
				}
			}
		}
	}

	if (IsRenderingStats)
	{
		for (FName StatGroup : StatGroupsToToggle)
		{
			EnableStatGroup(StatGroup);
		}
	}
	else
	{
		for (FName StatGroup : StatGroupsToToggle)
		{
			DisableStatGroup(StatGroup);
		}
	}

	return false;
}

void FCustomStatsRenderer::EnableStatGroup(FName StatGroupName)
{
	if (GEngine)
	{
		if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
		{
			if (StatsData->GroupNames.Contains(StatGroupName))
			{
				// stat group is already active!
				return;
			}
		}

		FString GroupName = StatGroupName.ToString();
		if (GroupName.RemoveFromStart(TEXT("STATGROUP_")))
		{
			const FString StatCommand = FString::Printf(TEXT("stat %s -nodisplay"), *GroupName);
			GEngine->Exec(nullptr, *StatCommand);
		}
	}
}

void FCustomStatsRenderer::DisableStatGroup(FName StatGroupName)
{
	if (GEngine)
	{
		if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
		{
			if (!StatsData->GroupNames.Contains(StatGroupName))
			{
				// stat group is not active!
				return;
			}

			FString GroupName = StatGroupName.ToString();
			if (GroupName.RemoveFromStart(TEXT("STATGROUP_")))
			{
				const FString StatCommand = FString::Printf(TEXT("stat %s -nodisplay"), *GroupName);
				GEngine->Exec(nullptr, *StatCommand);
			}
		}
	}
}

void FCustomStatsRenderer::SetEnabledPresets(TArray<FName> NewPresets)
{
	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();

	// if rendering we need to enable/disable stat-groups accordingly
	if (IsRenderingStats)
	{
		TSet<FName> StatGroupsToDisable;
		for (FName PresetName : EnabledPresets)
		{
			if (const UCustomStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
			{
				for (const FCustomStat& Stat : StatPreset->StatsToDisplay)
				{
					if (Stat.StatExpression)
					{
						StatGroupsToDisable.Append(Stat.StatExpression->GetRequiredStatGroupNames());
					}
				}
			}
		}

		TSet<FName> StatGroupsToEnable;
		for (FName PresetName : NewPresets)
		{
			if (const UCustomStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
			{
				for (const FCustomStat& Stat : StatPreset->StatsToDisplay)
				{
					if (Stat.StatExpression)
					{
						const TArray<FName> RequiredGroupNames = Stat.StatExpression->GetRequiredStatGroupNames().Array();
						for (FName GroupName : RequiredGroupNames)
						{
							StatGroupsToEnable.Add(GroupName);
							StatGroupsToDisable.Remove(GroupName);
						}
					}
				}
			}
		}

		for (FName StatGroup : StatGroupsToDisable)
		{
			DisableStatGroup(StatGroup);
		}
		for (FName StatGroup : StatGroupsToEnable)
		{
			EnableStatGroup(StatGroup);
		}
	}

	EnabledPresets = NewPresets;
}

void FCustomStatsRenderer::SetPresets_Command(const TArray<FName>& PresetNames)
{
	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();

	if (PresetNames.Contains(NAME_None))
	{
		SetEnabledPresets(TArray<FName>());
	}
	else
	{
		TArray<FName> NewPresets;
		NewPresets.Reserve(PresetNames.Num());

		for (FName PresetName : PresetNames)
		{
			if (Settings->StatPresets.Find(PresetName))
			{
				NewPresets.AddUnique(PresetName);
			}
		}

		if (NewPresets.Num() > 0)
		{
			SetEnabledPresets(NewPresets);
		}
	}
}

void FCustomStatsRenderer::EnablePresets_Command(const TArray<FName>& PresetNames)
{
	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();
	
	TArray<FName> NewPresets = EnabledPresets;
	
	for (FName PresetName : PresetNames)
	{
		if (const UCustomStatPreset* StatPresetToEnable = Settings->GetPresetByName(PresetName))
		{			
			NewPresets.AddUnique(PresetName);
		}
	}
	
	// did we add any new preset?
	if (NewPresets.Num() > EnabledPresets.Num())
	{
		SetEnabledPresets(NewPresets);
	}
}

void FCustomStatsRenderer::DisablePresets_Command(const TArray<FName>& PresetNames)
{
	const UStatsVisualizerSettings* Settings = GetDefault<UStatsVisualizerSettings>();

	if (PresetNames.Contains(FName(TEXT("All"))))
	{
		SetEnabledPresets(TArray<FName>());
	}
	else
	{
		TArray<FName> NewPresets = EnabledPresets;

		for (FName PresetName : PresetNames)
		{
			if (const UCustomStatPreset* StatPresetToDisable = Settings->GetPresetByName(PresetName))
			{
				NewPresets.Remove(PresetName);
			}
		}

		// did we disable any preset?
		if (NewPresets.Num() < EnabledPresets.Num())
		{
			SetEnabledPresets(NewPresets);
		}
	}
}

#endif // #if STATS