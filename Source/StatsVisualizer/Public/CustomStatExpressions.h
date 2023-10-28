// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "CustomStatExpressions.generated.h"

struct STATSVISUALIZER_API FCustomStatEvaluationContext
{
#if STATS
	const TMap<FName, const FComplexStatMessage*>& Stats;
	const TMap<FName, const FComplexStatMessage*>& CounterStats;
#endif
};

UCLASS(Abstract, BlueprintType, EditInlineNew, CollapseCategories)
class STATSVISUALIZER_API UCustomStatExpression : public UObject
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
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const { return false; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Constant"))
class STATSVISUALIZER_API UCustomStatExpressionConstant : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override { OutResult = Constant; return true; }

public:
	UPROPERTY(EditAnywhere)
	double Constant = 0.;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Read Stat"))
class STATSVISUALIZER_API UCustomStatExpressionReadStat : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const { return TSet<FName>{ StatGroupName }; }

	/*
	* Expression can be invalid if 
	*	1. Stat name and group doesn't match code declaration.
	*	2. Stat is not ready (we didn't encounter any code updating the stat).
	*/	
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override;

public:
	// StatName should match code STAT_*
	UPROPERTY(EditAnywhere)
	FName StatName = NAME_None;

	// StatName should match code STATGROUP_*
	UPROPERTY(EditAnywhere)
	FName StatGroupName = NAME_None;

	// If >=0 use default value in case stat is not available
	UPROPERTY(EditAnywhere)
	double DefaultValue = -1.;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Add"))
class STATSVISUALIZER_API UCustomStatExpressionAdd : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	UCustomStatExpressionAdd();

	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced)
	TArray<UCustomStatExpression*> Inputs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Subtract (A-B)"))
class STATSVISUALIZER_API UCustomStatExpressionSubtract : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced)
	UCustomStatExpression* InputA = nullptr;

	UPROPERTY(EditAnywhere, Instanced)
	UCustomStatExpression* InputB = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Multiply"))
class STATSVISUALIZER_API UCustomStatExpressionMultiply : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	UCustomStatExpressionMultiply();

	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced)
	TArray<UCustomStatExpression*> Inputs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Divide (A/B)"))
class STATSVISUALIZER_API UCustomStatExpressionDivide : public UCustomStatExpression
{
	GENERATED_BODY()

public:
	virtual TSet<FName> GetRequiredStatGroupNames() const override;
	virtual bool Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const override;

public:
	UPROPERTY(EditAnywhere, Instanced)
	UCustomStatExpression* InputA = nullptr;

	UPROPERTY(EditAnywhere, Instanced)
	UCustomStatExpression* InputB = nullptr;
};