// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "QuickStatExpressions.h"
#include "Engine/DataAsset.h"
#include "QuickStatSettings.generated.h"

USTRUCT()
struct QUICKSTATS_API FQuickStat
{
	GENERATED_BODY()

	// Stat description used by visualizer
	UPROPERTY(EditAnywhere, Category = "Quick Stat")
	FString StatDescription = TEXT("STAT_DESC");

	// Stat expression to evaluate
	UPROPERTY(EditAnywhere, Instanced, Category = "Quick Stat")
	UQuickStatExpression* StatExpression = nullptr;

	/*
	Budget allocated to the stat, used for coloring.
	> Budget		= Red
	> 75% of Budget	= Yellow
	< 75% of Budget	= Green
	*/
	UPROPERTY(EditAnywhere, Category = "Quick Stat")
	double Budget = 0.;
};

UCLASS()
class QUICKSTATS_API UQuickStatPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Stat Preset")
	TArray<FQuickStat> StatsToDisplay;
};

UCLASS(config = QuickStats, defaultconfig, meta = (DisplayName = "Quick Stats"))
class QUICKSTATS_API UQuickStatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual FName GetCategoryName() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	const UQuickStatPreset* GetPresetByName(FName PresetName) const;

public:
	// List of stats to display
	UPROPERTY(config, EditAnywhere, Category = "Stats")
	TMap<FName, TSoftObjectPtr<UQuickStatPreset>> StatPresets;

	// Horizontal offset to start stat rendering
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	int32 ViewportOffsetX = -50;

	// Vertical offset to start stat rendering
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	int32 ViewportOffsetY = 128;

	// Spacing between stat name and value
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	int32 ColumnSpacing = 256;

	// Maximum length after which stat description is trimmed
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	int32 StatDescriptionMaxLength = 32;

	// Background tint color
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	// Whether to show Preset categories
	UPROPERTY(config, EditAnywhere, Category = "Layout")
	bool ShowPresetNames = true;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UQuickStatPreset>> LoadedStatPresets;
};
