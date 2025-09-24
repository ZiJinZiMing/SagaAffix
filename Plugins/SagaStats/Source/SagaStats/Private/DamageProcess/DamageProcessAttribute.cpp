/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DamageProcessAttribute.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "DamageProcess/DamageProcessOperation.h"

void UDamageProcessAttribute::GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const
{
	// 将Children转换为UDamageProcessOperation*数组
	TArray<UDamageProcessOperation*> OrderedChildren;
	for (UDAGNode* Child : Children)
	{
		if (UDamageProcessOperation* OperationChild = Cast<UDamageProcessOperation>(Child))
		{
			OrderedChildren.Add(OperationChild);
		}
	}

	// 根据ModifierOp的int值进行排序，值越小越靠前
	OrderedChildren.Sort([this](const UDamageProcessOperation& A, const UDamageProcessOperation& B)
	{
		int32 ModifierOpA = MAX_int32; // 默认放到最后
		int32 ModifierOpB = MAX_int32;

		// 在A的Modifiers中查找匹配当前Attribute的第一个Modifier
		for (const FDamageProcessModifierInfo& Modifier : A.Modifiers)
		{
			if (Modifier.Attribute == this->Attribute)
			{
				ModifierOpA = static_cast<int32>(Modifier.ModifierOp);
				break; // 取第一个匹配的
			}
		}

		// 在B的Modifiers中查找匹配当前Attribute的第一个Modifier
		for (const FDamageProcessModifierInfo& Modifier : B.Modifiers)
		{
			if (Modifier.Attribute == this->Attribute)
			{
				ModifierOpB = static_cast<int32>(Modifier.ModifierOp);
				break; // 取第一个匹配的
			}
		}

		return ModifierOpA < ModifierOpB; // int值小的排在前面
	});

	// 转换回UDAGNode*数组
	OutOrderedChildren.Empty();
	for (UDamageProcessOperation* Child : OrderedChildren)
	{
		OutOrderedChildren.Add(Child);
	}
}

FString UDamageProcessAttribute::GetNodeDisplayName() const
{
	return this->Attribute.GetName();
}

FString UDamageProcessAttribute::GetMermaidNodeShape(const FString& NodeLabel) const
{
	// Attribute节点使用方框格式 / Attribute nodes use rectangle format
	return FString::Printf(TEXT("[\"%s\"]"), *NodeLabel);
}


void UDamageProcessAttribute::Execute()
{
	float BaseValue = 0;
	float AddBase = 0;
	float MultiplyAdditive = 1;
	float MultiplyCompound = 1;
	float AddFinal = 0;

	bool HasBaseValue = false;
	for (UDAGNode* Child : Children)
	{
		if (UDamageProcessOperation* OperationChild = Cast<UDamageProcessOperation>(Child))
		{
			float Magnitude = 0;
			int Index = OperationChild->GetAttributeMagnitude(this->Attribute, Magnitude);
			
			if (Index != -1)
			{
				const FDamageProcessModifierInfo& ModifierInfo = OperationChild->Modifiers[Index];
				
				switch (ModifierInfo.ModifierOp)
				{
					case EDamageProcessModOp::BaseValue:
						{
							if (!HasBaseValue)
							{
								HasBaseValue = true;
								BaseValue = Magnitude;
							}else
							{
								//todo:logerror
							}
						}
						break;
					case EDamageProcessModOp::AddBase:
						AddBase += Magnitude;
						break;
					case EDamageProcessModOp::MultiplyAdditive:
						MultiplyAdditive += (Magnitude - 1);
						break;
					case EDamageProcessModOp::MultiplyCompound:
						MultiplyCompound *= Magnitude;
						break;
					case EDamageProcessModOp::AddFinal:
						AddFinal += Magnitude;
					default:
						break;
				}
			}
		}
	}
	
	AttributeMagnitude = ((BaseValue + AddBase) * MultiplyAdditive  * MultiplyCompound) + AddFinal;

	if (OwnerActor != nullptr)
	{
		// 获取 AbilitySystemComponent
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor))
		{
			// 创建运行时 GameplayEffect Spec 来应用属性值
			UGameplayEffect* RuntimeGE = NewObject<UGameplayEffect>();
			RuntimeGE->DurationPolicy = EGameplayEffectDurationType::Instant;
			
			// 创建属性修改器
			FGameplayModifierInfo ModifierInfo;
			ModifierInfo.Attribute = this->Attribute;
			ModifierInfo.ModifierOp = EGameplayModOp::Override;
			
			// 设置修改器的量值
			FScalableFloat ScalableFloat(AttributeMagnitude);
			ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(ScalableFloat);
			
			// 添加修改器到 GameplayEffect
			RuntimeGE->Modifiers.Add(ModifierInfo);
			
			// 创建 GameplayEffect Spec 并应用
			FGameplayEffectSpec EffectSpec(RuntimeGE, FGameplayEffectContextHandle(), 1.0f);
			ASC->ApplyGameplayEffectSpecToSelf(EffectSpec);
		}
	}
}
