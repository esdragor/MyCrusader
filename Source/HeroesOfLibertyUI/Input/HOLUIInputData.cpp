#include "Input/HOLUIInputData.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::UI
{
	const TCHAR* MappingContextPath = TEXT("/Game/UI/Input/IMC_UI.IMC_UI");
}

UHOLUIInputData::UHOLUIInputData()
{
	static ConstructorHelpers::FObjectFinder<UInputAction> Confirm(TEXT("/Game/UI/Input/IA_UI_Confirm.IA_UI_Confirm"));
	static ConstructorHelpers::FObjectFinder<UInputAction> Back(TEXT("/Game/UI/Input/IA_UI_Back.IA_UI_Back"));
	EnhancedInputClickAction = Confirm.Object;
	EnhancedInputBackAction = Back.Object;
}

UInputMappingContext* UHOLUIInputData::GetMappingContext()
{
	return LoadObject<UInputMappingContext>(nullptr, UE::HeroesOfLiberty::UI::MappingContextPath);
}
