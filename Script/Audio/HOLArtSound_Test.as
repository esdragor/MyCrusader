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

// AR6-b: 200 units striking at once start at most MaxCombatVoices typed combat sounds (count computed by the limiter, not copied).
void Test_ArtSoundVoiceLimit(FUnitTest& T)
{
	TArray<float> Ends;
	int Started = 0;
	for (int Index = 0; Index < 200; Index++)
	{
		if (HOLArtSound::TryStartVoice(Ends, 10.0, HOLArtSound::CombatVoiceSeconds, HOLArtSound::MaxCombatVoices))
			Started++;
	}
	const int Playing = Ends.Num();
	T.AssertEquals(Started, HOLArtSound::MaxCombatVoices, "200 strikes at once: only the voice limit starts");
	T.AssertEquals(Playing, HOLArtSound::MaxCombatVoices, "the counted voices stay at the limit");
}

// AR6-b: a voice frees its place when it ends: full at 0.5 s, room again at 1.5 s (the ended ones dropped).
void Test_ArtSoundVoiceFreesWithTime(FUnitTest& T)
{
	TArray<float> Ends;
	for (int Index = 0; Index < 3; Index++)
		HOLArtSound::TryStartVoice(Ends, 0.0, 1.0, 3);
	const bool bWhileFull = HOLArtSound::TryStartVoice(Ends, 0.5, 1.0, 3);
	const bool bAfterEnd = HOLArtSound::TryStartVoice(Ends, 1.5, 1.0, 3);
	const int Playing = Ends.Num();
	T.AssertFalse(bWhileFull, "three playing, limit three: refused");
	T.AssertTrue(bAfterEnd, "the three ended: a new voice starts");
	T.AssertEquals(Playing, 1, "only the new voice is counted");
}

// AR6-d: an order's kind gives the table's order event (the five are silent by the user's choice: only an art asset's Order sound is heard).
void Test_ArtSoundOrderEventForKind(FUnitTest& T)
{
	T.AssertEquals(HOLAudioEvent::OrderMove, HOLArtSound::OrderEventFor(n"Move"), "Move order");
	T.AssertEquals(HOLAudioEvent::OrderAttack, HOLArtSound::OrderEventFor(n"Attack"), "Attack order");
	T.AssertEquals(HOLAudioEvent::OrderGather, HOLArtSound::OrderEventFor(n"Gather"), "Gather order");
	T.AssertEquals(HOLAudioEvent::OrderBuild, HOLArtSound::OrderEventFor(n"Build"), "Build order");
	T.AssertEquals(HOLAudioEvent::OrderGarrison, HOLArtSound::OrderEventFor(n"Garrison"), "Garrison order");
	T.AssertEquals(-1, HOLArtSound::OrderEventFor(n"Place"), "a foundation laid has no table cue");
	T.AssertEquals(-1, HOLArtSound::OrderEventFor(n"Dance"), "an unknown kind has none");
}

// AR6-d: a foundation laid rings the building's Construction sound, every other order the unit's Order sound.
void Test_ArtSoundOrderSlotForKind(FUnitTest& T)
{
	T.AssertTrue(HOLArtSound::OrderSlotFor(n"Place") == n"Construction", "Place -> Construction");
	T.AssertTrue(HOLArtSound::OrderSlotFor(n"Move") == n"Order", "Move -> Order");
	T.AssertTrue(HOLArtSound::OrderSlotFor(n"Garrison") == n"Order", "Garrison -> Order");
}
