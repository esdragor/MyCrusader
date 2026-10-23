#include "Screens/HOLEndBanner.h"

#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HOLGameMenuRules.h"
#include "Screens/HOLGameMenu.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLEndBanner"

namespace UE::HeroesOfLiberty::UI::EndBanner
{
	/** The menu's map (DefaultEngine.ini, GameDefaultMap). */
	const TCHAR* const MainMenuMap = TEXT("/Game/Maps/MainMenu");
}

UHOLEndBanner* UHOLEndBanner::Open(APlayerController* PlayerController, const EHOLEndBanner InResult)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout || !Layout->IsOverMatch())
	{
		return nullptr;
	}
	// The in-game menu gives way (its confirmation with it): the banner is the answer.
	if (UCommonActivatableWidget* Modal = Layout->GetActiveModal())
	{
		Modal->DeactivateWidget();
	}
	if (UHOLGameMenu* Menu = UHOLGameMenu::Find(PlayerController))
	{
		Menu->DeactivateWidget();
	}
	UHOLEndBanner* Banner = Cast<UHOLEndBanner>(Layout->PushMenu(StaticClass()));
	if (Banner)
	{
		Banner->SetResult(InResult);
	}
	return Banner;
}

void UHOLEndBanner::SetResult(const EHOLEndBanner InResult)
{
	Result = InResult;
	const bool bVictory = Result == EHOLEndBanner::Victory;
	if (Title)
	{
		Title->SetText(bVictory ? LOCTEXT("Victory", "Victoire") : LOCTEXT("Defeat", "Défaite"));
	}
	if (ContinueButton)
	{
		ContinueButton->SetLabel(bVictory ? LOCTEXT("Continue", "Continuer") : LOCTEXT("ContinueWatching", "Continuer à regarder"));
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: %s"), bVictory ? TEXT("Victory") : TEXT("Defeat"));
}

void UHOLEndBanner::Continue()
{
	if (!IsActivated())
	{
		return;
	}
	const EHOLEndBanner Closed = Result;
	DeactivateWidget();
	UE_LOG(LogHOLUI, Display, TEXT("End banner: closed"));
	OnClosedByPlayer.Broadcast(Closed);
}

void UHOLEndBanner::QuitToMenu()
{
	UE_LOG(LogHOLUI, Display, TEXT("End banner: quit to menu"));
	LeaveMatch(GetOwningPlayer());
}

void UHOLEndBanner::LeaveMatch(APlayerController* PlayerController)
{
	// The way back of UHOLNewGameScreen::Launch (OpenLevel): a local map, which ends this machine's network game, a
	// listen server's (its clients dropped) or a client's (D357).
	UGameplayStatics::OpenLevel(PlayerController, FName(UE::HeroesOfLiberty::UI::EndBanner::MainMenuMap));
}

void UHOLEndBanner::BuildLayout()
{
	UVerticalBox* Column = MakeDialogColumn();
	Title = AddText(Column, TEXT("Title"), FText::GetEmpty(), UHOLTextStyleTitle::StaticClass(), 24.f);
	UHorizontalBox* Buttons = AddButtonRow(Column);
	ContinueButton = AddButton(Buttons, TEXT("ContinueButton"), LOCTEXT("ContinueWatching", "Continuer à regarder"));
	QuitButton = AddButton(Buttons, TEXT("QuitButton"), LOCTEXT("QuitToMenu", "Quitter vers le menu"));
}

void UHOLEndBanner::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ContinueButton)
	{
		ContinueButton->OnClicked().AddUObject(this, &UHOLEndBanner::Continue);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UHOLEndBanner::QuitToMenu);
	}
}

void UHOLEndBanner::NativeOnActivated()
{
	Super::NativeOnActivated();
	UHOLGameMenu::SetMatchMenuInput(GetOwningPlayer(), true, this);
}

void UHOLEndBanner::NativeOnDeactivated()
{
	UHOLGameMenu::SetMatchMenuInput(GetOwningPlayer(), false, this);
	Super::NativeOnDeactivated();
}

bool UHOLEndBanner::NativeOnHandleBackAction()
{
	// Back is « Continuer à regarder ».
	Continue();
	return true;
}

UWidget* UHOLEndBanner::NativeGetDesiredFocusTarget() const
{
	return ContinueButton;
}

#undef LOCTEXT_NAMESPACE
