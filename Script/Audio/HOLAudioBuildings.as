// The table "building -> sound" (AudioBuildingsInProgress.md step 1; the user's idea of 2026-11-06: one sound per type of building). For each type
// of Tools/Data/BuildingTypes.csv, the audio event (HOLAudioEvent, HOLAudioTable.as) it rings at each kind of moment: selected, built, produced,
// destroyed. Pure: no sound played, nothing read from the game; the code that plays asks HOLAudioBuildings::EventFor / CueFor.
// STEP 1 changes nothing that is heard: every building points at the event the game rings today, so the generic Select.Building and
// Alert.BuildingDone stay SILENT (the user wanted them silent on 2026-11-06 and keeps them so until he asks again), a trained unit rings
// Alert.TrainingDone, a research Alert.TechDone (silent), a trade Alert.Trade, and nothing rings a destruction (no such event yet).
// The day a building gets its own sound, only its line changes here (and the table of HOLAudioTable.as gets a new cue).

// The moments a building can ring a sound at.
namespace HOLBuildingSound
{
	// The player selects the building.
	const int Selected = 0;
	// The construction ends.
	const int Built = 1;
	// The building finishes what it produces (a trained unit, a researched technology, a trade).
	const int Produced = 2;
	// The building is destroyed.
	const int Destroyed = 3;
	// The number of kinds (they are numbered from 0 without a gap).
	const int Count = 4;
}

// One line of the table: the type name as BuildingTypes.csv writes it, and the event for each kind (-1: no event).
struct FHOLBuildingSounds
{
	FString Building = "";
	TArray<int> Events;
}

// One family of buildings: its name and the name of its cue (AudioBuildingsInProgress.md B3/B4).
struct FHOLBuildingFamily
{
	FString Family = "";
	FString Cue = "";
}

namespace HOLAudioBuildings
{
	// One building: its production event, and the other kinds at today's defaults (generic selection and construction cues, nothing for a destruction).
	void Add(TArray<FHOLBuildingSounds>& Rows, FString Building, int ProducedEvent)
	{
		FHOLBuildingSounds Row;
		Row.Building = Building;
		Row.Events.Add(HOLAudioEvent::SelectBuilding);
		Row.Events.Add(HOLAudioEvent::AlertBuildingDone);
		Row.Events.Add(ProducedEvent);
		Row.Events.Add(-1);
		Rows.Add(Row);
	}

	// One line per type of BuildingTypes.csv, in its order: building, event rung when its production ends (-1: it produces nothing).
	void Build(TArray<FHOLBuildingSounds>& Rows)
	{
		Add(Rows, "MainCentre", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "House", -1);
		Add(Rows, "LumberCamp", -1);
		Add(Rows, "MiningCamp", -1);
		Add(Rows, "Mill", -1);
		Add(Rows, "Barracks", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "Farm", -1);
		Add(Rows, "Sanctuary", HOLAudioEvent::AlertTechDone);
		Add(Rows, "ArcheryRange", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "Market", HOLAudioEvent::AlertTrade);
		Add(Rows, "SiegeWorkshop", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "Chapel", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "Stable", HOLAudioEvent::AlertTrainingDone);
		Add(Rows, "Forge", HOLAudioEvent::AlertTechDone);
	}

	// The table, built at each call (script has no mutable global).
	TArray<FHOLBuildingSounds> Table()
	{
		TArray<FHOLBuildingSounds> Rows;
		Build(Rows);
		return Rows;
	}

	int Count()
	{
		return Table().Num();
	}

	// The type name of line Index ("" when out of range).
	FString NameAt(int Index)
	{
		TArray<FHOLBuildingSounds> Rows = Table();
		if (Index < 0 || Index >= Rows.Num()) return "";
		return Rows[Index].Building;
	}

	bool Has(FString Building)
	{
		TArray<FHOLBuildingSounds> Rows = Table();
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (Rows[Index].Building == Building) return true;
		}
		return false;
	}

	// The audio event a building rings at a kind of moment (HOLBuildingSound); -1 for an unknown building, an unknown kind, or no event.
	int EventFor(FString Building, int Kind)
	{
		if (Kind < 0 || Kind >= HOLBuildingSound::Count) return -1;
		TArray<FHOLBuildingSounds> Rows = Table();
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (Rows[Index].Building == Building) return Rows[Index].Events[Kind];
		}
		return -1;
	}

	// The cue of that event in the sound table (an empty cue when there is no event).
	FHOLAudioCue CueFor(FString Building, int Kind)
	{
		return HOLAudio::CueFor(EventFor(Building, Kind));
	}

	// Does it make a sound? (An event whose cue is silent, or no event, does not.)
	bool Rings(FString Building, int Kind)
	{
		return CueFor(Building, Kind).Stems.Num() > 0;
	}

	// ---- Families (the user's answer Q3 = A of 2026-11-07: ONE selection sound per FAMILY of buildings, about five sounds) ----
	// DATA only, nothing chosen, nothing heard: the family cues ("Building.Wood"...) are candidates the user listens to in the scenario
	// audio_compare_buildings (HOLAudioCandidates.as); they are NOT events of the sound table, so Select.Building stays silent until a
	// sound is elected and wired in C++ (step B5). Construction done, production and destruction stay silent.

	void AddFamily(TArray<FHOLBuildingFamily>& Rows, FString Family)
	{
		FHOLBuildingFamily Row;
		Row.Family = Family;
		Row.Cue = "Building." + Family;
		Rows.Add(Row);
	}

	// The table "family -> cue": wood, stone, sacred, metal, coins.
	TArray<FHOLBuildingFamily> Families()
	{
		TArray<FHOLBuildingFamily> Rows;
		AddFamily(Rows, "Wood");
		AddFamily(Rows, "Stone");
		AddFamily(Rows, "Sacred");
		AddFamily(Rows, "Metal");
		AddFamily(Rows, "Coins");
		return Rows;
	}

	int FamilyCount()
	{
		return Families().Num();
	}

	// The name of family Index ("" when out of range).
	FString FamilyNameAt(int Index)
	{
		TArray<FHOLBuildingFamily> Rows = Families();
		if (Index < 0 || Index >= Rows.Num()) return "";
		return Rows[Index].Family;
	}

	bool HasFamily(FString Family)
	{
		TArray<FHOLBuildingFamily> Rows = Families();
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (Rows[Index].Family == Family) return true;
		}
		return false;
	}

	// The cue name of a family ("" for an unknown family).
	FString CueNameOfFamily(FString Family)
	{
		TArray<FHOLBuildingFamily> Rows = Families();
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (Rows[Index].Family == Family) return Rows[Index].Cue;
		}
		return "";
	}

	// The table "building -> family", one line per type of BuildingTypes.csv ("" for an unknown building).
	FString FamilyOf(FString Building)
	{
		if (Building == "House" || Building == "LumberCamp" || Building == "MiningCamp" || Building == "Mill" || Building == "Farm") return "Wood";
		if (Building == "MainCentre" || Building == "Barracks" || Building == "ArcheryRange" || Building == "Stable" || Building == "SiegeWorkshop") return "Stone";
		if (Building == "Chapel" || Building == "Sanctuary") return "Sacred";
		if (Building == "Forge") return "Metal";
		if (Building == "Market") return "Coins";
		return "";
	}

	// The audio event of a family's elected selection sound (HOLAudioEvent::SelectBuildingWood / SelectBuildingSacred); -1 while the user has not
	// elected one for the family (Stone, Metal: ten more candidates asked; Coins: the purchase sound, see Q-audio-market-select) or for an unknown family.
	int SelectEventOfFamily(FString Family)
	{
		if (Family == "Wood") return HOLAudioEvent::SelectBuildingWood;
		if (Family == "Sacred") return HOLAudioEvent::SelectBuildingSacred;
		if (Family == "Coins") return HOLAudioEvent::SelectBuildingCoins;   // Q-audio-market-select = A
		return -1;
	}

	// The audio event rung when the player selects this building (-1: silent, or an unknown building). The code that plays (step B5, C++) asks this.
	int SelectEventFor(FString Building)
	{
		return SelectEventOfFamily(FamilyOf(Building));
	}

	// The family cue name of a building ("" for an unknown building).
	FString CueNameFor(FString Building)
	{
		return CueNameOfFamily(FamilyOf(Building));
	}
}
