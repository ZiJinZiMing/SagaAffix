// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameplayAbilityTargetData.h"


bool FAttackDamageTargetData::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	Ar << DamageType;
	Ar << Action;
	DamageContext.NetSerialize(Ar, Map, bOutSuccess);

	return bOutSuccess;
}
