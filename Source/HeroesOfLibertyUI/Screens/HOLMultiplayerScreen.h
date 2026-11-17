#pragma once

#include "CoreMinimal.h"
#include "Net/HOLManagerClientRules.h"
#include "Screens/HOLScreen.h"
#include "HOLMultiplayerScreen.generated.h"

class UCommonTextBlock;
class UEditableTextBox;
class UHorizontalBox;
class UHOLButton;
class UVerticalBox;

/** The button a lobby row offers (EHOLLobbyAction, a plain C++ enum, as the script reads it). */
UENUM(BlueprintType)
enum class EHOLLobbyRowAction : uint8
{
	None,
	Join,
	Watch,
	Rejoin,
};

/**
 * What the script shows of one lobby of the list: the texts and buttons FHOLManagerClientRules chose (ChooseLobbyAction, CanSpectate) and the
 * LOCTEXT labels, made in C++ by UHOLMultiplayerScreen::MakeLobbyRowView. The script lays the cells out and calls AddLobbyButton.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLLobbyRowView
{
	GENERATED_BODY()

	/** The lobby's index in the list (the buttons' names end with it, the clicks go back to it). */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	int32 Index = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FString Name;

	/** TC3: "Plaine - Grande" (empty from an older manager). */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FText Map;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	int32 Players = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	int32 MaxPlayers = 0;

	/** "{0}/{1}" of Players and MaxPlayers. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FText PlayersText;

	/** "au salon", "lancement" or "en partie (N min)". */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FText State;

	/** Join on a lobby gathering, Rejoin on a match I left (R7.6), Watch on another match under way, None otherwise. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	EHOLLobbyRowAction Action = EHOLLobbyRowAction::None;

	/** "JoinButton<i>", "RejoinButton<i>" or "WatchButton<i>" (the names the scenarios click); empty for None. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FName ActionName;

	/** "Rejoindre", "Revenir" or "Regarder". */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FText ActionLabel;

	/** SP1 (D642): the lobby also offers "Observer" ("SpectateButton<i>"), which takes no slot. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	bool bSpectate = false;

	/** "SpectateButton<i>" (empty without bSpectate). */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FName SpectateName;

	/** "Observer". */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	FText SpectateLabel;
};

/**
 * Multiplayer screen (M6 R4.2): the player's name (saved, 1 to 16 characters as the manager wants them), the lobbies of
 * the lobby manager (UHOLManagerClient) with Join on a lobby still gathering, Watch on a match under way (Rejoin on the
 * one I left, R7.6), Refresh,
 * Create a lobby, Back. Create and Join enter the lobby (R4.4) and open it: the New game screen in its network mode
 * (UHOLNewGameScreen::EnterLobby). Watch only logs its intention for now.
 *
 * Its layout and the rows of its list are written in script (a class deriving from this one, FindScriptClass:
 * Script/UI/HOLMultiplayerScreen.as, MIG-CPP bascule Multiplayer, D840/D839, as UHOLEndBannerScreen); this base keeps what the
 * script cannot reach: the opening (3 callers), the name rule, the manager client and its asynchronous answers, the status
 * texts (LOCTEXT), what a click does (enter a lobby, watch, rejoin) and the New game screen. No network logic changed.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLMultiplayerScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the menu of the given player (Multiplayer button, hol.UI.Open Multiplayer); null without a script class. */
	static UHOLMultiplayerScreen* Open(APlayerController* PlayerController);

	/** The script's screen (the newest class deriving from this one, as UHOLEndBannerScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|Multiplayer")
	static TSubclassOf<UHOLMultiplayerScreen> FindScriptClass();

	/** A name the manager accepts once trimmed (Manager/PROTOCOL.md, 8): 1 to 16 letters, digits, spaces, - or _. */
	UFUNCTION(BlueprintPure, Category = "HOL|Multiplayer")
	static bool IsValidPlayerName(const FString& Name);

	/** A fixed text of the screen (the LOCTEXTs stay here): Title, Refresh, Create, Back. */
	UFUNCTION(BlueprintPure, Category = "HOL|Multiplayer")
	static FText ScreenText(FName Key);

	/**
	 * The view of a lobby row for the lobby described by the arguments (the same chain as a list answer): ChooseLobbyAction and CanSpectate,
	 * then the texts. StateValue is EHOLLobbyState (0 Lobby, 1 Starting, 2 InGame). Used by ShowLobbies and by the script's test.
	 */
	UFUNCTION(BlueprintPure, Category = "HOL|Multiplayer")
	static FHOLLobbyRowView MakeLobbyRowView(int32 Index, const FString& Name, const FString& Map, const FString& Size, int32 Players, int32 MaxPlayers,
		int32 StateValue, int32 Minutes, bool bCanJoin, bool bCanWatch, const TArray<FString>& Away, bool bCanSpectate, const FString& MyName);

	/** Asks the manager for its lobbies again (Refresh button; done when the screen opens). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void RefreshLobbies();
	/** Why a lobby was just lost (R4.9, back to the lobby refused): shown over the list once refreshed. */
	void ShowLobbyLoss(const FHOLManagerError& Error);

	/**
	 * UHOLScreen::MakeCenteredColumn for the script. bVeil (D843, Q2 B): a dark veil (VeilColor) fills the screen behind the column, so the status
	 * line reads over the scene (this screen only; the other menus keep no background).
	 */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UVerticalBox* MakeMultiplayerColumn(bool bVeil);

	/** The colour of that veil: HOLUIStyle::Veil (black, 0.6 alpha), what MakeMultiplayerColumn(true) lays; read by the script's test. */
	UFUNCTION(BlueprintPure, Category = "HOL|Multiplayer")
	static FLinearColor VeilColor();

	/** A line of the column: the title (title style, 32) or the status/note (note style, no padding), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UCommonTextBlock* AddMultiplayerText(UVerticalBox* Column, FName Name, const FText& Text, bool bTitle);

	/** The name line (label and box, NameBox): sets NameBox, which the base fills on opening. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void AddNameRow(UVerticalBox* Column);

	/** The list (a box of the columns' width, LobbyList) in the column; sets LobbyList. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void AddLobbyListBox(UVerticalBox* Column, float Width, float MinHeight);

	/** UHOLScreen::AddButtonRow for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UHorizontalBox* AddMultiplayerRow(UVerticalBox* Column);

	/** UHOLScreen::AddButton for the script; its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UHOLButton* AddMultiplayerButton(UHorizontalBox* Row, FName Name, const FText& Label);

	/** A new line (a horizontal box) at the end of the list. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UHorizontalBox* AddLobbyRowBox();

	/** A cell of a list line: the text in a box of this width (body style, centered vertically, 8 of padding). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void AddLobbyCell(UHorizontalBox* Row, const FText& Text, float Width);

	/** A button of a list line in a box of this width: Action is the row's EHOLLobbyRowAction, or Spectate=true for "Observer"; its click does the action on Index. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	UHOLButton* AddLobbyButton(UHorizontalBox* Row, FName Name, const FText& Label, int32 Index, EHOLLobbyRowAction Action, bool bSpectate, float Width);

	/** "Créer un salon": takes the name, then creates the lobby (the status says why not). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void CreateLobby();

	/** Back: closes the screen. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Multiplayer")
	void CloseScreen() { Close(); }

	/** "0 salon", or why the manager failed (GetManagerErrorText, in red); empty when lobbies are shown. Set by the script. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UCommonTextBlock> StatusText;

	/** The title, set by the script. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UCommonTextBlock> Title;

	/** One row per lobby (name, players, state, JoinButton<i>, RejoinButton<i> or WatchButton<i>); set by AddLobbyListBox. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UVerticalBox> LobbyList;

	/** The name box, set by AddNameRow. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UEditableTextBox> NameBox;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UHOLButton> RefreshButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UHOLButton> CreateButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Multiplayer")
	TObjectPtr<UHOLButton> BackButton;

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** The script builds the column (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Multiplayer")
	void BP_BuildMultiplayer();

	/** The script adds the line of one lobby to the list (cells, buttons) after the base cleared it. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Multiplayer")
	void BP_AddLobbyRow(const FHOLLobbyRowView& Row);

private:
	void ShowLobbies(const TArray<FHOLLobbySummary>& InLobbies, const FHOLManagerError& Error);
	/** The status line; bAlert for an error (the alert red). */
	void SetStatus(const FText& Text, bool bAlert = false);

	/** The trimmed name, saved if valid; empty (and the status says why) if not. */
	FString TakePlayerName();

	void HandleJoin(int32 Index);
	/** SP1 (D642): enters the lobby as a spectator (no slot), even full. */
	void HandleSpectate(int32 Index);
	void EnterAs(int32 Index, bool bSpectator);
	void HandleWatch(int32 Index);
	/** R7.6: back in the match I left (my name among its players away): as Watch, but with my name. */
	void HandleRejoin(int32 Index);
	/** Watch (no name, a spectator) or Rejoin (my name, my slot): GET /watch for the port, then the match. */
	void OpenMatchOf(int32 Index, bool bRejoin);
	/** Create or Join answered: the lobby opened, or why not in the status line. */
	void HandleEntered(const FHOLLobby& Lobby, const FHOLManagerError& Error);

	TArray<FHOLLobbySummary> Lobbies;
	/** Why the last lobby was lost, shown over the next list. */
	FHOLManagerError LobbyLoss;
	/** Answers of an earlier refresh are dropped. */
	int32 RefreshCount = 0;
};
