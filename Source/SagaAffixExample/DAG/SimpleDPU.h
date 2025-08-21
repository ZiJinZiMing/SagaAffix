// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleDPU.generated.h"

/**
 * 依赖顺序映射项 - Dependency Order Mapping Item
 * 用于指定当前DPU对特定依赖项的处理顺序偏好
 */
USTRUCT(BlueprintType)
struct SAGAAFFIXEXAMPLE_API FDependencyOrderMapping
{
    GENERATED_BODY()

    // 依赖DPU的ID - ID of the dependency DPU
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DependencyDPUId;

    // 依赖顺序值(数值小优先处理) - Dependency order value (smaller value processes first)  
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DependencyOrder = 0;

    FDependencyOrderMapping()
    {
        DependencyOrder = 0;
    }

    FDependencyOrderMapping(const FString& InDPUId, int32 InOrder)
        : DependencyDPUId(InDPUId), DependencyOrder(InOrder)
    {
    }
};

/**
 * 简化的DPU基类 - Simplified DPU Base Class
 * 专注于依赖关系和DAG构建，处理逻辑仅用日志表示
 */
UCLASS(BlueprintType)
class SAGAAFFIXEXAMPLE_API USimpleDPU : public UObject
{
    GENERATED_BODY()

public:
    // DPU唯一标识符 - Unique ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DPUId;

    // DPU显示名称 - Display Name  
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    // 执行前必须获得的控制令牌 - Required Control Tokens (执行权限依赖)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredTokens;

    // 执行完成后释放的控制令牌 - Produced Control Tokens (执行权限产出)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ProducedTokens;

public:
    USimpleDPU();

    /**
     * 执行处理逻辑 - Execute Processing Logic
     * 仅用UE_LOG打印，表示处理过程
     */
    UFUNCTION(BlueprintCallable, Category = "DPU")
    virtual void Execute();

    /**
     * 获取调试信息 - Get Debug Information
     */
    UFUNCTION(BlueprintCallable, Category = "DPU")
    FString GetDebugInfo() const;

    /**
     * 静态工厂方法：创建基础伤害DPU - Create Apply Damage DPU
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateApplyDamageDPU();

    /**
     * 静态工厂方法：创建伤害计算DPU - Create Calc Damage DPU
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateCalcDamageDPU();

    /**
     * 静态工厂方法：创建护盾计算DPU - Create Calc Shield DPU
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateCalcShieldDPU();

    /**
     * 静态工厂方法：创建冲突DPU用于测试环路 - Create Conflict DPU for Testing Cycles
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateConflictDPU();

    /**
     * 静态工厂方法：创建健康值应用DPU - Create Apply Health DPU
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateApplyHealthDPU();

    /**
     * 静态工厂方法：创建护盾应用DPU - Create Apply Shield DPU
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Factory")
    static USimpleDPU* CreateApplyShieldDPU();
};
