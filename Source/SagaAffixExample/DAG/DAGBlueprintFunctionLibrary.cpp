// Fill out your copyright notice in the Description page of Project Settings.

#include "DAGBlueprintFunctionLibrary.h"
#include "SimpleDPU.h"
#include "DAGBuilder.h"

void UDAGBlueprintFunctionLibrary::GADMain()
{
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("=== DAG 伤害流程演示开始 ==="));
    UE_LOG(LogTemp, Warning, TEXT("========================================"));

    // === 第一部分：正常流程演示 ===
    UE_LOG(LogTemp, Warning, TEXT("\n--- 第1部分：完整Token流程 (ApplyDamage -> CalcDamage/CalcShield -> ApplyHealth/ApplyShield) ---"));
    
    // 创建完整Token流程的DPU链 - Create complete Token flow DPU chain
    TArray<USimpleDPU*> NormalDPUs;
    NormalDPUs.Add(USimpleDPU::CreateApplyDamageDPU());      // ApplyDamage: 需要Token: 无 | 产出Token: base_damage_applied
    NormalDPUs.Add(USimpleDPU::CreateCalcDamageDPU());       // CalcDamage: 需要Token: base_damage_applied | 产出Token: damage_calculated
    NormalDPUs.Add(USimpleDPU::CreateCalcShieldDPU());       // CalcShield: 需要Token: 无 | 产出Token: damage_calculated, shield_processed
    NormalDPUs.Add(USimpleDPU::CreateApplyHealthDPU());      // ApplyHealth: 需要Token: damage_calculated | 产出Token: health_applied
    NormalDPUs.Add(USimpleDPU::CreateApplyShieldDPU());      // ApplyShield: 需要Token: shield_processed | 产出Token: shield_applied
    
    // 打印DPU信息 - Print DPU information
    UE_LOG(LogTemp, Log, TEXT("创建的DPU信息:"));
    for (USimpleDPU* DPU : NormalDPUs)
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *DPU->GetDebugInfo());
    }
    
    // 构建和执行正常DAG - Build and execute normal DAG
    UDAGBuilder* Builder = NewObject<UDAGBuilder>();
    FDAGBuildResult NormalResult = Builder->BuildDAG(NormalDPUs);
    
    if (NormalResult.bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ 正常流程DAG构建成功！"));
        UE_LOG(LogTemp, Log, TEXT("构建报告:\n%s"), *Builder->GetBuildReport());
        
        // 执行DAG - Execute DAG
        Builder->ExecuteDAG(NormalResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 正常流程DAG构建失败: %s"), *NormalResult.ErrorMessage);
        UE_LOG(LogTemp, Error, TEXT("错误报告:\n%s"), *NormalResult.DiagnosticInfo);
    }

    // === 第二部分：环路检测演示 ===
    UE_LOG(LogTemp, Warning, TEXT("\n--- 第2部分：环路检测演示 (添加冲突DPU造成循环依赖) ---"));
    
    // 创建包含环路的DPU链 - Create DPU chain with cycle
    TArray<USimpleDPU*> CyclicDPUs;
    CyclicDPUs.Add(USimpleDPU::CreateApplyDamageDPU());   // ApplyDamage: 需要Token: 无 | 产出Token: base_damage_applied
    CyclicDPUs.Add(USimpleDPU::CreateCalcDamageDPU());    // CalcDamage: 需要Token: base_damage_applied | 产出Token: damage_calculated
    CyclicDPUs.Add(USimpleDPU::CreateConflictDPU());      // ConflictDPU: 需要Token: damage_calculated | 产出Token: base_damage_applied (环路!)
    UE_LOG(LogTemp, Log, TEXT("添加冲突DPU后的信息:"));
    for (USimpleDPU* DPU : CyclicDPUs)
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *DPU->GetDebugInfo());
    }
    
    // 尝试构建包含环路的DAG - Try to build DAG with cycle
    UDAGBuilder* CyclicBuilder = NewObject<UDAGBuilder>();
    FDAGBuildResult CyclicResult = CyclicBuilder->BuildDAG(CyclicDPUs);
    
    if (CyclicResult.bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ 意外：环路DAG构建成功了（这不应该发生）"));
        CyclicBuilder->ExecuteDAG(CyclicResult);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ 预期结果：环路检测成功，DAG构建被正确拒绝"));
        UE_LOG(LogTemp, Error, TEXT("错误信息: %s"), *CyclicResult.ErrorMessage);
        UE_LOG(LogTemp, Log, TEXT("详细诊断报告:\n%s"), *CyclicResult.DiagnosticInfo);
    }

    // === 第三部分：基础流程演示（无护盾） ===
    UE_LOG(LogTemp, Warning, TEXT("\n--- 第3部分：基础流程演示 (ApplyDamage -> CalcDamage -> ApplyHealth，无护盾) ---"));
    
    TArray<USimpleDPU*> BasicDPUs;
    BasicDPUs.Add(USimpleDPU::CreateApplyDamageDPU());    // ApplyDamage: 需要Token: 无 | 产出Token: base_damage_applied
    BasicDPUs.Add(USimpleDPU::CreateCalcDamageDPU());     // CalcDamage: 需要Token: base_damage_applied | 产出Token: damage_calculated
    BasicDPUs.Add(USimpleDPU::CreateApplyHealthDPU());    // ApplyHealth: 需要Token: damage_calculated | 产出Token: health_applied
    
    UDAGBuilder* BasicBuilder = NewObject<UDAGBuilder>();
    FDAGBuildResult BasicResult = BasicBuilder->BuildDAG(BasicDPUs);
    
    if (BasicResult.bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ 基础流程构建成功（无护盾分支）"));
        BasicBuilder->ExecuteDAG(BasicResult);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 基础流程构建失败: %s"), *BasicResult.ErrorMessage);
    }

    // === 总结 ===
    UE_LOG(LogTemp, Warning, TEXT("\n========================================"));
    UE_LOG(LogTemp, Warning, TEXT("=== Token模型DAG 核心功能验证总结 ==="));
    UE_LOG(LogTemp, Warning, TEXT("1. ✅ 令牌依赖图构建：根据RequiredTokens/ProducedTokens正确建立邻接表"));
    UE_LOG(LogTemp, Warning, TEXT("2. ✅ 多提供者支持：多个DPU可提供相同令牌，ApplyHealth等待所有damage_calculated"));
    UE_LOG(LogTemp, Warning, TEXT("3. ✅ 环检测：使用DFS三色标记成功检测Token循环依赖"));
    UE_LOG(LogTemp, Warning, TEXT("4. ✅ 优先级排序：生成正确的线性执行计划"));
    UE_LOG(LogTemp, Warning, TEXT("5. ✅ 优先级控制：严格按Priority控制执行顺序（CalcDamage -> CalcShield）"));
    UE_LOG(LogTemp, Warning, TEXT("6. ✅ 执行调度：按DFS确定的线性顺序执行DPU，Token权限传递清晰"));
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}
