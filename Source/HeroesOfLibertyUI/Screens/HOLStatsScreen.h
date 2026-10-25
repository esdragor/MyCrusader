#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLStatsScreen.generated.h"

class UGridPanel;
class UWidgetSwitcher;

/**
 * The statistics screen (C4, M6-Q10, M6-Q11), as Age of Empires II's end screen: tables by player in tabs, « Résumé »
 * (name in its colour, team, result, the two scores of HOLMatchStats), « Militaire », « Économie », and « Chronologie »,
 * the curves over time of a series picked by its buttons (C5: Péons, Armée, Population, Bâtiments, Ressources récoltées,
 * Stock), a player each in its colour, the defeats and fallen forums as dated marks. Opened over the match from the end banner (« Statistiques »), « Retour » or Back goes back
 * to it. It asks UHOLMatchStatsSubsystem for the statistics when it opens and redraws at each OnStatsUpdated; a refusal
 * (a player during the match) or the wait for the server's copy is said in place of the tables. A spectator opens it
 * during the match from the game screen's « Statistiques » (C6); while open it asks again every 10 s.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLStatsScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the match (« Statistiques », hol.UI.Open Stats); null when no match layout. */
	static UHOLStatsScreen* Open(APlayerController* PlayerController);

	void SelectTab(int32 Index);
	/** The timeline's series (EHOLMatchStatsSeries). */
	void SelectSeries(int32 Index);
	void Close();

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Status;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> TabPages;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> SummaryTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> MilitaryTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> EconomyTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> TimelineTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	UPROPERTY()
	TArray<TObjectPtr<UGridPanel>> Grids;

	/** The timeline's series buttons, in EHOLMatchStatsSeries's order. */
	UPROPERTY()
	TArray<TObjectPtr<UHOLButton>> SeriesButtons;

	UPROPERTY()
	TObjectPtr<class UHOLStatsGraph> Graph;

private:
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
