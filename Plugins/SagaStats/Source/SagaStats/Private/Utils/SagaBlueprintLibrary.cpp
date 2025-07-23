/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Utils/SagaBlueprintLibrary.h"

#include "SagaAttributeSet.h"

FString USagaBlueprintLibrary::GetDebugStringFromExecutionData(const FSagaAttributeSetExecutionData& ExecutionData)
{
	return ExecutionData.ToString(LINE_TERMINATOR);
}

FString USagaBlueprintLibrary::GetDebugStringFromExecutionDataSeparator(const FSagaAttributeSetExecutionData& ExecutionData, const FString& Separator)
{
	return ExecutionData.ToString(Separator);
}

FString USagaBlueprintLibrary::GetDebugStringFromAttribute(const FGameplayAttribute& Attribute)
{
	return Attribute.GetName();
}

bool USagaBlueprintLibrary::NotEqual_GameplayAttributeGameplayAttribute(const FGameplayAttribute A, const FString B)
{
	return A.GetName() != B;
}

FText USagaBlueprintLibrary::GetAttributeDisplayNameText(const FGameplayAttribute& InAttribute)
{
	return FText::FromString(InAttribute.GetName());
}

bool USagaBlueprintLibrary::GetAttributeByPropertyName(const TSubclassOf<UAttributeSet>& AttributeSetClass, const FName& PropertyName, FGameplayAttribute& OutAttribute)
{
	if (FProperty* Property = FindFieldChecked<FProperty>(AttributeSetClass, PropertyName))
	{
		OutAttribute = FGameplayAttribute(Property, AttributeSetClass);
		return true;
	}
	return false;
}



UAbilitySystemComponent* USagaBlueprintLibrary::GetInstigatorAbilitySystemComponentFromContext(FGameplayEffectContextHandle EffectContextHandle)
{
	// 检查上下文句柄是否有效 / Check if the context handle is valid
	if (!EffectContextHandle.IsValid())
	{
		return nullptr;
	}

	// 获取上下文 / Get the context
	const FGameplayEffectContext* Context = EffectContextHandle.Get();
	if (!Context)
	{
		return nullptr;
	}

	// 获取发起者的能力系统组件 / Get the instigator's ability system component
	return Context->GetInstigatorAbilitySystemComponent();
}
