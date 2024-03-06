// Copyright 2023 Amit Kumar Mehar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "QuickStatExpressions.h"

#include "IPropertyTypeCustomization.h"
#include "EditorUndoClient.h"

template <typename ItemType> class STreeView;
class SSearchBox;

class FCodeStatDefinitionCustomization : public IPropertyTypeCustomization, public FEditorUndoClient
{
	class FStatTreeNode
	{
	public:
		static TSharedPtr<FStatTreeNode> MakeStatGroupNode(FName InStatGroupName, int32 InChildrenIndex)
		{
			TSharedPtr<FStatTreeNode> Node = MakeShared<FStatTreeNode>();
			Node->bIsStatGroupNode = true;
			Node->StatGroupOrStatName = InStatGroupName;
			Node->ParentOrChildrenIndex = InChildrenIndex;
			return Node;
		}
		static TSharedPtr<FStatTreeNode> MakeStatNode(FName InStatName, int32 InParentIndex)
		{
			TSharedPtr<FStatTreeNode> Node = MakeShared<FStatTreeNode>();
			Node->bIsStatGroupNode = false;
			Node->StatGroupOrStatName = InStatName;
			Node->ParentOrChildrenIndex = InParentIndex;
			return Node;
		}

		bool IsStatGroupNode() const { return bIsStatGroupNode; }
		bool IsStatNode() const { return !IsStatGroupNode(); }

		FName GetStatGroupName() const
		{
			check(IsStatGroupNode());
			return StatGroupOrStatName;
		}
		int32 GetChildrenIndex() const
		{
			check(IsStatGroupNode());
			return ParentOrChildrenIndex;
		}

		FName GetStatName() const
		{
			check(IsStatNode());
			return StatGroupOrStatName;
		}
		int32 GetParentIndex() const
		{
			check(IsStatNode());
			return ParentOrChildrenIndex;
		}

		FString GetValueAsString() const
		{
			return StatGroupOrStatName.ToString();
		}

	private:
		FName StatGroupOrStatName = NAME_None;
		int32 ParentOrChildrenIndex = INDEX_NONE;
		bool bIsStatGroupNode = true;
	};
	using FTreeNodePtr = TSharedPtr<FStatTreeNode>;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	FCodeStatDefinitionCustomization();
	~FCodeStatDefinitionCustomization();

	// FEditorUndoClient interface
	virtual void PostUndo(bool bSuccess) override
	{
		if (bSuccess)
		{
			RefreshDefinitionFromProperty();
		}
	}
	virtual void PostRedo(bool bSuccess) override
	{
		PostUndo(bSuccess);
	}

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	
private:
	void RefreshDefinitionFromProperty();

	TSharedRef<SWidget> GetMenuContent();
	void OnMenuOpenChanged(bool bOpen) const;
	
	void ScrollTreeToSelectedNode() const;
	void RefreshStatTree();

	void OnFilterTextChanged(const FText& InFilterText);
	bool FilterNodeCheck(const FStatTreeNode* Node) const;
	bool FilterChildrenCheck(const FStatTreeNode* Node) const;

	static void RefreshAvailableStats();

private:
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	FCodeStatDefinition CurrentStatDefinition;
	FString StatExpressionDisplayName;

	TSharedPtr<STreeView<FTreeNodePtr>> StatTreeWidget;
	TSharedPtr<SSearchBox> StatFilterWidget;
	TArray<FString> FilterStringTokens;

	TArray<FTreeNodePtr> FilteredStatGroupNodes;

	// StatGroupName to children Stats
	static TMap<FName, TArray<FName>>   AvailableStatGroups;
	// All StatGroup nodes
	static TArray<FTreeNodePtr>         AvailableStatGroupNodes;
	// Children Stat node lookup
	static TArray<TArray<FTreeNodePtr>> AvailableChildrenNodes;
	// All Stat nodes, mostly used for searching nodes easily
	static TArray<FTreeNodePtr>         AvailableStatNodes;
};