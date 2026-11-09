// Before / after of the building selection sounds (Q-audio-select-gain = B, 2026-11-09; scenario audio_select_gain, AudioBuildingsInProgress.md).
//
// OLD = the file at the old gain (BuildingSelectOldGainPercent, x2.5: it CLIPPED), NEW = what the game plays now, read from the REAL table through
// HOLAudio::Plan (the limited file at 100 %). hol.Audio.SelectGain <wood|sacred|coins> <old|new> plays one of the two in 2D and writes
// "select gain played <family> <version> <percent> <asset>" to the log. The old files stay imported for this comparison only.

namespace HOLAudioSelectGain
{
	int EventOf(FString Family)
	{
		if (Family == "wood") return HOLAudioEvent::SelectBuildingWood;
		if (Family == "sacred") return HOLAudioEvent::SelectBuildingSacred;
		if (Family == "coins") return HOLAudioEvent::SelectBuildingCoins;
		return -1;
	}

	// The file the game played BEFORE the answer B (Wood F and Sacred H in /Game/Audio/Soft, the Market's handleCoins from the Kenney pack).
	FString OldAsset(FString Family)
	{
		if (Family == "wood") return "/Game/Audio/Soft/SW_s_wood_door.SW_s_wood_door";
		if (Family == "sacred") return "/Game/Audio/Soft/SW_s_sacred_book.SW_s_sacred_book";
		if (Family == "coins") return "/Game/Audio/Kenney/rpg-audio/SW_handleCoins.SW_handleCoins";
		return "";
	}

	// The plan the game builds today for the family (the real table).
	FHOLAudioPlan NewPlan(FString Family)
	{
		return HOLAudio::Plan(EventOf(Family), 5000, -1, 0, -1);
	}

	FString AssetOf(FString Family, FString Version)
	{
		if (Version == "old") return OldAsset(Family);
		if (Version == "new") return NewPlan(Family).Asset;
		return "";
	}

	int PercentOf(FString Family, FString Version)
	{
		if (Version == "old") return HOLAudio::BuildingSelectOldGainPercent;
		if (Version == "new") return NewPlan(Family).VolumePercent;
		return 0;
	}

	FString PlayedLine(FString Family, FString Version)
	{
		return f"select gain played {Family} {Version} {PercentOf(Family, Version)} {AssetOf(Family, Version)}";
	}
}

void HOLAudioSelectGainCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 2 || HOLAudioSelectGain::EventOf(Args[0]) < 0 || HOLAudioSelectGain::AssetOf(Args[0], Args[1]).IsEmpty())
	{
		Log(n"LogHOLMatch", "select gain usage: hol.Audio.SelectGain <wood|sacred|coins> <old|new>");
		return;
	}
	const FString Asset = HOLAudioSelectGain::AssetOf(Args[0], Args[1]);
	USoundBase Sound = Cast<USoundBase>(LoadObject(nullptr, Asset));
	if (Sound == nullptr)
	{
		Log(n"LogHOLMatch", f"select gain missing {Args[0]} {Args[1]} asset {Asset}");
		return;
	}
	Gameplay::PlaySound2D(Sound, float(HOLAudioSelectGain::PercentOf(Args[0], Args[1])) / 100.0, 1.0);
	Log(n"LogHOLMatch", HOLAudioSelectGain::PlayedLine(Args[0], Args[1]));
}

const FConsoleCommand HOLAudioSelectGainConsole("hol.Audio.SelectGain", n"HOLAudioSelectGainCommand");
