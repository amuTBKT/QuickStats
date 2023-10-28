// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "CustomStatExpressions.h"

bool UCustomStatExpressionReadStat::Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const
{
#if STATS
	if (const FComplexStatMessage* StatMessage = Context.Stats.FindRef(StatName))
	{
		OutResult = FPlatformTime::ToMilliseconds(StatMessage->GetValue_Duration(EComplexStatField::IncAve));
		return true;
	}
	else if (const FComplexStatMessage* CounterStatMessage = Context.CounterStats.FindRef(StatName))
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

UCustomStatExpressionAdd::UCustomStatExpressionAdd()
{
	Inputs.SetNum(2);
}

TSet<FName> UCustomStatExpressionAdd::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	for (UCustomStatExpression* Input : Inputs)
	{
		if (Input)
		{
			GroupNames.Append(Input->GetRequiredStatGroupNames());
		}
	}
	return GroupNames;
}

bool UCustomStatExpressionAdd::Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const
{
	OutResult = 0.;
	for (UCustomStatExpression* Input : Inputs)
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

TSet<FName> UCustomStatExpressionSubtract::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	if (InputA && InputB)
	{
		GroupNames.Append(InputA->GetRequiredStatGroupNames());
		GroupNames.Append(InputB->GetRequiredStatGroupNames());
	}
	return GroupNames;
}

bool UCustomStatExpressionSubtract::Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const
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

UCustomStatExpressionMultiply::UCustomStatExpressionMultiply()
{
	Inputs.SetNum(2);
}

TSet<FName> UCustomStatExpressionMultiply::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	for (UCustomStatExpression* Input : Inputs)
	{
		if (Input)
		{
			GroupNames.Append(Input->GetRequiredStatGroupNames());
		}
	}
	return GroupNames;
}

bool UCustomStatExpressionMultiply::Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const
{
	OutResult = 1.;
	for (UCustomStatExpression* Input : Inputs)
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

TSet<FName> UCustomStatExpressionDivide::GetRequiredStatGroupNames() const
{
	TSet<FName> GroupNames;
	if (InputA && InputB)
	{
		GroupNames.Append(InputA->GetRequiredStatGroupNames());
		GroupNames.Append(InputB->GetRequiredStatGroupNames());
	}
	return GroupNames;
}

bool UCustomStatExpressionDivide::Evaluate(const FCustomStatEvaluationContext& Context, double& OutResult) const
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