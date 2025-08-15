// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleDPU.generated.h"

/**
 * 依赖优先级映射项 - Dependency Priority Mapping Item
 * 用于指定当前DPU对特定依赖项的优先级偏好
 */
USTRUCT(BlueprintType)
struct SAGAAFFIXEXAMPLE_API FDependencyPriorityMapping
{
    GENERATED_BODY()

    // 依赖DPU的ID - ID of the dependency DPU
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DependencyDPUId;

    // 优先级值(数值小优先执行) - Priority value (smaller value executes first)  
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;

    FDependencyPriorityMapping()
    {
        Priority = 0;
    }

    FDependencyPriorityMapping(const FString& InDPUId, int32 InPriority)
        : DependencyDPUId(InDPUId), Priority(InPriority)
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

    // 优先级 - Priority (用于同层节点排序)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    // 依赖优先级映射 - Dependency Priority Mappings
    // 用于指定当前DPU对其依赖项的DFS遍历优先级偏好
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency Ordering")
    TArray<FDependencyPriorityMapping> DependencyPriorityMappings;

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
     * 获取指定依赖DPU的优先级 - Get priority for specific dependency DPU
     * @param DependencyDPU 依赖的DPU
     * @return 优先级值，如果未指定则返回0（相同优先级）
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Dependency")  
    int32 GetDependencyPriority(USimpleDPU* DependencyDPU) const;

    /**
     * 设置依赖优先级 - Set dependency priority
     * @param DependencyDPUId 依赖DPU的ID
     * @param PriorityValue 优先级值
     */
    UFUNCTION(BlueprintCallable, Category = "DPU Dependency")
    void SetDependencyPriority(const FString& DependencyDPUId, int32 PriorityValue);

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
