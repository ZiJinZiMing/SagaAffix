/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

SAGASTATSEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogSagaStatsEditor, Display, All);

#define SS_EDITOR_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStatsEditor, Verbosity, Format, ##__VA_ARGS__); \
}

#define SS_EDITOR_NS_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStatsEditor, Verbosity, TEXT("%s - %s"), *FString(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}