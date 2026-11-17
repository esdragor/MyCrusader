#include "Screens/HOLMultiplayerScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformProcess.h"
#include "HeroesOfLibertyUILog.h"
#include "HOLVersion.h"
#include "Misc/CommandLine.h"
#include "UObject/UObjectIterator.h"
#include "Net/HOLManagerClient.h"
#include "Net/HOLLocalPlayer.h"
#include "Net/HOLManagerErrorText.h"
#include "Screens/HOLNewGameScreen.h"
#include "Settings/HOLGameUserSettings.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLMultiplayerScreen"

namespace UE::HeroesOfLiberty::UI::Multiplayer
{
	constexpr int32 MaxNameLength = 16;
	bool IsNameChar(const TCHAR Char)
	{
		return FChar::IsAlnum(Char) || Char == TEXT(' ') || Char == TEXT('-') || Char == TEXT('_');
	}

	/** The account's name, kept to what the manager accepts: the name offered the first time. */
	FString MakeDefaultName()
	{
		FString Name;
		for (const TCHAR Char : FString(FPlatformProcess::UserName()))
		{
			if (IsNameChar(Char) && Name.Len() < MaxNameLength)
			{
				Name.AppendChar(Char);
			}
		}
		return Name.TrimStartAndEnd();
	}
}

UHOLMultiplayerScreen* UHOLMultiplayerScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout)
	{
		return nullptr;
	}
	const TSubclassOf<UHOLMultiplayerScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Multiplayer: no script class (Script/UI/HOLMultiplayerScreen.as)"));
		return nullptr;
	}
	return Cast<UHOLMultiplayerScreen>(Layout->PushMenu(ScriptClass.Get()));
}

TSubclassOf<UHOLMultiplayerScreen> UHOLMultiplayerScreen::FindScriptClass()
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

FText UHOLMultiplayerScreen::ScreenText(const FName Key)
{
	if (Key == TEXT("Title"))
	{
		return LOCTEXT("Title", "Multijoueur");
	}
	if (Key == TEXT("Refresh"))
	{
		return LOCTEXT("Refresh", "Actualiser");
	}
	if (Key == TEXT("Create"))
	{
		return LOCTEXT("Create", "Créer un salon");
	}
	if (Key == TEXT("Back"))
	{
		return LOCTEXT("Back", "Retour");
	}
	return FText::GetEmpty();
}

FHOLLobbyRowView UHOLMultiplayerScreen::MakeLobbyRowView(const int32 Index, const FString& Name, const FString& Map, const FString& Size, const int32 Players,
	const int32 MaxPlayers, const int32 StateValue, const int32 Minutes, const bool bCanJoin, const bool bCanWatch, const TArray<FString>& Away,
	const bool bCanSpectate, const FString& MyName)
{
	FHOLLobbySummary Lobby;
	Lobby.Name = Name;
	Lobby.Map = Map;
	Lobby.Size = Size;
	Lobby.Players = Players;
	Lobby.MaxPlayers = MaxPlayers;
	Lobby.State = static_cast<EHOLLobbyState>(FMath::Clamp(StateValue, 0, 2));
	Lobby.Minutes = Minutes;
	Lobby.bCanJoin = bCanJoin;
	Lobby.bCanWatch = bCanWatch;
	Lobby.Away = Away;
	Lobby.bCanSpectate = bCanSpectate;

	FHOLLobbyRowView View;
	View.Index = Index;
	View.Name = Lobby.Name;
	// TC3: the map and its size, "Plaine — Grande" (nothing from an older manager).
	View.Map = UHOLNewGameScreen::GetLobbyMapName(Lobby.Map, Lobby.Size);
	View.Players = Lobby.Players;
	View.MaxPlayers = Lobby.MaxPlayers;
	View.PlayersText = FText::Format(LOCTEXT("Players", "{0}/{1}"), Lobby.Players, Lobby.MaxPlayers);
	View.State = Lobby.State == EHOLLobbyState::Lobby ? LOCTEXT("StateLobby", "au salon")
		: Lobby.State == EHOLLobbyState::Starting ? LOCTEXT("StateStarting", "lancement")
		: FText::Format(LOCTEXT("StateInGame", "en partie ({0} min)"), FMath::Max(Lobby.Minutes, 0));
	// Join on a lobby still gathering, Rejoin on a match I left (R7.6), Watch on another match under way; nothing
	// otherwise (full, starting).
	const EHOLLobbyAction Action = FHOLManagerClientRules::ChooseLobbyAction(Lobby, MyName);
	if (Action != EHOLLobbyAction::None)
	{
		const TCHAR* ButtonName = Action == EHOLLobbyAction::Join ? TEXT("JoinButton") : Action == EHOLLobbyAction::Rejoin ? TEXT("RejoinButton") : TEXT("WatchButton");
		View.Action = Action == EHOLLobbyAction::Join ? EHOLLobbyRowAction::Join : Action == EHOLLobbyAction::Rejoin ? EHOLLobbyRowAction::Rejoin : EHOLLobbyRowAction::Watch;
		View.ActionName = FName(*FString::Printf(TEXT("%s%d"), ButtonName, Index));
		View.ActionLabel = Action == EHOLLobbyAction::Join ? LOCTEXT("Join", "Rejoindre")
			: Action == EHOLLobbyAction::Rejoin ? LOCTEXT("Rejoin", "Revenir") : LOCTEXT("Watch", "Regarder");
	}
	// SP1 (D642): a lobby gathering also offers « Observer », which takes no slot (the lobby may be full).
	View.bSpectate = FHOLManagerClientRules::CanSpectate(Lobby);
	if (View.bSpectate)
	{
		View.SpectateName = FName(*FString::Printf(TEXT("SpectateButton%d"), Index));
		View.SpectateLabel = LOCTEXT("Spectate", "Observer");
	}
	return View;
}

UVerticalBox* UHOLMultiplayerScreen::MakeMultiplayerColumn()
{
	return MakeCenteredColumn();
}

UCommonTextBlock* UHOLMultiplayerScreen::AddMultiplayerText(UVerticalBox* Column, const FName Name, const FText& Text, const bool bTitle)
{
	if (!Column)
	{
		return nullptr;
	}
	return bTitle ? AddText(Column, Name, Text, UHOLTextStyleTitle::StaticClass(), 32.f)
		: AddText(Column, Name, Text, UHOLTextStyleNote::StaticClass(), 0.f);
}

UHorizontalBox* UHOLMultiplayerScreen::AddMultiplayerRow(UVerticalBox* Column)
{
	return Column ? AddButtonRow(Column) : nullptr;
}

UHOLButton* UHOLMultiplayerScreen::AddMultiplayerButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return Row ? AddButton(Row, Name, Label) : nullptr;
}

void UHOLMultiplayerScreen::AddNameRow(UVerticalBox* Column)
{
	if (!Column)
	{
		return;
	}
	// The name: a label and its box on one line.
	UHorizontalBox* NameRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* NameRowSlot = Column->AddChildToVerticalBox(NameRow);
	NameRowSlot->SetHorizontalAlignment(HAlign_Center);
	NameRowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 24.f));
	UCommonTextBlock* NameLabel = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
	NameLabel->SetStyle(UHOLTextStyleBody::StaticClass());
	NameLabel->SetText(LOCTEXT("NameLabel", "Votre nom"));
	UHorizontalBoxSlot* NameLabelSlot = NameRow->AddChildToHorizontalBox(NameLabel);
	NameLabelSlot->SetVerticalAlignment(VAlign_Center);
	NameLabelSlot->SetPadding(FMargin(0.f, 0.f, 16.f, 0.f));
	USizeBox* NameBoxSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	NameBoxSize->SetWidthOverride(320.f);
	NameBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("NameBox"));
	// Dark text on the box's light background (the engine's default style).
	NameBox->SetForegroundColor(HOLUIStyle::Background);
	NameBox->SetHintText(LOCTEXT("NameHint", "1 à 16 caractères"));
	NameBoxSize->SetContent(NameBox);
	NameRow->AddChildToHorizontalBox(NameBoxSize)->SetVerticalAlignment(VAlign_Center);
}

void UHOLMultiplayerScreen::AddLobbyListBox(UVerticalBox* Column, const float Width, const float MinHeight)
{
	if (!Column)
	{
		return;
	}
	// The list, with a fixed width so that the columns line up.
	USizeBox* ListSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListSize->SetWidthOverride(Width);
	ListSize->SetMinDesiredHeight(MinHeight);
	LobbyList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyList"));
	ListSize->SetContent(LobbyList);
	Column->AddChildToVerticalBox(ListSize)->SetHorizontalAlignment(HAlign_Center);
}

UHorizontalBox* UHOLMultiplayerScreen::AddLobbyRowBox()
{
	if (!LobbyList)
	{
		return nullptr;
	}
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = LobbyList->AddChildToVerticalBox(Row);
	RowSlot->SetHorizontalAlignment(HAlign_Fill);
	RowSlot->SetPadding(FMargin(0.f, 4.f));
	return Row;
}

void UHOLMultiplayerScreen::AddLobbyCell(UHorizontalBox* Row, const FText& Text, const float Width)
{
	if (!Row)
	{
		return;
	}
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(Width);
	UCommonTextBlock* Cell = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
	Cell->SetStyle(UHOLTextStyleBody::StaticClass());
	Cell->SetText(Text);
	Box->SetContent(Cell);
	UHorizontalBoxSlot* CellSlot = Row->AddChildToHorizontalBox(Box);
	CellSlot->SetVerticalAlignment(VAlign_Center);
	CellSlot->SetPadding(FMargin(8.f, 0.f));
}

UHOLButton* UHOLMultiplayerScreen::AddLobbyButton(UHorizontalBox* Row, const FName Name, const FText& Label, const int32 Index, const EHOLLobbyRowAction Action,
	const bool bSpectate, const float Width)
{
	if (!Row)
	{
		return nullptr;
	}
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(Width);
	UHOLButton* Button = WidgetTree->ConstructWidget<UHOLButton>(UHOLButton::StaticClass(), Name);
	Button->SetLabel(Label);
	if (bSpectate)
	{
		Button->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::HandleSpectate, Index);
	}
	else if (Action == EHOLLobbyRowAction::Join)
	{
		Button->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::HandleJoin, Index);
	}
	else if (Action == EHOLLobbyRowAction::Rejoin)
	{
		Button->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::HandleRejoin, Index);
	}
	else if (Action == EHOLLobbyRowAction::Watch)
	{
		Button->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::HandleWatch, Index);
	}
	Box->SetContent(Button);
	Row->AddChildToHorizontalBox(Box)->SetVerticalAlignment(VAlign_Center);
	return Button;
}

bool UHOLMultiplayerScreen::IsValidPlayerName(const FString& Name)
{
	using namespace UE::HeroesOfLiberty::UI::Multiplayer;
	const FString Trimmed = Name.TrimStartAndEnd();
	if (Trimmed.IsEmpty() || Trimmed.Len() > MaxNameLength)
	{
		return false;
	}
	for (const TCHAR Char : Trimmed)
	{
		if (!IsNameChar(Char))
		{
			return false;
		}
	}
	return true;
}

void UHOLMultiplayerScreen::BuildLayout()
{
	BP_BuildMultiplayer();
}

void UHOLMultiplayerScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (NameBox)
	{
		const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
		FString Saved = Settings ? Settings->GetPlayerName() : FString();
		// -PlayerName=<name> (scripted checks: two players on one PC) goes over the saved one.
		FParse::Value(FCommandLine::Get(), TEXT("-PlayerName="), Saved);
		NameBox->SetText(FText::FromString(IsValidPlayerName(Saved) ? Saved : UE::HeroesOfLiberty::UI::Multiplayer::MakeDefaultName()));
	}
}

void UHOLMultiplayerScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	const UGameInstance* GameInstance = GetGameInstance();
	if (UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr)
	{
		LobbyLoss = Client->TakeLobbyLoss();
	}
	RefreshLobbies();
}

void UHOLMultiplayerScreen::NativeOnDeactivated()
{
	// A valid name is kept even if the player leaves without using it.
	if (NameBox && IsValidPlayerName(NameBox->GetText().ToString()))
	{
		TakePlayerName();
	}
	Super::NativeOnDeactivated();
}

UWidget* UHOLMultiplayerScreen::NativeGetDesiredFocusTarget() const
{
	return RefreshButton;
}

void UHOLMultiplayerScreen::ShowLobbyLoss(const FHOLManagerError& Error)
{
	LobbyLoss = Error;
	RefreshLobbies();
}

void UHOLMultiplayerScreen::RefreshLobbies()
{
	const UGameInstance* GameInstance = GetGameInstance();
	UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Multiplayer: no manager client"));
		FHOLManagerError Error;
		Error.Kind = FHOLManagerError::EKind::Unreachable;
		ShowLobbies({}, Error);
		return;
	}
	SetStatus(LOCTEXT("Searching", "Recherche des salons…"));
	const int32 ThisRefresh = ++RefreshCount;
	const TWeakObjectPtr<UHOLMultiplayerScreen> WeakThis(this);
	// The status first: a manager serving another version shows why and no list.
	Client->GetStatus(FHOLManagerStatusDone::CreateLambda([WeakThis, ThisRefresh](const FHOLManagerStatus& Status, const FHOLManagerError& StatusError)
	{
		if (!WeakThis.IsValid() || WeakThis->RefreshCount != ThisRefresh)
		{
			return;
		}
		const UGameInstance* GameInstance = WeakThis->GetGameInstance();
		UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
		if (StatusError.IsError() || !Client)
		{
			WeakThis->ShowLobbies({}, StatusError);
			return;
		}
		if (!UHOLManagerClient::IsSameVersion(Status))
		{
			WeakThis->ShowLobbies({}, MakeVersionMismatchError(Status.GameVersion, HOLVersion::Get()));
			return;
		}
		Client->GetLobbies(FHOLManagerLobbiesDone::CreateLambda([WeakThis, ThisRefresh](const TArray<FHOLLobbySummary>& Found, const FHOLManagerError& Error)
		{
			if (WeakThis.IsValid() && WeakThis->RefreshCount == ThisRefresh)
			{
				WeakThis->ShowLobbies(Found, Error);
			}
		}));
	}));
}

void UHOLMultiplayerScreen::ShowLobbies(const TArray<FHOLLobbySummary>& InLobbies, const FHOLManagerError& Error)
{
	Lobbies = Error.IsError() ? TArray<FHOLLobbySummary>() : InLobbies;
	if (LobbyList)
	{
		LobbyList->ClearChildren();
	}
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(CanCreateLobbyAfter(Error));
	}
	if (Error.IsError())
	{
		const FText Text = GetManagerErrorText(Error);
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: error %s (HTTP %d %s: %s)"), *Text.ToString(), Error.HttpCode, *Error.Code, *Error.Message);
		SetStatus(Text, true);
		return;
	}
	const FString MyName = NameBox ? NameBox->GetText().ToString() : FString();
	for (int32 Index = 0; Index < Lobbies.Num(); ++Index)
	{
		const FHOLLobbySummary& Lobby = Lobbies[Index];
		const FHOLLobbyRowView View = MakeLobbyRowView(Index, Lobby.Name, Lobby.Map, Lobby.Size, Lobby.Players, Lobby.MaxPlayers, static_cast<int32>(Lobby.State),
			Lobby.Minutes, Lobby.bCanJoin, Lobby.bCanWatch, Lobby.Away, Lobby.bCanSpectate, MyName);
		BP_AddLobbyRow(View);
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: lobby %s \"%s\" %s %d/%d %s%s%s"), *Lobby.Id, *Lobby.Name, *View.Map.ToString(), Lobby.Players,
			Lobby.MaxPlayers, *View.State.ToString(), View.Action == EHOLLobbyRowAction::Join ? TEXT(", join") : View.Action == EHOLLobbyRowAction::Rejoin ? TEXT(", rejoin")
			: View.Action == EHOLLobbyRowAction::Watch ? TEXT(", watch") : TEXT(""), View.bSpectate ? TEXT(", spectate") : TEXT(""));
	}
	SetStatus(Lobbies.IsEmpty() ? LOCTEXT("NoLobby", "0 salon") : FText::GetEmpty());
	UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: %d lobbies shown"), Lobbies.Num());
	// Back from a lobby lost (manager gone, removed): why, once, over the list.
	if (LobbyLoss.IsError())
	{
		const FText Text = GetManagerErrorText(LobbyLoss);
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: lobby lost: %s (%s: %s)"), *Text.ToString(), *LobbyLoss.Code, *LobbyLoss.Message);
		SetStatus(Text, true);
		LobbyLoss = FHOLManagerError();
	}
}

void UHOLMultiplayerScreen::SetStatus(const FText& Text, const bool bAlert)
{
	if (StatusText)
	{
		StatusText->SetText(Text);
		// An error in body size and the game's alert red (a missing cost), a note otherwise.
		StatusText->SetStyle(bAlert ? TSubclassOf<UCommonTextStyle>(UHOLTextStyleBody::StaticClass()) : TSubclassOf<UCommonTextStyle>(UHOLTextStyleNote::StaticClass()));
		if (bAlert)
		{
			StatusText->SetColorAndOpacity(HOLUIStyle::Missing);
		}
	}
}

FString UHOLMultiplayerScreen::TakePlayerName()
{
	const FString Name = NameBox ? NameBox->GetText().ToString().TrimStartAndEnd() : FString();
	if (!IsValidPlayerName(Name))
	{
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: name '%s' refused"), *Name);
		SetStatus(LOCTEXT("BadName", "Votre nom : 1 à 16 lettres, chiffres, espaces, - ou _"), true);
		return FString();
	}
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get(); Settings && Settings->GetPlayerName() != Name)
	{
		Settings->SetPlayerName(Name);
		Settings->SaveSettings();
	}
	// The lobby name is the login name too, over -PlayerName= (2026-10-29: renamed here, the player joined as a spectator).
	if (UHOLLocalPlayer* LocalPlayer = Cast<UHOLLocalPlayer>(GetOwningLocalPlayer()))
	{
		LocalPlayer->SetChosenName(Name);
	}
	return Name;
}

void UHOLMultiplayerScreen::CreateLobby()
{
	const FString Name = TakePlayerName();
	const UGameInstance* GameInstance = GetGameInstance();
	UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Name.IsEmpty() && Client)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: create a lobby as %s"), *Name);
		SetStatus(LOCTEXT("Entering", "Entrée dans le salon…"));
		// The lobby's name is the player's for now.
		Client->CreateLobby(FText::Format(LOCTEXT("LobbyOf", "Salon de {0}"), FText::FromString(Name)).ToString(), Name,
			FHOLManagerLobbyDone::CreateUObject(this, &UHOLMultiplayerScreen::HandleEntered));
	}
}

void UHOLMultiplayerScreen::HandleJoin(const int32 Index)
{
	EnterAs(Index, false);
}

void UHOLMultiplayerScreen::HandleSpectate(const int32 Index)
{
	EnterAs(Index, true);
}

void UHOLMultiplayerScreen::EnterAs(const int32 Index, const bool bSpectator)
{
	const FString Name = TakePlayerName();
	const UGameInstance* GameInstance = GetGameInstance();
	UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Name.IsEmpty() && Lobbies.IsValidIndex(Index) && Client)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: join %s as %s%s"), *Lobbies[Index].Id, *Name, bSpectator ? TEXT(" (spectator)") : TEXT(""));
		SetStatus(LOCTEXT("Entering", "Entrée dans le salon…"));
		Client->JoinLobby(Lobbies[Index].Id, Name, FHOLManagerLobbyDone::CreateUObject(this, &UHOLMultiplayerScreen::HandleEntered), bSpectator);
	}
}

void UHOLMultiplayerScreen::HandleEntered(const FHOLLobby& Lobby, const FHOLManagerError& Error)
{
	if (Error.IsError())
	{
		const FText Text = GetManagerErrorText(Error);
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: error %s (HTTP %d %s: %s)"), *Text.ToString(), Error.HttpCode, *Error.Code, *Error.Message);
		SetStatus(Text, true);
		return;
	}
	SetStatus(FText::GetEmpty());
	UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: in lobby %s as %s"), *Lobby.Id, *Lobby.You);
	// The lobby is the New game screen in its network mode (M6-Q3), over this one.
	if (UHOLNewGameScreen* Screen = UHOLNewGameScreen::Open(GetOwningPlayer()))
	{
		Screen->EnterLobby(Lobby);
	}
}

void UHOLMultiplayerScreen::HandleWatch(const int32 Index)
{
	OpenMatchOf(Index, false);
}

void UHOLMultiplayerScreen::HandleRejoin(const int32 Index)
{
	OpenMatchOf(Index, true);
}

void UHOLMultiplayerScreen::OpenMatchOf(const int32 Index, const bool bRejoin)
{
	const FString Name = TakePlayerName();
	if (!Name.IsEmpty() && Lobbies.IsValidIndex(Index))
	{
		const FString Id = Lobbies[Index].Id;
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: %s %s as %s"), bRejoin ? TEXT("rejoin") : TEXT("watch"), *Id, *Name);
		const UGameInstance* GameInstance = GetGameInstance();
		UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
		if (!Client)
		{
			return;
		}
		// The match's port from GET /watch. A spectator: no Name in the address (M6-Q9); back in the match (R7.6): his
		// name, which gives his slot back within R7's window (as R4.7).
		Client->WatchLobby(Id, FHOLManagerWatchDone::CreateWeakLambda(this, [this, Id, Name, bRejoin](const FString& MatchId, const int32 Port, const FHOLManagerError& Error)
		{
			const UGameInstance* Owner = GetGameInstance();
			UHOLManagerClient* Manager = Owner ? Owner->GetSubsystem<UHOLManagerClient>() : nullptr;
			if (Error.IsError() || !Manager)
			{
				UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: %s %s refused (HTTP %d %s: %s)"), bRejoin ? TEXT("rejoin") : TEXT("watch"), *Id, Error.HttpCode, *Error.Code, *Error.Message);
				SetStatus(GetManagerErrorText(Error), true);
				return;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: %s match %s of %s on port %d"), bRejoin ? TEXT("back in") : TEXT("watching"), *MatchId, *Id, Port);
			// Back in: the lobby kept without a token, so the end banner offers « Retour au salon » (B6c).
			if (bRejoin)
			{
				Manager->KeepRejoinedLobby(Id);
			}
			if (!Manager->OpenMatch(Port, bRejoin ? Name : FString(), HOLVersion::Get()) && bRejoin)
			{
				Manager->ForgetLobby();
			}
		}));
	}
}

#undef LOCTEXT_NAMESPACE
