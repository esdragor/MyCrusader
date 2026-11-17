#include "Screens/HOLGameMenuScreen.h"

#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/HOLUIInputData.h"
#include "InputMappingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPauseLibrary.h"
#include "Match/HOLPlayerState.h"
#include "HUD/HOLGameHUD.h"
#include "Net/HOLManagerClient.h"
#include "Player/HOLGameMenuRulesBridge.h"
#include "Player/HOLPlayerController.h"
#include "Screens/HOLConfirmDialogScreen.h"
#include "Screens/HOLEndBannerScreen.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLGameMenu"

UHOLGameMenuScreen* UHOLGameMenuScreen::Open(APlayerController* PlayerController)
{
	if (UHOLGameMenuScreen* Shown = Find(PlayerController))
	{
		return Shown;
	}
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout || !Layout->IsOverMatch())
	{
		return nullptr;
	}
	const TSubclassOf<UHOLGameMenuScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Game menu: no script class (Script/UI/HOLGameMenuScreen.as)"));
		return nullptr;
	}
	UHOLGameMenuScreen* Menu = Cast<UHOLGameMenuScreen>(Layout->PushMenu(ScriptClass.Get()));
	if (Menu)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: opened"));
	}
	return Menu;
}

UHOLGameMenuScreen* UHOLGameMenuScreen::Find(const APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLGameMenuScreen>(Layout->GetActiveMenu()) : nullptr;
}

TSubclassOf<UHOLGameMenuScreen> UHOLGameMenuScreen::FindScriptClass()
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

FHOLGameMenuFacts UHOLGameMenuScreen::ReadFacts(APlayerController* PlayerController)
{
	FHOLGameMenuFacts Facts;
	const UGameInstance* GameInstance = PlayerController ? PlayerController->GetGameInstance() : nullptr;
	const UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	Facts.Origin = static_cast<int32>(Client ? Client->GetMatchOrigin() : EHOLMatchOrigin::Local);
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(PlayerController);
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	const UWorld* World = PlayerController ? PlayerController->GetWorld() : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!PlayerState || !GameState)
	{
		return Facts;
	}
	Facts.bKnown = true;
	Facts.MySlot = PlayerState->GetPlayerIndex();
	Facts.bDefeated = GameState->IsSlotDefeated(Facts.MySlot);
	Facts.bMatchOver = GameState->IsMatchOver();
	Facts.bPaused = GameState->IsMatchPaused();
	Facts.bPauseFree = GameState->IsPauseFree();
	Facts.PausesLeft = GameState->GetPausesLeft();
	Facts.bCanPauseNow = Facts.MySlot != INDEX_NONE && UHOLPauseLibrary::CanPauseNow(PlayerController, Facts.MySlot);
	Facts.DefeatedMask = GameState->GetDefeatedMask();
	Facts.WinnerMask = GameState->GetWinnerMask();
	return Facts;
}

bool UHOLGameMenuScreen::CanSurrender(APlayerController* PlayerController)
{
	const FHOLGameMenuFacts Facts = ReadFacts(PlayerController);
	return Facts.bKnown && UHOLGameMenuRulesBridge::CanSurrender(Facts.bDefeated, Facts.bMatchOver);
}

bool UHOLGameMenuScreen::AskToSurrender(APlayerController* PlayerController)
{
	const UHOLGameMenuScreen* Menu = Find(PlayerController);
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Menu || !CanSurrender(PlayerController) || !Layout)
	{
		return false;
	}
	// Yes: the server defeats this player (D356), then the menu closes on the match; No: back to the menu.
	const TWeakObjectPtr<APlayerController> WeakController(PlayerController);
	if (!UHOLConfirmDialogScreen::Ask(PlayerController, LOCTEXT("SurrenderQuestion", "Abandonner la partie ?"), FText::GetEmpty(), [WeakController]()
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: Surrender"));
		// The menu first: the end banner then opens over the match (at once in solo or on a listen server).
		if (UHOLGameMenuScreen* Shown = Find(WeakController.Get()))
		{
			Shown->DeactivateWidget();
		}
		if (AHOLPlayerController* Controller = Cast<AHOLPlayerController>(WeakController.Get()))
		{
			Controller->RequestSurrender();
		}
	}))
	{
		return false;
	}
	return true;
}

void UHOLGameMenuScreen::SetButtonLabel(UHOLButton* Button, const FText& Label)
{
	if (Button)
	{
		Button->SetLabel(Label);
	}
}

void UHOLGameMenuScreen::MenuLog(const FString& Line)
{
	UE_LOG(LogHOLUI, Display, TEXT("%s"), *Line);
}

UVerticalBox* UHOLGameMenuScreen::MakeMenuColumn()
{
	return MakeDialogColumn();
}

UCommonTextBlock* UHOLGameMenuScreen::AddMenuTitle(UVerticalBox* Column, const FName Name, const FText& Text)
{
	return Column ? AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 16.f) : nullptr;
}

UHOLButton* UHOLGameMenuScreen::AddMenuButton(UVerticalBox* Column, const FName Name, const FText& Label)
{
	return Column ? AddButton(Column, Name, Label) : nullptr;
}

void UHOLGameMenuScreen::AskSurrender()
{
	AskToSurrender(GetOwningPlayer());
}

void UHOLGameMenuScreen::PauseMatch()
{
	AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	if (!Controller || !PlayerState || !UHOLPauseLibrary::CanPauseNow(this, PlayerState->GetPlayerIndex()))
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("Game menu: Pause"));
	// The menu closes: the pause screen comes over the match once the server has paused it.
	DeactivateWidget();
	Controller->RequestPause();
}

void UHOLGameMenuScreen::QuitMatch()
{
	// From the manager (a lobby's match or a watched one), the list, as the end banner's quit (R4.8, M6-Q10).
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (Client && UHOLGameMenuRulesBridge::QuitsToList(Client->GetMatchOrigin()))
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: quit to list"));
		Client->ForgetLobby();
		Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::List);
	}
	else
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: quit to menu"));
	}
	UHOLEndBannerScreen::LeaveMatch(GetOwningPlayer());
}

void UHOLGameMenuScreen::ReturnToLobby()
{
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("Game menu: back to lobby"));
	Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::Lobby);
	UHOLEndBannerScreen::LeaveMatch(GetOwningPlayer());
}

void UHOLGameMenuScreen::ShowResult()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (AHOLGameHUD* HUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Game menu: result"));
		// The banner closes this menu as it opens (UHOLEndBannerScreen::Open).
		HUD->ReopenEndBanner();
	}
}

void UHOLGameMenuScreen::BuildLayout()
{
	BP_BuildMenu();
}

void UHOLGameMenuScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	BP_RefreshMenu(true);
	SetMatchMenuInput(GetOwningPlayer(), true, this);
}

void UHOLGameMenuScreen::NativeOnDeactivated()
{
	SetMatchMenuInput(GetOwningPlayer(), false, this);
	UE_LOG(LogHOLUI, Display, TEXT("Game menu: closed"));
	Super::NativeOnDeactivated();
}

void UHOLGameMenuScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// A defeat or the match's end may come while the menu is open.
	BP_RefreshMenu(false);
}

UWidget* UHOLGameMenuScreen::NativeGetDesiredFocusTarget() const
{
	return FocusTarget;
}

void UHOLGameMenuScreen::SetMatchMenuInput(APlayerController* PlayerController, const bool bOpening, const UCommonActivatableWidget* Screen)
{
	// V1.4a: the menu closed over the end banner leaves the banner its Escape.
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	const bool bOpen = UHOLGameMenuRulesBridge::KeepsMatchMenuInput(bOpening, Layout && Layout->HasMenuOtherThan(Screen));
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
