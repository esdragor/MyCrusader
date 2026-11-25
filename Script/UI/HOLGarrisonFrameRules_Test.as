// HOLGarrisonFrameRules (F7 lot 4a, D813 Q-garrison-ui Q1 = C): the frame draws a garrison for the player's own finished building with places only;
// each value asserted is computed by HOLGarrisonView through the rule from the occupants it is given.

void Test_GarrisonFrameOwnFinishedBuildingDrawsTheGarrison(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	Occupants.Add(21);
	FHOLGarrisonView View = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	T.AssertEquals(View.CounterText, "2 / 5", "own finished tower: the counter");
	T.AssertEquals(View.PortraitUnitIds.Num(), 2, "one portrait per occupant");
	T.AssertTrue(View.bShowUngarrisonAll, "Sortir tous is offered");
	FHOLGarrisonOrder Click = HOLGarrisonView::OrderForPortraitClick(View, 1);
	T.AssertEquals(Click.Kind, HOLGarrisonView::OrderUngarrisonUnit, "a click on the second portrait takes that unit out");
	T.AssertEquals(Click.UnitId, 21, "the unit of the clicked portrait");
}

void Test_GarrisonFrameOtherPlayersOrUnfinishedBuildingDrawsNothing(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	FHOLGarrisonView Other = HOLGarrisonFrameRules::ViewFor(Occupants, 5, false, true);
	T.AssertEquals(Other.CounterText, "", "another player's tower: no counter");
	T.AssertEquals(Other.PortraitUnitIds.Num(), 0, "no portrait of an enemy's occupants");
	T.AssertFalse(Other.bShowUngarrisonAll, "no cell");
	FHOLGarrisonOrder OtherKey = HOLGarrisonView::OrderForUngarrisonAll(Other);
	T.AssertEquals(OtherKey.Kind, HOLGarrisonView::OrderNone, "no order from it");

	FHOLGarrisonView Site = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, false);
	T.AssertEquals(Site.CounterText, "", "a foundation or a site holds nobody: nothing drawn");
	FHOLGarrisonView NoPlaces = HOLGarrisonFrameRules::ViewFor(Occupants, 0, true, true);
	T.AssertEquals(NoPlaces.CounterText, "", "a building without places: nothing drawn");
}

void Test_GarrisonFramePortraitNamesAreDistinct(FUnitTest& T)
{
	const FString First = HOLGarrisonFrameRules::PortraitName(0).ToString();
	const FString Second = HOLGarrisonFrameRules::PortraitName(1).ToString();
	T.AssertEquals(First, "GarrisonPortrait0", "the first portrait's widget name");
	T.AssertEquals(Second, "GarrisonPortrait1", "the second portrait's widget name");
}

void Test_GarrisonFrameUnitIconPathIsTheTypesTexture(FUnitTest& T)
{
	T.AssertEquals(HOLGarrisonFrameRules::UnitIconPath("Spearman"), "/Game/UI/Icons/Units/T_Unit_Spearman.T_Unit_Spearman", "a known type: its texture path");
	T.AssertEquals(HOLGarrisonFrameRules::UnitIconPath("Archer"), "/Game/UI/Icons/Units/T_Unit_Archer.T_Unit_Archer", "another type: its own path");
	T.AssertEquals(HOLGarrisonFrameRules::UnitIconPath(""), "", "an unknown type (empty key): no icon path");
}

void Test_GarrisonFrameOccupantsBecomeButtonsWithTheirTypeIcon(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	Occupants.Add(21);
	Occupants.Add(34);
	TArray<FString> Types;
	Types.Add("Spearman");
	Types.Add("Archer");
	Types.Add("");
	FHOLGarrisonView View = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	TArray<FHOLGarrisonButtonView> Buttons = HOLGarrisonFrameRules::ButtonsFor(View, Types);
	T.AssertEquals(Buttons.Num(), 3, "one button per occupant");
	if (Buttons.Num() != 3)
		return;
	T.AssertEquals(Buttons[0].UnitId, 8, "the first button is the first occupant");
	T.AssertEquals(Buttons[1].UnitId, 21, "the second button is the second occupant");
	T.AssertEquals(Buttons[0].TypeKey, "Spearman", "its type");
	T.AssertEquals(Buttons[0].IconPath, HOLGarrisonFrameRules::UnitIconPath("Spearman"), "the icon of its type");
	T.AssertEquals(Buttons[1].IconPath, HOLGarrisonFrameRules::UnitIconPath("Archer"), "the second one's own icon");
	T.AssertTrue(Buttons[0].IconPath != Buttons[1].IconPath, "two types, two icons");
	T.AssertEquals(Buttons[2].IconPath, "", "a unit this machine does not know: no icon");
	T.AssertEquals(Buttons[2].Label, "34", "no icon: the unit id as label");
	T.AssertEquals(Buttons[0].Label, "Spearman", "an icon may be missing in the content: the type's name as label");
}

void Test_GarrisonFrameButtonsTolerateAShortTypeListAndAnotherPlayersBuilding(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	Occupants.Add(21);
	TArray<FString> OneType;
	OneType.Add("Spearman");
	FHOLGarrisonView Own = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	TArray<FHOLGarrisonButtonView> Short = HOLGarrisonFrameRules::ButtonsFor(Own, OneType);
	T.AssertEquals(Short.Num(), 2, "a types list shorter than the occupants: still one button per occupant");
	if (Short.Num() == 2)
		T.AssertEquals(Short[1].IconPath, "", "the occupant without a type: no icon");
	FHOLGarrisonView Other = HOLGarrisonFrameRules::ViewFor(Occupants, 5, false, true);
	TArray<FHOLGarrisonButtonView> None = HOLGarrisonFrameRules::ButtonsFor(Other, OneType);
	T.AssertEquals(None.Num(), 0, "another player's building: no button");
}
// D924 (AR0-a2): the occupant's icon is the type's art asset icon (DA_Art_<Type>.Icon) when it sets one, the conventional path otherwise.
void Test_GarrisonFrameUnitIconPathReadsTheArtAssetFirst(FUnitTest& T)
{
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualIcon(n"Spearman", NAME_None, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman");
	const FString FromAsset = HOLGarrisonFrameRules::UnitIconPath("Spearman");
	const FString Conventional = HOLGarrisonFrameRules::UnitIconPath("Archer");
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(FromAsset, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman", "a type whose art asset sets an icon: that icon");
	T.AssertEquals(Conventional, "/Game/UI/Icons/Units/T_Unit_Archer.T_Unit_Archer", "a type without art asset: the conventional path");
}

void Test_GarrisonFrameUnitIconPathReadsTheOwnersFactionFirst(FUnitTest& T)
{
	// AR0-a3: the icon of the occupant owner's faction variant (DA_Art_<Type>_<Faction>) first, the common asset for another faction.
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualIcon(n"Spearman", NAME_None, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman");
	UHOLArtVisualRegistry::AddTestArtVisualIcon(n"Spearman", n"Legions", "/Game/Art/Units/Spearman/T_Icon_Spearman_Legions.T_Icon_Spearman_Legions");
	const FString Own = HOLGarrisonFrameRules::UnitIconPath("Spearman", "Legions");
	const FString Other = HOLGarrisonFrameRules::UnitIconPath("Spearman", "Dawn");
	TArray<int> Occupants;
	Occupants.Add(12);
	TArray<FString> OneType;
	OneType.Add("Spearman");
	FHOLGarrisonView View = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	TArray<FHOLGarrisonButtonView> Buttons = HOLGarrisonFrameRules::ButtonsFor(View, OneType, "Legions");
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(Own, "/Game/Art/Units/Spearman/T_Icon_Spearman_Legions.T_Icon_Spearman_Legions", "a Legions occupant: his faction's icon");
	T.AssertEquals(Other, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman", "an occupant of another faction: the common icon");
	T.AssertEquals(Buttons.Num(), 1, "one occupant, one button");
	if (Buttons.Num() == 1)
		T.AssertEquals(Buttons[0].IconPath, "/Game/Art/Units/Spearman/T_Icon_Spearman_Legions.T_Icon_Spearman_Legions", "the button of a Legions building's occupant: the Legions icon");
}

void Test_GarrisonFrameOwnerFactionKeyGivesTheFactionsIcon(FUnitTest& T)
{
	// AR0-a4: the frame reads its local player's faction index (AHOLGameState::GetSlotFaction) as a key: a Legions player sees his occupants' Legions icon.
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualIcon(n"Spearman", NAME_None, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman");
	UHOLArtVisualRegistry::AddTestArtVisualIcon(n"Spearman", n"Legions", "/Game/Art/Units/Spearman/T_Icon_Spearman_Legions.T_Icon_Spearman_Legions");
	TArray<int> Occupants;
	Occupants.Add(12);
	TArray<FString> OneType;
	OneType.Add("Spearman");
	FHOLGarrisonView View = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	const FString LegionsKey = HOLGarrisonFrameRules::OwnerFactionKey(int(EHOLFaction::BlackLegions));
	const FString DawnKey = HOLGarrisonFrameRules::OwnerFactionKey(int(EHOLFaction::DawnOrder));
	const FString NoKey = HOLGarrisonFrameRules::OwnerFactionKey(-1);
	TArray<FHOLGarrisonButtonView> LegionsButtons = HOLGarrisonFrameRules::ButtonsFor(View, OneType, LegionsKey);
	TArray<FHOLGarrisonButtonView> DawnButtons = HOLGarrisonFrameRules::ButtonsFor(View, OneType, DawnKey);
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(LegionsButtons.Num(), 1, "one occupant: one button");
	T.AssertEquals(DawnButtons.Num(), 1, "one occupant: one button (Dawn)");
	if (LegionsButtons.Num() == 1 && DawnButtons.Num() == 1)
	{
		T.AssertEquals(LegionsButtons[0].IconPath, "/Game/Art/Units/Spearman/T_Icon_Spearman_Legions.T_Icon_Spearman_Legions", "a Legions player's occupant: the Legions icon");
		T.AssertEquals(DawnButtons[0].IconPath, "/Game/Art/Units/Spearman/T_Icon_Spearman.T_Icon_Spearman", "a Dawn player's occupant: the common icon (no Dawn variant)");
	}
	T.AssertEquals(NoKey, "", "no slot: no faction key");
}