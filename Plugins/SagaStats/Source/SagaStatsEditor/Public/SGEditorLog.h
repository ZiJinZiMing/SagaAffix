/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"

SAGASTATSEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogSagaStatsEditor, Display, All);

#define SG_EDITOR_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStatsEditor, Verbosity, Format, ##__VA_ARGS__); \
}

#define SG_EDITOR_NS_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogSagaStatsEditor, Verbosity, TEXT("%s - %s"), *FString(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}