// The sounds the user ELECTED by ear in the audio_compare trial (Retours/E-audio_compare.md, answer of 2026-11-06; AudioInProgress.md,
// D736: only the elected sounds are imported and committed, never the 145 candidates). Checked on the real table and on the real plan
// that the game plays (HOLAudio::Plan): the elected cue plays its elected file whatever the seed, the cues the user wants silent
// never play, and the cues he has not answered about keep the table they had.

// Click H, Open F, Close A, Back B, Error A, Confirm G, Select.Unit D: one file each, in the pack it comes from.
void Test_AudioElectedCuesPointAtTheElectedSound(FUnitTest& T)
{
	TArray<int> Events;
	TArray<FString> Packs;
	TArray<FString> Stems;
	Events.Add(HOLAudioEvent::UiClick);   Packs.Add("rpg-audio");     Stems.Add("beltHandle1");
	Events.Add(HOLAudioEvent::UiOpen);    Packs.Add("rpg-audio");     Stems.Add("bookFlip2");
	Events.Add(HOLAudioEvent::UiClose);   Packs.Add("rpg-audio");     Stems.Add("bookClose");
	Events.Add(HOLAudioEvent::SelectUnit); Packs.Add("rpg-audio");    Stems.Add("handleSmallLeather");
	Events.Add(HOLAudioEvent::UiBack);    Packs.Add("rpg-audio");     Stems.Add("cloth2");
	Events.Add(HOLAudioEvent::UiError);   Packs.Add("impact-sounds"); Stems.Add("impactMetal_heavy_000");
	Events.Add(HOLAudioEvent::UiConfirm); Packs.Add("rpg-audio");     Stems.Add("metalClick");
	Events.Add(HOLAudioEvent::AlertUnderAttack); Packs.Add("impact-sounds"); Stems.Add("impactBell_heavy_000");
	Events.Add(HOLAudioEvent::AlertTrainingDone); Packs.Add("rpg-audio"); Stems.Add("bookPlace2");
	Events.Add(HOLAudioEvent::AlertVolcano); Packs.Add("impact-sounds"); Stems.Add("impactBell_heavy_003");
	Events.Add(HOLAudioEvent::AlertDiplomacy); Packs.Add("vsco"); Stems.Add("v_harp_arp_up");
	for (int Index = 0; Index < Events.Num(); Index++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Events[Index]);
		FString Expected = f"/Game/Audio/Kenney/{Packs[Index]}/SW_{Stems[Index]}.SW_{Stems[Index]}";
		if (Packs[Index] == "vsco")
			Expected = f"/Game/Audio/Vsco/SW_{Stems[Index]}.SW_{Stems[Index]}";
		T.AssertEquals(Packs[Index], Cue.Pack, f"{Cue.Name}: the elected pack");
		T.AssertEquals(1, Cue.Stems.Num(), f"{Cue.Name}: only the elected file");
		for (int Seed = -7; Seed < 40; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Events[Index], 5000, -1, Seed, -1);
			T.AssertTrue(Plan.bPlay, f"{Cue.Name}, seed {Seed}: plays");
			T.AssertEquals(Expected, Plan.Asset, f"{Cue.Name}, seed {Seed}: the elected asset");
		}
	}
}

// Hover, Toggle, Tick, Scroll, the five orders, Select.Building (every building will have its own sound later), BuildingDone and TechDone: the user wants NO sound. The cue stays in the table (same numbering) but nothing ever plays, at any
// time, with any seed, and there is no asset to load.
void Test_AudioSilentCuesNeverPlay(FUnitTest& T)
{
	TArray<int> Events;
	Events.Add(HOLAudioEvent::UiHover);
	Events.Add(HOLAudioEvent::UiToggle);
	Events.Add(HOLAudioEvent::UiTick);
	Events.Add(HOLAudioEvent::UiScroll);
	Events.Add(HOLAudioEvent::OrderMove);
	Events.Add(HOLAudioEvent::OrderAttack);
	Events.Add(HOLAudioEvent::OrderGather);
	Events.Add(HOLAudioEvent::OrderBuild);
	Events.Add(HOLAudioEvent::OrderGarrison);
	Events.Add(HOLAudioEvent::SelectBuilding);
	Events.Add(HOLAudioEvent::AlertBuildingDone);
	Events.Add(HOLAudioEvent::AlertTechDone);
	Events.Add(HOLAudioEvent::AlertTrade);   // Q-audio-market-select = A (2026-11-08): the purchase sound moved to the Market's selection, buying and selling is silent
	T.AssertEquals(33, HOLAudio::Count(), "the table has its 24 events, the 2 elected building selections, the Market's, the 3 of D798, the 2 of D832 and the Stable's of D834");
	for (int Index = 0; Index < Events.Num(); Index++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Events[Index]);
		T.AssertEquals(Events[Index], Cue.Event, f"{Cue.Name}: still in the table, same event number");
		T.AssertTrue(Cue.Name.Contains("."), f"{Cue.Name}: still named");
		T.AssertEquals(0, Cue.Stems.Num(), f"{Cue.Name}: no file");
		T.AssertFalse(HOLAudio::MayPlay(Cue, 100000, -1), f"{Cue.Name}: may not play");
		for (int Seed = -7; Seed < 40; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Events[Index], 100000 + Seed * 1000, -1, Seed, -1);
			T.AssertFalse(Plan.bPlay, f"{Cue.Name}, seed {Seed}: nothing plays");
			T.AssertTrue(Plan.Asset.IsEmpty(), f"{Cue.Name}, seed {Seed}: no asset");
		}
	}
}

// The user's answer of 2026-11-07 (E-audio_compare_buildings_wood / _sacred): Wood = F (s_wood_door), Sacred = H (s_sacred_book), both with volume
// x2 to x3. EFFECT on the real plan the game plays: the building of the family rings the elected soft file, loud (the head line
// HOLAudio::BuildingSelectGainPercent), whatever the seed; Stone, Metal and Coins (no election yet) ring nothing; the generic Select.Building stays silent.
void Test_AudioElectedBuildingSelectionWoodAndSacred(FUnitTest& T)
{
	// Q-audio-select-gain = B (2026-11-09): the x2.5 clipped, so the gain is BAKED into limited files (+8 dB then a -1 dBFS limiter, *_lim) played at 100 %.
	T.AssertEquals(100, HOLAudio::BuildingSelectGainPercent, "the limited files ring at 100 %, no gain above 100 left to clip");
	TArray<FString> Buildings;
	Buildings.Add("House");        Buildings.Add("LumberCamp"); Buildings.Add("MiningCamp"); Buildings.Add("Mill"); Buildings.Add("Farm");
	Buildings.Add("Chapel");       Buildings.Add("Sanctuary");
	for (int Index = 0; Index < Buildings.Num(); Index++)
	{
		FString Name = Buildings[Index];
		FString Stem = "s_wood_door_lim";
		if (HOLAudioBuildings::FamilyOf(Name) == "Sacred") Stem = "s_sacred_book_lim";
		int Event = HOLAudioBuildings::SelectEventFor(Name);
		T.AssertTrue(Event >= 0, f"{Name}: selecting rings an elected event");
		for (int Seed = -7; Seed < 20; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, -1);
			T.AssertTrue(Plan.bPlay, f"{Name}, seed {Seed}: plays");
			T.AssertEquals(f"/Game/Audio/Soft/SW_{Stem}.SW_{Stem}", Plan.Asset, f"{Name}, seed {Seed}: the elected soft sound");
			T.AssertEquals(HOLAudio::BuildingSelectGainPercent, Plan.VolumePercent, f"{Name}, seed {Seed}: at the elected gain");
		}
	}
	TArray<FString> Silent;
	Silent.Add("Spaceship");   // MainCentre, SiegeWorkshop, Forge: their own sound (D798); Barracks, ArcheryRange: D832; Stable: D834 (unknown building: silent)
	for (int Index = 0; Index < Silent.Num(); Index++)
	{
		T.AssertEquals(-1, HOLAudioBuildings::SelectEventFor(Silent[Index]), f"{Silent[Index]}: no election yet, no event");
		T.AssertFalse(HOLAudioBuildings::Rings(Silent[Index], HOLBuildingSound::Selected), f"{Silent[Index]}: the generic selection stays silent");
	}
	T.AssertEquals(-1, HOLAudioBuildings::SelectEventFor("Spaceship"), "an unknown building rings nothing");
	T.AssertFalse(HOLAudio::Plan(HOLAudioEvent::SelectBuilding, 5000, -1, 3, -1).bPlay, "the generic Select.Building is still silent");
}

// The user's answer A to Q-audio-market-select (2026-11-08): handleCoins, the old purchase sound, becomes the SELECTION sound of the Market (the
// coins family), at the same head gain as wood and sacred; buying and selling (Alert.Trade) no longer ring. EFFECT on the real plan.
void Test_AudioMarketSelectionRingsTheCoinsAndTradingIsSilent(FUnitTest& T)
{
	int Event = HOLAudioBuildings::SelectEventFor("Market");
	T.AssertEquals(HOLAudioEvent::SelectBuildingCoins, Event, "selecting the Market rings the coins event");
	for (int Seed = -7; Seed < 20; Seed++)
	{
		FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, -1);
		T.AssertTrue(Plan.bPlay, f"Market selection, seed {Seed}: plays");
		T.AssertEquals("/Game/Audio/Soft/SW_s_coins_select_lim.SW_s_coins_select_lim", Plan.Asset, f"Market selection, seed {Seed}: handleCoins, limited (Q-audio-select-gain = B)");
		T.AssertEquals(HOLAudio::BuildingSelectGainPercent, Plan.VolumePercent, f"Market selection, seed {Seed}: at the building selection gain");
		T.AssertFalse(HOLAudio::Plan(HOLAudioEvent::AlertTrade, 5000, -1, Seed, -1).bPlay, f"a trade, seed {Seed}: silent now");
	}
	T.AssertEquals("Alert.Trade", HOLAudio::CueFor(HOLAudioEvent::AlertTrade).Name, "the Alert.Trade cue stays in the table (same numbering)");
}

// D798 (2026-11-14, the user's ear on audio_compare_buildings_stone / _metal, round 3): Siege workshop = Building.Stone A, Forum (MainCentre) =
// Building.Metal B, Forge = Building.Stone G. The elected file is CALCULATED from the letter through the candidate list the user listened to
// (HOLAudioCandidates::ForCue + IndexOfLabel), never copied; EFFECT on the real plan the game plays: selecting the building rings that file, from
// /Game/Audio/Soft (the committed elected assets, not the candidates), at the building selection gain, whatever the seed. Barracks, Archery range and
// Stable stay silent (round 4: voices, shots, a horse, not chosen yet).
void Test_AudioElectedSiegeForumForgeRingTheirLetters(FUnitTest& T)
{
	TArray<FString> Buildings;
	TArray<FString> FamilyCues;
	TArray<FString> Letters;
	Buildings.Add("SiegeWorkshop"); FamilyCues.Add("Building.Stone"); Letters.Add("A");
	Buildings.Add("MainCentre");    FamilyCues.Add("Building.Metal"); Letters.Add("B");
	Buildings.Add("Forge");         FamilyCues.Add("Building.Stone"); Letters.Add("G");
	TArray<int> Seen;
	for (int Index = 0; Index < Buildings.Num(); Index++)
	{
		FString Name = Buildings[Index];
		FHOLAudioCandidates Heard = HOLAudioCandidates::ForCue(FamilyCues[Index]);
		int Letter = HOLAudioCandidates::IndexOfLabel(Letters[Index]);
		T.AssertTrue(Letter >= 0 && Letter < Heard.Stems.Num(), f"{Name}: letter {Letters[Index]} exists in {FamilyCues[Index]}");
		if (Letter < 0 || Letter >= Heard.Stems.Num()) continue;
		FString Stem = Heard.Stems[Letter];
		int Event = HOLAudioBuildings::SelectEventFor(Name);
		T.AssertTrue(Event >= 0, f"{Name}: selecting rings an elected event");
		T.AssertFalse(Seen.Contains(Event), f"{Name}: its own event");
		Seen.Add(Event);
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertEquals("soft", Cue.Pack, f"{Name}: the pack of the elected soft sounds");
		T.AssertEquals(1, Cue.Stems.Num(), f"{Name}: only the elected file");
		for (int Seed = -7; Seed < 20; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, -1);
			T.AssertTrue(Plan.bPlay, f"{Name}, seed {Seed}: plays");
			T.AssertEquals(f"/Game/Audio/Soft/SW_{Stem}.SW_{Stem}", Plan.Asset, f"{Name}, seed {Seed}: {FamilyCues[Index]} {Letters[Index]}");
			T.AssertEquals(HOLAudio::BuildingSelectGainPercent, Plan.VolumePercent, f"{Name}, seed {Seed}: at the building selection gain");
		}
	}
	TArray<FString> Silent;
	Silent.Add("Spaceship");   // D832/D834: every building answered (Barracks, Archery range, Stable: the tests below); an unknown one stays silent
	for (int Index = 0; Index < Silent.Num(); Index++)
		T.AssertEquals(-1, HOLAudioBuildings::SelectEventFor(Silent[Index]), f"{Silent[Index]}: not chosen yet, silent");
}

// D832 (2026-11-16, the user's ear on audio_compare_buildings_range / _barracks, round 6): Archery range = Building.Range B (the round 5 bow shot G,
// clean, no filter), Barracks = Building.Barracks A (one blade drawn, no voice). The elected file is CALCULATED from the letter through the candidate
// list the user listened to (HOLAudioCandidates::ForCue + IndexOfLabel), never copied; EFFECT on the real plan the game plays: selecting the building
// rings that file from /Game/Audio/Soft (the committed elected assets), at the building selection gain, whatever the seed, each on its own event.
void Test_AudioElectedRangeAndBarracksRingTheirRound6Letters(FUnitTest& T)
{
	TArray<FString> Buildings;
	TArray<FString> HeardCues;
	TArray<FString> Letters;
	Buildings.Add("ArcheryRange"); HeardCues.Add("Building.Range");    Letters.Add("B");
	Buildings.Add("Barracks");     HeardCues.Add("Building.Barracks"); Letters.Add("A");
	TArray<int> Seen;
	Seen.Add(HOLAudioBuildings::SelectEventFor("SiegeWorkshop"));
	Seen.Add(HOLAudioBuildings::SelectEventFor("MainCentre"));
	Seen.Add(HOLAudioBuildings::SelectEventFor("Forge"));
	Seen.Add(HOLAudioBuildings::SelectEventFor("House"));
	for (int Index = 0; Index < Buildings.Num(); Index++)
	{
		FString Name = Buildings[Index];
		FHOLAudioCandidates Heard = HOLAudioCandidates::ForCue(HeardCues[Index]);
		int Letter = HOLAudioCandidates::IndexOfLabel(Letters[Index]);
		T.AssertTrue(Letter >= 0 && Letter < Heard.Stems.Num(), f"{Name}: letter {Letters[Index]} exists in {HeardCues[Index]}");
		if (Letter < 0 || Letter >= Heard.Stems.Num()) continue;
		FString Stem = Heard.Stems[Letter];
		int Event = HOLAudioBuildings::SelectEventFor(Name);
		T.AssertTrue(Event >= 0, f"{Name}: selecting rings an elected event");
		T.AssertFalse(Seen.Contains(Event), f"{Name}: its own event, not a family's nor another building's");
		Seen.Add(Event);
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertEquals("soft", Cue.Pack, f"{Name}: the pack of the elected soft sounds");
		T.AssertEquals(1, Cue.Stems.Num(), f"{Name}: only the elected file");
		for (int Seed = -7; Seed < 20; Seed++)
		{
			FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, -1);
			T.AssertTrue(Plan.bPlay, f"{Name}, seed {Seed}: plays");
			T.AssertEquals(f"/Game/Audio/Soft/SW_{Stem}.SW_{Stem}", Plan.Asset, f"{Name}, seed {Seed}: {HeardCues[Index]} {Letters[Index]}");
			T.AssertEquals(HOLAudio::BuildingSelectGainPercent, Plan.VolumePercent, f"{Name}, seed {Seed}: at the building selection gain");
		}
	}
}

// D834 (2026-11-16, the user's answer after audio_compare_buildings_stable_cut): Stable = Building.CutStable A, the snort alone ("pfff") of Stable B
// without its neigh. The elected file is CALCULATED from the letter through the candidate list the user listened to (HOLAudioCandidates::ForCue +
// IndexOfLabel), never copied; EFFECT on the real plan the game plays: selecting a Stable rings that file from /Game/Audio/Soft, at the building
// selection gain, whatever the seed, on its own event (not a family's, not Range's nor Barracks').
void Test_AudioElectedStableRingsTheCutSnort(FUnitTest& T)
{
	FHOLAudioCandidates Heard = HOLAudioCandidates::ForCue("Building.CutStable");
	int Letter = HOLAudioCandidates::IndexOfLabel("A");
	T.AssertTrue(Letter >= 0 && Letter < Heard.Stems.Num(), "Stable: letter A exists in Building.CutStable");
	if (Letter < 0 || Letter >= Heard.Stems.Num()) return;
	FString Stem = Heard.Stems[Letter];
	T.AssertTrue(Stem.Contains("snort"), "Stable: the letter the user chose is the snort, not the whole neigh");
	TArray<int> Others;
	Others.Add(HOLAudioBuildings::SelectEventFor("SiegeWorkshop"));
	Others.Add(HOLAudioBuildings::SelectEventFor("MainCentre"));
	Others.Add(HOLAudioBuildings::SelectEventFor("Forge"));
	Others.Add(HOLAudioBuildings::SelectEventFor("ArcheryRange"));
	Others.Add(HOLAudioBuildings::SelectEventFor("Barracks"));
	Others.Add(HOLAudioBuildings::SelectEventFor("House"));
	int Event = HOLAudioBuildings::SelectEventFor("Stable");
	T.AssertTrue(Event >= 0, "Stable: selecting rings an elected event");
	T.AssertFalse(Others.Contains(Event), "Stable: its own event, not a family's nor another building's");
	if (Event < 0) return;
	FHOLAudioCue Cue = HOLAudio::CueFor(Event);
	T.AssertEquals("soft", Cue.Pack, "Stable: the pack of the elected soft sounds");
	T.AssertEquals(1, Cue.Stems.Num(), "Stable: only the elected file");
	for (int Seed = -7; Seed < 20; Seed++)
	{
		FHOLAudioPlan Plan = HOLAudio::Plan(Event, 5000, -1, Seed, -1);
		T.AssertTrue(Plan.bPlay, f"Stable, seed {Seed}: plays");
		T.AssertEquals(f"/Game/Audio/Soft/SW_{Stem}.SW_{Stem}", Plan.Asset, f"Stable, seed {Seed}: Building.CutStable A");
		T.AssertEquals(HOLAudio::BuildingSelectGainPercent, Plan.VolumePercent, f"Stable, seed {Seed}: at the building selection gain");
	}
}

// Every cue is answered now: none is left on the spaceship console packs the user rejected.
void Test_AudioNoSpaceshipPackIsLeft(FUnitTest& T)
{
	for (int Event = 0; Event < HOLAudio::Count(); Event++)
	{
		FHOLAudioCue Cue = HOLAudio::CueFor(Event);
		T.AssertTrue(Cue.Pack != "interface-sounds" && Cue.Pack != "ui-audio", f"{Cue.Name}: not a spaceship console sound");
	}
}

// The volcano's bell rings several times (the user: "mais plusieurs fois"): 3 times, about 1 s apart; the other cues ring once.
void Test_AudioVolcanoRepeats(FUnitTest& T)
{
	FHOLAudioCue Volcano = HOLAudio::CueFor(HOLAudioEvent::AlertVolcano);
	T.AssertEquals(3, Volcano.RepeatCount, "the volcano rings 3 times");
	T.AssertEquals(1000, Volcano.RepeatGapMs, "about 1 s apart");
	FHOLAudioPlan Plan = HOLAudio::Plan(HOLAudioEvent::AlertVolcano, 20000, -1, 5, -1);
	T.AssertEquals(3, Plan.RepeatCount, "the plan carries the repeats");
	T.AssertEquals(1000, Plan.RepeatGapMs, "and their gap");
	// The clock: the first ring at 10000 (one done); nothing before 11000; one more at 11000; the third at 12000; never a fourth.
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 10999, 1), "999 ms later: not yet");
	T.AssertEquals(1, HOLAudio::RepeatsDue(Volcano, 10000, 11000, 1), "1 s later: the second ring");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 11999, 2), "after the second: waits");
	T.AssertEquals(1, HOLAudio::RepeatsDue(Volcano, 10000, 12000, 2), "2 s later: the third ring");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 99999, 3), "never a fourth");
	T.AssertEquals(2, HOLAudio::RepeatsDue(Volcano, 10000, 99999, 1), "a long freeze: the two missing rings");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Volcano, 10000, 5000, 1), "a clock that went back: nothing");
	FHOLAudioCue Attack = HOLAudio::CueFor(HOLAudioEvent::AlertUnderAttack);
	T.AssertEquals(1, Attack.RepeatCount, "the attack bell rings once");
	T.AssertEquals(0, HOLAudio::RepeatsDue(Attack, 10000, 50000, 1), "no repeat for a cue that rings once");
}
