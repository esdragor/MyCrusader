#include "Screens/HOLTitleScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "HeroesOfLibertyUILog.h"
#include "HOLVersion.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Screens/HOLConfirmDialogScreen.h"
#include "Screens/HOLMultiplayerScreen.h"
#include "Screens/HOLNewGameScreen.h"
#include "Screens/HOLSettingsScreen.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLTitleScreen"

UHOLTitleScreen::UHOLTitleScreen()
{
	bIsBackHandler = false;
}

TSubclassOf<UHOLTitleScreen> UHOLTitleScreen::FindScriptClass()
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

FText UHOLTitleScreen::ButtonLabel(const FName ButtonName)
{
	if (ButtonName == TEXT("PlayButton"))
	{
		return LOCTEXT("Play", "Jouer");
	}
	if (ButtonName == TEXT("MultiplayerButton"))
	{
		return LOCTEXT("Multiplayer", "Multijoueur");
	}
	if (ButtonName == TEXT("SettingsButton"))
	{
		return LOCTEXT("Settings", "Réglages");
	}
	if (ButtonName == TEXT("QuitButton"))
	{
		return LOCTEXT("Quit", "Quitter");
	}
	return FText::GetEmpty();
}

UVerticalBox* UHOLTitleScreen::MakeTitleColumn()
{
	return MakeCenteredColumn();
}

UCommonTextBlock* UHOLTitleScreen::AddTitleText(UVerticalBox* Column, const FName Name, const FText& Text, const bool bTitle)
{
	if (!Column)
	{
		return nullptr;
	}
	return bTitle ? AddText(Column, Name, Text, UHOLTextStyleTitle::StaticClass(), 48.f) : AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 8.f);
}

UHOLButton* UHOLTitleScreen::AddTitleButton(UVerticalBox* Column, const FName Name, const FText& Label)
{
	return Column ? AddButton(Column, Name, Label) : nullptr;
}

UCommonTextBlock* UHOLTitleScreen::AddVersionText()
{
	// The version (R1.7), small in the bottom right corner, not translated.
	if (UOverlay* Frame = Cast<UOverlay>(WidgetTree->RootWidget))
	{
		Version = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("Version"));
		Version->SetStyle(UHOLTextStyleNote::StaticClass());
		Version->SetText(FText::AsCultureInvariant(HOLVersion::Display(HOLVersion::Get())));
		UOverlaySlot* VersionSlot = Frame->AddChildToOverlay(Version);
		VersionSlot->SetHorizontalAlignment(HAlign_Right);
		VersionSlot->SetVerticalAlignment(VAlign_Bottom);
		VersionSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 12.f));
		return Version;
	}
	return nullptr;
}

void UHOLTitleScreen::BuildLayout()
{
	BP_BuildTitle();
}

FReply UHOLTitleScreen::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bool bHandled = false;
	const FReply Reply = CountTitleClick(InGeometry, InMouseEvent, bHandled);
	return bHandled ? Reply : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UHOLTitleScreen::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bool bHandled = false;
	const FReply Reply = CountTitleClick(InGeometry, InMouseEvent, bHandled);
	return bHandled ? Reply : Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply UHOLTitleScreen::CountTitleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, bool& bHandled)
{
	bHandled = false;
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || !Title || !Title->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
	{
		return FReply::Unhandled();
	}
	// Every click on the title is swallowed (a press or the double click Slate makes of a quick second one); the third of a series switches the cheat.
	bHandled = true;
	if (TitleClicks.Click(FPlatformTime::Seconds()))
	{
		const bool bOn = !HOLFactionChoice::IsCheatOn();
		HOLFactionChoice::SetCheatOn(bOn);
		UE_LOG(LogHOLUI, Display, TEXT("Title screen: faction cheat %s"), bOn ? TEXT("on") : TEXT("off"));
	}
	return FReply::Handled();
}

UWidget* UHOLTitleScreen::NativeGetDesiredFocusTarget() const
{
	return PlayButton && PlayButton->GetIsEnabled() ? PlayButton : QuitButton;
}

void UHOLTitleScreen::OpenNewGame()
{
	UHOLNewGameScreen::Open(GetOwningPlayer());
}

void UHOLTitleScreen::OpenMultiplayer()
{
	UHOLMultiplayerScreen::Open(GetOwningPlayer());
}

void UHOLTitleScreen::OpenSettings()
{
	UHOLSettingsScreen::Open(GetOwningPlayer());
}

void UHOLTitleScreen::AskQuit()
{
	AskToQuit(GetOwningPlayer());
}

void UHOLTitleScreen::AskToQuit(APlayerController* PlayerController)
{
	const TWeakObjectPtr<APlayerController> WeakController(PlayerController);
	UHOLConfirmDialogScreen::Ask(PlayerController, LOCTEXT("QuitQuestion", "Quitter le jeu ?"), LOCTEXT("QuitConfirm", "Quitter"), [WeakController]()
	{
		UE_LOG(LogHOLUI, Display, TEXT("Title screen: Quit"));
		UKismetSystemLibrary::QuitGame(WeakController.Get(), WeakController.Get(), EQuitPreference::Quit, false);
	});
}

#undef LOCTEXT_NAMESPACE
