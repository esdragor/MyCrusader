#include "Menu/HOLMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/HOLUIInputData.h"
#include "InputMappingContext.h"
#include "Screens/HOLTitleScreen.h"
#include "Widgets/HOLRootLayout.h"

AHOLMenuPlayerController::AHOLMenuPlayerController()
{
	bShowMouseCursor = true;
}

void AHOLMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* Context = UHOLUIInputData::GetMappingContext())
		{
			Input->AddMappingContext(Context, 0);
		}
		else
		{
			UE_LOG(LogHOLUI, Error, TEXT("Interface keys missing (/Game/UI/Input/IMC_UI): run Tools/Editor/make_ui_input.py"));
		}
	}

	RootLayout = CreateWidget<UHOLRootLayout>(this, UHOLRootLayout::StaticClass());
	RootLayout->AddToViewport();
	RootLayout->PushMenu(UHOLTitleScreen::StaticClass());
}
