/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"

SAGASTATS_API DECLARE_LOG_CATEGORY_EXTERN(LogSagaStats, Display, All);

#define SG_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStats, Verbosity, Format, ##__VA_ARGS__); \
}

#define SG_NS_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStats, Verbosity, TEXT("%s - %s"), *FString(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}