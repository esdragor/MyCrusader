// F7 lot 4a, garrison frame (D813 Q-garrison-ui Q1 = C): what the building frame does with HOLGarrisonView. The frame draws a garrison only for the
// player's OWN FINISHED building with places (the occupants are replicated to the owner only, HOLBuilding.h GetGarrisonOccupants): any other
// building gives the empty view (nothing drawn, no order). The widget is HOLGarrisonFrame.as.
// One occupant's button (HOLGarrisonFrameRules::ButtonsFor): the unit, its type, the icon to look up and the label shown while there is no icon.
struct FHOLGarrisonButtonView
{
	int UnitId = -1;
	FString TypeKey;
	FString IconPath;
	FString Label;
}
namespace HOLGarrisonFrameRules
{
	// The view of the selected building: the garrison of the player's own finished building with places (HOLGarrisonView::Build), else the empty view.
	FHOLGarrisonView ViewFor(const TArray<int>& OccupantUnitIds, int Slots, bool bOwn, bool bFinished)
	{
		if (!bOwn || !bFinished)
		{
			FHOLGarrisonView Empty;
			return Empty;
		}
		return HOLGarrisonView::Build(OccupantUnitIds, Slots);
	}

	// The icon of a unit type (D874: the occupant's button shows the type's icon, Q-garrison-portrait A). The texture is looked up by this path; no unit
	// icon is drawn yet (Placeholders.txt), so the button then falls back to the label. Empty for an unknown type (empty key).
	FString UnitIconPath(const FString& TypeKey, const FString& FactionKey = "")
	{
		if (TypeKey == "")
			return "";
		// D924 (AR0-a2, AR0-a3): the art asset icon first (DA_Art_<Type>_<Faction> of the occupant owner's faction, then DA_Art_<Type>), the conventional path below.
		const FString ArtIcon = UHOLArtVisualRegistry::GetArtIconPath(nullptr, FName(TypeKey), FactionKey != "" ? FName(FactionKey) : NAME_None);
		if (ArtIcon != "")
			return ArtIcon;
		return f"/Game/UI/Icons/Units/T_Unit_{TypeKey}.T_Unit_{TypeKey}";
	}

	// AR0-a4: the faction key (Dawn, Legions...) the occupants' icons are read with, from the owner's EHOLFaction index (GetSlotFaction); none (-1): "".
	FString OwnerFactionKey(int FactionIndex)
	{
		// HOLFactionRules::GetFactionKey stays the one source of the keys (C++, exposed as HOLFactionRulesRead::GetFactionKey).
		return FactionIndex < 0 ? "" : HOLFactionRulesRead::GetFactionKey(FactionIndex);
	}

	// The text of the "Sortir tous" button (the grid cell's name of the GDD).
	FString UngarrisonAllLabel()
	{
		return "Sortir tous";
	}

	// The buttons of a view: one per portrait, in order. OccupantTypes is parallel to the view's occupants (FHOLBuildingSelView.GarrisonOccupantTypes);
	// a type missing or empty (this machine does not know the unit) gives no icon path and the unit id as label.
	TArray<FHOLGarrisonButtonView> ButtonsFor(const FHOLGarrisonView& View, const TArray<FString>& OccupantTypes, const FString& OwnerFaction = "")
	{
		TArray<FHOLGarrisonButtonView> Buttons;
		for (int Index = 0; Index < View.PortraitUnitIds.Num(); Index++)
		{
			FHOLGarrisonButtonView Button;
			Button.UnitId = View.PortraitUnitIds[Index];
			Button.TypeKey = Index < OccupantTypes.Num() ? OccupantTypes[Index] : "";
			Button.IconPath = UnitIconPath(Button.TypeKey, OwnerFaction);
			Button.Label = Button.TypeKey != "" ? Button.TypeKey : f"{Button.UnitId}";
			Buttons.Add(Button);
		}
		return Buttons;
	}
	// The widget name of the Index-th portrait button (hol.HUD.Click names it).
	FName PortraitName(int Index)
	{
		return FName(f"GarrisonPortrait{Index}");
	}
}