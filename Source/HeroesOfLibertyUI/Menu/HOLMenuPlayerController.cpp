#include "Menu/HOLMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/HOLUIInputData.h"
#include "InputMappingContext.h"
#include "Net/HOLManagerClient.h"
#include "Net/HOLManagerErrorText.h"
#include "Screens/HOLMultiplayerScreen.h"
#include "Screens/HOLNewGameScreen.h"
#include "Screens/HOLTitleScreen.h"
#include "Settings/HOLGameUserSettings.h"
#include "Widgets/HOLRootLayout.h"

AHOLMenuPlayerController::AHOLMenuPlayerController()
{
	bShowMouseCursor = true;
}

void AHOLMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* Context = UHOLUIInputData::GetMappingContext())
		{
			Input->AddMappingContext(Context, 0);
		}
		else
		{
			UE_LOG(LogHOLUI, Error, TEXT("Interface keys missing (/Game/UI/Input/IMC_UI): run Tools/Editor/make_ui_input.py"));
		}
	}

	// The saved volumes (D245), once the audio device is up.
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		Settings->ApplySound();
	}

	RootLayout = CreateWidget<UHOLRootLayout>(this, UHOLRootLayout::StaticClass());
	RootLayout->AddToViewport();
	RootLayout->PushMenu(UHOLTitleScreen::StaticClass());

	// Back from a lobby's match (R4.8): the menu's map is up, so the Multiplayer screen, then the lobby joined again.
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	const UHOLManagerClient::EAfterMatch AfterMatch = Client ? Client->TakeAfterMatch() : UHOLManagerClient::EAfterMatch::None;
	UHOLMultiplayerScreen* List = AfterMatch != UHOLManagerClient::EAfterMatch::None ? UHOLMultiplayerScreen::Open(this) : nullptr;
	if (!List)
	{
		return;
	}
	if (AfterMatch == UHOLManagerClient::EAfterMatch::Lobby)
	{
		TWeakObjectPtr<UHOLMultiplayerScreen> WeakList = List;
		Client->ReturnToLobby(FHOLManagerLobbyDone::CreateWeakLambda(this, [this, Client, WeakList](const FHOLLobby& Lobby, const FHOLManagerError& Error)
		{
			if (Error.IsError())
			{
				// Name taken, lobby full, gone or relaunched (R4.9, M6-Q14): the lobby forgotten, why shown over the list.
				UE_LOG(LogHOLUI, Warning, TEXT("Menu: back to lobby refused (HTTP %d %s: %s)"), Error.HttpCode, *Error.Code, *Error.Message);
				Client->ForgetLobby();
				if (WeakList.IsValid())
				{
					WeakList->ShowLobbyLoss(MakeReturnToLobbyError(Error));
				}
				return;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Menu: back in lobby %s as %s"), *Lobby.Id, *Lobby.You);
			if (UHOLNewGameScreen* Screen = UHOLNewGameScreen::Open(this))
			{
				Screen->EnterLobby(Lobby);
			}
		}));
	}
}
