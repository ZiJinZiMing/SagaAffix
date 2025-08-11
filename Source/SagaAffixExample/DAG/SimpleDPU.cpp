// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleDPU.h"

USimpleDPU::USimpleDPU()
{
    DPUId = TEXT("");
    DisplayName = TEXT("Simple DPU");
    Priority = 0;
}

void USimpleDPU::Execute()
{
    UE_LOG(LogTemp, Warning, TEXT("[DPU Execution] %s (%s) - Processing completed"), *DisplayName, *DPUId);
}

FString USimpleDPU::GetDebugInfo() const
{
    FString DebugInfo = FString::Printf(TEXT("DPU[%s]: %s (Priority: %d)"), *DPUId, *DisplayName, Priority);
    
    if (RequiredTokens.Num() > 0)
    {
        DebugInfo += TEXT("\n  Required Tokens: ");
        for (const FString& Token : RequiredTokens)
        {
            DebugInfo += FString::Printf(TEXT("%s "), *Token);
        }
    }
    
    if (ProducedTokens.Num() > 0)
    {
        DebugInfo += TEXT("\n  Produced Tokens: ");
        for (const FString& Token : ProducedTokens)
        {
            DebugInfo += FString::Printf(TEXT("%s "), *Token);
        }
    }
    
    return DebugInfo;
}

USimpleDPU* USimpleDPU::CreateApplyDamageDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("ApplyDamage");
    DPU->DisplayName = TEXT("基础伤害应用 Apply Damage");
    DPU->Priority = 0;
    DPU->RequiredTokens.Empty();
    DPU->ProducedTokens.Add(TEXT("base_damage_applied"));
    return DPU;
}

USimpleDPU* USimpleDPU::CreateCalcDamageDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("CalcDamage");
    DPU->DisplayName = TEXT("伤害计算 Calc Damage");
    DPU->Priority = 0;
    DPU->RequiredTokens.Add(TEXT("base_damage_applied"));
    DPU->ProducedTokens.Add(TEXT("damage_calculated"));
    return DPU;
}

USimpleDPU* USimpleDPU::CreateCalcShieldDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("CalcShield");
    DPU->DisplayName = TEXT("护盾计算 Calc Shield");
    DPU->Priority = 100;
    DPU->RequiredTokens.Empty();
    DPU->ProducedTokens.Add(TEXT("damage_calculated"));
    DPU->ProducedTokens.Add(TEXT("shield_processed"));
    return DPU;
}

USimpleDPU* USimpleDPU::CreateConflictDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("ConflictDPU");
    DPU->DisplayName = TEXT("冲突测试DPU Conflict Test DPU");
    DPU->Priority = 10;
    
    // 创建环路：需要伤害计算完成，产出基础伤害令牌（形成循环依赖）
    // Create cycle: requires damage calculated, produces base damage token (circular dependency)
    DPU->RequiredTokens.Add(TEXT("damage_calculated"));
    DPU->ProducedTokens.Add(TEXT("base_damage_applied"));
    
    return DPU;
}

USimpleDPU* USimpleDPU::CreateApplyHealthDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("ApplyHealth");
    DPU->DisplayName = TEXT("健康值应用 Apply Health");
    DPU->Priority = 0;
    
    // 健康值应用器：等待所有伤害计算完成，产出健康应用令牌
    // Health applicator: waits for all damage calculations, produces health applied token
    DPU->RequiredTokens.Add(TEXT("damage_calculated"));
    DPU->ProducedTokens.Add(TEXT("health_applied"));
    
    return DPU;
}

USimpleDPU* USimpleDPU::CreateApplyShieldDPU()
{
    USimpleDPU* DPU = NewObject<USimpleDPU>();
    DPU->DPUId = TEXT("ApplyShield");
    DPU->DisplayName = TEXT("护盾应用 Apply Shield");
    DPU->Priority = 0;
    
    // 护盾应用器：等待护盾处理完成，产出护盾应用令牌
    // Shield applicator: waits for shield processing, produces shield applied token
    DPU->RequiredTokens.Add(TEXT("shield_processed"));
    DPU->ProducedTokens.Add(TEXT("shield_applied"));
    
    return DPU;
}
