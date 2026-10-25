#include "Screens/HOLEndBanner.h"

#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/GameplayStatics.h"
#include "Net/HOLManagerClient.h"
#include "Player/HOLGameMenuRules.h"
#include "Screens/HOLGameMenu.h"
#include "Screens/HOLStatsScreen.h"
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
	const bool bMatchOver = Result == EHOLEndBanner::MatchOver;
	if (Title)
	{
		Title->SetText(bVictory ? LOCTEXT("Victory", "Victoire") : bMatchOver ? LOCTEXT("MatchOver", "Partie terminée") : LOCTEXT("Defeat", "Défaite"));
	}
	if (ContinueButton)
	{
		ContinueButton->SetLabel(bVictory ? LOCTEXT("Continue", "Continuer") : LOCTEXT("ContinueWatching", "Continuer à regarder"));
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: %s"), bVictory ? TEXT("Victory") : bMatchOver ? TEXT("MatchOver") : TEXT("Defeat"));
	// Open pushes (and activates) the banner before giving it its result: the way back follows the result.
	if (IsActivated())
	{
		RefreshWayBack();
	}
}

void UHOLEndBanner::SetDetail(const FText& InDetail)
{
	if (Detail)
	{
		Detail->SetText(InDetail);
		Detail->SetVisibility(InDetail.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (!InDetail.IsEmpty())
	{
		UE_LOG(LogHOLUI, Display, TEXT("End banner: detail \"%s\""), *InDetail.ToString());
	}
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

void UHOLEndBanner::ReturnToLobby()
{
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: back to lobby"));
	Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::Lobby);
	LeaveMatch(GetOwningPlayer());
}

void UHOLEndBanner::QuitToList()
{
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: quit to list"));
	Client->ForgetLobby();
	Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::List);
	LeaveMatch(GetOwningPlayer());
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
	// Under the title: the spectator's winner (M6-Q10); the match's statistics come below it later (C2-C6).
	Detail = AddText(Column, TEXT("Detail"), FText::GetEmpty(), UHOLTextStyleBody::StaticClass(), 16.f);
	Detail->SetVisibility(ESlateVisibility::Collapsed);
	UHorizontalBox* Buttons = AddButtonRow(Column);
	ContinueButton = AddButton(Buttons, TEXT("ContinueButton"), LOCTEXT("ContinueWatching", "Continuer à regarder"));
	QuitButton = AddButton(Buttons, TEXT("QuitButton"), LOCTEXT("QuitToMenu", "Quitter vers le menu"));
	ReturnButton = AddButton(Buttons, TEXT("ReturnButton"), LOCTEXT("ReturnToLobby", "Retour au salon"));
	StatsButton = AddButton(Buttons, TEXT("StatsButton"), LOCTEXT("Stats", "Statistiques"));
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
		QuitButton->OnClicked().AddWeakLambda(this, [this]() { HOLGameMenuRules::QuitsToList(Origin) ? QuitToList() : QuitToMenu(); });
	}
	if (ReturnButton)
	{
		ReturnButton->OnClicked().AddUObject(this, &UHOLEndBanner::ReturnToLobby);
	}
	if (StatsButton)
	{
		StatsButton->OnClicked().AddWeakLambda(this, [this]() { UHOLStatsScreen::Open(GetOwningPlayer()); });
	}
}

void UHOLEndBanner::NativeOnActivated()
{
	Super::NativeOnActivated();
	UHOLGameMenu::SetMatchMenuInput(GetOwningPlayer(), true, this);
	// From a lobby (R4.8): « Retour au salon » and « Quitter vers la liste », nobody sent back by himself (M6-Q7
	// revised); the spectator's banner (M6-Q10), from a lobby or the list's « Regarder », has no way back to a lobby he
	// is not in, but quits to the list as well.
	const UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	Origin = Client ? Client->GetMatchOrigin() : EHOLMatchOrigin::Local;
	RefreshWayBack();
}

void UHOLEndBanner::RefreshWayBack()
{
	const bool bReturns = HOLGameMenuRules::OffersReturnToLobby(Result, Origin);
	if (ReturnButton)
	{
		ReturnButton->SetVisibility(bReturns ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (QuitButton)
	{
		QuitButton->SetLabel(HOLGameMenuRules::QuitsToList(Origin) ? LOCTEXT("QuitToList", "Quitter vers la liste") : LOCTEXT("QuitToMenu", "Quitter vers le menu"));
		UE_LOG(LogHOLUI, Display, TEXT("End banner: way out %s (origin %d)"), HOLGameMenuRules::QuitsToList(Origin) ? TEXT("list") : TEXT("menu"), static_cast<int32>(Origin));
	}
	if (bReturns)
	{
		UE_LOG(LogHOLUI, Display, TEXT("End banner: lobby match, back to lobby offered, no countdown"));
	}
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
