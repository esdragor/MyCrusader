// The table "event -> sound" (AudioInProgress.md A2, D483, D484): for each event of the interface, the orders, the selection
// and the alerts, one cue = the pack and the files of the Kenney CC0 sounds (kept outside the repository, Docs/assets_audio.csv),
// a volume, a pitch range, a cooldown and a priority. Sounds are presentation: they are played when the state is received, like
// the other visual feedback, so nothing here touches the simulation or the network. Pure functions of a seed and of a clock in
// milliseconds (no random inside, no sound played here); the code that plays (A2 next step) asks this table.
// The file names come from the packs' own names and are NOT listened to yet; the check Tools/Audio/check_audio_table.ps1 reads
// this file and verifies that every pack and file exists on disk. Choices by the user's ear (E-audio_compare, E-audio_compare_orders,
// 2026-11-06): the elected cues have ONE file; a SILENT cue (empty pack and files: Hover, Toggle, Tick, Scroll, the five orders, the generic
// Select.Building, each building will have its own sound later) stays in the table but never plays. The alerts follow audio_compare_alerts: UnderAttack,
// Trade, Volcano (rung 3 times) and Diplomacy elected, TrainingDone takes TechDone's variant G, BuildingDone and TechDone silent.

// What a cue says.
struct FHOLAudioCue
{
	// The event it answers (HOLAudioEvent), -1 for no cue.
	int Event = -1;

	// "Group.Name"; the group is Ui, Order, Select or Alert. Empty: no cue.
	FString Name = "";

	// The pack folder under MyCrusader_Audio\Kenney (ui-audio, interface-sounds, rpg-audio, impact-sounds).
	FString Pack = "";

	// The files of the variants, without extension.
	TArray<FString> Stems;

	// 1 to 100.
	int VolumePercent = 100;

	// The pitch is drawn between these two (100 = unchanged), by the code that plays.
	int PitchMinPercent = 100;
	int PitchMaxPercent = 100;

	// The same cue is not played again before this delay (milliseconds).
	int CooldownMs = 0;

	// HOLAudioPriority: what stays when the sound budget is full (A3).
	int Priority = 0;

	// The cue rings this many times in a row (1 = once), RepeatGapMs apart (the volcano's bell, HOLAudio::SetRepeat).
	int RepeatCount = 1;
	int RepeatGapMs = 0;
}

namespace HOLAudioPriority
{
	const int Ui = 0;
	const int Order = 1;
	const int Alert = 2;
}

namespace HOLAudioEvent
{
	// Interface.
	const int UiClick = 0;
	const int UiHover = 1;
	const int UiOpen = 2;
	const int UiClose = 3;
	const int UiBack = 4;
	const int UiError = 5;
	const int UiConfirm = 6;
	const int UiToggle = 7;
	const int UiTick = 8;
	const int UiScroll = 9;
	// Orders given to the units.
	const int OrderMove = 10;
	const int OrderAttack = 11;
	const int OrderGather = 12;
	const int OrderBuild = 13;
	const int OrderGarrison = 14;
	// Selection.
	const int SelectUnit = 15;
	const int SelectBuilding = 16;
	// Alerts.
	const int AlertUnderAttack = 17;
	const int AlertBuildingDone = 18;
	const int AlertTrainingDone = 19;
	const int AlertTechDone = 20;
	const int AlertTrade = 21;
	const int AlertVolcano = 22;
	const int AlertDiplomacy = 23;
	// The selection of a building by FAMILY (AudioBuildingsInProgress.md; user's answer of 2026-11-07, E-audio_compare_buildings_*): only the
	// families he elected are events. Wood = F (s_wood_door), Sacred = H (s_sacred_book). Stone, Metal (no letter, ten more candidates) and Coins
	// (reuse the purchase sound, Q-audio-market-select) are NOT events yet. The generic SelectBuilding above stays silent.
	const int SelectBuildingWood = 24;
	const int SelectBuildingSacred = 25;
	// The Market's selection (Q-audio-market-select = A, 2026-11-08): handleCoins, the old purchase sound, moved here; Alert.Trade is now silent.
	const int SelectBuildingCoins = 26;
}

namespace HOLAudio
{
	// The volcano's bell rings several times (the user's answer to audio_compare_alerts: "Alert Volcano A, mais plusieurs fois"). THE setting:
	// how many times and how far apart (milliseconds).
	const int VolcanoRepeatCount = 3;
	const int VolcanoRepeatGapMs = 1000;

	// THE volume setting of the elected building selection sounds (wood F, sacred H): the user heard them in the trial at volume 100 % and
	// asked for "volume x2 to x3" (2026-11-07). 250 = x2.5, the middle; ONE line to change (200 = x2, 300 = x3). Above 100 on purpose.
	// Q-audio-select-gain = B (2026-11-09): at x2.5 the sounds CLIPPED (+4 dBFS Wood and Sacred, +8 dBFS the Market's coins). The gain is now BAKED into
	// limited files (Tools/Audio/make_limited_selects.py: +8 dB then a -1 dBFS limiter, stems *_lim) played at 100 %. BuildingSelectOldGainPercent is the
	// OLD gain, kept only for the before / after scenario audio_select_gain (hol.Audio.SelectGain) ; the old files stay imported for it.
	const int BuildingSelectGainPercent = 100;
	const int BuildingSelectOldGainPercent = 250;

	void Add(TArray<FHOLAudioCue>& Cues, int Event, FString Name, FString Pack, FString StemList, int Volume, int PitchMin, int PitchMax, int CooldownMs, int Priority)
	{
		FHOLAudioCue Cue;
		Cue.Event = Event;
		Cue.Name = Name;
		Cue.Pack = Pack;
		StemList.ParseIntoArray(Cue.Stems, ",");
		Cue.VolumePercent = Volume;
		Cue.PitchMinPercent = PitchMin;
		Cue.PitchMaxPercent = PitchMax;
		Cue.CooldownMs = CooldownMs;
		Cue.Priority = Priority;
		Cues.Add(Cue);
	}

	// Makes a cue ring Count times in a row, GapMs apart.
	void SetRepeat(TArray<FHOLAudioCue>& Cues, int Event, int Count, int GapMs)
	{
		Cues[Event].RepeatCount = Count;
		Cues[Event].RepeatGapMs = GapMs;
	}

	// One line per event, in the order of HOLAudioEvent: event, name, pack, files, volume, pitch min, pitch max, cooldown, priority.
	void Build(TArray<FHOLAudioCue>& Cues)
	{
		Add(Cues, HOLAudioEvent::UiClick, "Ui.Click", "rpg-audio", "beltHandle1", 80, 97, 103, 40, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiHover, "Ui.Hover", "", "", 40, 100, 100, 80, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiOpen, "Ui.Open", "rpg-audio", "bookFlip2", 70, 100, 100, 100, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiClose, "Ui.Close", "rpg-audio", "bookClose", 70, 100, 100, 100, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiBack, "Ui.Back", "rpg-audio", "cloth2", 70, 100, 100, 100, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiError, "Ui.Error", "impact-sounds", "impactMetal_heavy_000", 80, 100, 100, 300, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiConfirm, "Ui.Confirm", "rpg-audio", "metalClick", 80, 100, 100, 100, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiToggle, "Ui.Toggle", "", "", 60, 100, 100, 60, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiTick, "Ui.Tick", "", "", 50, 100, 100, 40, HOLAudioPriority::Ui);
		Add(Cues, HOLAudioEvent::UiScroll, "Ui.Scroll", "", "", 40, 100, 100, 50, HOLAudioPriority::Ui);

		Add(Cues, HOLAudioEvent::OrderMove, "Order.Move", "", "", 55, 95, 105, 120, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::OrderAttack, "Order.Attack", "", "", 70, 95, 105, 150, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::OrderGather, "Order.Gather", "", "", 60, 95, 105, 150, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::OrderBuild, "Order.Build", "", "", 65, 95, 105, 150, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::OrderGarrison, "Order.Garrison", "", "", 60, 95, 105, 200, HOLAudioPriority::Order);

		Add(Cues, HOLAudioEvent::SelectUnit, "Select.Unit", "rpg-audio", "handleSmallLeather", 55, 97, 103, 60, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::SelectBuilding, "Select.Building", "", "", 60, 97, 103, 60, HOLAudioPriority::Order);

		Add(Cues, HOLAudioEvent::AlertUnderAttack, "Alert.UnderAttack", "impact-sounds", "impactBell_heavy_000", 100, 100, 100, 10000, HOLAudioPriority::Alert);
		Add(Cues, HOLAudioEvent::AlertBuildingDone, "Alert.BuildingDone", "", "", 80, 100, 100, 500, HOLAudioPriority::Alert);
		Add(Cues, HOLAudioEvent::AlertTrainingDone, "Alert.TrainingDone", "rpg-audio", "bookPlace2", 80, 100, 100, 400, HOLAudioPriority::Alert);
		Add(Cues, HOLAudioEvent::AlertTechDone, "Alert.TechDone", "", "", 85, 100, 100, 1000, HOLAudioPriority::Alert);
		Add(Cues, HOLAudioEvent::AlertTrade, "Alert.Trade", "", "", 80, 100, 100, 200, HOLAudioPriority::Alert);   // silent since Q-audio-market-select = A
		Add(Cues, HOLAudioEvent::AlertVolcano, "Alert.Volcano", "impact-sounds", "impactBell_heavy_003", 100, 100, 100, 5000, HOLAudioPriority::Alert);
		Add(Cues, HOLAudioEvent::AlertDiplomacy, "Alert.Diplomacy", "vsco", "v_harp_arp_up", 80, 100, 100, 1000, HOLAudioPriority::Alert);
		SetRepeat(Cues, HOLAudioEvent::AlertVolcano, VolcanoRepeatCount, VolcanoRepeatGapMs);

		// The elected building sounds (soft files made by Tools/Audio/make_soft_candidates.py, pack "soft"): volume = BuildingSelectGainPercent.
		Add(Cues, HOLAudioEvent::SelectBuildingWood, "Select.BuildingWood", "soft", "s_wood_door_lim", BuildingSelectGainPercent, 97, 103, 60, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::SelectBuildingSacred, "Select.BuildingSacred", "soft", "s_sacred_book_lim", BuildingSelectGainPercent, 97, 103, 60, HOLAudioPriority::Order);
		Add(Cues, HOLAudioEvent::SelectBuildingCoins, "Select.BuildingCoins", "soft", "s_coins_select_lim", BuildingSelectGainPercent, 97, 103, 60, HOLAudioPriority::Order);
	}

	// The table, built at each call (script has no mutable global; 24 small structs, a sound is not played 100 times a frame).
	TArray<FHOLAudioCue> Table()
	{
		TArray<FHOLAudioCue> Cues;
		Build(Cues);
		return Cues;
	}

	// The number of events (they are numbered from 0 without a gap).
	int Count()
	{
		return Table().Num();
	}

	bool IsValid(int Event)
	{
		return Event >= 0 && Event < Count();
	}

	// The cue of an event; an empty cue (no name, no file, Event -1) when the event is unknown.
	FHOLAudioCue CueFor(int Event)
	{
		if (!IsValid(Event)) return FHOLAudioCue();
		return Table()[Event];
	}

	// The group of an event: the name's part before the dot ("" when unknown).
	FString GroupOf(int Event)
	{
		FString Name = CueFor(Event).Name;
		int Dot = Name.Find(".");
		if (Dot < 0) return "";
		return Name.Left(Dot);
	}

	// The file of a cue chosen by a seed (any whole number, negative included): the same seed gives the same file; -1 when the
	// cue has no file.
	int Pick(FHOLAudioCue Cue, int Seed)
	{
		const int Num = Cue.Stems.Num();
		if (Num <= 0) return -1;
		int Index = Seed % Num;
		if (Index < 0) Index += Num;
		return Index;
	}

	// As Pick, but never the file just played (Last, -1 for none) when the cue has another one.
	int PickNext(FHOLAudioCue Cue, int Seed, int Last)
	{
		int Index = Pick(Cue, Seed);
		if (Index >= 0 && Index == Last && Cue.Stems.Num() > 1) Index = (Index + 1) % Cue.Stems.Num();
		return Index;
	}

	// The asset of a file of a cue (the sounds are imported under /Game/Audio/Kenney/<pack>/SW_<file>, like SW_HeroCry); "" when
	// there is no such file.
	FString SoundPath(FHOLAudioCue Cue, int Index)
	{
		if (Index < 0 || Index >= Cue.Stems.Num()) return "";
		// The VSCO 2 CE recordings (CC0) are not a Kenney pack: /Game/Audio/Vsco.
		if (Cue.Pack == "vsco") return f"/Game/Audio/Vsco/SW_{Cue.Stems[Index]}.SW_{Cue.Stems[Index]}";
		// The softened sounds the user elected (made from CC0 recordings, Tools/Audio/make_soft_candidates.py): /Game/Audio/Soft.
		if (Cue.Pack == "soft") return f"/Game/Audio/Soft/SW_{Cue.Stems[Index]}.SW_{Cue.Stems[Index]}";
		return f"/Game/Audio/Kenney/{Cue.Pack}/SW_{Cue.Stems[Index]}.SW_{Cue.Stems[Index]}";
	}

	// May the cue play now? Now and Last in milliseconds on the same clock; Last -1 = never played. A clock that went back (a new
	// match) plays. An empty cue never plays.
	bool MayPlay(FHOLAudioCue Cue, int NowMs, int LastMs)
	{
		if (Cue.Name.IsEmpty() || Cue.Stems.Num() == 0) return false;
		if (LastMs < 0) return true;
		if (NowMs < LastMs) return true;
		return NowMs - LastMs >= Cue.CooldownMs;
	}
}
