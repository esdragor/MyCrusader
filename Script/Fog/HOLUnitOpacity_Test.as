// The render opacity setter (UHOLUnitOpacityLibrary, Source/HeroesOfLiberty/Presentation/HOLUnitOpacity.h): the value comes from the real stealth rule
// (HOLStealth::OwnUnitOpacity on a real zone), goes through the setter, and is read back from the place the drawing pass reads. Not a copied constant.

void Test_UnitOpacitySetterCarriesTheStealthRuleValue(FUnitTest& T)
{
	HOLUnitOpacity::ClearStealthOpacities();
	TArray<FHOLStealthZone> Zones;
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(400.0, 400.0);
	Zones.Add(Zone);
	float RuleOpacity = HOLStealth::OwnUnitOpacity(Zones, FVector2D(0.0, 0.0));
	HOLUnitOpacity::SetStealthOpacity(7, RuleOpacity);
	float Drawn = HOLUnitOpacity::GetStealthOpacity(7);
	T.AssertTrue(Math::Abs(Drawn - RuleOpacity) < 0.001, "the unit is drawn with the opacity the rule gave");
	T.AssertTrue(Drawn < 0.99, "a unit in the zone is drawn translucent");
	HOLUnitOpacity::ClearStealthOpacities();
}

void Test_UnitOpacityUnsetUnitIsOpaqueAndOthersAreUntouched(FUnitTest& T)
{
	HOLUnitOpacity::ClearStealthOpacities();
	HOLUnitOpacity::SetStealthOpacity(7, 0.5);
	float Other = HOLUnitOpacity::GetStealthOpacity(8);
	T.AssertTrue(Math::Abs(Other - 1.0) < 0.001, "a unit nobody set is fully opaque");
	HOLUnitOpacity::SetStealthOpacity(7, 1.0);
	float Back = HOLUnitOpacity::GetStealthOpacity(7);
	T.AssertTrue(Math::Abs(Back - 1.0) < 0.001, "setting 1 gives the unit back its full opacity");
	HOLUnitOpacity::ClearStealthOpacities();
}

void Test_StealthApplyOwnUnitsSetsOpacityFromTheRule(FUnitTest& T)
{
	HOLUnitOpacity::ClearStealthOpacities();
	TArray<FHOLStealthZone> Zones;
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(400.0, 400.0);
	Zones.Add(Zone);
	TArray<int> Ids;
	Ids.Add(7);
	Ids.Add(9);
	TArray<FVector2D> Positions;
	Positions.Add(FVector2D(0.0, 0.0));
	Positions.Add(FVector2D(HOLStealth::RevealDistance() * 50.0, 900.0));
	HOLStealthApply::ApplyOwnUnits(Zones, Ids, Positions);
	float InZone = HOLUnitOpacity::GetStealthOpacity(7);
	float OutZone = HOLUnitOpacity::GetStealthOpacity(9);
	T.AssertTrue(InZone < 0.99, "the own unit in the zone is drawn translucent");
	T.AssertTrue(Math::Abs(OutZone - 1.0) < 0.001, "the own unit outside every zone stays opaque");
	HOLUnitOpacity::ClearStealthOpacities();
}
