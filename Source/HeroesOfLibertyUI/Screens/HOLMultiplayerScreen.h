#pragma once

#include "CoreMinimal.h"
#include "Net/HOLManagerClientRules.h"
#include "Screens/HOLScreen.h"
#include "HOLMultiplayerScreen.generated.h"

class UEditableTextBox;

/**
 * Multiplayer screen (M6 R4.2): the player's name (saved, 1 to 16 characters as the manager wants them), the lobbies of
 * the lobby manager (UHOLManagerClient) with Join on a lobby still gathering, Watch on a match under way (Rejoin on the
 * one I left, R7.6), Refresh,
 * Create a lobby, Back. Create and Join enter the lobby (R4.4) and open it: the New game screen in its network mode
 * (UHOLNewGameScreen::EnterLobby). Watch only logs its intention for now.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLMultiplayerScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the menu of the given player (Multiplayer button, hol.UI.Open Multiplayer). */
	static UHOLMultiplayerScreen* Open(APlayerController* PlayerController);

	/** A name the manager accepts once trimmed (Manager/PROTOCOL.md, 8): 1 to 16 letters, digits, spaces, - or _. */
	static bool IsValidPlayerName(const FString& Name);

	/** Asks the manager for its lobbies again (Refresh button; done when the screen opens). */
	void RefreshLobbies();
	/** Why a lobby was just lost (R4.9, back to the lobby refused): shown over the list once refreshed. */
	void ShowLobbyLoss(const FHOLManagerError& Error);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> NameBox;

	/** "0 salon", or why the manager failed (GetManagerErrorText, in red); empty when lobbies are shown. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StatusText;

	/** One row per lobby (name, players, state, JoinButton<i>, RejoinButton<i> or WatchButton<i>). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UVerticalBox> LobbyList;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> RefreshButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> CreateButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

private:
	void ShowLobbies(const TArray<FHOLLobbySummary>& InLobbies, const FHOLManagerError& Error);
	void AddLobbyRow(int32 Index, const FHOLLobbySummary& Lobby);
	/** The status line; bAlert for an error (the alert red). */
	void SetStatus(const FText& Text, bool bAlert = false);

	/** The trimmed name, saved if valid; empty (and the status says why) if not. */
	FString TakePlayerName();

	void HandleCreate();
	void HandleJoin(int32 Index);
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
