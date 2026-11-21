// The render opacity setter (UHOLUnitOpacityLibrary, Source/HeroesOfLiberty/Presentation/HOLUnitOpacity.h): the value comes from the real stealth rule
// (HOLStealth::OwnUnitOpacity on a real zone), goes through the setter, and is read back from the place the drawing pass reads. Not a copied constant.

void Test_UnitOpacitySetterCarriesTheStealthRuleValue(FUnitTest& T)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	TArray<FHOLStealthZone> Zones;
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(400.0, 400.0);
	Zones.Add(Zone);
	float RuleOpacity = HOLStealth::OwnUnitOpacity(Zones, FVector2D(0.0, 0.0));
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, RuleOpacity);
	float Drawn = UHOLUnitOpacityLibrary::GetStealthOpacity(7);
	T.AssertTrue(Math::Abs(Drawn - RuleOpacity) < 0.001, "the unit is drawn with the opacity the rule gave");
	T.AssertTrue(Drawn < 0.99, "a unit in the zone is drawn translucent");
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
}

void Test_UnitOpacityUnsetUnitIsOpaqueAndOthersAreUntouched(FUnitTest& T)
{
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 0.5);
	float Other = UHOLUnitOpacityLibrary::GetStealthOpacity(8);
	T.AssertTrue(Math::Abs(Other - 1.0) < 0.001, "a unit nobody set is fully opaque");
	UHOLUnitOpacityLibrary::SetStealthOpacity(7, 1.0);
	float Back = UHOLUnitOpacityLibrary::GetStealthOpacity(7);
	T.AssertTrue(Math::Abs(Back - 1.0) < 0.001, "setting 1 gives the unit back its full opacity");
	UHOLUnitOpacityLibrary::ClearStealthOpacities();
}
