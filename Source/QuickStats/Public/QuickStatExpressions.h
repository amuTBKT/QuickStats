// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "QuickStatExpressions.generated.h"

struct QUICKSTATS_API FQuickStatEvaluationContext
{
#if STATS
	const TMap<FName, const FComplexStatMessage*>& Stats;
	const TMap<FName, const FComplexStatMessage*>& CounterStats;
#endif
};

UCLASS(Abstract, BlueprintType, EditInlineNew, CollapseCategories)
class QUICKSTATS_API UQuickStatExpression : public UObject
{
	GENERATED_BODY()

public:
	/*
	* Stat groups required to be active for this stat expression to evaluate.
	*/
	virtual TSet<FName> GetRequiredStatGroupNames() const { return TSet<FName>{}; }

	/*
	* Evaluates a stat expression and returns true if expression is valid.
	*/
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const { return false; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Constant"))
class QUICKSTATS_API UQuickStatExpressionConstant : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override { OutResult = Constant; return true; }

public:
	UPROPERTY(EditAnywhere, Category = "QuickStatExpression")
	double Constant = 0.;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct QUICKSTATS_API FCodeStatDefinition
{
	GENERATED_BODY()

	// Something like STATGROUP_*
	UPROPERTY(EditAnywhere, Category = "Stat Definition")
	FName StatGroupName = NAME_None;

	// Something like STAT_*
	UPROPERTY(EditAnywhere, Category = "Stat Definition")
	FName StatName = NAME_None;
};

UCLASS(meta = (DisplayName = "Read Stat"))
class QUICKSTATS_API UQuickStatExpressionReadStat : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const { return TSet<FName>{ StatDefinition.StatGroupName }; }

	/*
	* Expression can be invalid if 
	*	1. Stat name and group doesn't match code declaration.
	*	2. Stat is not ready (we didn't encounter any code updating the stat).
	*/	
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Category = "QuickStatExpression")
	FCodeStatDefinition StatDefinition;

	// If >=0 use default value in case stat is not available
	UPROPERTY(EditAnywhere, Category = "QuickStatExpression")
	double DefaultValue = -1.;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Add"))
class QUICKSTATS_API UQuickStatExpressionAdd : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	UQuickStatExpressionAdd();

	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	TArray<UQuickStatExpression*> Inputs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Subtract (A-B)"))
class QUICKSTATS_API UQuickStatExpressionSubtract : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	UQuickStatExpression* InputA = nullptr;

	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	UQuickStatExpression* InputB = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Multiply"))
class QUICKSTATS_API UQuickStatExpressionMultiply : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	UQuickStatExpressionMultiply();

	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	TArray<UQuickStatExpression*> Inputs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Divide (A/B)"))
class QUICKSTATS_API UQuickStatExpressionDivide : public UQuickStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	UQuickStatExpression* InputA = nullptr;

	UPROPERTY(EditAnywhere, Instanced, Category = "QuickStatExpression")
	UQuickStatExpression* InputB = nullptr;
};
