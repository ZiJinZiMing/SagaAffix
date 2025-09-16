// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Object.h"
#include "SagaStatsType.generated.h"


/*State of Meter*/
UENUM(BlueprintType)
enum class EMeterState : uint8
{
	Normal,
	Lock,
	Reset,
};
DECLARE_ENUM_TO_STRING(EMeterState);

//todo:Meter间关系，需要删除
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SagaMeter_Calc_Reduce);
