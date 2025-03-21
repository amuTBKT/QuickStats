// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#include "StatCustomization.h"

#include "Runtime/Launch/Resources/Version.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "String/Find.h"
#include "Editor.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FCodeStatDefinitionCustomization"

TMap<FName, TArray<FName>>                                        FCodeStatDefinitionCustomization::AvailableStatGroups;
TArray<FCodeStatDefinitionCustomization::FTreeNodePtr>            FCodeStatDefinitionCustomization::AvailableStatGroupNodes;
TArray<TArray<FCodeStatDefinitionCustomization::FTreeNodePtr>>    FCodeStatDefinitionCustomization::AvailableChildrenNodes;
TArray<FCodeStatDefinitionCustomization::FTreeNodePtr>            FCodeStatDefinitionCustomization::AvailableStatNodes;

class FStatGroupCollector final : public FOutputDevice
{
public:
	FStatGroupCollector(TMap<FName, TArray<FName>>& OutStatGroups)
		:StatGroups(OutStatGroups)
	{
		StatGroups.Reset();

		if (GLog)
		{
			GLog->AddOutputDevice(this);
			GEngine->Exec(nullptr, TEXT("stat group listall"));
		}
	}
	~FStatGroupCollector()
	{
		if (GLog)
		{
			GLog->FlushThreadedLogs();
			GLog->RemoveOutputDevice(this);
		}
	}
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		static const FName LogCategory = FName(TEXT("LogStatGroupEnableManager"));
		if (Category == LogCategory)
		{
			if (FCString::Strlen(V) > 0)
			{
				// for compatibility (UE4.27 doesn't have StringView.Find function)
				auto StringViewFind = [](const FStringView& StringView, const FStringView& Search, int32 StartPosition = 0)
				{
					const int32 Index = UE::String::FindFirst(StringView.RightChop(StartPosition), Search);
					return Index == INDEX_NONE ? INDEX_NONE : Index + StartPosition;
				};

				FStringView SanitizedParsingView = FStringView(V);

				// required to contain "//STATGROUP_"
				int32 GroupNameStartIndex = StringViewFind(SanitizedParsingView, TEXT("//STATGROUP_"));
				if (GroupNameStartIndex > INDEX_NONE)
				{
					// remove "//" from front
					SanitizedParsingView = SanitizedParsingView.Mid(GroupNameStartIndex + 2);

					// required to contain terminating "//"
					int32 GroupNameEndIndex = StringViewFind(SanitizedParsingView, TEXT("//"));
					if (GroupNameEndIndex > INDEX_NONE)
					{
						FName StatGroupName = FName(SanitizedParsingView.SubStr(0, GroupNameEndIndex));

						// start parsing stat name
						{
							// remove "//" from front
							SanitizedParsingView = SanitizedParsingView.Mid(GroupNameEndIndex + 2);

							// Optional: remove characters after "///"
							int32 Index = StringViewFind(SanitizedParsingView, TEXT("///"));
							if (Index > INDEX_NONE)
							{
								SanitizedParsingView = SanitizedParsingView.SubStr(0, Index);
							}

							// Optional: remove characters after "####"
							Index = StringViewFind(SanitizedParsingView, TEXT("####"));
							if (Index > INDEX_NONE)
							{
								SanitizedParsingView = SanitizedParsingView.SubStr(0, Index);
							}
						}
						FName StatName = FName(SanitizedParsingView);

						StatGroups.FindOrAdd(StatGroupName).Add(StatName);
					}
				}
			}
		}
	}
private:
	TMap<FName, TArray<FName>>& StatGroups;
};

TSharedRef<IPropertyTypeCustomization> FCodeStatDefinitionCustomization::MakeInstance()
{
	return MakeShareable(new FCodeStatDefinitionCustomization);
}

FCodeStatDefinitionCustomization::FCodeStatDefinitionCustomization()
{
	if (AvailableStatGroups.Num() == 0)
	{
		RefreshAvailableStats();
	}

	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}
}

FCodeStatDefinitionCustomization::~FCodeStatDefinitionCustomization()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

void FCodeStatDefinitionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;

	FSimpleDelegate PropertyChangedDelegatge = FSimpleDelegate::CreateLambda([this]()
		{
			RefreshDefinitionFromProperty();
		});
	StructPropertyHandle->SetOnPropertyResetToDefault(PropertyChangedDelegatge);
	StructPropertyHandle->SetOnPropertyValueChanged(PropertyChangedDelegatge);
	StructPropertyHandle->SetOnChildPropertyValueChanged(PropertyChangedDelegatge);
	RefreshDefinitionFromProperty();

	HeaderRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text_Lambda([this]()
		{
			return FText::FromString(StatExpressionDisplayName);
		})
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			StructPropertyHandle->CreatePropertyValueWidget(true)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Clear", "Clear"))
			.IsEnabled_Lambda([this]
			{
				return StructPropertyHandle->CanResetToDefault();
			})
			.OnClicked_Lambda([this]()
			{
				StructPropertyHandle->ResetToDefault();
				return FReply::Handled();
			})                
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(StatTreeMenuWidget, SComboButton)
			.HasDownArrow(true)
			.ContentPadding(1)
			.IsEnabled(true)
			.Clipping(EWidgetClipping::OnDemand)
			.OnGetMenuContent(this, &FCodeStatDefinitionCustomization::GetMenuContent)
			.OnMenuOpenChanged(this, &FCodeStatDefinitionCustomization::OnMenuOpenChanged)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Browse", "Browse"))
			]
		]
	];
}

void FCodeStatDefinitionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> StatGroupNameProperty = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCodeStatDefinition, StatGroupName));
	TSharedPtr<IPropertyHandle> StatNameProperty = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCodeStatDefinition, StatName));

	// allow user to manually edit the property as well.
	ChildBuilder.AddProperty(StatGroupNameProperty.ToSharedRef());
	ChildBuilder.AddProperty(StatNameProperty.ToSharedRef());
}

void FCodeStatDefinitionCustomization::RefreshDefinitionFromProperty()
{
	TArray<void*> RawData;
	StructPropertyHandle->AccessRawData(RawData);
	if ((RawData.Num() != 1) || (RawData.IsEmpty() || RawData[0] == nullptr))
	{
		return;
	}

	CurrentStatDefinition = *reinterpret_cast<FCodeStatDefinition*>(RawData[0]);

	if (CurrentStatDefinition.StatGroupName != NAME_None && CurrentStatDefinition.StatName != NAME_None)
	{
		FString StatGroupName = CurrentStatDefinition.StatGroupName.ToString();
		StatGroupName.RemoveFromStart(TEXT("STATGROUP_"));

		FString StatName = CurrentStatDefinition.StatName.ToString();
		StatName.RemoveFromStart(TEXT("STAT_"));

		StatExpressionDisplayName = StatGroupName + TEXT(" -> ") + StatName;
	}
	else
	{
		StatExpressionDisplayName = TEXT("None");
	}
}

TSharedRef<SWidget> FCodeStatDefinitionCustomization::GetMenuContent()
{
	FilterStringTokens.Reset();

	StatTreeWidget = SNew(STreeView<FTreeNodePtr>)
	.TreeItemsSource(&AvailableStatGroupNodes)
	.OnGenerateRow_Lambda([this](FTreeNodePtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		FString DisplayName = InItem->GetValueAsString();
		FText ToolTipText = FText::GetEmpty();

		// clamp display name for Stat nodes and show tooltip if display name is trimmed.
		constexpr int32 DisplayNameMaxLength = 50;
		if (InItem->IsStatNode() && (DisplayName.Len() > DisplayNameMaxLength))
		{
			ToolTipText = FText::FromString(DisplayName);

			DisplayName = FString(TEXT("...")) + DisplayName.Right(DisplayNameMaxLength);
		}

		return
			SNew(STableRow<FTreeNodePtr>, OwnerTable)
			[
				SNew(STextBlock)
				.Text(FText::FromString(DisplayName))
				.ToolTipText(ToolTipText)
			];
	})
	.OnGetChildren_Lambda([this](FTreeNodePtr Row, TArray<FTreeNodePtr>& OutChildren)
	{
		if (Row->IsStatGroupNode())
		{
			if ((FilterStringTokens.Num() > 0) && !FilterNodeCheck(Row.Get()))
			{
				OutChildren.Reserve(AvailableChildrenNodes[Row->GetChildrenIndex()].Num());
				for (const FTreeNodePtr& Child : AvailableChildrenNodes[Row->GetChildrenIndex()])
				{
					if (FilterNodeCheck(Child.Get()))
					{
						OutChildren.Add(Child);
					}
				}
			}
			else
			{
				OutChildren = AvailableChildrenNodes[Row->GetChildrenIndex()];
			}
		}
	})
	.SelectionMode(ESelectionMode::Single)
	.OnSelectionChanged_Lambda([this](FTreeNodePtr SelectedItem, ESelectInfo::Type SelectInfo)
	{
		if (SelectInfo == ESelectInfo::OnMouseClick)
		{
			if (SelectedItem->IsStatNode())
			{
				FTreeNodePtr StatGroupNode = AvailableStatGroupNodes[SelectedItem->GetParentIndex()];

				FString Value = FString::Printf(TEXT("(StatGroupName=\"%s\",StatName=\"%s\")"),
					*StatGroupNode->GetValueAsString(),
					*SelectedItem->GetValueAsString());
				StructPropertyHandle->SetValueFromFormattedString(Value);

				StatTreeMenuWidget->SetIsOpen(false);
			}
		}
	})
#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3)
	.OnKeyDownHandler_Lambda([this](const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
	{
		if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
		{
			TArray<FTreeNodePtr> Selection = StatTreeWidget->GetSelectedItems();
			FTreeNodePtr SelectedItem;
			if (!Selection.IsEmpty())
			{
				SelectedItem = Selection[0];
			}

			if (SelectedItem.IsValid() && SelectedItem->IsStatNode())
			{
				FTreeNodePtr StatGroupNode = AvailableStatGroupNodes[SelectedItem->GetParentIndex()];

				FString Value = FString::Printf(TEXT("(StatGroupName=\"%s\",StatName=\"%s\")"),
					*StatGroupNode->GetValueAsString(),
					*SelectedItem->GetValueAsString());
				StructPropertyHandle->SetValueFromFormattedString(Value);

				StatTreeMenuWidget->SetIsOpen(false);
			}

			return FReply::Handled();
		}

		return FReply::Unhandled();
	})
#endif
	.ItemHeight(16);
		
	return
		SNew(SBorder)
		.Padding(2)
		.BorderImage(FStyleDefaults::GetNoBrush())
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Top)
			[
				SNew(SHorizontalBox)
					
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.f)
				.Padding(2)
				[
					SAssignNew(StatFilterWidget, SSearchBox)
					.OnTextChanged(this, &FCodeStatDefinitionCustomization::OnFilterTextChanged)
				]
					
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("Refresh", "Refresh"))
					.OnClicked_Lambda([this]()
					{
						RefreshAvailableStats();

						RefreshStatTree();

						return FReply::Handled();
					})
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Top)
			.Padding(FMargin(4, 2))
			[
				SNew(SBox)
				.WidthOverride(400.0f)
				.HeightOverride(256.f)
				[
					StatTreeWidget.ToSharedRef()
				]
			]
		];
}

void FCodeStatDefinitionCustomization::OnMenuOpenChanged(bool bOpen) const
{
	if (bOpen)
	{
		ScrollTreeToSelectedNode();

		GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateLambda(
			[SearchBoxWidget=StatFilterWidget]()
			{
				if (SearchBoxWidget.IsValid())
				{
					FSlateApplication::Get().SetKeyboardFocus(SearchBoxWidget);
					FSlateApplication::Get().SetUserFocus(0, SearchBoxWidget);
				}
			}));
	}
}

void FCodeStatDefinitionCustomization::ScrollTreeToSelectedNode() const
{
	const FName& SelectedStatGroupName = CurrentStatDefinition.StatGroupName;
	const FName& SelectedStatName = CurrentStatDefinition.StatName;

	FTreeNodePtr* SelectedStatGroupNode = AvailableStatGroupNodes.FindByPredicate(
		[&](const FTreeNodePtr& Node)
		{
			return Node->GetStatGroupName() == SelectedStatGroupName;
		});

	if (SelectedStatGroupNode && SelectedStatGroupNode->IsValid())
	{
		// to handle cases where StatName is unset but StatGroup is valid.
		FTreeNodePtr* NodeToSelect = SelectedStatGroupNode;

		FTreeNodePtr* SelectedStatNode = AvailableStatNodes.FindByPredicate(
			[&](const FTreeNodePtr& Node)
			{
				return Node->GetStatName() == SelectedStatName && AvailableStatGroupNodes[Node->GetParentIndex()]->GetStatGroupName() == SelectedStatGroupName;
			});

		if (SelectedStatNode && SelectedStatNode->IsValid())
		{
			StatTreeWidget->SetItemExpansion(*SelectedStatGroupNode, true);

			NodeToSelect = SelectedStatNode;
		}

		StatTreeWidget->ClearSelection();
		StatTreeWidget->SetItemSelection(*NodeToSelect, true);
		StatTreeWidget->RequestScrollIntoView(*NodeToSelect);
	}
}

void FCodeStatDefinitionCustomization::RefreshStatTree()
{
	if (FilterStringTokens.Num() == 0)
	{
		StatTreeWidget->SetTreeItemsSource(&AvailableStatGroupNodes);
	}
	else
	{
		FilteredStatGroupNodes.Empty();

		for (const FTreeNodePtr& StatGroupNode : AvailableStatGroupNodes)
		{
			if (FilterChildrenCheck(StatGroupNode.Get()))
			{
				// if any child node passes the filter, expand group node.
				FilteredStatGroupNodes.Add(StatGroupNode);
				StatTreeWidget->SetItemExpansion(StatGroupNode, true);
			}
			else
			{
				if (FilterNodeCheck(StatGroupNode.Get()))
				{
					// add group node in collapsed state.
					FilteredStatGroupNodes.Add(StatGroupNode);
				}
				StatTreeWidget->SetItemExpansion(StatGroupNode, false);
			}
		}
		StatTreeWidget->SetTreeItemsSource(&FilteredStatGroupNodes);
	}

	StatTreeWidget->RequestTreeRefresh();

	if (FilterStringTokens.Num() == 0)
	{
		ScrollTreeToSelectedNode();
	}
}

void FCodeStatDefinitionCustomization::OnFilterTextChanged(const FText& InFilterText)
{
	FString FilterString = InFilterText.ToString().TrimStartAndEnd();
	if (FilterString.Len() > 0)
	{
		FilterString.ParseIntoArray(FilterStringTokens, TEXT(" "));
	}
	else
	{
		FilterStringTokens.Reset();
	}

	RefreshStatTree();
}

bool FCodeStatDefinitionCustomization::FilterNodeCheck(const FStatTreeNode* Node) const
{
	for (const FString& FilterString : FilterStringTokens)
	{
		if (!Node->GetValueAsString().Contains(FilterString))
		{
			return false;
		}
	}
	return true;
}

bool FCodeStatDefinitionCustomization::FilterChildrenCheck(const FStatTreeNode* Node) const
{
	if (Node->IsStatGroupNode())
	{
		for (const FTreeNodePtr& Child : AvailableChildrenNodes[Node->GetChildrenIndex()])
		{
			if (FilterNodeCheck(Child.Get()))
			{
				return true;
			}
		}
	}
	return false;
}

void FCodeStatDefinitionCustomization::RefreshAvailableStats()
{
	{
		FStatGroupCollector Collector(AvailableStatGroups);
	}

	AvailableStatGroupNodes.Reset(AvailableStatGroups.Num());
	AvailableChildrenNodes.SetNum(AvailableStatGroups.Num());
	AvailableStatNodes.Reset();

	int32 ChildIndex = 0;
	for (auto& Itr : AvailableStatGroups)
	{
		AvailableStatGroupNodes.Add(FStatTreeNode::MakeStatGroupNode(Itr.Key, ChildIndex));
		ChildIndex++;
	}
	AvailableStatGroupNodes.Sort([](const FTreeNodePtr& A, const FTreeNodePtr& B) { return A->GetStatGroupName().ToString() < B->GetStatGroupName().ToString(); });

	ChildIndex = 0;
	for (const auto& Itr : AvailableStatGroups)
	{
		int32 StatGroupIndex = AvailableStatGroupNodes.IndexOfByPredicate([StatGroupName=Itr.Key](const FTreeNodePtr& Node) { return StatGroupName == Node->GetStatGroupName(); });
		check(StatGroupIndex != INDEX_NONE);

		AvailableChildrenNodes[ChildIndex].Reset(Itr.Value.Num());
		for (FName StatName : Itr.Value)
		{
			AvailableChildrenNodes[ChildIndex].Add(FStatTreeNode::MakeStatNode(StatName, StatGroupIndex));
			AvailableStatNodes.Add(AvailableChildrenNodes[ChildIndex].Last());
		}
		ChildIndex += 1;
	}
}

#undef LOCTEXT_NAMESPACE