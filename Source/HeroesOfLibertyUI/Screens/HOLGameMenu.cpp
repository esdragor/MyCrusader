#include "Screens/HOLGameMenu.h"

#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/HOLUIInputData.h"
#include "InputMappingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRules.h"
#include "Player/HOLPlayerController.h"
#include "Screens/HOLConfirmDialog.h"
#include "Screens/HOLEndBanner.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLGameMenu"

UHOLGameMenu* UHOLGameMenu::Open(APlayerController* PlayerController)
{
	if (UHOLGameMenu* Shown = Find(PlayerController))
	{
		return Shown;
	}
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	UHOLGameMenu* Menu = Layout && Layout->IsOverMatch() ? Cast<UHOLGameMenu>(Layout->PushMenu(StaticClass())) : nullptr;
	if (Menu)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: opened"));
	}
	return Menu;
}

UHOLGameMenu* UHOLGameMenu::Find(const APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLGameMenu>(Layout->GetActiveMenu()) : nullptr;
}

bool UHOLGameMenu::AskToSurrender(APlayerController* PlayerController)
{
	const UHOLGameMenu* Menu = Find(PlayerController);
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Menu || !Menu->CanSurrender() || !Layout)
	{
		return false;
	}
	UHOLConfirmDialog* Dialog = Cast<UHOLConfirmDialog>(Layout->PushModal(UHOLConfirmDialog::StaticClass()));
	if (!Dialog)
	{
		return false;
	}
	// Yes: the server defeats this player (D356), then the menu closes on the match; No: back to the menu.
	const TWeakObjectPtr<APlayerController> WeakController(PlayerController);
	Dialog->SetUp(LOCTEXT("SurrenderQuestion", "Abandonner la partie ?"), FText::GetEmpty(), [WeakController]()
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: Surrender"));
		// The menu first: the end banner then opens over the match (at once in solo or on a listen server).
		if (UHOLGameMenu* Shown = Find(WeakController.Get()))
		{
			Shown->Close();
		}
		if (AHOLPlayerController* Controller = Cast<AHOLPlayerController>(WeakController.Get()))
		{
			Controller->RequestSurrender();
		}
	});
	return true;
}

bool UHOLGameMenu::GetMatchState(bool& bDefeated, bool& bMatchOver) const
{
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	const AHOLGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	if (!PlayerState || !GameState)
	{
		return false;
	}
	bDefeated = GameState->IsSlotDefeated(PlayerState->GetPlayerIndex());
	bMatchOver = GameState->IsMatchOver();
	return true;
}

bool UHOLGameMenu::CanSurrender() const
{
	bool bDefeated = false;
	bool bMatchOver = false;
	return GetMatchState(bDefeated, bMatchOver) && HOLGameMenuRules::CanSurrender(bDefeated, bMatchOver);
}

void UHOLGameMenu::BuildLayout()
{
	UVerticalBox* Column = MakeDialogColumn();
	Title = AddText(Column, TEXT("Title"), LOCTEXT("Title", "Menu"), UHOLTextStyleHeading::StaticClass(), 16.f);
	ResumeButton = AddButton(Column, TEXT("ResumeButton"), LOCTEXT("Resume", "Reprendre"));
	SurrenderButton = AddButton(Column, TEXT("SurrenderButton"), LOCTEXT("Surrender", "Abandonner"));
	// The end banner's text (same key, already translated).
	QuitButton = AddButton(Column, TEXT("QuitButton"), NSLOCTEXT("HOLEndBanner", "QuitToMenu", "Quitter vers le menu"));
}

void UHOLGameMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ResumeButton)
	{
		ResumeButton->OnClicked().AddUObject(this, &UHOLGameMenu::Close);
	}
	if (SurrenderButton)
	{
		SurrenderButton->OnClicked().AddUObject(this, &UHOLGameMenu::HandleSurrender);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UHOLGameMenu::HandleQuitToMenu);
	}
}

void UHOLGameMenu::NativeOnActivated()
{
	Super::NativeOnActivated();
	SetOpen(true);
}

void UHOLGameMenu::NativeOnDeactivated()
{
	SetOpen(false);
	UE_LOG(LogHOLUI, Display, TEXT("Game menu: closed"));
	Super::NativeOnDeactivated();
}

void UHOLGameMenu::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// A defeat or the match's end may come while the menu is open.
	RefreshSurrender();
}

UWidget* UHOLGameMenu::NativeGetDesiredFocusTarget() const
{
	return ResumeButton;
}

void UHOLGameMenu::HandleSurrender()
{
	AskToSurrender(GetOwningPlayer());
}

void UHOLGameMenu::HandleQuitToMenu()
{
	UE_LOG(LogHOLUI, Display, TEXT("Game menu: quit to menu"));
	UHOLEndBanner::LeaveMatch(GetOwningPlayer());
}

void UHOLGameMenu::RefreshSurrender()
{
	// D357: a spectator (or anyone once the match is over) has no « Abandonner » at all, but « Quitter vers le menu ».
	bool bDefeated = false;
	bool bMatchOver = false;
	const bool bKnown = GetMatchState(bDefeated, bMatchOver);
	const auto Show = [](UHOLButton* Button, const bool bShown)
	{
		const ESlateVisibility Wanted = bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (Button && Button->GetVisibility() != Wanted)
		{
			Button->SetVisibility(Wanted);
		}
	};
	Show(SurrenderButton, bKnown && HOLGameMenuRules::CanSurrender(bDefeated, bMatchOver));
	Show(QuitButton, bKnown && HOLGameMenuRules::CanQuitToMenu(bDefeated, bMatchOver));
}

void UHOLGameMenu::SetOpen(const bool bOpen)
{
	RefreshSurrender();
	SetMatchMenuInput(GetOwningPlayer(), bOpen, this);
}

void UHOLGameMenu::SetMatchMenuInput(APlayerController* PlayerController, const bool bOpening, const UCommonActivatableWidget* Screen)
{
	// V1.4a: the menu closed over the end banner leaves the banner its Escape.
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	const bool bOpen = HOLGameMenuRules::KeepsMatchMenuInput(bOpening, Layout && Layout->HasMenuOtherThan(Screen));
	if (AHOLPlayerController* Controller = Cast<AHOLPlayerController>(PlayerController))
	{
		Controller->SetGameMenuOpen(bOpen);
	}
	// Common UI finds Back's key (Escape) in IMC_UI, which the match has only while the menu is open: closed, Escape
	// stays IA_Deselect's.
	UEnhancedInputLocalPlayerSubsystem* Input = PlayerController
		? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()) : nullptr;
	UInputMappingContext* Context = UHOLUIInputData::GetMappingContext();
	if (Input && Context)
	{
		if (bOpen)
		{
			Input->AddMappingContext(Context, /*Priority*/ 1);
		}
		else
		{
			Input->RemoveMappingContext(Context);
		}
	}
}

#undef LOCTEXT_NAMESPACE
