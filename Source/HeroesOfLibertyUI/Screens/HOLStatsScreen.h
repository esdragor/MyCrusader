#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLStatsScreen.generated.h"

class UGridPanel;
class UWidgetSwitcher;
class UVerticalBox;
class UHorizontalBox;
class UCommonTextBlock;
class UHOLButton;
class UHOLStatsGraph;

/**
 * The statistics screen (C4, M6-Q10, M6-Q11), as Age of Empires II's end screen: tables by player in tabs, « Résumé »
 * (name in its colour, team, result, the two scores of HOLMatchStats), « Militaire », « Économie », and « Chronologie »,
 * the curves over time of a series picked by its buttons (C5: Péons, Armée, Population, Bâtiments, Ressources récoltées,
 * Stock), a player each in its colour, the defeats and fallen forums as dated marks. Opened over the match from the end banner (« Statistiques »), « Retour » or Back goes back
 * to it. It asks UHOLMatchStatsSubsystem for the statistics when it opens and redraws at each OnStatsUpdated; a refusal
 * (a player during the match) or the wait for the server's copy is said in place of the tables. A spectator opens it
 * during the match from the game screen's « Statistiques » (C6); while open it asks again every 10 s.
 */
UCLASS(Blueprintable)
class HEROESOFLIBERTYUI_API UHOLStatsScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the match (« Statistiques », hol.UI.Open Stats); null when no match layout. */
	static UHOLStatsScreen* Open(APlayerController* PlayerController);

	/** The script's screen (the newest class deriving from this one, never the C++ class itself); null: none. ST-3 will make Open push it. */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	static TSubclassOf<UHOLStatsScreen> FindScriptClass();

	/** 0 Summary, 1 Military, 2 Economy, 3 Timeline. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	void SelectTab(int32 Index);
	/** The timeline's series (EHOLMatchStatsSeries). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	void SelectSeries(int32 Index);
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	void Close();

	/** The tab shown (0 Summary ... 3 Timeline). */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	int32 GetActiveTab() const { return TabIndex; }
	/** The timeline's series shown (EHOLMatchStatsSeries order: 0 Workers, 1 Military, 2 Population, 3 Buildings, 4 Gathered, 5 Stock). */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	int32 GetActiveSeries() const { return SeriesIndex; }
	/** True once the server refused the statistics (a player during the match, M6-Q11); cleared at each activation. */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	bool IsRefused() const { return bRefused; }

	/** UHOLScreen::MakeDialogColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UVerticalBox* MakeStatsColumn();
	/** A line of the column: style 0 the heading (16 under it), 1 the body (0 under it), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UCommonTextBlock* AddStatsText(UVerticalBox* Column, FName Name, const FText& Text, int32 Style);
	/** UHOLScreen::AddButtonRow for the script (the tabs' row, the Back button's row). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UHorizontalBox* AddStatsRow(UVerticalBox* Column);
	/** A button of the row (UHOLScreen::AddButton): the tabs and Back; the script keeps it in SummaryTab, MilitaryTab, EconomyTab, TimelineTab or BackButton (NativeOnInitialized wires them). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UHOLButton* AddStatsButton(UHorizontalBox* Row, FName Name, const FText& Label);
	/** The switcher of the pages (TabPages), added under the column with its padding; a page: AddStatsGridPage then AddStatsTimelinePage. Resets Grids. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UWidgetSwitcher* AddStatsPages(UVerticalBox* Column);
	/** A table page (an empty grid appended to Grids and to TabPages): the Summary, Military and Economy ones, in that order. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UGridPanel* AddStatsGridPage();
	/** The Timeline page (a vertical box in TabPages); resets SeriesButtons. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UVerticalBox* AddStatsTimelinePage();
	/** The centred row of the series' buttons, on top of the Timeline page. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UHorizontalBox* AddStatsSeriesRow(UVerticalBox* TimelinePage);
	/** A series' button (selectable, toggleable and wired to SelectSeries by NativeOnInitialized), appended to SeriesButtons in EHOLMatchStatsSeries's order. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UHOLButton* AddStatsSeriesButton(UHorizontalBox* Row, FName Name, const FText& Label);
	/** The graph in its SizeBox (1100 x 440) under the series' buttons; sets Graph. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	UHOLStatsGraph* AddStatsGraph(UVerticalBox* TimelinePage);
	/** A table cell: the header style or the body style, the name column (0) on the left and the figures on the right; bColored puts a swatch of Color before it (a player's name). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Stats")
	void AddStatsCell(UGridPanel* Grid, int32 Row, int32 Column, const FText& Text, bool bHeader, bool bColored, const FLinearColor& Color);

	/** The column titles of a table: 0 Summary (Player, Team, Result, MilitaryScore, EconomyScore, Total), 1 Military, 2 Economy; empty out of range. */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	static TArray<FText> HeadersOf(int32 Tab);
	/** A text of the screen by its key (Heading, Summary, Military, Economy, Timeline, SeriesWorkers, SeriesMilitary, SeriesPopulation, SeriesBuildings, SeriesGathered, SeriesStock, Back, Refused, Waiting, Winner, Defeated, Playing); empty for another key. */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	static FText TextOf(FName Key);
	/** The dated mark's label: Kind 0 Defeated "{Player} vaincu", 1 ForumFell "Forum de {Player} tombé" (the order of EHOLMatchStatsEvent). */
	UFUNCTION(BlueprintPure, Category = "HOL|Stats")
	static FText MakeMarkLabel(int32 Kind, const FText& PlayerName);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** The script builds the screen (in place of BuildLayout), with the AddStats* helpers above. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Stats")
	void BP_BuildStats();

	/** The script shows the tables again (in place of Refresh): at each OnStatsUpdated and refusal; bRefusedNow: the server refused. It may run before the script has the statistics (the screen asks for them on activation). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Stats")
	void BP_RefreshStats(bool bRefusedNow);

	/** The script redraws the graph (in place of RefreshTimeline): at each SelectSeries; Series: the series now selected. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Stats")
	void BP_RefreshTimeline(int32 Series);

public:
	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Status;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> TabPages;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> SummaryTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> MilitaryTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> EconomyTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> TimelineTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|Stats")
	TArray<TObjectPtr<UGridPanel>> Grids;

	/** The timeline's series buttons, in EHOLMatchStatsSeries's order. */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|Stats")
	TArray<TObjectPtr<UHOLButton>> SeriesButtons;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Stats")
	TObjectPtr<UHOLStatsGraph> Graph;

private:
	/** True for a class deriving from this one (the script's): it builds and fills the screen through the BP_ events. */
	bool IsScriptScreen() const;
	/** The tables again from the statistics now held, or the status line when there are none. */
	void Refresh();
	void OnRefused();
	/** The curves of the selected series again from the statistics now held. */
	void RefreshTimeline();

	FDelegateHandle UpdatedHandle;
	FDelegateHandle RefusedHandle;
	/** Asks the statistics again every RequestInterval while the screen is open. */
	FTimerHandle RequestTimer;
	bool bRefused = false;
	int32 SeriesIndex = 0;
	int32 TabIndex = 0;
};
