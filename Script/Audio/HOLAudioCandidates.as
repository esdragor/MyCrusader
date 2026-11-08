// The candidates for the interface sounds (AudioInProgress.md, user's REGLAGE of 2026-11-05: the Kenney interface-sounds pack
// sounds like a spaceship console; the interface must sound medieval fantasy: wood, leather, parchment, dull metal, bell, stone,
// strings). For EACH cue of the table (HOLAudioTable.as), ten variants A to J, listened to one after the other in the scenarios
// audio_compare, audio_compare_orders and audio_compare_alerts, and picked BY EAR by the user (nothing is chosen here).
// Sources, all CC0 (Docs/assets_audio.csv): the Kenney packs rpg-audio and impact-sounds (wood, leather, cloth, paper, doors, metal,
// bells) and a few recordings of VSCO 2 Community Edition (plucked strings, glockenspiel, bells, drum, wood clacks), prepared by
// Tools/Audio/make_ui_candidates.py; the stems starting with "v_" are the VSCO ones. Assets: /Game/Audio/Candidates/<kenney|vsco>/
// SW_<stem> (Tools/Editor/import_audio_candidates.py). The console command hol.Audio.Candidate <Cue> <A-J> plays one.
// Tools/Audio/check_audio_table.ps1 -Candidates verifies every name on disk. Presentation only.

struct FHOLAudioCandidates
{
	// The cue of the table ("Ui.Open").
	FString Cue = "";

	// The ten variants, without extension; the first is A.
	TArray<FString> Stems;
}

namespace HOLAudioCandidates
{
	void Add(TArray<FHOLAudioCandidates>& List, FString Cue, FString StemList)
	{
		FHOLAudioCandidates Candidates;
		Candidates.Cue = Cue;
		StemList.ParseIntoArray(Candidates.Stems, ",");
		List.Add(Candidates);
	}

	// One line per cue of the table, in its order: cue, ten stems.
	void Build(TArray<FHOLAudioCandidates>& List)
	{
		Add(List, "Ui.Click", "impactWood_light_000,impactWood_light_001,metalClick,handleSmallLeather,v_claves_mf,v_slapstick_quiet1,impactTin_medium_000,beltHandle1,bookPlace1,impactPlank_medium_000");
		Add(List, "Ui.Hover", "cloth1,cloth3,clothBelt,bookFlip1,handleSmallLeather2,impactSoft_medium_000,v_triangle_muted,v_harp_A6,beltHandle2,v_claves_mp");
		Add(List, "Ui.Open", "bookOpen,doorOpen_1,doorOpen_2,creak1,creak2,bookFlip2,clothBelt2,v_harp_C5,v_nbell_s01,metalLatch");
		Add(List, "Ui.Close", "bookClose,doorClose_1,doorClose_2,doorClose_3,bookPlace2,dropLeather,impactWood_medium_000,impactPlank_medium_001,metalLatch,v_cello_pizz_D4");
		Add(List, "Ui.Back", "bookFlip3,cloth2,clothBelt,doorClose_4,impactSoft_medium_001,dropLeather,beltHandle1,v_vln_pizz_C5,impactWood_light_002,v_slapstick_quiet2");
		Add(List, "Ui.Error", "impactMetal_heavy_000,impactPlate_heavy_000,impactWood_heavy_000,metalPot1,metalPot2,impactTin_medium_001,v_bass_pizz_E1,v_mallet_hit_mf,creak3,impactPunch_medium_000");
		Add(List, "Ui.Confirm", "v_harp_E5,v_harp_G5,v_vln_pizz_E5,v_glock_G5,handleCoins,impactBell_heavy_000,metalClick,v_nbell_s04,bookPlace3,impactPlate_light_000");
		Add(List, "Ui.Toggle", "metalLatch,metalClick,impactWood_light_003,handleSmallLeather,beltHandle2,v_claves_mf,impactTin_medium_002,bookPlace1,clothBelt2,v_slapstick_quiet1");
		Add(List, "Ui.Tick", "v_claves_mp,impactWood_light_004,metalClick,v_triangle_muted,impactTin_medium_003,v_harp_D6,v_vln_pizz_A5,cloth4,impactPlank_medium_000,beltHandle1");
		Add(List, "Ui.Scroll", "bookFlip1,bookFlip2,bookFlip3,cloth1,cloth2,clothBelt,clothBelt2,cloth3,cloth4,handleSmallLeather2");

		Add(List, "Order.Move", "handleSmallLeather,beltHandle1,impactWood_light_001,v_cello_pizz_D4,v_vln_pizz_A4,cloth1,footstep_wood_000,impactSoft_medium_002,v_claves_mf,v_harp_F6");
		Add(List, "Order.Attack", "drawKnife1,drawKnife2,drawKnife3,knifeSlice,knifeSlice2,impactMetal_light_000,impactMetal_medium_000,impactPlate_medium_000,v_mallet_hit_ff,v_gong_smack_light");
		Add(List, "Order.Gather", "chop,cloth1,impactMining_000,impactWood_medium_001,handleSmallLeather,impactPlank_medium_002,impactMining_001,impactWood_heavy_001,beltHandle2,impactSoft_heavy_000");
		Add(List, "Order.Build", "impactWood_medium_000,impactPlank_medium_000,impactWood_heavy_002,impactMining_002,chop,metalPot3,impactPlate_light_001,impactMetal_light_001,impactWood_light_000,impactTin_medium_000");
		Add(List, "Order.Garrison", "doorClose_1,doorClose_2,doorClose_3,doorClose_4,doorOpen_1,creak2,metalLatch,impactWood_heavy_003,impactPlank_medium_003,v_bass_pizz_E1");

		Add(List, "Select.Unit", "handleSmallLeather,beltHandle1,impactSoft_medium_003,cloth2,v_claves_mp,v_vln_pizz_C5,metalClick,impactTin_medium_004,v_harp_F6,footstep_wood_001");
		Add(List, "Select.Building", "metalLatch,doorOpen_2,impactWood_medium_002,impactPlank_medium_004,creak1,metalPot1,impactMining_003,v_mallet_hit_mf,v_bass_pizz_E1,impactPlate_medium_001");

		Add(List, "Alert.UnderAttack", "s_atk_drum_double,s_atk_drum_deep,s_atk_punch_double,s_atk_wood_double,s_atk_plate_double,s_atk_mining_double,s_atk_bass_double,s_atk_gong_double,s_atk_soft_double,s_atk_metal_double");
		Add(List, "Alert.BuildingDone", "s_bdone_wood_tap,s_bdone_plank,s_bdone_door_close,s_bdone_hammer,s_bdone_mallet,s_bdone_harp_low,s_bdone_cello,s_bdone_bass,s_bdone_stone,s_bdone_book");
		Add(List, "Alert.TrainingDone", "s_train_leather_thud,s_train_wood_knock,s_train_plank_dull,s_train_drum_soft,s_train_bass_pluck,s_train_mallet_dull,s_train_stone_tap,s_train_shield_muffled,s_train_cloth_thump,s_train_book_close");
		Add(List, "Alert.TechDone", "s_tech_book_flip,s_tech_book_place,s_tech_book_open,s_tech_harp_low,s_tech_harp_d4,s_tech_cello,s_tech_wood_soft,s_tech_cloth,s_tech_plate_light,s_tech_knock_double");
		Add(List, "Alert.Trade", "s_trade_purse,s_trade_purse_b,s_trade_leather,s_trade_drop,s_trade_tin,s_trade_wood_double,s_trade_cloth_belt,s_trade_soft,s_trade_coins_double,s_trade_generic");
		Add(List, "Alert.Volcano", "s_volc_timpani_deep,s_volc_gong_low,s_volc_bass,s_volc_heavy_metal,s_volc_punch,s_volc_wood_rumble,s_volc_plate,s_volc_soft_boom,s_volc_creak,s_volc_mining");
		Add(List, "Alert.Diplomacy", "s_dipl_harp_low,s_dipl_harp_arp,s_dipl_cello,s_dipl_bass,s_dipl_book_open,s_dipl_door,s_dipl_cloth,s_dipl_leather_double,s_dipl_creak,s_dipl_wood_double");
		// The elected building selection sounds (2026-11-07): the same ten as Building.Wood / Building.Sacred below (the elected one is F / H).
		Add(List, "Select.BuildingWood", "s_wood_light,s_wood_medium,s_wood_heavy,s_wood_plank,s_wood_chop,s_wood_door,s_wood_creak_low,s_wood_step,s_wood_slap_low,s_wood_deep_knock");
		Add(List, "Select.BuildingSacred", "s_sacred_bass_low,s_sacred_cello,s_sacred_harp_b3,s_sacred_harp_d4,s_sacred_harp_octave,s_sacred_gong_low,s_sacred_drum,s_sacred_book,s_sacred_low_toll,s_sacred_cloth");
		// The Market's selection (Q-audio-market-select = A, 2026-11-08): a sounding cue has its ten too (the same as Building.Coins).
		Add(List, "Select.BuildingCoins", "s_coins_purse,s_coins_leather_b,s_coins_leather,s_coins_drop,s_coins_tin_b,s_coins_counter,s_coins_tin,s_coins_cloth,s_coins_soft,s_coins_slap");
	}

	// The ten candidates of each FAMILY of buildings (AudioBuildingsInProgress.md B4, the user's answer Q3 = A of 2026-11-07: one selection sound per
	// family). Cue names "Building.<Family>" (HOLAudioBuildings::CueNameOfFamily); NOT cues of the sound table (so not counted by CueCount), only
	// listened to in the scenario audio_compare_buildings. Materials: wood and planks, stone (mining, dull hits), bells and harp for the sacred,
	// dull metal, coins with leather purses. Same CC0 packs as above (Kenney rpg-audio and impact-sounds, VSCO 2 CE, Docs/assets_audio.csv).
	void BuildBuildings(TArray<FHOLAudioCandidates>& List)
	{
		Add(List, "Building.Wood", "s_wood_light,s_wood_medium,s_wood_heavy,s_wood_plank,s_wood_chop,s_wood_door,s_wood_creak_low,s_wood_step,s_wood_slap_low,s_wood_deep_knock");
		Add(List, "Building.Stone", "s_stone2_mine_deep,s_stone2_mine_slow,s_stone2_thud_low,s_stone2_punch_deep,s_stone2_timpani,s_stone2_bass,s_stone2_wood_heavy,s_stone2_door_slow,s_stone2_plank_low,s_stone2_gong_dull");
		Add(List, "Building.Sacred", "s_sacred_bass_low,s_sacred_cello,s_sacred_harp_b3,s_sacred_harp_d4,s_sacred_harp_octave,s_sacred_gong_low,s_sacred_drum,s_sacred_book,s_sacred_low_toll,s_sacred_cloth");
		Add(List, "Building.Metal", "s_metal2_anvil_low,s_metal2_plate_deep,s_metal2_pot_deep,s_metal2_pot_three,s_metal2_tin_deep,s_metal2_latch_deep,s_metal2_click_deep,s_metal2_mallet_soft,s_metal2_gong_deep,s_metal2_double");
		Add(List, "Building.Coins", "s_coins_purse,s_coins_leather_b,s_coins_leather,s_coins_drop,s_coins_tin_b,s_coins_counter,s_coins_tin,s_coins_cloth,s_coins_soft,s_coins_slap");
	}

	// The lists, built at each call (script has no mutable global).
	TArray<FHOLAudioCandidates> Table()
	{
		TArray<FHOLAudioCandidates> List;
		Build(List);
		return List;
	}

	int CueCount()
	{
		return Table().Num();
	}

	// The list of a cue by its name ("Ui.Open", or a family cue "Building.Wood"); an empty list (no cue name, no stem) when the table does not know it.
	FHOLAudioCandidates ForCue(FString Cue)
	{
		TArray<FHOLAudioCandidates> List = Table();
		for (int Index = 0; Index < List.Num(); Index++)
			if (List[Index].Cue == Cue) return List[Index];
		TArray<FHOLAudioCandidates> Buildings;
		BuildBuildings(Buildings);
		for (int Index = 0; Index < Buildings.Num(); Index++)
			if (Buildings[Index].Cue == Cue) return Buildings[Index];
		return FHOLAudioCandidates();
	}

	// "A" for the first variant, "B" for the second...; "" outside A-Z.
	FString Label(int Index)
	{
		if (Index < 0 || Index >= 26) return "";
		return FString("ABCDEFGHIJKLMNOPQRSTUVWXYZ").Mid(Index, 1);
	}

	// The index of a label ("c" or "C" gives 2), -1 when it is not a letter.
	int IndexOfLabel(FString Letter)
	{
		if (Letter.Len() != 1) return -1;
		return FString("ABCDEFGHIJKLMNOPQRSTUVWXYZ").Find(Letter.ToUpper());
	}

	// "vsco" for the stems of the VSCO recordings (v_...), "soft" for the softened sounds of make_soft_candidates.py (s_...), "kenney" for the others.
	FString Folder(FString Stem)
	{
		if (Stem.StartsWith("v_")) return "vsco";
		if (Stem.StartsWith("s_")) return "soft";
		return "kenney";
	}

	FString AssetPath(FString Stem)
	{
		return f"/Game/Audio/Candidates/{Folder(Stem)}/SW_{Stem}.SW_{Stem}";
	}

	FString PlayedLine(FString Cue, int Index, FString Stem)
	{
		return f"candidate played {Cue} {Label(Index)} {Stem}";
	}

	FString MissingLine(FString Cue, int Index, FString Asset)
	{
		return f"candidate missing {Cue} {Label(Index)} asset {Asset}";
	}
}

// hol.Audio.Candidate <Cue> <A-J>: plays one candidate in 2D and writes "candidate played <Cue> <letter>
// <stem>" to the log (or "candidate missing ..." when the sound wave is not imported). Used by the audio_compare scenarios, and
// by hand to hear a variant again.
void HOLAudioCandidateCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		Log(n"LogHOLMatch", "candidate usage: hol.Audio.Candidate <Cue> <A-J>");
		return;
	}
	FHOLAudioCandidates List = HOLAudioCandidates::ForCue(Args[0]);
	const int Index = HOLAudioCandidates::IndexOfLabel(Args[1]);
	if (Index < 0 || Index >= List.Stems.Num())
	{
		Log(n"LogHOLMatch", f"candidate unknown {Args[0]} {Args[1]}");
		return;
	}
	const FString Stem = List.Stems[Index];
	const FString Asset = HOLAudioCandidates::AssetPath(Stem);
	USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, Asset));
	if (Sound == nullptr)
	{
		Log(n"LogHOLMatch", HOLAudioCandidates::MissingLine(Args[0], Index, Asset));
		return;
	}
	Gameplay::PlaySound2D(Sound, 1.0, 1.0);
	Log(n"LogHOLMatch", HOLAudioCandidates::PlayedLine(Args[0], Index, Stem));
}

const FConsoleCommand HOLAudioCandidateConsole("hol.Audio.Candidate", n"HOLAudioCandidateCommand");
