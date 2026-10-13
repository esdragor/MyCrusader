#include "Screens/HOLTitleScreen.h"

#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Screens/HOLConfirmDialog.h"
#include "Screens/HOLNewGameScreen.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLTitleScreen"

UHOLTitleScreen::UHOLTitleScreen()
{
	bIsBackHandler = false;
}

void UHOLTitleScreen::BuildLayout()
{
	UVerticalBox* Column = MakeCenteredColumn();
	// The game's name is not translated.
	Title = AddText(Column, TEXT("Title"), INVTEXT("Heroes of Liberty"), UHOLTextStyleTitle::StaticClass(), 48.f);
	PlayButton = AddButton(Column, TEXT("PlayButton"), LOCTEXT("Play", "Jouer"));
	SettingsButton = AddButton(Column, TEXT("SettingsButton"), LOCTEXT("Settings", "Réglages"));
	QuitButton = AddButton(Column, TEXT("QuitButton"), LOCTEXT("Quit", "Quitter"));
}

void UHOLTitleScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (PlayButton)
	{
		PlayButton->OnClicked().AddUObject(this, &UHOLTitleScreen::HandlePlay);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked().AddUObject(this, &UHOLTitleScreen::HandleSettings);
		// The Settings screen comes next (front-end step 4, part 3).
		SettingsButton->SetIsEnabled(false);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UHOLTitleScreen::HandleQuit);
	}
}

UWidget* UHOLTitleScreen::NativeGetDesiredFocusTarget() const
{
	return PlayButton && PlayButton->GetIsEnabled() ? PlayButton : QuitButton;
}

void UHOLTitleScreen::HandlePlay()
{
	UHOLNewGameScreen::Open(GetOwningPlayer());
}

void UHOLTitleScreen::HandleSettings()
{
}

void UHOLTitleScreen::HandleQuit()
{
	AskToQuit(GetOwningPlayer());
}

void UHOLTitleScreen::AskToQuit(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	UHOLConfirmDialog* Dialog = Layout ? Cast<UHOLConfirmDialog>(Layout->PushModal(UHOLConfirmDialog::StaticClass())) : nullptr;
	if (!Dialog)
	{
		return;
	}
	const TWeakObjectPtr<APlayerController> WeakController(PlayerController);
	Dialog->SetUp(LOCTEXT("QuitQuestion", "Quitter le jeu ?"), LOCTEXT("QuitConfirm", "Quitter"), [WeakController]()
	{
		UE_LOG(LogHOLUI, Display, TEXT("Title screen: Quit"));
		UKismetSystemLibrary::QuitGame(WeakController.Get(), WeakController.Get(), EQuitPreference::Quit, false);
	});
}

#undef LOCTEXT_NAMESPACE
