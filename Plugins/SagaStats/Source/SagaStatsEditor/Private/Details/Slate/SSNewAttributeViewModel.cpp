/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "SSNewAttributeViewModel.h"

FSSNewAttributeViewModel::FSSNewAttributeViewModel()
	: bIsReplicated(true)
{
}

void FSSNewAttributeViewModel::Initialize()
{
}

FString FSSNewAttributeViewModel::ToString() const
{
	return FString::Printf(
		TEXT("VariableName: %s, Description: %s, bIsReplicated: %s, CustomizedBlueprint: %s, PinType: %s"),
		*GetVariableName(),
		*GetDescription(),
		GetbIsReplicated() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(GetCustomizedBlueprint().Get()),
		*GetNameSafe(GetPinType().PinSubCategoryObject.Get())
	);
}
