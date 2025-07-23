/******************************************************************************
* ProjectName:  SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************/

#pragma once

#include "CoreMinimal.h"

SAGASTATS_API DECLARE_LOG_CATEGORY_EXTERN(LogSagaStats, Display, All);

#define SS_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStats, Verbosity, Format, ##__VA_ARGS__); \
}

#define SS_NS_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStats, Verbosity, TEXT("%s - %s"), *FString(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}