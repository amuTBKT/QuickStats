// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "QuickStatsRenderer.h"

#if STATS

#include "QuickStatSettings.h"
#include "String/ParseTokens.h"
#include "Stats/StatsData.h"

#include "Misc/CommandLine.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

bool			FQuickStatsRenderer::bIsRenderingStats = false;
TArray<FName>	FQuickStatsRenderer::EnabledPresets;
TSet<FName>		FQuickStatsRenderer::EnabledStatGroups;

const FName		FQuickStatsRenderer::QuickStatsPresetName = FName(TEXT("STAT_QuickStats"));
const FName		FQuickStatsRenderer::QuickStatsPresetCategory = FName(TEXT("STATCAT_QuickStats"));
const FText		FQuickStatsRenderer::QuickStatsPresetDescription = FText::FromString(FString(TEXT("Visualizer for quick stats.")));

FDelegateHandle FQuickStatsRenderer::ConsoleAutoCompleteHandle;
FDelegateHandle FQuickStatsRenderer::OnObjectPropertyChangedHandle;

static TAutoConsoleVariable<FString> CVarEnabledPresets(
	TEXT("qstats.Presets"),
	TEXT(""),
	TEXT("Presets to enable on boot, can be overriden by -statpresets"),
	ECVF_ReadOnly);

static FAutoConsoleCommand SetPresetCommand(
	TEXT("qstats.SetPresets"),
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
				FQuickStatsRenderer::SetPresets_Command(Presets);
			}
		}
	)
);

static FAutoConsoleCommand EnablePresetCommand(
	TEXT("qstats.EnablePresets"),
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
				FQuickStatsRenderer::EnablePresets_Command(Presets);
			}
		}
	)
);

static FAutoConsoleCommand DisablePresetCommand(
	TEXT("qstats.DisablePresets"),
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
				FQuickStatsRenderer::DisablePresets_Command(Presets);
			}
		}
	)
);

void FQuickStatsRenderer::RegisterStatPresets()
{
	checkf(GEngine, TEXT("GEngine is not valid, the stat visualizer won't be functional!"));
	if (GEngine)
	{
		UEngine::FEngineStatRender RenderFunc = UEngine::FEngineStatRender::CreateStatic(&FQuickStatsRenderer::OnRenderStats);
		UEngine::FEngineStatToggle ToggleFunc = UEngine::FEngineStatToggle::CreateStatic(&FQuickStatsRenderer::OnToggleStats);
		GEngine->AddEngineStat(QuickStatsPresetName, QuickStatsPresetCategory, QuickStatsPresetDescription, RenderFunc, ToggleFunc, false);
	}

	ConsoleAutoCompleteHandle = UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&FQuickStatsRenderer::PopulateAutoCompletePresetNames);

#if WITH_EDITOR
	OnObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FQuickStatsRenderer::OnObjectPropertyChanged);
#endif

	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();
	check(Settings);

	// check commandline for enabled presets
	FString RequestedPresets = TEXT("");
	if (!FParse::Value(FCommandLine::Get(), TEXT("-qstatpresets="), RequestedPresets, false))
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
					UE_LOG(LogTemp, Warning, TEXT("Preset(%s) is not defined in QuickStatSettings!"), *PresetName.ToString());
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

void FQuickStatsRenderer::UnregisterStatPresets()
{
	if (GEngine)
	{
		GEngine->RemoveEngineStat(QuickStatsPresetName);
	}

	UConsole::RegisterConsoleAutoCompleteEntries.Remove(ConsoleAutoCompleteHandle);

#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnObjectPropertyChangedHandle);
#endif
}

#if WITH_EDITOR
void FQuickStatsRenderer::OnObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& InChangeEvent)
{
	if (InObject->IsA(UQuickStatPreset::StaticClass()))
	{
		SetEnabledPresets(EnabledPresets);
	}
}
#endif

void FQuickStatsRenderer::PopulateAutoCompletePresetNames(TArray<FAutoCompleteCommand>& AutoCompleteList)
{
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();

	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();
	check(Settings);

	// disable all presets
	{
		FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
		AutoCompleteCommand.Command = TEXT("qstats.SetPresets None");
		AutoCompleteCommand.Desc = TEXT("Disable all presets");
		AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
	}
	{
		FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
		AutoCompleteCommand.Command = TEXT("qstats.DisablePresets All");
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
			AutoCompleteCommand.Command = *FString::Printf(TEXT("qstats.SetPresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Set this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}

		// enable preset
		{
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
			AutoCompleteCommand.Command = *FString::Printf(TEXT("qstats.EnablePresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Enable this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}

		// disable preset
		{
			FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.AddDefaulted_GetRef();
			AutoCompleteCommand.Command = *FString::Printf(TEXT("qstats.DisablePresets %s"), *PresetName);
			AutoCompleteCommand.Desc = TEXT("Disable this preset");
			AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
		}
	}
}

int32 FQuickStatsRenderer::OnRenderStats(UWorld* World, FViewport* Viewport, FCanvas* Canvas, int32 X, int32 Y, const FVector* ViewLocation, const FRotator* ViewRotation)
{
	if (GAreScreenMessagesEnabled && bIsRenderingStats)
	{
		const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();
		const float ViewportOffsetX = Settings->ViewportOffsetX;
		const float ViewportOffsetY = Settings->ViewportOffsetY;
		const int32 ColumnSpacing = Settings->ColumnSpacing;
		const int32 StatDescriptionMaxLength = Settings->StatDescriptionMaxLength;
		const FLinearColor& BackgroundColor = Settings->BackgroundColor;
		const bool bShowPresetNames = Settings->ShowPresetNames;

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
			if (const UQuickStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
			{
				NumStatsToRender += StatPreset->StatsToDisplay.Num();
			}
		}

		if (NumStatsToRender > 0)
		{
			if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
			{
				const int32 NumRowsToDraw = NumStatsToRender + (bShowPresetNames ? EnabledPresets.Num() : 0);

				// padding and size are sort of magic numbers :^)
				const int32 UniformPadding = 8;
				const int32 PresetScopePadding = bShowPresetNames ? 8 : 0;
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

				const FQuickStatEvaluationContext EvaluationContext{ StatsData->NameToStatMap, StatNameToCounterStats };

				for (FName PresetName : EnabledPresets)
				{
					if (bShowPresetNames)
					{
						Canvas->DrawShadowedString(X, Y, *PresetName.ToString(), Font, FColor::Green);
						Y += RowHeight;
					}

					if (const UQuickStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
					{
						const TArray<FQuickStat>& StatsToDisplay = StatPreset->StatsToDisplay;
						for (const FQuickStat& Stat : StatsToDisplay)
						{
							// default values for invalid stat
							const FString StatDescStr = ShortenName(Stat.StatDescription);
							FString StatValueStr = TEXT("N/A");
							FColor StatColor = FColor::Magenta;

							double StatValue;
							if (Stat.StatExpression && Stat.StatExpression->Evaluate(EvaluationContext, StatValue))
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

bool FQuickStatsRenderer::OnToggleStats(UWorld* World, FCommonViewportClient* ViewportClient, const TCHAR* Stream)
{
	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();

	bIsRenderingStats = !bIsRenderingStats;
	
	TSet<FName> StatGroupsToToggle;
	for (FName PresetName : EnabledPresets)
	{
		if (const UQuickStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
		{
			for (const FQuickStat& Stat : StatPreset->StatsToDisplay)
			{
				if (Stat.StatExpression)
				{
					StatGroupsToToggle.Append(Stat.StatExpression->GetRequiredStatGroupNames());
				}
			}
		}
	}

	if (bIsRenderingStats)
	{
		for (FName StatGroup : StatGroupsToToggle)
		{
			EnableStatGroup(StatGroup);
		}

		EnabledStatGroups = StatGroupsToToggle;
	}
	else
	{
		for (FName StatGroup : StatGroupsToToggle)
		{
			DisableStatGroup(StatGroup);
		}

		EnabledStatGroups.Reset();
	}

	return false;
}

void FQuickStatsRenderer::EnableStatGroup(FName StatGroupName)
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

void FQuickStatsRenderer::DisableStatGroup(FName StatGroupName)
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

void FQuickStatsRenderer::SetEnabledPresets(TArray<FName> NewPresets)
{
	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();

	// if rendering we need to enable/disable stat-groups accordingly
	if (bIsRenderingStats)
	{
		TSet<FName> StatGroupsToDisable = EnabledStatGroups;

		TSet<FName> StatGroupsToEnable;
		for (FName PresetName : NewPresets)
		{
			if (const UQuickStatPreset* StatPreset = Settings->GetPresetByName(PresetName))
			{
				for (const FQuickStat& Stat : StatPreset->StatsToDisplay)
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

		EnabledStatGroups = StatGroupsToEnable;
	}

	EnabledPresets = NewPresets;
}

void FQuickStatsRenderer::SetPresets_Command(const TArray<FName>& PresetNames)
{
	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();

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

void FQuickStatsRenderer::EnablePresets_Command(const TArray<FName>& PresetNames)
{
	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();
	
	TArray<FName> NewPresets = EnabledPresets;
	
	for (FName PresetName : PresetNames)
	{
		if (const UQuickStatPreset* StatPresetToEnable = Settings->GetPresetByName(PresetName))
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

void FQuickStatsRenderer::DisablePresets_Command(const TArray<FName>& PresetNames)
{
	const UQuickStatSettings* Settings = GetDefault<UQuickStatSettings>();

	if (PresetNames.Contains(FName(TEXT("All"))))
	{
		SetEnabledPresets(TArray<FName>());
	}
	else
	{
		TArray<FName> NewPresets = EnabledPresets;

		for (FName PresetName : PresetNames)
		{
			if (const UQuickStatPreset* StatPresetToDisable = Settings->GetPresetByName(PresetName))
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
