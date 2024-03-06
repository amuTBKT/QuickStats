// Copyright 2023-2024 Amit Kumar Mehar. All Rights Reserved.

#include "QuickStatExpressions.h"

bool UQuickStatExpressionReadStat::Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const
{
#if STATS
	if (const FComplexStatMessage* StatMessage = Context.Stats.FindRef(StatDefinition.StatName))
	{
		OutResult = FPlatformTime::ToMilliseconds(StatMessage->GetValue_Duration(EComplexStatField::IncAve));
		return true;
	}
	else if (const FComplexStatMessage* CounterStatMessage = Context.CounterStats.FindRef(StatDefinition.StatName))
	{
		if (CounterStatMessage->NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_double)
		{
			OutResult = CounterStatMessage->GetValue_double(EComplexStatField::IncAve);
			return true;
		}
		else if (CounterStatMessage->NameAndInfo.GetField<EStatDataType>() == EStatDataType::ST_int64)
		{
			OutResult = CounterStatMessage->GetValue_int64(EComplexStatField::IncAve);
			return true;
		}
	}

	// some stat are not always available (occluded primitives can be zero for example)
	if (DefaultValue >= 0.)
	{
		OutResult = DefaultValue;
		return true;
	}
#endif // #if STATS

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

UQuickStatExpressionAdd::UQuickStatExpressionAdd()
{
	Inputs.SetNum(2);
}

TSet<FName> UQuickStatExpressionAdd::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	for (UQuickStatExpression* Input : Inputs)
	{
		if (Input)
		{
			GroupNames.Append(Input->GetRequiredStatGroupNames());
		}
	}
	return GroupNames;
}

bool UQuickStatExpressionAdd::Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const
{
	OutResult = 0.;
	for (UQuickStatExpression* Input : Inputs)
	{
		double StatValue;
		if (Input && Input->Evaluate(Context, StatValue))
		{
			OutResult += StatValue;
		}
		else
		{
			// encountered at least one invalid stat
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TSet<FName> UQuickStatExpressionSubtract::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	if (InputA && InputB)
	{
		GroupNames.Append(InputA->GetRequiredStatGroupNames());
		GroupNames.Append(InputB->GetRequiredStatGroupNames());
	}
	return GroupNames;
}

bool UQuickStatExpressionSubtract::Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const
{
	if (InputA && InputB)
	{
		double StatA, StatB;
		if (InputA->Evaluate(Context, StatA) && InputB->Evaluate(Context, StatB))
		{
			OutResult = StatA - StatB;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

UQuickStatExpressionMultiply::UQuickStatExpressionMultiply()
{
	Inputs.SetNum(2);
}

TSet<FName> UQuickStatExpressionMultiply::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	for (UQuickStatExpression* Input : Inputs)
	{
		if (Input)
		{
			GroupNames.Append(Input->GetRequiredStatGroupNames());
		}
	}
	return GroupNames;
}

bool UQuickStatExpressionMultiply::Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const
{
	OutResult = 1.;
	for (UQuickStatExpression* Input : Inputs)
	{
		double StatValue;
		if (Input && Input->Evaluate(Context, StatValue))
		{
			OutResult *= StatValue;
		}
		else
		{
			// encountered at least one invalid stat
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TSet<FName> UQuickStatExpressionDivide::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	if (InputA && InputB)
	{
		GroupNames.Append(InputA->GetRequiredStatGroupNames());
		GroupNames.Append(InputB->GetRequiredStatGroupNames());
	}
	return GroupNames;
}

bool UQuickStatExpressionDivide::Evaluate(const FQuickStatEvaluationContext& Context, double& OutResult) const
{
	if (InputA && InputB)
	{
		double StatA, StatB;
		if (InputA->Evaluate(Context, StatA) && InputB->Evaluate(Context, StatB))
		{
			if (StatB == 0)
			{
				return false;
			}

			OutResult = StatA / StatB;
			return true;
		}
	}
	return false;
}
