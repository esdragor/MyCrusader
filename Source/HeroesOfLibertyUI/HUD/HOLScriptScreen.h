#pragma once

#include "CoreMinimal.h"
#include "Player/HOLCommandGridReadLibrary.h"

class UHorizontalBox;
class UHOLGridCell;
#include "Blueprint/UserWidget.h"
#include "Match/HOLForumMessagesReadLibrary.h"
#include "Match/HOLResources.h"
#include "HOLScriptScreen.generated.h"

class UBorder;
class UButton;
class UProgressBar;
class UCommonTextBlock;
class UCommonTextStyle;
class UWidget;

/** A HUD box and its name (Slate units of the HUD), as UHOLScriptScreen::GatherPanelBoxes lists them. */
struct FHOLHUDNamedBox
{
	FString Name;
	FBox2D Box;
};

/**
 * The game screen's part written in Angelscript (J8a.5 loop 2): the HUD's blocks move to script one by one, the hero's
 * block first (Script/UI/HOLGameScreenScript.as). UHOLGameScreen makes the one script class derived from this one
 * (FindScriptClass) and lays it over its whole frame; like the frame, it lets the clicks through to the game (only its
 * panels, MakePanel, take them). The script builds its tree in OnInitialized (ConstructWidget, SetRootWidget: the names
 * are found by hol.HUD.Click), refreshes it in Refresh (RefreshSeconds) and says what it shows in Describe.
 */
UCLASS(Abstract)
class HEROESOFLIBERTYUI_API UHOLScriptScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * What the script's blocks show, one line per block "<block>: <what>" ("hero: alive level 1 hp 450/450"), lines
	 * separated by "\n": hol.HUD.Report writes each one as "HUD <line>". Empty: none.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UI")
	FString Describe() const;

	/** A click on the widget named Name by the mouse's path (hol.HUD.Click); false: not the script's, hidden or greyed. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UI")
	bool ClickWidget(FName Name);

	/** Called every RefreshSeconds while shown, and once at its construction: the blocks read the game and redraw. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void Refresh();

	/** The builders' age panels read the game and redraw at once (HUD H-8): the screen's grid, chosen age or tier changed. Not the whole Refresh. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void RefreshAges();

	/**
	 * A key the script screen listens to went down this frame, read from the player's input state (nothing bound nor consumed, as the Enter
	 * of a test scenario): "T" opens the talent tree (TQ4 = A). Called from the tick, at most once a press.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void KeyPressed(FName KeyName);

	/**
	 * The player selected one finished building alone (a click, not a unit, a source or a group): TypeName as BuildingTypes.csv writes it
	 * ("House"). The script rings the selection sound of its family (Audio B5, HOLAudioBuildings::SelectEventFor).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void BuildingSelected(const FString& TypeName);

	/**
	 * Refresh at once, the timer started again: hol.HUD.Report calls it before Describe, the screen then showing what the
	 * game holds now (the tests' waits draw no frame, so tick no widget).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void RefreshNow();

	/** The local player's slot, INDEX_NONE (-1) for a spectator or without a player state yet. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetLocalSlot() const;

	/**
	 * The slot of the player the C++ screen shows (its bound player state, what its players' table was read for): INDEX_NONE for a
	 * spectator or before the player state arrives. HUD H-4: the script's players table reads its rows for this slot.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetShownSlot() const;

	/** The C++ screen's PlayersPanel (the right column's table host, kept in C++ with the thread of messages); the script fills it (HUD H-4). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UBorder* GetPlayersPanel() const;

	/** The C++ screen's GroupsPanel (the control groups bar's host, in its column, margin and gap, collapsed); the script fills it (HUD H-6). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UBorder* GetGroupsPanel() const;

	/** AHOLPlayerController::NumControlGroups (10). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static int32 GetNumControlGroups();

	/** The owning controller AND the world exist (the C++ bar did nothing before them). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool GetControlGroupsAvailable() const;

	/**
	 * AHOLPlayerController::GetControlGroup(Group): returns bFull (the group holds something). OutCount: the units, or the buildings when it holds no unit;
	 * bOutBuildings: it holds no unit. False and 0 without a controller or out of range.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool GetControlGroupNow(int32 Group, int32& OutCount, bool& bOutBuildings) const;

	/** The group's types in the order they are met (the script sorts), names already localised, one count by type (UHOLGameScreen::GetGroupTypes). Empty without a group. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void GetControlGroupTypes(int32 Group, TArray<FString>& OutNames, TArray<int32>& OutCounts) const;

	/** AHOLPlayerController::GetLastSelectedGroup: the group, INDEX_NONE (-1) when none or without a controller. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetLastSelectedGroup() const;

	/** Real seconds since that selection (1e9, never lit, without a selection or a controller). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	float GetLastSelectedGroupAge() const;

	/** AHOLPlayerController::SelectControlGroup (the script writes the journal line of the click). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SelectControlGroup(int32 Group);

	/** The C++ screen's QueuePanel (built by BuildLayout in the CommandColumn, collapsed, under the short message): the script fills it (HUD H-7). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UBorder* GetQueuePanel() const;

	/**
	 * What the queue panel shows (RefreshQueue + UpdateHead of the C++ screen, in one read): the own selected building's queue, one entry per item in
	 * order (Research: it is a technology; TechNames: its name, empty for a unit; UnitNames: the unit's name, empty for a research), and its head:
	 * started, its progress (0 when not started), held at the end for lack of room, waiting for room. False, all empty: no own building selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool GetQueueNow(TArray<FName>& OutTypes, TArray<bool>& OutResearch, TArray<FString>& OutTechNames, TArray<FString>& OutUnitNames,
		bool& bOutHeadStarted, float& OutHeadProgress, bool& bOutBlockedAtFull, bool& bOutPopulationShort) const;

	/** The queue part of Describe's line (queue + Line): the script writes it at each refresh. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetQueueReport(const FString& Line);

	/** The progress bar's fill colour (HOLUIStyle::Accent). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetAccentColor();

	/** AHOLPlayerController::RequestCancelProduction(Index) (not exposed itself); false without a controller (the script writes the journal line). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool CancelProduction(int32 Index);

	/** True while the queue panel shows a queue: NativeTick then calls QueueFrameTick every frame (the head's bar moves each frame, as UpdateHead did), never otherwise. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetQueueLive(bool bLive);

	/** Once a frame, ONLY while SetQueueLive(true): the script reads the head again. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void QueueFrameTick();

	/** The C++ screen changed what the queue shows (selection, queue event): the next frame refreshes the script screen instead of waiting for its 4 Hz timer. */
	void RequestRefresh() { RefreshLeft = 0.f; }

	/**
	 * The minor alerts (D202, HOLAlertRules.as): the shown player's places taken and given (D282) and the HUD's real time (s).
	 * False for a spectator or before the player state arrives.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool GetPopulationNow(int32& OutUsed, int32& OutCap, float& OutRealSeconds) const;

	/**
	 * The top bar's economy (H-3), of the shown player (UHOLGameScreen::GetBoundPlayerState: a spectator's followed player too): stock, places taken and
	 * given, gatherers by resource, income a minute (GetIncomePerMinute). False, all zero, before the player state arrives.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool GetEconomyNow(FHOLResources& OutStock, int32& OutPopulationUsed, int32& OutPopulationCap, FHOLResources& OutGatherers, FHOLResources& OutIncome) const;

	/** The owning controller's idle workers (GetIdleWorkers().Num()), 0 without a controller. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetIdleWorkerCount() const;

	/** True: « Statistiques » is offered (UHOLGameMenuRulesBridge::NativeShowsStatsButton: a spectator during the match, everyone after it); false without a controller. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool GetStatsButtonShown() const;

	/** AHOLPlayerController::SelectNextIdleWorker (the script writes the journal line of the click). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SelectNextIdleWorker();

	/** UHOLGameMenuScreen::Open for the owning player (the « Menu » button). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void OpenGameMenu();

	/** UHOLStatsScreen::Open for the owning player (the « Statistiques » button). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void OpenStats();

	/** The resource's colour in the top bar (HOLUIStyle::Food / Wood / Stone / Gold for 0..3), grey otherwise. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetResourceColor(int32 Resource);

	/** A minor alert's line in the C++ screen's thread of messages (D446), on this machine only. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void AddThreadLine(const FText& Line);

	/** A HUD panel, the C++ screen's look (rounded, HOLUIStyle::HUDPanel), the only kind of widget taking clicks. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UBorder* MakePanel(FName Name);

	/** A text of the HUD in the style TextStyle (UHOLTextStyleHUD, UHOLTextStyleHUDLabel...), empty. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UCommonTextBlock* MakeText(FName Name, TSubclassOf<UCommonTextStyle> TextStyle);

	/**
	 * A HUD button, the look of the C++ screen's (UHOLHUDButtonStyle: dark, a gold edge hovered and pressed, plain dark
	 * greyed), never focusable; the script sets its content (SetContent) and binds OnClicked. Its clicks stop there.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UButton* MakeButton(FName Name);

	/**
	 * Button greyed or not, by its look only (UHOLHUDButtonStyleGreyed's: plain dark, a dim edge hovered): it stays enabled
	 * so that its tooltip shows and its content keeps its colours (a red price); the script ignores its clicks.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	static void SetButtonGreyed(UButton* Button, bool bGreyed);

	/** A HUD bar as the site's progress (dark background, rounded), filled in FillColor, at 0. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UProgressBar* MakeBar(FName Name, FLinearColor FillColor);

	/** The local player's colour (HOLPlayerColors, as the menu's swatches), grey for a spectator. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	FLinearColor GetLocalColor() const;

	/** The HUD's colours (HOLUIStyle): its text, its labels', a missing amount (red). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetTextColor();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetMutedColor();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetMissingColor();

	/**
	 * The room of a block bottom left, above the selection's block and the control groups' bar (UHOLGameScreen's
	 * places): the padding of a block aligned bottom left in the script's frame.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	FMargin GetBottomLeftPlace() const;

	/**
	 * The bottom edge (Slate units under the screen's top) of the cheats' panel, top left under the top bar: a block of the script stacked under it
	 * starts there (the AI level line). 0 without the panel (Shipping, or no HUD).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	float GetCheatPanelBottom() const;

	/**
	 * Tells the cheats' panel whether the hero's block is shown (the script, as it applies the block's view): in a low window (under 700 px) the
	 * panel folds when the block opens, as they would cover each other (Q-heros-triches-600 = A). Nothing without the panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetCheatsHeroBlockShown(bool bShown) const;

	/** The window as the HUD sees it, in Slate units (the pixels over the DPI scale): the room the panels are laid out in. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	FVector2D GetViewportSlateSize() const;

	/** The box a drawn, visible widget of this screen covers, in Slate units from the window's top left; false: hidden or not drawn yet. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool GetWidgetBox(UWidget* Widget, FVector2D& OutMin, FVector2D& OutMax) const;

	/** The same for a widget of the C++ game screen by name ("PlayersPanel", "MenuPanel"): where a block of the script must not go. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool GetGameScreenBox(FName WidgetName, FVector2D& OutMin, FVector2D& OutMax) const;

	/**
	 * Which of the script's visible panels (MakePanel) cover each other or the cheats' panel, as drawn: "none" or "A x B; C x cheats" (hol.HUD.Report's
	 * "HUD overlap" line; preflight.ps1 fails on anything else). Boxes not drawn yet are left out.
	 */
	FString DescribeOverlaps() const;

	/** The boxes DescribeOverlaps compares, "name=x0,y0,x1,y1; ..." (Slate units; empty: none drawn): the "HUD rects" line of hol.HUD.Report (L1-12b). */
	FString DescribeRects() const;

	/** The cheats' panel, PlayersPanel, MenuPanel and the script's outermost visible panels, as drawn (DescribeOverlaps and DescribeRects read it). */
	void GatherPanelBoxes(TArray<FHOLHUDNamedBox>& OutBoxes) const;

	/** The class made by the game screen: the newest script class derived from this one, not abstract; null: none. */
	static UClass* FindScriptClass();

	/** The game screen's short message (D313) for the script to show: Key names it in the logs and in Describe (empty: it never ends, as in the C++ it replaced), Text is the C++'s (GetFeedbackText). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void ShortMessage(const FString& Key, const FText& Text);

	/** Once a frame, but ONLY while a message with a key is shown (bMessageLive, set by SetShownMessageKey): the script's fade and the end of the message. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void FrameTick(float DeltaSeconds);

	/** The lit control group changed (a recall, or its 0.5 s end): NativeTick detects it each frame and calls this only then (HUD H-6, HOLGroupBar.as RefreshLit). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void GroupLitChanged();

	/** True when a building of the shown player holds a finished unit for lack of room (D829, the loop of the C++ screen's old CheckPopulationBlocked). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	bool IsProductionHeldForRoom() const;

	/** The key Describe prints (", message <Key>"; empty: none): the screen mirrors it for hol.HUD.Report. Non-empty starts the per-frame FrameTick, empty stops it. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetShownMessageKey(const FString& Key);

	/** Wraps Text at Width (Slate units): the message wraps at the grid's width less 24. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetTextWrapWidth(UCommonTextBlock* Text, float Width) const;

	/**
	 * HUD H-5a: the major alert banner the C++ screen's state (UHOLGameScreen::Alert, FHOLForumAlert: the state machine stays C++, one source) shows now.
	 * bOutShown = Alert.bShown; OutSerial = the count of banners presented so far (UHOLGameScreen::GetAlertPresentCount, never reset); OutSlot = Alert.Slot;
	 * OutOpacity = Alert.GetOpacity(real time); bOutSound = Alert.bSound; OutText = Alert.Message.Text; OutPieces = HOLForumMessages::SplitNames as the shown
	 * player reads them; OutColors parallel to OutPieces: the colour of the name's player, white for a word. False without the screen or world (all neutral).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool GetAlertBanner(bool& bOutShown, int32& OutSerial, int32& OutSlot, float& OutOpacity, bool& bOutSound, FString& OutText,
		TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const;

	/** FHOLForumAlert::SoundPath played 2D; false when the asset did not load. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool PlayAlertSound();

	/** The owning controller's FocusCameraOnForum(Slot): the camera only, the journal line is the script's. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void FocusCameraOnForum(int32 ForumSlot);

	/** The alert banner's panel takes the left mouse button: the C++ receives it (FOnPointerEvent) and asks the script's ClickWidget(AlertPanel), which centres the camera. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void BindAlertClick(UBorder* Panel);

	UFUNCTION()
	FEventReply HandleAlertMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent);

	/** Once a frame, but ONLY while the major alert is shown (and once more when it ends): the banner's fade and its end (HOLAlertBanner.as). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void AlertFrameTick(float DeltaSeconds);

	/** HUD H-8: the C++ bottom bar's AgePanelsHost (an empty horizontal box where the age panels were), filled by the script's four panels. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	UHorizontalBox* GetAgePanelsHost() const;

	/** The grid context the age panels read (UHOLGameScreen::RefreshAges' rule): the units' context while no own building or group is selected, else an empty one. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void GetAgesContext(FHOLGridContextView& OutContext) const;

	/** The age the controller has chosen (AHOLPlayerController::GetChosenAge), -1 without a controller. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetChosenAge() const;

	/** The shown player's tier (BoundTableState->GetTier), -1 without one. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetTier() const;

	/** AHOLPlayerController::ClickAge(Age, Cell) (the script writes the journal line of the click). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void ClickAge(int32 Age, int32 Cell);

	/** A cell of an age panel shows Building (name, Ready or Greyed by the stock, tooltip with the cost and the effect, as the C++ screen's RefreshAges did). False: nothing to show. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	bool FillAgeCell(UHOLGridCell* Cell, const FHOLGridCommandView& Building);

	/** What the age panels show, for UHOLGameScreen::Describe and ClickWidget (hol.HUD.Report's "grid ages" line, read without widgets). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetAgesReport(bool bShown, int32 Chosen, const TArray<FName>& Types, const TArray<bool>& Reached, const TArray<FString>& Tooltips);

	/** A border's brush as the C++ screen's rounded boxes (FSlateRoundedBoxBrush(Fill, Radius, Outline, OutlineWidth)). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetRoundedBrush(UBorder* Border, FLinearColor Fill, float Radius, FLinearColor Outline, float OutlineWidth) const;

	/** HOLUIStyle colour: 0 Accent, 1 Text, 2 TextMuted, 3 HUDPanel (grey otherwise). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetStyleColor(int32 Which);

	/** Seconds between two Refresh (4 per second). */
	static constexpr float RefreshSeconds = 0.25f;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	float RefreshLeft = 0.f;
	/** A message with a key is shown: NativeTick calls FrameTick (D839: no per-frame script call otherwise). */
	bool bMessageLive = false;
	/** The alert banner was shown at the last frame: NativeTick calls AlertFrameTick while it is, and once more when it ends. */
	bool bAlertLive = false;
	/** The control group lit as NativeTick last saw it (INDEX_NONE: none): a change calls GroupLitChanged. */
	int32 LitGroupSeen = INDEX_NONE;

	/** The queue panel shows a queue: NativeTick calls QueueFrameTick (HUD H-7). */
	bool bQueueLive = false;
};
