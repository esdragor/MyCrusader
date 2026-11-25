// F7 lot 4a, garrison frame widget (D813 Q-garrison-ui Q1 = C): a row of the building frame, the counter "2 / 5" then one button per occupant. The rule
// is HOLGarrisonFrameRules.as / HOLGarrisonView.as (tested there; a script test cannot read a widget). WIRED (ELROND-1120pd): HOLSelection.as builds it in
// the selection column and RefreshBuilding calls Refresh with FHOLBuildingSelView.GarrisonOccupants / GarrisonOccupantTypes (GLORFINDEL-1120pc's branch,
// ability-columns: this file compiles only with it); a click sends its order through HOLControllerCommand::RequestUngarrisonUnitOf / RequestUngarrisonAllSelected.
// Each occupant's button shows the icon of its unit type (D874, /Game/UI/Icons/Units/T_Unit_<Type>, none drawn yet: the type's name shows then, Placeholders.txt).

// One portrait's click, to the frame (the handler pattern of HOLSelectionChips.as).
class UHOLGarrisonPortraitHandler : UObject
{
	UHOLGarrisonFrame Frame;
	int Index = -1;

	UFUNCTION()
	void Clicked()
	{
		if (Frame == nullptr)
			return;
		if (Index >= 0)
			Frame.ClickPortrait(Index);
		else
			Frame.ClickUngarrisonAll();
	}
}

class UHOLGarrisonFrame : UObject
{
	UHOLScriptScreen Screen;
	UHorizontalBox Row;
	UTextBlock Counter;
	TArray<UButton> Buttons;
	TArray<UTextBlock> Labels;
	TArray<UImage> Icons;
	TArray<UHOLGarrisonPortraitHandler> Handlers;
	// The Sortir tous button (a handler with Index -1), shown while somebody is inside.
	UButton AllButton;
	UHOLGarrisonPortraitHandler AllHandler;
	// The icon textures found by type key (nullptr stored for a type without a texture, so a missing asset is looked up once).
	TMap<FString, UTexture2D> IconCache;
	// What the row shows now, one entry per button (the rule's ButtonsFor).
	TArray<FHOLGarrisonButtonView> ButtonViews;

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
		AllButton = Cast<UButton>(Screen.ConstructWidget(UButton, n"GarrisonUngarrisonAll"));
		UTextBlock AllLabel = Cast<UTextBlock>(Screen.ConstructWidget(UTextBlock, n"GarrisonUngarrisonAllLabel"));
		AllLabel.SetText(FText::FromString(HOLGarrisonFrameRules::UngarrisonAllLabel()));
		AllButton.SetContent(AllLabel);
		AllButton.SetVisibility(ESlateVisibility::Collapsed);
		AllHandler = Cast<UHOLGarrisonPortraitHandler>(NewObject(this, UHOLGarrisonPortraitHandler));
		AllHandler.Frame = this;
		AllHandler.Index = -1;
		AllButton.OnClicked.AddUFunction(AllHandler, n"Clicked");
		Row.AddChildToHorizontalBox(AllButton).SetPadding(FMargin(0.0, 0.0, 8.0, 0.0));
	}

	// The row hidden (the selection is not a building): nothing drawn, no button kept.
	void Hide()
	{
		if (Row != nullptr)
			Row.SetVisibility(ESlateVisibility::Collapsed);
		bRowShown = false;
		View = FHOLGarrisonView();
		ButtonViews.Reset();
	}

	// The texture of a type's icon, or nullptr (looked up once per type; a type without a texture shows its name instead).
	private UTexture2D IconOf(const FHOLGarrisonButtonView& Button)
	{
		if (Button.IconPath == "")
			return nullptr;
		UTexture2D Found = nullptr;
		if (IconCache.Find(Button.TypeKey, Found))
			return Found;
		Found = Cast<UTexture2D>(LoadObject(nullptr, Button.IconPath));
		IconCache.Add(Button.TypeKey, Found);
		return Found;
	}

	// One more portrait button at the end of the row (the pool grows to the most occupants seen).
	private void AddPortrait()
	{
		const int Index = Buttons.Num();
		UButton Button = Cast<UButton>(Screen.ConstructWidget(UButton, HOLGarrisonFrameRules::PortraitName(Index)));
		UTextBlock Label = Cast<UTextBlock>(Screen.ConstructWidget(UTextBlock, FName(f"GarrisonPortraitLabel{Index}")));
		UImage Icon = Cast<UImage>(Screen.ConstructWidget(UImage, FName(f"GarrisonPortraitIcon{Index}")));
		Icon.SetDesiredSizeOverride(FVector2D(32.0, 32.0));
		UHorizontalBox Face = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, FName(f"GarrisonPortraitFace{Index}")));
		Face.AddChildToHorizontalBox(Icon);
		Face.AddChildToHorizontalBox(Label);
		Button.SetContent(Face);
		Button.SetVisibility(ESlateVisibility::Collapsed);
		UHOLGarrisonPortraitHandler Handler = Cast<UHOLGarrisonPortraitHandler>(NewObject(this, UHOLGarrisonPortraitHandler));
		Handler.Frame = this;
		Handler.Index = Index;
		Button.OnClicked.AddUFunction(Handler, n"Clicked");
		Row.AddChildToHorizontalBox(Button).SetPadding(FMargin(0.0, 0.0, 4.0, 0.0));
		Buttons.Add(Button);
		Labels.Add(Label);
		Icons.Add(Icon);
		Handlers.Add(Handler);
	}

	// AR0-a4: the local player's faction (AHOLGameState::GetSlotFaction of his slot, an EHOLFaction index); -1 without a slot or a state.
	int LocalFactionIndex()
	{
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		const int Slot = Screen != nullptr ? Screen.GetLocalSlot() : -1;
		return State != nullptr && Slot >= 0 ? State.GetSlotFaction(Slot) : -1;
	}

	// The selected building's garrison, each time the selection is read: the counter, one button per occupant, the row hidden when HOLGarrisonFrameRules draws nothing.
	void Refresh(const TArray<int>& OccupantUnitIds, const TArray<FString>& OccupantTypes, int Slots, bool bOwn, bool bFinished)
	{
		if (Screen == nullptr || Row == nullptr)
			return;
		View = HOLGarrisonFrameRules::ViewFor(OccupantUnitIds, Slots, bOwn, bFinished);
		bRowShown = View.CounterText != "";
		Row.SetVisibility(bRowShown ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		Counter.SetText(FText::FromString(View.CounterText));
		// AR0-a4: the occupants are the building owner's units, and the row is drawn for an own building only (ViewFor): their owner is the local player,
		// whose faction's icons (DA_Art_<Type>_<Faction>) the buttons show.
		const FString OwnerFaction = bOwn ? HOLGarrisonFrameRules::OwnerFactionKey(LocalFactionIndex()) : "";
		ButtonViews = HOLGarrisonFrameRules::ButtonsFor(View, OccupantTypes, OwnerFaction);
		AllButton.SetVisibility(View.bShowUngarrisonAll ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		while (Buttons.Num() < View.PortraitUnitIds.Num())
			AddPortrait();
		for (int Index = 0; Index < Buttons.Num(); Index++)
		{
			const bool bPortrait = Index < View.PortraitUnitIds.Num();
			Buttons[Index].SetVisibility(bPortrait ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (!bPortrait)
				continue;
			UTexture2D Texture = IconOf(ButtonViews[Index]);
			const bool bIcon = Texture != nullptr;
			if (bIcon)
				Icons[Index].SetBrushFromTexture(Texture, false);
			Icons[Index].SetVisibility(bIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
			Labels[Index].SetVisibility(bIcon ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
			Labels[Index].SetText(FText::FromString(ButtonViews[Index].Label));
		}
	}

	// A portrait's click: that unit comes out (the order is kept in LastOrder; None for an index outside the portraits).
	bool ClickPortrait(int Index)
	{
		LastOrder = HOLGarrisonView::OrderForPortraitClick(View, Index);
		SendLastOrder();
		return LastOrder.Kind != HOLGarrisonView::OrderNone;
	}

	// The Sortir tous cell or key: every occupant out (None while nobody is inside).
	bool ClickUngarrisonAll()
	{
		LastOrder = HOLGarrisonView::OrderForUngarrisonAll(View);
		SendLastOrder();
		return LastOrder.Kind != HOLGarrisonView::OrderNone;
	}

	// The order the last click gave, to the player's controller (the server applies it; the selected building is the one read).
	private void SendLastOrder()
	{
		if (Screen == nullptr || LastOrder.Kind == HOLGarrisonView::OrderNone)
			return;
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Screen.GetOwningPlayer());
		if (Controller == nullptr)
			return;
		if (LastOrder.Kind == HOLGarrisonView::OrderUngarrisonUnit)
			HOLControllerCommand::RequestUngarrisonUnitOf(Controller, LastOrder.UnitId);
		else if (LastOrder.Kind == HOLGarrisonView::OrderUngarrisonAll)
			HOLControllerCommand::RequestUngarrisonAllSelected(Controller);
	}
}