#include "HUD/HOLGameHUD.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameScreen.h"
#include "Player/HOLPlayerController.h"

namespace UE::HeroesOfLiberty::UI::GameHUD
{
	/** The selection box: a faint fill and a light outline. */
	const FLinearColor BoxFill(1.f, 1.f, 1.f, 0.08f);
	const FLinearColor BoxOutline(1.f, 1.f, 1.f, 0.85f);
	constexpr float BoxOutlineThickness = 1.5f;
}

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

void AHOLGameHUD::DrawHUD()
{
	using namespace UE::HeroesOfLiberty::UI::GameHUD;

	Super::DrawHUD();
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayerController());
	FVector2D Min, Max;
	if (!Canvas || !PlayerController || !PlayerController->GetSelectionBox(Min, Max))
	{
		return;
	}
	const FVector2D Size(Canvas->ClipX, Canvas->ClipY);
	const FVector2D TopLeft = Min * Size;
	const FVector2D BottomRight = Max * Size;
	DrawRect(BoxFill, TopLeft.X, TopLeft.Y, BottomRight.X - TopLeft.X, BottomRight.Y - TopLeft.Y);
	DrawLine(TopLeft.X, TopLeft.Y, BottomRight.X, TopLeft.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(BottomRight.X, TopLeft.Y, BottomRight.X, BottomRight.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(BottomRight.X, BottomRight.Y, TopLeft.X, BottomRight.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(TopLeft.X, BottomRight.Y, TopLeft.X, TopLeft.Y, BoxOutline, BoxOutlineThickness);
}

UHOLGameScreen* AHOLGameHUD::FindScreen(const APlayerController* PlayerController)
{
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetScreen() : nullptr;
}
