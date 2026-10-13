#include "Input/HOLUIInputData.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::UI
{
	const TCHAR* MappingContextPath = TEXT("/Game/UI/Input/IMC_UI.IMC_UI");
	const TCHAR* NextColumnPath = TEXT("/Game/UI/Input/IA_UI_NextColumn.IA_UI_NextColumn");
	const TCHAR* PreviousColumnPath = TEXT("/Game/UI/Input/IA_UI_PreviousColumn.IA_UI_PreviousColumn");
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

const UInputAction* UHOLUIInputData::GetNextColumnAction()
{
	return LoadObject<UInputAction>(nullptr, UE::HeroesOfLiberty::UI::NextColumnPath);
}

const UInputAction* UHOLUIInputData::GetPreviousColumnAction()
{
	return LoadObject<UInputAction>(nullptr, UE::HeroesOfLiberty::UI::PreviousColumnPath);
}
