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
#include "Net/HOLManagerClient.h"
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
	constexpr float NameWidth = 340.f;
	constexpr float PlayersWidth = 90.f;
	constexpr float StateWidth = 220.f;
	constexpr float ActionWidth = 200.f;

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
	return Layout ? Cast<UHOLMultiplayerScreen>(Layout->PushMenu(StaticClass())) : nullptr;
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
	using namespace UE::HeroesOfLiberty::UI::Multiplayer;

	UVerticalBox* Column = MakeCenteredColumn();
	Title = AddText(Column, TEXT("Title"), LOCTEXT("Title", "Multijoueur"), UHOLTextStyleTitle::StaticClass(), 32.f);

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

	// The list, with a fixed width so that the columns line up.
	USizeBox* ListSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ListSize->SetWidthOverride(NameWidth + PlayersWidth + StateWidth + ActionWidth + 32.f);
	ListSize->SetMinDesiredHeight(240.f);
	LobbyList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyList"));
	ListSize->SetContent(LobbyList);
	Column->AddChildToVerticalBox(ListSize)->SetHorizontalAlignment(HAlign_Center);

	StatusText = AddText(Column, TEXT("StatusText"), FText::GetEmpty(), UHOLTextStyleNote::StaticClass(), 0.f);

	UHorizontalBox* Buttons = AddButtonRow(Column);
	RefreshButton = AddButton(Buttons, TEXT("RefreshButton"), LOCTEXT("Refresh", "Actualiser"));
	CreateButton = AddButton(Buttons, TEXT("CreateButton"), LOCTEXT("Create", "Créer un salon"));
	BackButton = AddButton(Buttons, TEXT("BackButton"), LOCTEXT("Back", "Retour"));
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
	if (RefreshButton)
	{
		RefreshButton->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::RefreshLobbies);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::HandleCreate);
	}
	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &UHOLMultiplayerScreen::Close);
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
	for (int32 Index = 0; Index < Lobbies.Num(); ++Index)
	{
		AddLobbyRow(Index, Lobbies[Index]);
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

void UHOLMultiplayerScreen::AddLobbyRow(const int32 Index, const FHOLLobbySummary& Lobby)
{
	using namespace UE::HeroesOfLiberty::UI::Multiplayer;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = LobbyList->AddChildToVerticalBox(Row);
	RowSlot->SetHorizontalAlignment(HAlign_Fill);
	RowSlot->SetPadding(FMargin(0.f, 4.f));

	auto AddCell = [this, Row](const FText& Text, const float Width)
	{
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(Width);
		UCommonTextBlock* Cell = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
		Cell->SetStyle(UHOLTextStyleBody::StaticClass());
		Cell->SetText(Text);
		Box->SetContent(Cell);
		UHorizontalBoxSlot* CellSlot = Row->AddChildToHorizontalBox(Box);
		CellSlot->SetVerticalAlignment(VAlign_Center);
		CellSlot->SetPadding(FMargin(8.f, 0.f));
	};
	AddCell(FText::FromString(Lobby.Name), NameWidth);
	AddCell(FText::Format(LOCTEXT("Players", "{0}/{1}"), Lobby.Players, Lobby.MaxPlayers), PlayersWidth);
	const FText State = Lobby.State == EHOLLobbyState::Lobby ? LOCTEXT("StateLobby", "au salon")
		: Lobby.State == EHOLLobbyState::Starting ? LOCTEXT("StateStarting", "lancement")
		: FText::Format(LOCTEXT("StateInGame", "en partie ({0} min)"), FMath::Max(Lobby.Minutes, 0));
	AddCell(State, StateWidth);

	// Join on a lobby still gathering, Watch on a match under way; nothing otherwise (full, starting).
	const bool bJoin = Lobby.State == EHOLLobbyState::Lobby && Lobby.bCanJoin;
	const bool bWatch = !bJoin && Lobby.State == EHOLLobbyState::InGame && Lobby.bCanWatch;
	if (bJoin || bWatch)
	{
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(ActionWidth);
		UHOLButton* Button = WidgetTree->ConstructWidget<UHOLButton>(UHOLButton::StaticClass(),
			*FString::Printf(TEXT("%s%d"), bJoin ? TEXT("JoinButton") : TEXT("WatchButton"), Index));
		Button->SetLabel(bJoin ? LOCTEXT("Join", "Rejoindre") : LOCTEXT("Watch", "Regarder"));
		Button->OnClicked().AddUObject(this, bJoin ? &UHOLMultiplayerScreen::HandleJoin : &UHOLMultiplayerScreen::HandleWatch, Index);
		Box->SetContent(Button);
		Row->AddChildToHorizontalBox(Box)->SetVerticalAlignment(VAlign_Center);
	}
	UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: lobby %s \"%s\" %d/%d %s%s"), *Lobby.Id, *Lobby.Name, Lobby.Players, Lobby.MaxPlayers,
		*State.ToString(), bJoin ? TEXT(", join") : bWatch ? TEXT(", watch") : TEXT(""));
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
	return Name;
}

void UHOLMultiplayerScreen::HandleCreate()
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
	const FString Name = TakePlayerName();
	const UGameInstance* GameInstance = GetGameInstance();
	UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Name.IsEmpty() && Lobbies.IsValidIndex(Index) && Client)
	{
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: join %s as %s"), *Lobbies[Index].Id, *Name);
		SetStatus(LOCTEXT("Entering", "Entrée dans le salon…"));
		Client->JoinLobby(Lobbies[Index].Id, Name, FHOLManagerLobbyDone::CreateUObject(this, &UHOLMultiplayerScreen::HandleEntered));
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
	const FString Name = TakePlayerName();
	if (!Name.IsEmpty() && Lobbies.IsValidIndex(Index))
	{
		const FString Id = Lobbies[Index].Id;
		UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: watch %s as %s"), *Id, *Name);
		const UGameInstance* GameInstance = GetGameInstance();
		UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
		if (!Client)
		{
			return;
		}
		// A spectator: no Name in the address (M6-Q9).
		Client->WatchLobby(Id, FHOLManagerWatchDone::CreateWeakLambda(this, [this, Id](const FString& MatchId, const int32 Port, const FHOLManagerError& Error)
		{
			const UGameInstance* Owner = GetGameInstance();
			UHOLManagerClient* Manager = Owner ? Owner->GetSubsystem<UHOLManagerClient>() : nullptr;
			if (Error.IsError() || !Manager)
			{
				UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: watch %s refused (HTTP %d %s: %s)"), *Id, Error.HttpCode, *Error.Code, *Error.Message);
				SetStatus(GetManagerErrorText(Error), true);
				return;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Multiplayer: watching match %s of %s on port %d"), *MatchId, *Id, Port);
			Manager->OpenMatch(Port, FString(), HOLVersion::Get());
		}));
	}
}

#undef LOCTEXT_NAMESPACE
