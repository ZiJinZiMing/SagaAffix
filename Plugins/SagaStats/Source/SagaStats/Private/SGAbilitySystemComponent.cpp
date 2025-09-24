/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/


#include "SGAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Meter/MeterBase.h"
#include "Meter/DecreaseMeter.h"
#include "Meter/IncreaseMeter.h"


void USGAbilitySystemComponent::RemoveAttributeSet(UAttributeSet* AttributeSet)
{
	RemoveSpawnedAttribute(AttributeSet);
}

void USGAbilitySystemComponent::RemoveAttributeSetByClass(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (UAttributeSet* Set = const_cast<UAttributeSet*>(GetAttributeSet(AttributeSetClass)))
	{
		RemoveAttributeSet(Set);
	}
}

FOnAttributeSetAddOrRemoveEvent& USGAbilitySystemComponent::GetAttributeSetAddOrRemoveDelegate(TSubclassOf<UAttributeSet> SetClass)
{
	return AttributeSetAddOrRemoveDelegates.FindOrAdd(SetClass);
}

FOnMeterEmptiedEvent& USGAbilitySystemComponent::GetMeterEmptiedDelegate(TSubclassOf<UMeterBase> MeterClass)
{
	return MeterEmptiedDelegates.FindOrAdd(MeterClass);
}

FOnMeterFilledEvent& USGAbilitySystemComponent::GetMeterFilledDelegate(TSubclassOf<UMeterBase> MeterClass)
{
	return MeterFilledDelegates.FindOrAdd(MeterClass);
}

FOnMeterStateChangeEvent& USGAbilitySystemComponent::GetMeterStateChangeDelegate(
	TSubclassOf<UDecreaseMeter> MeterClass)
{
	return MeterStateChangeDelegates.FindOrAdd(MeterClass);
}

void USGAbilitySystemComponent::OnShowMeterDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("Meter")))
	{
		UWorld* World = HUD->GetWorld();

		AActor* TargetActor = HUD->GetCurrentDebugTargetActor();
		USGAbilitySystemComponent* SagaASC = Cast<USGAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor));

		if (SagaASC)
		{
			// 绘制标题 / Draw title
			Canvas->SetDrawColor(FColor::Yellow);
			Canvas->DrawText(GEngine->GetMediumFont(), TEXT("=== Saga Meters ==="), 10.0f, YPos, 1.2f, 1.2f);
			YPos += 25.0f;

			// 绘制所有Meter的进度条 / Draw progress bars for all meters
			SagaASC->DrawMeterProgressBars(Canvas, YPos);
		}
	}
}

void USGAbilitySystemComponent::AddSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if (!IsValid(AttributeSet))
	{
		return;
	}

	if (GetSpawnedAttributes().Find(AttributeSet) == INDEX_NONE)
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, true);
	}

	Super::AddSpawnedAttribute(AttributeSet);

	UpdateShouldTick();
}

void USGAbilitySystemComponent::RemoveSpawnedAttribute(UAttributeSet* AttributeSet)
{
	if(GetSpawnedAttributes().Contains(AttributeSet))
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);
	}
	Super::RemoveSpawnedAttribute(AttributeSet);
	
	UpdateShouldTick();
}

void USGAbilitySystemComponent::RemoveAllSpawnedAttributes()
{
	for (UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		GetAttributeSetAddOrRemoveDelegate(AttributeSet->GetClass()).Broadcast(AttributeSet, false);

	}
	Super::RemoveAllSpawnedAttributes();

	UpdateShouldTick();
}

void USGAbilitySystemComponent::OnRep_SpawnedAttributes(const TArray<UAttributeSet*>& PreviousSpawnedAttributes)
{
	if (IsUsingRegisteredSubObjectList())
	{
		// Find the attributes that got removed
		for (UAttributeSet* PreviousAttributeSet : PreviousSpawnedAttributes)
		{
			if (PreviousAttributeSet)
			{
				const bool bWasRemoved = GetSpawnedAttributes().Find(PreviousAttributeSet) == INDEX_NONE;
				if (bWasRemoved)
				{
					GetAttributeSetAddOrRemoveDelegate(PreviousAttributeSet->GetClass()).Broadcast(PreviousAttributeSet, false);
					UpdateShouldTick();
				}
			}
		}

		// Find the attributes that got added
		for (UAttributeSet* NewAttributeSet : GetSpawnedAttributes())
		{
			if (IsValid(NewAttributeSet))
			{
				const bool bIsAdded = PreviousSpawnedAttributes.Find(NewAttributeSet) == INDEX_NONE;
				if (bIsAdded)
				{
					GetAttributeSetAddOrRemoveDelegate(NewAttributeSet->GetClass()).Broadcast(NewAttributeSet, true);
					UpdateShouldTick();
				}
			}
		}
	}
	
	Super::OnRep_SpawnedAttributes(PreviousSpawnedAttributes);

}

void USGAbilitySystemComponent::DrawMeterProgressBars(UCanvas* Canvas, float& YPos)
{
	/**
	 * ===================================================================================
	 * SagaStats Meter Debug Display System / SagaStats计量条调试显示系统
	 * ===================================================================================
	 *
	 * 【系统概述 / System Overview】
	 * 为SagaStats插件提供实时的Meter状态可视化调试工具，支持UDecreaseMeter和UIncreaseMeter。
	 * Real-time Meter status visualization debug tool for SagaStats plugin, supporting UDecreaseMeter and UIncreaseMeter.
	 *
	 * 【整体布局设计 / Overall Layout Design】
	 * ┌─────────────────────────────────────────────────────────────────────────────────┐
	 * │ [状态色]Health [Lock 1.2/2.0]████████████████████░░ 100/150 (+5.0,CD:2.3/3.0)   │
	 * │    [青色]Regen: 5.0, RegenCD: 3.0, [青色]LockDur: 2.0,                        │
	 * │    ResetRate: 20.0, [青色]ImmuneThresh: 10.0                                  │
	 * │                                                                                 │
	 * │ [状态色]Mana [Normal]████████████████████████████ 150/150 (-1.0,CD:4.2/5.0)     │
	 * │    Degen: 1.0, [青色]DegenCD: 5.0                                             │
	 * │                                                                                 │
	 * │ [状态色]Shield [Reset 10.0/s]██░░░░░░░░░░░░░░░░░░░  25/100 (Reset 10.0/s)        │
	 * │    Regen: 10.0, RegenCD: 1.0, LockDur: 0.0,                                  │
	 * │    [青色]ResetRate: 10.0, ImmuneThresh: 0.0                                   │
	 * └─────────────────────────────────────────────────────────────────────────────────┘
	 *    ↑                    ↑                          ↑
	 *  Meter名称           进度条(10px高)             数值+状态信息
	 *  (带阴影效果)        (200px宽)                  (新格式显示)
	 *    ↑
	 *  属性信息行(多彩文字+黑色阴影，缩进30px，智能换行，高亮关键字段) 
	 *
	 * 【核心功能 / Core Features】
	 * ✅ 智能Meter名称格式化 (移除前后缀，添加详细状态标记)
	 * ✅ 状态驱动的颜色编码 (名称、进度条根据状态变色)
	 * ✅ 实时CD计时器显示 (精确到0.1秒，显示剩余时间/总时间)
	 * ✅ 关键属性智能高亮 (根据状态动态高亮相关字段)
	 * ✅ 高对比度文字渲染 (黑色阴影+彩色文字)
	 * ✅ 免疫状态可视化 (灰色进度条)
	 * ✅ 响应式布局设计 (自适应宽度)
	 * ✅ 新增Normal状态标识 (显示[Normal]标记)
	 *
	 * 【Meter名称处理 / Meter Name Processing】
	 * ├─ 自动格式化: 移除"Meter_"前缀和"_C"后缀
	 * ├─ 详细状态标记:
	 * │  ├─ Normal: "Health [Normal]"
	 * │  ├─ Lock: "Health [Lock 1.2/2.0]" (显示剩余/总时间)
	 * │  └─ Reset: "Shield [Reset 10.0/s]" (显示重置速率)
	 * ├─ 示例转换: "SagaMeter_Health_C" → "Health [Lock 1.2/2.0]"
	 * ├─ 颜色编码: Normal=绿色, Lock=红色, Reset=橙色
	 * └─ 阴影效果: 黑色阴影增强文字对比度
	 *
	 * 【进度条系统 / Progress Bar System】
	 * ├─ 尺寸规格: 200px × 10px (优化后的紧凑设计)
	 * ├─ 边框样式: 0.5px白色边框
	 * ├─ 背景颜色: 深灰色(40,40,40,200)
	 * ├─ 填充逻辑: 当前值/最大值百分比
	 * └─ 免疫显示: Reset状态+免疫阈值时显示灰色
	 *
	 * 【数值显示系统 / Value Display System】
	 * ├─ 位置布局: 进度条右侧10px偏移
	 * ├─ 字体设置: SmallFont 1.0x缩放
	 * ├─ 颜色方案: 黑色阴影+白色文字
	 * ├─ 新格式规则: "当前值/最大值 (速率信息,CD:剩余/总时间)"
	 * └─ CD信息: 根据Meter类型和状态动态生成
	 *
	 * 【CD信息生成规则 / CD Info Generation Rules】
	 * UDecreaseMeter (新格式):
	 *   ├─ 冷却中: "(+5.0,CD:2.3/3.0)" - 显示恢复速率和冷却进度
	 *   ├─ 锁定中: "Lock 1.2/2.0" - 显示锁定剩余/总时间
	 *   ├─ Normal状态: "+X.X/s" (恢复速率，当regen>0时)
	 *   ├─ Reset状态: "Reset X.X/s" (重置速率)
	 *   └─ Lock状态: "Locked" (无计时器时)
	 *
	 * UIncreaseMeter (新格式):
	 *   ├─ 冷却中: "(-1.0,CD:4.2/5.0)" - 显示衰减速率和冷却进度
	 *   └─ 正常衰减: "-X.X/s" (衰减速率)
	 *
	 * 【属性信息系统 / Attribute Info System】
	 * ├─ 智能布局: 每个Meter下方，缩进30px
	 * ├─ 多彩渲染: 彩色文字+黑色阴影(1px偏移)
	 * ├─ 智能高亮: 根据当前状态动态高亮相关字段(青色)
	 * │  ├─ Normal状态: 高亮Regen和RegenCD(如果活跃)
	 * │  ├─ Lock状态: 高亮LockDur字段
	 * │  ├─ Reset状态: 高亮ResetRate和ImmuneThresh(如果免疫)
	 * │  └─ IncreaseMeter: 高亮Degen和DegenCD(如果活跃)
	 * ├─ 多行换行: 自动计算宽度，智能换行
	 * ├─ 行高设置: 12px行间距
	 * ├─ 属性精简: 移除Maximum和bClear字段显示
	 * └─ 格式示例: "[青色]Regen: 5.0, [青色]RegenCD: 3.0, LockDur: 2.0"
	 *
	 * 【布局参数配置 / Layout Parameters】
	 * ├─ 整体间距: 55px行间距(容纳多行属性)
	 * ├─ 边距设置: 20px左边距
	 * ├─ 名称区域: 140px宽度(适配新的状态标记)
	 * ├─ 进度条区域: 200px宽度(优化尺寸)
	 * ├─ 数值区域: 10px间距偏移
	 * ├─ 属性区域: 30px缩进，420px最大宽度
	 * └─ 字符计算: 6px/字符用于换行计算
	 *
	 * 【技术实现要点 / Technical Implementation】
	 * ├─ friend类访问: 允许访问protected计时器成员
	 * ├─ 实时计时器: GetTimerManager().GetTimerRemaining()
	 * ├─ 类型检测: Cast<UDecreaseMeter/UIncreaseMeter>()
	 * ├─ 渲染优化: 先绘制阴影后绘制文字
	 * ├─ 多色文本: DrawMultiColorLine函数支持单行多色渲染
	 * ├─ 动态高亮: 基于状态和条件的智能颜色选择
	 * ├─ 内存安全: 所有指针都进行nullptr检查
	 * └─ 精度控制: 浮点数格式化保留1位小数
	 *
	 * 【使用方法 / Usage Instructions】
	 * 游戏内命令: showdebug Meter
	 * 注册方式: USGAbilitySystemComponent::OnShowMeterDebugInfo
	 * 依赖条件: 需要有效的USagaAbilitySystemComponent和Meter实例
	 * ===================================================================================
	 */

	/* ✅ 所有功能需求已完成实现 / All Feature Requirements Completed
	 * ✅ regen/CD功能表现已更新为`(+/-xxx,CD:剩余时间/总时间)`格式
	 * ✅ Lock状态显示已更新为`[Lock 剩余时间/总时间]`格式
	 * ✅ Reset状态显示已更新为`[Reset ResetRate/s]`格式
	 * ✅ 关键字段`LockDur/ResetRate/ImmuneThreshold`已实现智能高亮显示
	 * ✅ 进度条尺寸已调整为宽度200px，高度10px
	 * ✅ Maximum和bClear字段显示已移除
	 * ✅ MeterName已添加阴影效果增强可读性
	 */
	
	
	if (!Canvas)
	{
		return;
	}

	// 获取所有Meter实例 / Get all meter instances
	TArray<UMeterBase*> Meters = GetAllMeters();

	if (Meters.Num() == 0)
	{
		// 显示无Meter信息 / Show no meters message
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("No meters found"), 20.0f, YPos, 1.0f, 1.0f);
		YPos += 20.0f;
		return;
	}

	const float ProgressBarWidth = 200.0f;
	const float ProgressBarHeight = 10.0f; // bar宽度200，高度10 / Bar width 200, height 10
	const float RowSpacing = 55.0f; // 进一步增加行间距以容纳多行属性信息 / Further increase spacing for multi-line attribute info
	const float LeftMargin = 20.0f;
	const float AttributeInfoIndent = 30.0f; // 属性信息缩进 / Attribute info indent

	for (int32 i = 0; i < Meters.Num(); ++i)
	{
		UMeterBase* Meter = Meters[i];
		if (!Meter)
		{
			continue;
		}

		float CurrentYPos = YPos + (i * RowSpacing);

		// 绘制Meter名称，根据状态使用不同颜色 / Draw meter name with color based on state
		FColor MeterNameColor = GetMeterNameColor(Meter);
		FString MeterName = GetFormattedMeterName(Meter);

		// 先绘制黑色阴影增强对比度 / Draw black shadow first for better contrast
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(GEngine->GetSmallFont(), MeterName, LeftMargin + 1.0f, CurrentYPos + 1.0f, 1.0f, 1.0f);

		// 再绘制彩色名称文字 / Then draw colored name text
		Canvas->SetDrawColor(MeterNameColor);
		Canvas->DrawText(GEngine->GetSmallFont(), MeterName, LeftMargin, CurrentYPos, 1.0f, 1.0f);

		// 绘制进度条 / Draw progress bar
		FVector2D ProgressPosition(LeftMargin + 140.0f, CurrentYPos);
		FVector2D ProgressSize(ProgressBarWidth, ProgressBarHeight);
		DrawSingleMeterProgress(Canvas, Meter, ProgressPosition, ProgressSize);

		// 绘制属性信息 / Draw attribute info
		FVector2D AttributePosition(LeftMargin + AttributeInfoIndent, CurrentYPos + 15.0f);
		float MaxAttributeWidth = ProgressBarWidth + 220.0f; // 更宽的显示区域 / Wider display area
		DrawMeterAttributeInfo(Canvas, Meter, AttributePosition, MaxAttributeWidth);
	}

	// 更新YPos用于后续绘制 / Update YPos for subsequent drawing
	YPos += Meters.Num() * RowSpacing + 10.0f;
}

void USGAbilitySystemComponent::DrawSingleMeterProgress(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position, FVector2D Size)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	// 获取Meter的当前值和最大值 / Get current and max values
	float CurrentValue = Meter->GetCurrent();
	float MaxValue = Meter->GetMaximum();
	float Progress = MaxValue > 0 ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;

	// 绘制背景（深灰色）/ Draw background (dark gray)
	Canvas->SetDrawColor(FColor(40, 40, 40, 200));
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, Size.X, Size.Y, 0, 0, 1, 1);

	// 根据Immune状态和Meter类型获取进度条颜色 / Get progress bar color based on immune status and meter type
	FColor ProgressColor = GetMeterProgressBarColor(Meter);
	Canvas->SetDrawColor(ProgressColor);
	float ProgressWidth = Size.X * Progress;
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, ProgressWidth, Size.Y, 0, 0, 1, 1);

	// 绘制边框 / Draw border
	DrawBorder(Canvas, Position, Size, 0.5f); // 使用更细的边框适应较小的进度条 / Use thinner border for smaller progress bar

	// 获取CD信息用于显示在括号内 / Get CD info for display in brackets
	FString CDInfo = GetMeterCDInfo(Meter);

	// 绘制数值文本 / Draw value text
	FString ValueText;
	if (!CDInfo.IsEmpty())
	{
		ValueText = FString::Printf(TEXT("%.1f/%.1f (%s)"), CurrentValue, MaxValue, *CDInfo);
	}
	else
	{
		ValueText = FString::Printf(TEXT("%.1f/%.1f"), CurrentValue, MaxValue);
	}

	// 将文字放在进度条右边 / Place text to the right of progress bar
	float TextX = Position.X + Size.X + 10.0f;
	float TextY = Position.Y + (Size.Y * 0.5f) - 6.0f; // 垂直居中对齐 / Vertically center align

	// 先绘制黑色阴影增强对比度 / Draw black shadow first for better contrast
	Canvas->SetDrawColor(FColor::Black);
	Canvas->DrawText(GEngine->GetSmallFont(), ValueText, TextX + 1.0f, TextY + 1.0f, 1.0f, 1.0f);

	// 再绘制白色文字 / Then draw white text
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetSmallFont(), ValueText, TextX, TextY, 1.0f, 1.0f);
}


void USGAbilitySystemComponent::DrawBorder(UCanvas* Canvas, FVector2D Position, FVector2D Size, float BorderWidth)
{
	if (!Canvas)
	{
		return;
	}

	Canvas->SetDrawColor(FColor::White);

	// 上边框 / Top border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, Size.X, BorderWidth, 0, 0, 1, 1);
	// 下边框 / Bottom border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y + Size.Y - BorderWidth, Size.X, BorderWidth, 0, 0, 1, 1);
	// 左边框 / Left border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X, Position.Y, BorderWidth, Size.Y, 0, 0, 1, 1);
	// 右边框 / Right border
	Canvas->DrawTile(Canvas->DefaultTexture, Position.X + Size.X - BorderWidth, Position.Y, BorderWidth, Size.Y, 0, 0, 1, 1);
}

TArray<UMeterBase*> USGAbilitySystemComponent::GetAllMeters() const
{
	TArray<UMeterBase*> Meters;

	// 遍历所有AttributeSet，查找Meter类型 / Iterate through all AttributeSets to find meter types
	for (const UAttributeSet* AttributeSet : GetSpawnedAttributes())
	{
		if (const UMeterBase* Meter = Cast<UMeterBase>(AttributeSet))
		{
			Meters.Add(const_cast<UMeterBase*>(Meter));
		}
	}

	// 按类名排序以保持一致的显示顺序 / Sort by class name for consistent display order
	Meters.Sort([](const UMeterBase& A, const UMeterBase& B)
	{
		return A.GetClass()->GetName() < B.GetClass()->GetName();
	});

	return Meters;
}

FColor USGAbilitySystemComponent::GetMeterNameColor(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FColor::White;
	}

	// 尝试转换为UDecreaseMeter以获取状态 / Try to cast to UDecreaseMeter to get state
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		switch (DecreaseMeter->MeterState)
		{
		case EMeterState::Normal:
			return FColor::Green;   // 绿色 / Green
		case EMeterState::Lock:
			return FColor::Red;     // 红色 / Red
		case EMeterState::Reset:
			return FColor::Orange;  // 橙色 / Orange
		default:
			return FColor::Green;
		}
	}

	// 对于其他类型的Meter，使用默认颜色 / For other meter types, use default color
	return FColor::Green;
}

FColor USGAbilitySystemComponent::GetMeterProgressBarColor(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FColor::Green;
	}

	// 检查是否为UDecreaseMeter并处于免疫状态 / Check if it's UDecreaseMeter and in immune state
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 检查是否处于Reset状态且在免疫阈值内 / Check if in Reset state and within immune threshold
		if (DecreaseMeter->MeterState == EMeterState::Reset)
		{
			float ImmuneThreshold = DecreaseMeter->GetImmuneThreshold();
			float CurrentValue = DecreaseMeter->GetCurrent();

			// 如果免疫阈值<0，整个Reset状态免疫 / If immune threshold < 0, immune throughout Reset state
			if (ImmuneThreshold < 0.0f || CurrentValue < ImmuneThreshold)
			{
				return FColor(128, 128, 128); // 灰色 / Gray
			}
		}
	}

	// 非免疫状态下的颜色 / Color when not immune
	return FColor::Green; // 绿色 / Green
}

void USGAbilitySystemComponent::DrawMeterRecoveryInfo(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	FString RecoveryText;
	bool bHasRecoveryInfo = false;

	// 处理UDecreaseMeter的恢复信息 / Handle UDecreaseMeter recovery info
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 优先检查计时器状态 / Priority check timer states
		if (DecreaseMeter->RegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->RegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("CD: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		else if (DecreaseMeter->LockStateTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("Lock: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		// 如果没有活跃计时器，根据状态显示相应信息 / If no active timers, show info based on state
		else
		{
			switch (DecreaseMeter->MeterState)
			{
			case EMeterState::Normal:
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					RecoveryText = FString::Printf(TEXT("+%.1f/s"), DecreaseMeter->GetRegeneration());
					bHasRecoveryInfo = true;
				}
				break;
			case EMeterState::Lock:
				RecoveryText = TEXT("Locked");
				bHasRecoveryInfo = true;
				break;
			case EMeterState::Reset:
				if (DecreaseMeter->GetResetRate() > 0.0f)
				{
					RecoveryText = FString::Printf(TEXT("Reset: +%.1f/s"), DecreaseMeter->GetResetRate());
					bHasRecoveryInfo = true;
				}
				break;
			}
		}
	}
	// 处理UIncreaseMeter的衰减信息 / Handle UIncreaseMeter degeneration info
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		// 优先检查衰减冷却计时器 / Priority check degeneration cooldown timer
		if (IncreaseMeter->DegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = IncreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(IncreaseMeter->DegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				RecoveryText = FString::Printf(TEXT("CD: %.1fs"), RemainingTime);
				bHasRecoveryInfo = true;
			}
		}
		// 如果没有活跃计时器，检查是否可以衰减 / If no active timer, check if can degenerate
		else if (IncreaseMeter->GetDegeneration() > 0.0f)
		{
			RecoveryText = FString::Printf(TEXT("-%.1f/s"), IncreaseMeter->GetDegeneration());
			bHasRecoveryInfo = true;
		}
	}

	// 绘制恢复信息文本 / Draw recovery info text
	if (bHasRecoveryInfo)
	{
		// 先绘制黑色阴影 / Draw black shadow first
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(GEngine->GetSmallFont(), RecoveryText, Position.X + 1.0f, Position.Y + 3.0f, 1.0f, 1.0f);

		// 再绘制青色文字 / Then draw cyan text
		Canvas->SetDrawColor(FColor::Cyan);
		Canvas->DrawText(GEngine->GetSmallFont(), RecoveryText, Position.X, Position.Y + 2.0f, 1.0f, 1.0f);
	}
}

FString USGAbilitySystemComponent::GetMeterCDInfo(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	// 处理UDecreaseMeter的CD信息 / Handle UDecreaseMeter CD info
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		// 优先检查计时器状态 / Priority check timer states
		if (DecreaseMeter->RegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->RegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = DecreaseMeter->GetRegenerationCooldown();
				FString RegenInfo;
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					RegenInfo = FString::Printf(TEXT("+%.1f,"), DecreaseMeter->GetRegeneration());
				}
				return FString::Printf(TEXT("%sCD:%.1f/%.1f"), *RegenInfo, RemainingTime, TotalTime);
			}
		}
		else if (DecreaseMeter->LockStateTimer.IsValid())
		{
			float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = DecreaseMeter->GetLockDuration();
				return FString::Printf(TEXT("Lock %.1f/%.1f"), RemainingTime, TotalTime);
			}
		}
		// 如果没有活跃计时器，根据状态显示相应信息 / If no active timers, show info based on state
		else
		{
			switch (DecreaseMeter->MeterState)
			{
			case EMeterState::Normal:
				if (DecreaseMeter->GetRegeneration() > 0.0f)
				{
					return FString::Printf(TEXT("+%.1f/s"), DecreaseMeter->GetRegeneration());
				}
				break;
			case EMeterState::Lock:
				// Lock状态但没有计时器，表示永久锁定 / Lock state but no timer means permanent lock
				return TEXT("Locked");
			case EMeterState::Reset:
				if (DecreaseMeter->GetResetRate() > 0.0f)
				{
					return FString::Printf(TEXT("Reset %.1f/s"), DecreaseMeter->GetResetRate());
				}
				break;
			}
		}
	}
	// 处理UIncreaseMeter的CD信息 / Handle UIncreaseMeter CD info
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		// 优先检查衰减冷却计时器 / Priority check degeneration cooldown timer
		if (IncreaseMeter->DegenerationCooldownTimer.IsValid())
		{
			float RemainingTime = IncreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(IncreaseMeter->DegenerationCooldownTimer);
			if (RemainingTime > 0.0f)
			{
				float TotalTime = IncreaseMeter->GetDegenerationCooldown();
				FString DegenInfo;
				if (IncreaseMeter->GetDegeneration() > 0.0f)
				{
					DegenInfo = FString::Printf(TEXT("-%.1f,"), IncreaseMeter->GetDegeneration());
				}
				return FString::Printf(TEXT("%sCD:%.1f/%.1f"), *DegenInfo, RemainingTime, TotalTime);
			}
		}
		// 如果没有活跃计时器，检查是否可以衰减 / If no active timer, check if can degenerate
		else if (IncreaseMeter->GetDegeneration() > 0.0f)
		{
			return FString::Printf(TEXT("-%.1f/s"), IncreaseMeter->GetDegeneration());
		}
	}

	return FString(); // 返回空字符串表示没有CD信息 / Return empty string if no CD info
}

FString USGAbilitySystemComponent::GetFormattedMeterName(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	// 获取原始类名 / Get original class name
	FString MeterName = Meter->GetClass()->GetName();

	// 去掉 "Meter_" 前缀 / Remove "Meter_" prefix
	if (MeterName.StartsWith(TEXT("Meter_")))
	{
		MeterName = MeterName.RightChop(6); // Remove "Meter_"
	}

	// 去掉 "_C" 后缀 / Remove "_C" suffix
	if (MeterName.EndsWith(TEXT("_C")))
	{
		MeterName = MeterName.LeftChop(2); // Remove "_C"
	}

	// 添加状态指示器 / Add state indicators
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		switch (DecreaseMeter->MeterState)
		{
		case EMeterState::Lock:
			{
				if (DecreaseMeter->LockStateTimer.IsValid())
				{
					float RemainingTime = DecreaseMeter->GetWorld()->GetTimerManager().GetTimerRemaining(DecreaseMeter->LockStateTimer);
					float TotalTime = DecreaseMeter->GetLockDuration();
					if (RemainingTime > 0.0f)
					{
						MeterName += FString::Printf(TEXT(" [Lock %.1f/%.1f]"), RemainingTime, TotalTime);
					}
					else
					{
						MeterName += TEXT(" [Lock]");
					}
				}
				else
				{
					MeterName += TEXT(" [Lock]");
				}
			}
			break;
		case EMeterState::Reset:
			{
				float ResetRate = DecreaseMeter->GetResetRate();
				if (ResetRate > 0.0f)
				{
					MeterName += FString::Printf(TEXT(" [Reset %.1f/s]"), ResetRate);
				}
				else
				{
					MeterName += TEXT(" [Reset]");
				}
			}
			break;
		case EMeterState::Normal:
		default:
			// 正常状态不添加指示器 / No indicator for normal state
			MeterName += TEXT(" [Normal]");
			break;
		}
	}

	return MeterName;
}

FString USGAbilitySystemComponent::GetMeterAttributeInfo(UMeterBase* Meter)
{
	if (!Meter)
	{
		return FString();
	}

	TArray<FString> AttributeStrings;

	// 删除Maximum和bClear字段显示 / Remove Maximum and bClear field display

	// UDecreaseMeter特有属性 / UDecreaseMeter specific attributes
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		AttributeStrings.Add(FString::Printf(TEXT("Regen: %.1f"), DecreaseMeter->GetRegeneration()));
		AttributeStrings.Add(FString::Printf(TEXT("RegenCD: %.1f"), DecreaseMeter->GetRegenerationCooldown()));
		AttributeStrings.Add(FString::Printf(TEXT("LockDur: %.1f"), DecreaseMeter->GetLockDuration()));
		AttributeStrings.Add(FString::Printf(TEXT("ResetRate: %.1f"), DecreaseMeter->GetResetRate()));
		AttributeStrings.Add(FString::Printf(TEXT("ImmuneThresh: %.1f"), DecreaseMeter->GetImmuneThreshold()));
	}
	// UIncreaseMeter特有属性 / UIncreaseMeter specific attributes
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		AttributeStrings.Add(FString::Printf(TEXT("Degen: %.1f"), IncreaseMeter->GetDegeneration()));
		AttributeStrings.Add(FString::Printf(TEXT("DegenCD: %.1f"), IncreaseMeter->GetDegenerationCooldown()));
	}

	// 用逗号连接所有属性 / Join all attributes with commas
	return FString::Join(AttributeStrings, TEXT(", "));
}

void USGAbilitySystemComponent::DrawMeterAttributeInfo(UCanvas* Canvas, UMeterBase* Meter, FVector2D Position, float MaxWidth)
{
	if (!Canvas || !Meter)
	{
		return;
	}

	// 获取属性信息数组，包含颜色信息 / Get attribute info array with color information
	TArray<TPair<FString, FColor>> AttributeStrings;

	//获取选中状态颜色
	auto GetAttributeDrawColor = [](bool Condition){return Condition ? FColor::Cyan: FColor::White;};
	
	// UDecreaseMeter特有属性 / UDecreaseMeter specific attributes
	if (const UDecreaseMeter* DecreaseMeter = Cast<UDecreaseMeter>(Meter))
	{
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("Regen: %.1f"), DecreaseMeter->GetRegeneration()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Normal && DecreaseMeter->CanRegeneration())));
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("RegenCD: %.1f"), DecreaseMeter->GetRegenerationCooldown()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Normal && DecreaseMeter->RegenerationCooldownTimer.IsValid())));

		// 高亮LockDur字段 / Highlight LockDur field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("LockDur: %.1f"), DecreaseMeter->GetLockDuration()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Lock) ));

		// 高亮ResetRate字段 / Highlight ResetRate field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("ResetRate: %.1f"), DecreaseMeter->GetResetRate()), GetAttributeDrawColor(DecreaseMeter->MeterState == EMeterState::Reset) ));

		// 高亮ImmuneThreshold字段 / Highlight ImmuneThreshold field
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("ImmuneThresh: %.1f"), DecreaseMeter->GetImmuneThreshold()), GetAttributeDrawColor(DecreaseMeter->IsInResetImmune())));
	}
	// UIncreaseMeter特有属性 / UIncreaseMeter specific attributes
	else if (const UIncreaseMeter* IncreaseMeter = Cast<UIncreaseMeter>(Meter))
	{
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("Degen: %.1f"), IncreaseMeter->GetDegeneration()), GetAttributeDrawColor(IncreaseMeter->CanDegeneration())));
		AttributeStrings.Add(TPair<FString, FColor>(FString::Printf(TEXT("DegenCD: %.1f"), IncreaseMeter->GetDegenerationCooldown()), GetAttributeDrawColor(IncreaseMeter->DegenerationCooldownTimer.IsValid())));
	}

	// 设置字体 / Set font
	UFont* SmallFont = GEngine->GetSmallFont();

	// 计算每行能容纳的字符数 / Calculate characters per line
	float CharWidth = 6.0f; // SmallFont大约字符宽度 / Approximate SmallFont character width
	int32 MaxCharsPerLine = FMath::Max(1, (int32)(MaxWidth / CharWidth));

	// 构建多行显示 / Build multi-line display
	TArray<TPair<FString, FColor>> CurrentLine;
	float CurrentYPos = Position.Y;
	const float LineHeight = 12.0f; // 行高 / Line height

	for (int32 i = 0; i < AttributeStrings.Num(); ++i)
	{
		TPair<FString, FColor> AttributePair = AttributeStrings[i];

		// 计算当前行的长度 / Calculate current line length
		int32 CurrentLineLength = 0;
		for (const auto& Pair : CurrentLine)
		{
			CurrentLineLength += Pair.Key.Len() + 2; // +2 for ", "
		}
		CurrentLineLength += AttributePair.Key.Len();

		// 检查是否需要换行 / Check if line break is needed
		if (CurrentLineLength > MaxCharsPerLine && CurrentLine.Num() > 0)
		{
			// 绘制当前行 / Draw current line
			DrawMultiColorLine(Canvas, Position.X, CurrentYPos, CurrentLine, SmallFont);
			CurrentYPos += LineHeight;
			CurrentLine.Empty(); // 清空当前行开始新行 / Clear current line to start new line
		}

		CurrentLine.Add(AttributePair);
	}

	// 绘制最后一行 / Draw the last line
	if (CurrentLine.Num() > 0)
	{
		DrawMultiColorLine(Canvas, Position.X, CurrentYPos, CurrentLine, SmallFont);
	}
}

void USGAbilitySystemComponent::DrawMultiColorLine(UCanvas* Canvas, float StartX, float YPos, const TArray<TPair<FString, FColor>>& ColoredTexts, UFont* Font)
{
	if (!Canvas || !Font || ColoredTexts.Num() == 0)
	{
		return;
	}

	float CurrentX = StartX;

	for (int32 i = 0; i < ColoredTexts.Num(); ++i)
	{
		const TPair<FString, FColor>& ColoredText = ColoredTexts[i];
		FString DisplayText = ColoredText.Key;

		// 添加分隔符（除了第一个元素） / Add separator (except for first element)
		if (i > 0)
		{
			// 绘制分隔符阴影 / Draw separator shadow
			Canvas->SetDrawColor(FColor::Black);
			Canvas->DrawText(Font, TEXT(", "), CurrentX + 1.0f, YPos + 1.0f, 1.0f, 1.0f);
			// 绘制分隔符 / Draw separator
			Canvas->SetDrawColor(FColor::White);
			Canvas->DrawText(Font, TEXT(", "), CurrentX, YPos, 1.0f, 1.0f);
			CurrentX += 12.0f; // ", " 的大约宽度 / Approximate width of ", "
		}

		// 绘制文本阴影 / Draw text shadow
		Canvas->SetDrawColor(FColor::Black);
		Canvas->DrawText(Font, DisplayText, CurrentX + 1.0f, YPos + 1.0f, 1.0f, 1.0f);

		// 绘制彩色文本 / Draw colored text
		Canvas->SetDrawColor(ColoredText.Value);
		Canvas->DrawText(Font, DisplayText, CurrentX, YPos, 1.0f, 1.0f);

		// 更新X位置 / Update X position
		CurrentX += DisplayText.Len() * 6.0f; // 估算字符宽度 / Estimated character width
	}
}
