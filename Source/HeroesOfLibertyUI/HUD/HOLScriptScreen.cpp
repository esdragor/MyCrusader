#include "HUD/HOLScriptScreen.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLHUDLayoutRules.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "HUD/HOLGameScreen.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"

FString UHOLScriptScreen::Describe_Implementation() const
{
	return FString();
}

bool UHOLScriptScreen::ClickWidget_Implementation(const FName Name)
{
	return false;
}

void UHOLScriptScreen::RefreshNow()
{
	RefreshLeft = RefreshSeconds;
	Refresh();
}

int32 UHOLScriptScreen::GetLocalSlot() const
{
	const AHOLPlayerState* PlayerState = GetOwningPlayerState<AHOLPlayerState>();
	return PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
}

UBorder* UHOLScriptScreen::MakePanel(const FName Name)
{
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
	Panel->SetPadding(FMargin(12.f, 8.f));
	Panel->SetVisibility(ESlateVisibility::Visible);
	return Panel;
}

UCommonTextBlock* UHOLScriptScreen::MakeText(const FName Name, const TSubclassOf<UCommonTextStyle> TextStyle)
{
	UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
	Block->SetStyle(TextStyle);
	return Block;
}

UButton* UHOLScriptScreen::MakeButton(const FName Name)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	SetButtonGreyed(Button, false);
	// Never focusable (as the HUD's other buttons): set before its Slate widget is built. InitIsFocusable is protected.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Button->IsFocusable = false;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	Button->SetVisibility(ESlateVisibility::Visible);
	return Button;
}

void UHOLScriptScreen::SetButtonGreyed(UButton* Button, const bool bGreyed)
{
	if (!Button)
	{
		return;
	}
	// UHOLHUDButtonStyle's and UHOLHUDButtonStyleGreyed's brushes (HOLUIStyles.cpp), for a UMG button the script fills.
	constexpr float Radius = 3.f;
	const FLinearColor Greyed(0.035f, 0.035f, 0.045f, 1.f);
	FButtonStyle Look;
	if (bGreyed)
	{
		Look.SetNormal(FSlateRoundedBoxBrush(Greyed, Radius));
		Look.SetHovered(FSlateRoundedBoxBrush(Greyed, Radius, HOLUIStyle::TextMuted, 1.f));
		Look.SetPressed(FSlateRoundedBoxBrush(Greyed, Radius, HOLUIStyle::TextMuted, 1.f));
	}
	else
	{
		Look.SetNormal(FSlateRoundedBoxBrush(FLinearColor(0.07f, 0.07f, 0.09f, 1.f), Radius));
		Look.SetHovered(FSlateRoundedBoxBrush(FLinearColor(0.16f, 0.12f, 0.05f, 1.f), Radius, HOLUIStyle::Accent, 1.f));
		Look.SetPressed(FSlateRoundedBoxBrush(FLinearColor(0.25f, 0.18f, 0.06f, 1.f), Radius, HOLUIStyle::Accent, 1.f));
	}
	Look.SetDisabled(FSlateRoundedBoxBrush(Greyed, Radius));
	Look.SetNormalPadding(FMargin(4.f));
	Look.SetPressedPadding(FMargin(4.f));
	Button->SetStyle(Look);
}

UProgressBar* UHOLScriptScreen::MakeBar(const FName Name, const FLinearColor FillColor)
{
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
	FProgressBarStyle Look;
	Look.SetBackgroundImage(FSlateRoundedBoxBrush(FLinearColor(0.08f, 0.08f, 0.1f, 1.f), 2.f));
	Look.SetFillImage(FSlateRoundedBoxBrush(FLinearColor::White, 2.f));
	Bar->SetWidgetStyle(Look);
	Bar->SetFillColorAndOpacity(FillColor);
	Bar->SetPercent(0.f);
	return Bar;
}

FLinearColor UHOLScriptScreen::GetLocalColor() const
{
	const AHOLPlayerState* PlayerState = GetOwningPlayerState<AHOLPlayerState>();
	if (!PlayerState || PlayerState->GetPlayerIndex() == INDEX_NONE)
	{
		return HOLUIStyle::TextMuted;
	}
	return FLinearColor(HOLPlayerColors::GetColor(PlayerState->GetColorIndex()));
}

FLinearColor UHOLScriptScreen::GetTextColor()
{
	return HOLUIStyle::Text;
}

FLinearColor UHOLScriptScreen::GetMutedColor()
{
	return HOLUIStyle::TextMuted;
}

FLinearColor UHOLScriptScreen::GetMissingColor()
{
	return HOLUIStyle::Missing;
}

float UHOLScriptScreen::GetCheatPanelBottom() const
{
	const UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer());
	return CheatPanel ? CheatPanel->GetBottom() : 0.f;
}

void UHOLScriptScreen::SetCheatsHeroBlockShown(const bool bShown) const
{
	if (UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer()))
	{
		CheatPanel->SetHeroBlockShown(bShown);
	}
}

FVector2D UHOLScriptScreen::GetViewportSlateSize() const
{
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), UE_KINDA_SMALL_NUMBER);
	return UWidgetLayoutLibrary::GetViewportSize(this) / Dpi;
}

bool UHOLScriptScreen::GetWidgetBox(UWidget* Widget, FVector2D& OutMin, FVector2D& OutMax) const
{
	if (!Widget || !Widget->IsVisible())
	{
		return false;
	}
	const FGeometry& Geometry = Widget->GetCachedGeometry();
	if (Geometry.GetAbsoluteSize().IsNearlyZero())
	{
		return false;
	}
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), UE_KINDA_SMALL_NUMBER);
	OutMin = FVector2D(Geometry.GetAbsolutePosition()) / Dpi;
	OutMax = FVector2D(Geometry.GetAbsolutePosition() + Geometry.GetAbsoluteSize()) / Dpi;
	return true;
}

bool UHOLScriptScreen::GetGameScreenBox(const FName WidgetName, FVector2D& OutMin, FVector2D& OutMax) const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	UWidget* Widget = GameScreen && GameScreen->WidgetTree ? GameScreen->WidgetTree->FindWidget(WidgetName) : nullptr;
	return GetWidgetBox(Widget, OutMin, OutMax);
}

FString UHOLScriptScreen::DescribeOverlaps() const
{
	struct FNamedBox
	{
		FString Name;
		FBox2D Box;
	};
	TArray<FNamedBox> Boxes;
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), UE_KINDA_SMALL_NUMBER);
	if (const UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer()))
	{
		FBox2D CheatBox;
		if (CheatPanel->GetBox(CheatBox))
		{
			Boxes.Add({ TEXT("cheats"), CheatBox });
		}
	}
	// The C++ blocks a panel of the script must not cover (the Diplomacy panel's trial): the players' table, the menu's buttons.
	const TCHAR* const GameNames[] = { TEXT("PlayersPanel"), TEXT("MenuPanel") };
	for (const TCHAR* GameName : GameNames)
	{
		FVector2D Low;
		FVector2D High;
		if (GetGameScreenBox(FName(GameName), Low, High))
		{
			Boxes.Add({ GameName, FBox2D(Low, High) });
		}
	}
	if (WidgetTree)
	{
		TArray<UWidget*> Widgets;
		WidgetTree->GetAllWidgets(Widgets);
		for (UWidget* Widget : Widgets)
		{
			const UBorder* Border = Cast<UBorder>(Widget);
			if (!Border || !Border->IsVisible())
			{
				continue;
			}
			// A border inside another one (the hero's portrait in the hero's block, its ability boxes) is part of that panel, not a panel
			// covering it: only the outermost borders are compared (the hero's block read "HeroPanel x HeroPortraitFill" in every scenario).
			bool bNested = false;
			for (const UWidget* Parent = Border->GetParent(); Parent && !bNested; Parent = Parent->GetParent())
			{
				bNested = Parent->IsA<UBorder>();
			}
			if (bNested)
			{
				continue;
			}
			const FGeometry& Geometry = Border->GetCachedGeometry();
			if (Geometry.GetAbsoluteSize().IsNearlyZero())
			{
				continue;
			}
			Boxes.Add({ Border->GetName(), FBox2D(FVector2D(Geometry.GetAbsolutePosition()) / Dpi, FVector2D(Geometry.GetAbsolutePosition() + Geometry.GetAbsoluteSize()) / Dpi) });
		}
	}
	FString Result;
	for (int32 First = 0; First < Boxes.Num(); ++First)
	{
		for (int32 Second = First + 1; Second < Boxes.Num(); ++Second)
		{
			if (HOLHUDLayout::Overlaps(Boxes[First].Box, Boxes[Second].Box))
			{
				Result += (Result.IsEmpty() ? TEXT("") : TEXT("; ")) + FString::Printf(TEXT("%s x %s"), *Boxes[First].Name, *Boxes[Second].Name);
			}
		}
	}
	return Result.IsEmpty() ? FString(TEXT("none")) : Result;
}

FMargin UHOLScriptScreen::GetBottomLeftPlace() const
{
	// The game screen made this widget (CreateWidget: its widget tree is the outer); EtapeDefini T: the place follows the
	// bar's scale, so the script reads it on every refresh. Without it: the places at scale 1.
	if (const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		return GameScreen->GetBottomLeftPlace();
	}
	return GetDefault<UHOLGameScreen>()->GetBottomLeftPlace();
}

UClass* UHOLScriptScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

void UHOLScriptScreen::NativeOnInitialized()
{
	// The script's OnInitialized builds the tree.
	Super::NativeOnInitialized();
	// As the C++ frame: the clicks go through, only the panels take them.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UWidget* Root = GetRootWidget())
	{
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UHOLScriptScreen::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshNow();
}

void UHOLScriptScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// The T key (the talent tree, TQ4 = A): the viewport's key state, as the Enter of a test scenario; the console open or a focused menu keeps it.
	if (const APlayerController* Controller = GetOwningPlayer())
	{
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::T))
		{
			KeyPressed(TEXT("T"));
		}
		// F11: the Diplomacy panel (D387, AoE2's key), a default of the panel's trial until the keys are chosen.
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::F11))
		{
			KeyPressed(TEXT("F11"));
		}
	}
	RefreshLeft -= InDeltaTime;
	if (RefreshLeft <= 0.f)
	{
		RefreshNow();
	}
}
