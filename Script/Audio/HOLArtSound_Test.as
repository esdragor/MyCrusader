// AR6-a: the sound of a type read from its art asset before the table (HOLArtSound.as), on test assets of the registry.

// The faction's asset first, the type's common asset for another faction, and field by field (the variant without Order: the common Order).
void Test_ArtSoundFactionFirstThenCommon(FUnitTest& T)
{
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualSound(n"Spearman", NAME_None, n"Selection", "/Game/Art/Units/Spearman/S_Select.S_Select");
	UHOLArtVisualRegistry::AddTestArtVisualSound(n"Spearman", n"Legions", n"Selection", "/Game/Art/Units/Spearman/S_Select_Legions.S_Select_Legions");
	const FString Own = HOLArtSound::PathFor(nullptr, n"Spearman", n"Legions", n"Selection");
	const FString Other = HOLArtSound::PathFor(nullptr, n"Spearman", n"Dawn", n"Selection");
	const FString NoSlot = HOLArtSound::PathFor(nullptr, n"Spearman", n"Legions", n"Death");
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(Own, "/Game/Art/Units/Spearman/S_Select_Legions.S_Select_Legions", "the faction's variant first");
	T.AssertEquals(Other, "/Game/Art/Units/Spearman/S_Select.S_Select", "another faction: the common asset");
	T.AssertEquals(NoSlot, "", "no asset sets Death: empty");
}

// A variant that sets only its Selection: its Order comes from the common asset (field by field, not asset by asset).
void Test_ArtSoundFieldByField(FUnitTest& T)
{
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualSound(n"Spearman", NAME_None, n"Order", "/Game/Art/Units/Spearman/S_Order.S_Order");
	UHOLArtVisualRegistry::AddTestArtVisualSound(n"Spearman", n"Legions", n"Selection", "/Game/Art/Units/Spearman/S_Select_Legions.S_Select_Legions");
	const FString Order = HOLArtSound::PathFor(nullptr, n"Spearman", n"Legions", n"Order");
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(Order, "/Game/Art/Units/Spearman/S_Order.S_Order", "the variant has no Order: the common one");
}

// The art sound is chosen before the table's; a type without art sound falls back on the table.
void Test_ArtSoundBeforeTheTable(FUnitTest& T)
{
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	UHOLArtVisualRegistry::AddTestArtVisualSound(n"Barracks", NAME_None, n"Selection", "/Game/Art/Buildings/Barracks/S_Select.S_Select");
	const FString FromArt = HOLArtSound::AssetToPlay(nullptr, n"Barracks", NAME_None, n"Selection", "/Game/Audio/Kenney/table.table");
	const FString FromTable = HOLArtSound::AssetToPlay(nullptr, n"Forum", NAME_None, n"Selection", "/Game/Audio/Kenney/table.table");
	UHOLArtVisualRegistry::ClearTestArtVisuals();
	T.AssertEquals(FromArt, "/Game/Art/Buildings/Barracks/S_Select.S_Select", "the art asset's sound before the table");
	T.AssertEquals(FromTable, "/Game/Audio/Kenney/table.table", "no art sound: the table's");
}
