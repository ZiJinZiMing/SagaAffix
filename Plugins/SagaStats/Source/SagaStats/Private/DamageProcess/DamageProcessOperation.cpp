// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DamageProcessOperation.h"

#include "DamageProcess/DamageProcessAttribute.h"

bool FDamageProcessModifierInfo::operator==(const FDamageProcessModifierInfo& Other) const
{
	return Attribute == Other.Attribute && ModifierOp == Other.ModifierOp;
}

bool FDamageProcessModifierInfo::operator!=(const FDamageProcessModifierInfo& Other) const
{
	return !(*this == Other);
}


int UDamageProcessOperation::GetAttributeMagnitude(FGameplayAttribute Attribute, float& OutMagnitude) const
{
	for (int i = 0; i < Modifiers.Num(); i++)
	{
		if (Modifiers[i].Attribute == Attribute)
		{
			OutMagnitude = ModifierMagnitudes[i];
			return i;
		}
	}

	return -1;
}

int UDamageProcessOperation::SetAttributeMagnitude(FGameplayAttribute Attribute, float InMagnitude)
{
	for (int i = 0; i < Modifiers.Num(); i++)
	{
		if (Modifiers[i].Attribute == Attribute)
		{
			ModifierMagnitudes[i] = InMagnitude;
			return i;
		}
	}
	return -1;
}

void UDamageProcessOperation::PostInitProperties()
{
	Super::PostInitProperties();

	ModifierMagnitudes.SetNum(Modifiers.Num());
	
}

void UDamageProcessOperation::GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const
{
	// 将Children转换为UDamageProcessAttribute*数组
	TArray<UDamageProcessAttribute*> OrderedChildren;
	for (UDAGNode* Child : Children)
	{
		if (UDamageProcessAttribute* AttributeChild = Cast<UDamageProcessAttribute>(Child))
		{
			OrderedChildren.Add(AttributeChild);
		}
	}

	// 根据BackingAttributes中的索引进行排序，索引越小越靠前
	OrderedChildren.Sort([this](const UDamageProcessAttribute& A, const UDamageProcessAttribute& B)
	{
		int32 IndexA = BackingAttributes.IndexOfByKey(A.Attribute);
		int32 IndexB = BackingAttributes.IndexOfByKey(B.Attribute);

		//todo:logerror
		// 如果Attribute不在BackingAttributes中，放到最后
		if (IndexA == INDEX_NONE) IndexA = MAX_int32;
		if (IndexB == INDEX_NONE) IndexB = MAX_int32;

		return IndexA < IndexB; // 索引小的排在前面
	});

	// 转换回UDAGNode*数组
	OutOrderedChildren.Empty();
	for (UDamageProcessAttribute* Child : OrderedChildren)
	{
		OutOrderedChildren.Add(Child);
	}
}

FString UDamageProcessOperation::GetNodeDisplayName() const
{
	return this->OperationName;
}

FString UDamageProcessOperation::GetMermaidNodeShape(const FString& NodeLabel) const
{
	// Operation节点使用圆角方框格式 / Operation nodes use rounded rectangle format
	return FString::Printf(TEXT("(\"%s\")"), *NodeLabel);
}

void UDamageProcessOperation::Execute()
{
	OnNodeExecuted();
}
