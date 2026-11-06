// The table of the morale effects (M2, Tools/Data/Effects.csv -> HOLMoraleTable.as made by Tools/Data/make_effects.py; MoraleInProgress.md D708).
// The script cannot read the CSV: its copy is generated, and `python Tools/Data/make_effects.py --check` says whether it is in step. The agreement
// of the table with the C++ constants of HOLHeroAbilityRules.h waits for the C++ loader (M3); here the migrated numbers are the ones of that file.

// Every row is well formed: a name and a text, a form with what it needs, a sign that agrees with its levers.
void Test_MoraleTableRowsAreWellFormed(FUnitTest& T)
{
	T.AssertTrue(HOLMoraleTable::Count >= 6, "the six starting effects are there");
	TArray<FString> Keys;
	for (int Id = 0; Id < HOLMoraleTable::Count; Id++)
	{
		const HOLMoraleTable::FHOLMoraleRow Row = HOLMoraleTable::Row(Id);
		T.AssertFalse(Row.Key.IsEmpty(), f"row {Id} has a key");
		T.AssertFalse(Keys.Contains(Row.Key), f"{Row.Key} is the key of one row only");
		Keys.Add(Row.Key);
		T.AssertFalse(HOLMoraleTable::Name(Id).ToString().IsEmpty(), f"{Row.Key} has a name");
		T.AssertFalse(HOLMoraleTable::Description(Id).ToString().IsEmpty(), f"{Row.Key} has a description");
		T.AssertFalse(Row.SourceKind.IsEmpty(), f"{Row.Key} says where it comes from");
		T.AssertTrue(Row.RadiusCm >= 0 && Row.DurationMs >= 0, f"{Row.Key}: no negative reach or time");
		const HOLMorale::FHOLMoraleEffect Effect = HOLMoraleTable::Effect(Id);
		T.AssertTrue(HOLMorale::HasIntensity(Effect), f"{Row.Key} acts on a lever");
		T.AssertEquals(Row.bMalusSign, HOLMorale::IsMalus(Effect), f"{Row.Key}: the sign agrees with the levers");
		if (Row.Gives < 0)
		{
			// Bonus levers all >= 0, malus levers all <= 0 (a mixed effect would be neither).
			const bool bAllNonNegative = Row.Dealt >= 0 && Row.Taken >= 0 && Row.Rate >= 0;
			const bool bAllNonPositive = Row.Dealt <= 0 && Row.Taken <= 0 && Row.Rate <= 0;
			T.AssertTrue(Row.bMalusSign ? bAllNonPositive : bAllNonNegative, f"{Row.Key}: every lever on the side of its sign");
			if (!Row.bAura)
				T.AssertTrue(Row.DurationMs > 0, f"{Row.Key}: a temporary effect has a time");
			else
				T.AssertTrue(Row.RadiusCm > 0, f"{Row.Key}: an aura has a radius");
			T.AssertEquals(Row.bMalusSign, Row.bAttenuable, f"{Row.Key}: only a malus is attenuable");
		}
		else
		{
			const HOLMoraleTable::FHOLMoraleRow Given = HOLMoraleTable::Row(Row.Gives);
			T.AssertFalse(Given.Key.IsEmpty(), f"{Row.Key} gives a row of the table");
			T.AssertTrue(Given.bAura, f"{Row.Key} gives an aura");
			T.AssertEquals(0, Row.Dealt + Row.Taken + Row.Rate, f"{Row.Key} has no levers of its own");
			T.AssertTrue(Row.RadiusCm > 0 && Row.DurationMs > 0, f"{Row.Key}: a planted source has a radius and a time");
		}
	}
	T.AssertTrue(HOLMoraleTable::Row(HOLMoraleTable::Count).Key.IsEmpty(), "an id out of the table is an empty row");
}

// What the hero's aura, cry and banner do today (HOLHeroAbilityRules.h: AuraReductionPercent 15, CloseRanksReductionPercent 25,
// CloseRanksDurationMs 15000, BannerDurationMs 45000, AuraRadius 1200 cm) is what the table says.
void Test_MoraleTableMigratedValues(FUnitTest& T)
{
	const HOLMoraleTable::FHOLMoraleRow Aura = HOLMoraleTable::Row(HOLMoraleTable::DawnAura);
	T.AssertEquals(15, Aura.Taken, "the Dawn Aura: 15 % less damage taken");
	T.AssertEquals(0, Aura.Dealt + Aura.Rate, "and nothing else");
	T.AssertTrue(Aura.bAura, "an aura");
	T.AssertEquals(1200, Aura.RadiusCm, "12 m");
	const HOLMoraleTable::FHOLMoraleRow Cry = HOLMoraleTable::Row(HOLMoraleTable::CloseRanks);
	T.AssertEquals(25, Cry.Taken, "Close Ranks: 25 % less damage taken");
	T.AssertFalse(Cry.bAura, "temporary");
	T.AssertEquals(15000, Cry.DurationMs, "15 s");
	T.AssertEquals(1200, Cry.RadiusCm, "12 m");
	const HOLMoraleTable::FHOLMoraleRow Banner = HOLMoraleTable::Row(HOLMoraleTable::Banner);
	T.AssertEquals(45000, Banner.DurationMs, "the banner stands 45 s");
	T.AssertEquals(1200, Banner.RadiusCm, "its aura reaches 12 m");
	T.AssertEquals(HOLMoraleTable::DawnAura, Banner.Gives, "it gives the Dawn Aura");
}

// One aura counts however many cover the unit: the hero's and the banner's are the same effect, so they never add up (the aura 15 %, the cry on top 25 %).
void Test_MoraleTableOneAuraCounts(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::DawnAura));
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::Banner));
	T.AssertEquals(1, HOLMorale::Count(State), "the hero's aura and a banner's are one effect");
	T.AssertEquals(85, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "15 % less taken: 85 % of the damage, not 70");
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::CloseRanks));
	T.AssertEquals(60, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "the cry on top: 60 % of the damage taken, as GetDamageTakenPercent(true, true)");
}

// The duel's effects: the times are those of HOLDuelRules.as, Hesitation is much weaker than Demoralised (D16), the malus can be attenuated.
void Test_MoraleTableDuelEffects(FUnitTest& T)
{
	const FHOLDuelSettings Settings = HOLDuel::Preset();
	const HOLMoraleTable::FHOLMoraleRow Triumph = HOLMoraleTable::Row(HOLMoraleTable::Triumph);
	const HOLMoraleTable::FHOLMoraleRow Demoralised = HOLMoraleTable::Row(HOLMoraleTable::Demoralised);
	const HOLMoraleTable::FHOLMoraleRow Hesitation = HOLMoraleTable::Row(HOLMoraleTable::Hesitation);
	T.AssertEquals(Settings.TriumphMs, Triumph.DurationMs, "Triumph lasts as the duel says");
	T.AssertEquals(Settings.TriumphMs, Demoralised.DurationMs, "Demoralised lasts as long as Triumph (the duel's MoraleMs)");
	T.AssertEquals(Settings.HesitationMs, Hesitation.DurationMs, "Hesitation lasts as the duel says");
	T.AssertFalse(Triumph.bMalusSign, "Triumph is a bonus");
	T.AssertTrue(Demoralised.bMalusSign && Hesitation.bMalusSign, "Demoralised and Hesitation are malus");
	T.AssertTrue(Demoralised.bAttenuable && Hesitation.bAttenuable, "both can be attenuated");
	const int DemoralisedWeight = (Demoralised.Dealt < 0 ? -Demoralised.Dealt : Demoralised.Dealt) + (Demoralised.Taken < 0 ? -Demoralised.Taken : Demoralised.Taken);
	const int HesitationWeight = (Hesitation.Dealt < 0 ? -Hesitation.Dealt : Hesitation.Dealt) + (Hesitation.Taken < 0 ? -Hesitation.Taken : Hesitation.Taken);
	T.AssertTrue(HesitationWeight * 2 <= DemoralisedWeight, "Hesitation is much weaker than Demoralised");
	T.AssertEquals(0, Triumph.Rate + Demoralised.Rate + Hesitation.Rate, "the duel does not touch the attack rate");
}

// A malus of the table is attenuated on the real chain: Demoralised under a half attenuation lasts half as long.
void Test_MoraleTableAttenuatesDemoralised(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::Demoralised), 50);
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Demoralised).DurationMs / 2, State.Effects[0].DurationMs, "half the time");
	HOLMorale::RemoveMalus(State);
	T.AssertEquals(0, HOLMorale::Count(State), "removed by the soothing");
}
