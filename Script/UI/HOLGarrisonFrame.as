// F7 lot 4a, garrison frame widget (D813 Q-garrison-ui Q1 = C): a row of the building frame, the counter "2 / 5" then one button per occupant. The rule
// is HOLGarrisonFrameRules.as / HOLGarrisonView.as (tested there; a script test cannot read a widget). NOT WIRED YET and NOT COMPILED: it needs the C++
// lot (see ELROND-1120pb.md): the occupants and the selected building's places in FHOLBuildingSelView, then HOLSelection.as RefreshBuilding calls Refresh,
// and the clicks' orders (LastOrder) go to the controller (RequestUngarrisonUnit, RequestUngarrisonAllOf).

// One portrait's click, to the frame (the handler pattern of HOLSelectionChips.as).
class UHOLGarrisonPortraitHandler : UObject
{
	UHOLGarrisonFrame Frame;
	int Index = -1;

	UFUNCTION()
	void Clicked()
	{
		if (Frame != nullptr)
			Frame.ClickPortrait(Index);
	}
}

class UHOLGarrisonFrame : UObject
{
	UHOLScriptScreen Screen;
	UHorizontalBox Row;
	UTextBlock Counter;
	TArray<UButton> Buttons;
	TArray<UTextBlock> Labels;
	TArray<UHOLGarrisonPortraitHandler> Handlers;

	// What the row shows now (for the tests and the click rule) and the order the last click gave (the owner of the row sends it).
	FHOLGarrisonView View;
	FHOLGarrisonOrder LastOrder;
	bool bRowShown = false;

	void Build(UHOLScriptScreen InScreen, UVerticalBox Host)
	{
		Screen = InScreen;
		Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"GarrisonRow"));
		Row.SetVisibility(ESlateVisibility::Collapsed);
		Host.AddChildToVerticalBox(Row);
		Counter = Cast<UTextBlock>(Screen.ConstructWidget(UTextBlock, n"GarrisonCounter"));
		Row.AddChildToHorizontalBox(Counter).SetPadding(FMargin(0.0, 0.0, 8.0, 0.0));
	}

	// One more portrait button at the end of the row (the pool grows to the most occupants seen).
	private void AddPortrait()
	{
		const int Index = Buttons.Num();
		UButton Button = Cast<UButton>(Screen.ConstructWidget(UButton, HOLGarrisonFrameRules::PortraitName(Index)));
		UTextBlock Label = Cast<UTextBlock>(Screen.ConstructWidget(UTextBlock, FName(f"GarrisonPortraitLabel{Index}")));
		Button.SetContent(Label);
		Button.SetVisibility(ESlateVisibility::Collapsed);
		UHOLGarrisonPortraitHandler Handler = Cast<UHOLGarrisonPortraitHandler>(NewObject(this, UHOLGarrisonPortraitHandler));
		Handler.Frame = this;
		Handler.Index = Index;
		Button.OnClicked.AddUFunction(Handler, n"Clicked");
		Row.AddChildToHorizontalBox(Button).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
		Buttons.Add(Button);
		Labels.Add(Label);
		Handlers.Add(Handler);
	}

	// The selected building's garrison, each time the selection is read: the counter, one button per occupant, the row hidden when HOLGarrisonFrameRules draws nothing.
	void Refresh(const TArray<int>& OccupantUnitIds, int Slots, bool bOwn, bool bFinished)
	{
		if (Screen == nullptr || Row == nullptr)
			return;
		View = HOLGarrisonFrameRules::ViewFor(OccupantUnitIds, Slots, bOwn, bFinished);
		bRowShown = View.CounterText != "";
		Row.SetVisibility(bRowShown ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		Counter.SetText(FText::FromString(View.CounterText));
		while (Buttons.Num() < View.PortraitUnitIds.Num())
			AddPortrait();
		for (int Index = 0; Index < Buttons.Num(); Index++)
		{
			const bool bPortrait = Index < View.PortraitUnitIds.Num();
			Buttons[Index].SetVisibility(bPortrait ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (bPortrait)
				Labels[Index].SetText(FText::FromString(f"{View.PortraitUnitIds[Index]}"));
		}
	}

	// A portrait's click: that unit comes out (the order is kept in LastOrder; None for an index outside the portraits).
	bool ClickPortrait(int Index)
	{
		LastOrder = HOLGarrisonView::OrderForPortraitClick(View, Index);
		return LastOrder.Kind != HOLGarrisonView::OrderNone;
	}

	// The Sortir tous cell or key: every occupant out (None while nobody is inside).
	bool ClickUngarrisonAll()
	{
		LastOrder = HOLGarrisonView::OrderForUngarrisonAll(View);
		return LastOrder.Kind != HOLGarrisonView::OrderNone;
	}
}