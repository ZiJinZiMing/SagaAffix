/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SagaAbilitySystemComponent.generated.h"

class USagaMeterBase;
class USagaDecreaseMeter;
enum class EMeterState : uint8;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributeSetAddOrRemoveEvent, UAttributeSet* /*AttributeSet*/, bool /*IsAdd*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeterEmptiedEvent, USagaMeterBase* /*Meter*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeterFilledEvent, USagaMeterBase* /*Meter*/)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMeterStateChangeEvent, USagaDecreaseMeter* /*Meter*/, EMeterState /*OldState*/)


UCLASS(ClassGroup=(AbilitySystem), meta=(BlueprintSpawnableComponent))
class SAGASTATS_API USagaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Attribute")
	void RemoveAttributeSet(UAttributeSet* AttributeSet);

	UFUNCTION(BlueprintCallable, Category="Attribute")
	void RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass);

	FOnAttributeSetAddOrRemoveEvent& GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass);
	
	FOnMeterEmptiedEvent& GetMeterEmptiedDelegate(TSubclassOf<USagaMeterBase> MeterClass);
	
	FOnMeterFilledEvent& GetMeterFilledDelegate(TSubclassOf<USagaMeterBase> MeterClass);
	
	FOnMeterStateChangeEvent& GetMeterStateChangeDelegate(TSubclassOf<USagaDecreaseMeter> MeterClass);
	
	static void OnShowMeterDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& X, float& YPos);

protected:
	TMap<TSubclassOf<UAttributeSet>, FOnAttributeSetAddOrRemoveEvent> AttributeSetAddOrRemoveDelegates;
	
	TMap<TSubclassOf<USagaMeterBase>, FOnMeterEmptiedEvent> MeterEmptiedDelegates;

	TMap<TSubclassOf<USagaMeterBase>, FOnMeterFilledEvent> MeterFilledDelegates;
	
	TMap<TSubclassOf<USagaDecreaseMeter>, FOnMeterStateChangeEvent> MeterStateChangeDelegates;
	
protected:
	/** Add a new attribute set */
	virtual void AddSpawnedAttribute(UAttributeSet* AttributeSet) override;

	/** Remove an existing attribute set */
	virtual void RemoveSpawnedAttribute(UAttributeSet* AttributeSet) override;

	/** Remove all attribute sets */
	virtual void RemoveAllSpawnedAttributes() override;

	virtual void OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes) override;

private:
	/** 绘制所有Meter的进度条 / Draw progress bars for all meters */
	void DrawMeterProgressBars(class UCanvas* Canvas, float& YPos);

	/** 绘制单个Meter的进度条 / Draw progress bar for a single meter */
	void DrawSingleMeterProgress(class UCanvas* Canvas, class USagaMeterBase* Meter, FVector2D Position, FVector2D Size);

	/** 根据Meter状态获取名称颜色 / Get meter name color based on state */
	FColor GetMeterNameColor(class USagaMeterBase* Meter);

	/** 根据Meter状态获取进度条颜色 / Get progress bar color based on meter state */
	FColor GetMeterProgressBarColor(class USagaMeterBase* Meter);

	/** 绘制Meter自动恢复信息 / Draw meter auto-recovery info */
	void DrawMeterRecoveryInfo(class UCanvas* Canvas, class USagaMeterBase* Meter, FVector2D Position);

	/** 获取Meter的CD信息字符串 / Get meter CD info string */
	FString GetMeterCDInfo(class USagaMeterBase* Meter);

	/** 获取格式化的Meter名称 / Get formatted meter name */
	FString GetFormattedMeterName(class USagaMeterBase* Meter);

	/** 获取Meter的属性信息字符串 / Get meter attribute info string */
	FString GetMeterAttributeInfo(class USagaMeterBase* Meter);

	/** 绘制多行属性信息 / Draw multi-line attribute info */
	void DrawMeterAttributeInfo(class UCanvas* Canvas, class USagaMeterBase* Meter, FVector2D Position, float MaxWidth);

	/** 绘制边框 / Draw border */
	void DrawBorder(class UCanvas* Canvas, FVector2D Position, FVector2D Size, float BorderWidth);

	/** 获取所有Meter实例 / Get all meter instances */
	TArray<class USagaMeterBase*> GetAllMeters() const;

	/** 绘制多色文本行 / Draw multi-color text line */
	void DrawMultiColorLine(class UCanvas* Canvas, float StartX, float YPos, const TArray<TPair<FString, FColor>>& ColoredTexts, class UFont* Font);

	// ----------------------------------------------------------------------------------------------------------------
	//	Abilities
	// ----------------------------------------------------------------------------------------------------------------
};
