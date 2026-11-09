// Before / after of the building selection sounds (Q-audio-select-gain = B): the comparison plays the OLD x2.5 file against the file the game plays NOW,
// read from the real table. EFFECT: for each family the new sound is a limited file at 100 %, the old one the clipping x2.5, the two differ.

void Test_AudioSelectGainOldAndNewDiffer(FUnitTest& T)
{
	TArray<FString> Families;
	Families.Add("wood"); Families.Add("sacred"); Families.Add("coins");
	for (int Index = 0; Index < Families.Num(); Index++)
	{
		FString Family = Families[Index];
		T.AssertTrue(HOLAudioSelectGain::EventOf(Family) >= 0, f"{Family}: a selection event");
		T.AssertEquals(250, HOLAudioSelectGain::PercentOf(Family, "old"), f"{Family}: the old gain is the clipping x2.5");
		T.AssertEquals(100, HOLAudioSelectGain::PercentOf(Family, "new"), f"{Family}: the new sound rings at 100 %");
		T.AssertTrue(HOLAudioSelectGain::AssetOf(Family, "new").Contains("_lim."), f"{Family}: the new sound is a limited file");
		T.AssertFalse(HOLAudioSelectGain::AssetOf(Family, "old").Contains("_lim."), f"{Family}: the old sound is the unlimited file");
		T.AssertTrue(HOLAudioSelectGain::AssetOf(Family, "old") != HOLAudioSelectGain::AssetOf(Family, "new"), f"{Family}: two different files");
		T.AssertTrue(HOLAudioSelectGain::PlayedLine(Family, "new").StartsWith(f"select gain played {Family} new 100 "), f"{Family}: the log line names family and version");
	}
	T.AssertEquals(-1, HOLAudioSelectGain::EventOf("stone"), "stone has no election: no comparison");
	T.AssertTrue(HOLAudioSelectGain::AssetOf("wood", "other").IsEmpty(), "an unknown version plays nothing");
}
