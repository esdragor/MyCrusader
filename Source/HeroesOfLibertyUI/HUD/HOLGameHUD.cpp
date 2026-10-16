#include "HUD/HOLGameHUD.h"

#include "Blueprint/UserWidget.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameScreen.h"
#include "Player/HOLPlayerController.h"
#include "Style/HOLUIStyles.h"

namespace UE::HeroesOfLiberty::UI::GameHUD
{
	/** The selection box: a faint fill and a light outline. */
	const FLinearColor BoxFill(1.f, 1.f, 1.f, 0.08f);
	const FLinearColor BoxOutline(1.f, 1.f, 1.f, 0.85f);
	constexpr float BoxOutlineThickness = 1.5f;

	/**
	 * The « +10 » (J5c.3): starts this high above the ground where the Peasant unloaded (cm, over its head), rises this
	 * much while it shows, fully opaque for the first part of its life, then fades out.
	 */
	constexpr double PopupStartHeight = 200.0;
	constexpr double PopupRise = 120.0;
	constexpr float PopupOpaqueShare = 0.5f;
	/** Text size (points), the resource's marker (pixels), the gap between them and a stacked one's line height. */
	constexpr float PopupFontSize = 20.f;
	constexpr float PopupMarkerSize = 13.f;
	constexpr float PopupMarkerGap = 5.f;
	constexpr float PopupLineHeight = 24.f;
	const FLinearColor PopupText(1.f, 0.97f, 0.88f, 1.f);
	const FLinearColor PopupOutline(0.f, 0.f, 0.f, 0.85f);

	/** A resource's colour, as its marker in the top bar. */
	const FLinearColor& GetResourceColor(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return HOLUIStyle::Wood;
		case EHOLResource::Stone: return HOLUIStyle::Stone;
		case EHOLResource::Gold: return HOLUIStyle::Gold;
		default: return HOLUIStyle::Food;
		}
	}
}

void AHOLGameHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	PopupFont = LoadObject<UFont>(nullptr, TEXT("/Game/UI/Fonts/F_HOL.F_HOL"));
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
	Super::DrawHUD();
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayerController());
	if (!Canvas || !PlayerController)
	{
		return;
	}
	DrawDepositPopups(*PlayerController);
	DrawSelectionBox(*PlayerController);
}

void AHOLGameHUD::DrawDepositPopups(const AHOLPlayerController& PlayerController)
{
	using namespace UE::HeroesOfLiberty::UI::GameHUD;

	const TConstArrayView<AHOLPlayerController::FShownDepositPopup> Popups = PlayerController.GetDepositPopups();
	const UWorld* World = GetWorld();
	if (Popups.IsEmpty() || !World)
	{
		return;
	}
	const UFont* Font = PopupFont ? PopupFont.Get() : GEngine->GetLargeFont();
	const FSlateFontInfo FontInfo(Font, FMath::RoundToInt(PopupFontSize), TEXT("Bold"));
	const float Now = World->GetTimeSeconds();
	for (const AHOLPlayerController::FShownDepositPopup& Shown : Popups)
	{
		const float Life = (Now - Shown.BornSeconds) / AHOLPlayerController::DepositPopupSeconds;
		if (Life < 0.f || Life >= 1.f)
		{
			continue;
		}
		const FVector Anchor = FVector(Shown.Popup.Location) + FVector(0., 0., PopupStartHeight + PopupRise * Life);
		const FVector OnScreen = Project(Anchor);
		if (OnScreen.Z <= 0. || OnScreen.X < -50. || OnScreen.Y < -50. || OnScreen.X > Canvas->ClipX + 50. || OnScreen.Y > Canvas->ClipY + 50.)
		{
			continue;
		}
		const float Alpha = Life < PopupOpaqueShare ? 1.f : 1.f - (Life - PopupOpaqueShare) / (1.f - PopupOpaqueShare);
		const FVector2D Position(OnScreen.X, OnScreen.Y - Shown.Stack * PopupLineHeight);

		// The amount, its left edge just right of the anchor; the marker left of it, centred on its height.
		FCanvasTextItem Text(FVector2D(Position.X + PopupMarkerGap * 0.5, Position.Y), FText::AsCultureInvariant(FString::Printf(TEXT("+%d"), Shown.Popup.Amount)),
			FontInfo, PopupText.CopyWithNewOpacity(Alpha));
		Text.bOutlined = true;
		Text.OutlineColor = PopupOutline.CopyWithNewOpacity(PopupOutline.A * Alpha);
		Canvas->DrawItem(Text);
		const double TextHeight = Text.DrawnSize.Y > 0. ? Text.DrawnSize.Y : PopupFontSize;
		const double MarkerX = Position.X - PopupMarkerGap * 0.5 - PopupMarkerSize;
		const double MarkerY = Position.Y + (TextHeight - PopupMarkerSize) * 0.5;
		DrawRect(PopupOutline.CopyWithNewOpacity(PopupOutline.A * Alpha), MarkerX - 1., MarkerY - 1., PopupMarkerSize + 2., PopupMarkerSize + 2.);
		DrawRect(GetResourceColor(Shown.Popup.Resource).CopyWithNewOpacity(Alpha), MarkerX, MarkerY, PopupMarkerSize, PopupMarkerSize);
	}
}

void AHOLGameHUD::DrawSelectionBox(const AHOLPlayerController& PlayerController)
{
	using namespace UE::HeroesOfLiberty::UI::GameHUD;

	FVector2D Min, Max;
	if (!PlayerController.GetSelectionBox(Min, Max))
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
