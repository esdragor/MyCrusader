#include "HUD/HOLScriptScreen.h"

#include "Blueprint/WidgetTree.h"
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

FMargin UHOLScriptScreen::GetBottomLeftPlace() const
{
	return UHOLGameScreen::GetBottomLeftPlace();
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
	RefreshLeft -= InDeltaTime;
	if (RefreshLeft <= 0.f)
	{
		RefreshNow();
	}
}
