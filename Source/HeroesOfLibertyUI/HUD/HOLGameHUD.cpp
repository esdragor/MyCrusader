#include "HUD/HOLGameHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameScreen.h"

void AHOLGameHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	Screen = CreateWidget<UHOLGameScreen>(PlayerController, UHOLGameScreen::StaticClass());
	if (Screen)
	{
		Screen->AddToViewport();
		UE_LOG(LogHOLUI, Display, TEXT("Game HUD: game screen shown"));
	}
}

void AHOLGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Screen)
	{
		Screen->RemoveFromParent();
		Screen = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

UHOLGameScreen* AHOLGameHUD::FindScreen(const APlayerController* PlayerController)
{
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetScreen() : nullptr;
}
