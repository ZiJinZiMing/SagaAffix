/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "SGNewAttributeViewModel.h"

FSGNewAttributeViewModel::FSGNewAttributeViewModel()
	: bIsReplicated(true)
{
}

void FSGNewAttributeViewModel::Initialize()
{
}

FString FSGNewAttributeViewModel::ToString() const
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
