// Socle step 1 of the migration (D830, D836, SOCLE-1116a): the C++ rules that STAY in C++ (Siege, Diplomacy, UnitAbility), read from
// script through their new libraries (HOLSiegeCheck, HOLDiplomacyRead, HOLUnitAbilityRead), on the SAME vectors as their C++ tests
// (HOLSiegeTests.cpp Siege.Tiers, HOLDiplomacyRulesTests.cpp Diplomacy.Rules, HOLDamnedWarriorTests.cpp against UnitAbilities.csv).
// The script copies of Siege (HOLSiegeRules.as, MIG-S1c-Siege, ELROND-1117i) and UnitAbility are gone; HOLDiplomacyRules.as is not yet: next lot.

// HeroesOfLiberty.Siege.Tiers through HOLSiegeCheck (engine and block numbers: EHOLSiegeEngine, int(EHOLSiegeBlock::Available)... of the script).
void Test_SiegeCheckLibraryVectors(FUnitTest& T)
{
	const int Ram = int(EHOLSiegeEngine::Ram);
	const int Mangonel = int(EHOLSiegeEngine::Mangonel);
	const int Trebuchet = int(EHOLSiegeEngine::Trebuchet);
	const int Cannon = int(EHOLSiegeEngine::Cannon);
	T.AssertEquals(1, HOLSiegeCheck::GetTier(Ram), "the Ram opens at tier 1");
	T.AssertEquals(2, HOLSiegeCheck::GetTier(Mangonel), "the Mangonel at tier 2");
	T.AssertEquals(3, HOLSiegeCheck::GetTier(Trebuchet), "the Trebuchet at tier 3");
	T.AssertEquals(3, HOLSiegeCheck::GetTier(Cannon), "the Cannon at tier 3 (D180)");
	T.AssertTrue(HOLSiegeCheck::InPrototype(Ram) && HOLSiegeCheck::InPrototype(Mangonel), "Ram and Mangonel in the prototype (D77)");
	T.AssertFalse(HOLSiegeCheck::InPrototype(Trebuchet) || HOLSiegeCheck::InPrototype(Cannon), "the other two wait (D77)");

	T.AssertEquals(int(EHOLSiegeBlock::Available), HOLSiegeCheck::Check(Ram, 1, true, true), "tier 1, workshop, paid: the Ram");
	T.AssertEquals(int(EHOLSiegeBlock::TierTooLow), HOLSiegeCheck::Check(Ram, 0, true, true), "the Ram needs tier 1");
	T.AssertEquals(int(EHOLSiegeBlock::TierTooLow), HOLSiegeCheck::Check(Mangonel, 1, true, true), "the Mangonel needs tier 2");
	T.AssertEquals(int(EHOLSiegeBlock::Available), HOLSiegeCheck::Check(Mangonel, 2, true, true), "tier 2 opens the Mangonel");
	T.AssertEquals(int(EHOLSiegeBlock::NoWorkshop), HOLSiegeCheck::Check(Ram, 1, false, true), "no finished workshop");
	T.AssertEquals(int(EHOLSiegeBlock::CannotPay), HOLSiegeCheck::Check(Ram, 1, true, false), "cannot pay");
	T.AssertEquals(int(EHOLSiegeBlock::NotInPrototype), HOLSiegeCheck::Check(Trebuchet, 3, true, true), "the Trebuchet is refused even at tier 3");
	T.AssertEquals(int(EHOLSiegeBlock::NotInPrototype), HOLSiegeCheck::Check(Cannon, 0, false, false), "outside the prototype comes first");
	T.AssertEquals(int(EHOLSiegeBlock::TierTooLow), HOLSiegeCheck::Check(Mangonel, 0, false, false), "then the tier");
	T.AssertEquals(int(EHOLSiegeBlock::NoWorkshop), HOLSiegeCheck::Check(Mangonel, 2, false, false), "then the workshop");
	T.AssertEquals(int(EHOLSiegeBlock::CannotPay), HOLSiegeCheck::Check(Mangonel, 2, true, false), "then the cost");

	// MIG-S1c-Siege: the script copy is gone; the library's reason is checked on every engine, tier and pair of booleans against the order of its own parts (prototype, tier, workshop, cost).
	for (int Index = 0; Index < HOLSiegeCheck::GetEngineCount(); Index++)
	{
		const EHOLSiegeEngine Engine = EHOLSiegeEngine(Index);
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			for (int Mask = 0; Mask < 4; Mask++)
			{
				const bool bWorkshop = (Mask & 1) != 0;
				const bool bPay = (Mask & 2) != 0;
				const int Expected = !HOLSiegeCheck::InPrototype(int(Engine)) ? int(EHOLSiegeBlock::NotInPrototype) : (Tier < HOLSiegeCheck::GetTier(int(Engine)) ? int(EHOLSiegeBlock::TierTooLow) : (!bWorkshop ? int(EHOLSiegeBlock::NoWorkshop) : (!bPay ? int(EHOLSiegeBlock::CannotPay) : int(EHOLSiegeBlock::Available))));
				T.AssertEquals(Expected, HOLSiegeCheck::Check(int(Engine), Tier, bWorkshop, bPay), f"engine {Index} tier {Tier} mask {Mask}: the reason follows the order of its parts");
			}
		}
	}

	int Found = -1;
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Ram", Found) && Found == Ram, "Ram is the Ram");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Mangonel", Found) && Found == Mangonel, "Mangonel is the Mangonel");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Trebuchet", Found) && Found == Trebuchet, "Trebuchet is the Trebuchet");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Cannon", Found) && Found == Cannon, "Cannon is the Cannon");
	T.AssertFalse(HOLSiegeCheck::FindEngine(n"Spearman", Found), "the Spearman is no engine");
	T.AssertFalse(HOLSiegeCheck::FindEngine(NAME_None, Found), "no name is no engine");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(int(EHOLSiegeBlock::TierTooLow)) == "tier too low", "tier words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(int(EHOLSiegeBlock::NotInPrototype)) == "not in the prototype", "prototype words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(int(EHOLSiegeBlock::NoWorkshop)) == "no workshop", "workshop words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(int(EHOLSiegeBlock::CannotPay)) == "cannot pay", "cost words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(int(EHOLSiegeBlock::Available)) == "none", "nothing stops it");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Ram") == n"RamHealth", "the Ram reads RamHealth");
	T.AssertTrue(HOLSiegeCheck::GetDamageStat(n"Mangonel") == n"MangonelDamage", "the Mangonel reads MangonelDamage");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Spearman") == NAME_None, "a Spearman has none");
}

FHOLDiplomacyStances SocleHumans(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
		NoAI.Add(false);
	return HOLDiplomacyRead::NewBook(Slots, NoAI);
}

void SocleMakeAllies(FHOLDiplomacyStances& Book, int A, int B)
{
	HOLDiplomacyRead::SetStance(Book, A, B, 2);
	HOLDiplomacyRead::SetStance(Book, B, A, 2);
}

// HeroesOfLiberty.Diplomacy.Rules through HOLDiplomacyRead (stances 0 Enemy, 1 Neutral, 2 Ally; results of HOLDiplomacy::EResult). The
// change records (bAnnounce, bFormed, bBroke, Before / After) are not exposed: only the result and the book after it.
void Test_DiplomacyReadLibraryVectors(FUnitTest& T)
{
	const int Enemy = 0;
	const int Neutral = 1;
	const int Ally = 2;
	FHOLDiplomacyStances Book = SocleHumans(4);
	T.AssertEquals(4, Book.Slots, "four slots");
	T.AssertEquals(Enemy, HOLDiplomacyRead::Relation(Book, 0, 3), "default relation");
	T.AssertEquals(Ally, HOLDiplomacyRead::Relation(Book, 2, 2), "oneself is an ally");

	FHOLDiplomacyStances Pair = SocleHumans(2);
	T.AssertEquals(0, HOLDiplomacyRead::SetStance(Pair, 0, 1, Ally), "an offer is allowed");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Pair, 0, 1) || HOLDiplomacyRead::IsAlly(Pair, 1, 0), "one side is not an alliance");
	HOLDiplomacyRead::SetStance(Pair, 1, 0, Ally);
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Pair, 0, 1) && HOLDiplomacyRead::IsAlly(Pair, 1, 0), "allied both ways");
	for (int A = Enemy; A <= Ally; A++)
	{
		for (int B = Enemy; B <= Ally; B++)
		{
			FHOLDiplomacyStances Mixed = SocleHumans(2);
			HOLDiplomacyRead::SetStance(Mixed, 0, 1, A);
			HOLDiplomacyRead::SetStance(Mixed, 1, 0, B);
			T.AssertEquals(Math::Min(A, B), HOLDiplomacyRead::Relation(Mixed, 0, 1), f"{A}/{B}: the most hostile applies (0 to 1)");
			T.AssertEquals(Math::Min(A, B), HOLDiplomacyRead::Relation(Mixed, 1, 0), f"{A}/{B}: and to both (1 to 0)");
		}
	}

	T.AssertEquals(0, HOLDiplomacyRead::SetVision(Pair, 0, 1, true), "vision opens between allies");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "1 sees what 0 sees");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 1, 0), "one-sided");
	HOLDiplomacyRead::SetStance(Pair, 1, 0, Enemy);
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Pair, 0, 1) && HOLDiplomacyRead::FiresAtOnce(Pair, 1, 0), "the break: both fire at once");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "the vision is cut at that second");
	T.AssertEquals(Enemy, HOLDiplomacyRead::StanceOf(Pair, 0, 1), "the betrayed one's stance drops to Enemy too");
	SocleMakeAllies(Pair, 0, 1);
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Pair, 0, 1), "allied again");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "the vision must be opened again");

	FHOLDiplomacyStances Calm = SocleHumans(4);
	SocleMakeAllies(Calm, 0, 2);
	HOLDiplomacyRead::SetStance(Calm, 0, 1, Neutral);
	HOLDiplomacyRead::SetStance(Calm, 1, 0, Neutral);
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Calm, 0, 1), "Neutral does not fire");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Calm, 0, 1) && HOLDiplomacyRead::IsAdversary(Calm, 1, 0), "Neutral is an adversary");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Calm, 0, 2) || HOLDiplomacyRead::FiresAtOnce(Calm, 0, 2), "an ally is no adversary");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Calm, 0, 3) && HOLDiplomacyRead::IsAdversary(Calm, 0, 3), "Enemy fires");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Calm, 0, 0) || HOLDiplomacyRead::IsAdversary(Calm, 0, 9) || HOLDiplomacyRead::FiresAtOnce(Calm, 0, 9), "oneself and outside the match: nobody");

	FHOLDiplomacyStances Three = SocleHumans(3);
	T.AssertEquals(1, HOLDiplomacyRead::SetStance(Three, 3, 1, Ally), "sender beyond the slots");
	T.AssertEquals(1, HOLDiplomacyRead::SetStance(Three, 0, -1, Ally), "negative target");
	T.AssertEquals(2, HOLDiplomacyRead::SetStance(Three, 1, 1, Ally), "toward oneself");
	T.AssertEquals(3, HOLDiplomacyRead::SetStance(Three, 0, 1, 3), "no such stance");
	T.AssertEquals(1, HOLDiplomacyRead::SetStance(Three, 7, 7, 9), "the slots come first");
	T.AssertEquals(2, HOLDiplomacyRead::SetStance(Three, 1, 1, 9), "then oneself");
	T.AssertEquals(Enemy, HOLDiplomacyRead::StanceOf(Three, 0, 1), "a refusal changes nothing");
	SocleMakeAllies(Three, 0, 1);
	HOLDiplomacyRead::SetStance(Three, 0, 2, Ally);
	T.AssertEquals(6, HOLDiplomacyRead::SetVision(Three, 0, 2, true), "vision: an offer not accepted is not an alliance");
	T.AssertEquals(2, HOLDiplomacyRead::SetVision(Three, 1, 1, true), "vision: toward oneself");

	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(true);
	FHOLDiplomacyStances WithAI = HOLDiplomacyRead::NewBook(4, IsAI);
	T.AssertEquals(0, HOLDiplomacyRead::SetStance(WithAI, 0, 2, Ally), "a human may say Ally to an AI");
	T.AssertEquals(Enemy, HOLDiplomacyRead::Relation(WithAI, 0, 2), "it stays Enemy");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(WithAI, 2, 0), "and fires");
	T.AssertEquals(4, HOLDiplomacyRead::SetStance(WithAI, 2, 0, Ally), "an AI takes no stance");
	T.AssertEquals(Enemy, HOLDiplomacyRead::Relation(WithAI, 2, 3), "two AIs are enemies");
	T.AssertEquals(4, HOLDiplomacyRead::SetVision(WithAI, 2, 0, true), "an AI shares nothing");

	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	Teams.Add(0);
	Teams.Add(0);
	TArray<bool> TeamAI;
	TeamAI.Add(false);
	TeamAI.Add(true);
	TeamAI.Add(false);
	TeamAI.Add(false);
	TeamAI.Add(true);
	FHOLDiplomacyStances Locked = HOLDiplomacyRead::NewTeamBook(Teams, TeamAI);
	T.AssertTrue(Locked.bLocked, "locked");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Locked, 0, 1) && HOLDiplomacyRead::IsAlly(Locked, 1, 0), "same team: allies");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Locked, 2, 1) && HOLDiplomacyRead::IsAdversary(Locked, 0, 2), "other teams: enemies");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Locked, 3, 4) && !HOLDiplomacyRead::IsAlly(Locked, 3, 4), "team 0 is alone");
	T.AssertEquals(5, HOLDiplomacyRead::SetStance(Locked, 0, 2, Ally), "no stance can be set");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Locked, 0, 1) && HOLDiplomacyRead::SharesVision(Locked, 1, 0), "teammates see together");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Locked, 0, 2), "not the other team");
	T.AssertEquals(5, HOLDiplomacyRead::SetVision(Locked, 0, 1, false), "no box to tick");

	FHOLDiplomacyStances Trio = SocleHumans(3);
	SocleMakeAllies(Trio, 0, 1);
	T.AssertTrue(HOLDiplomacyRead::CanLeaveRampart(Trio, 0, 1), "allied: down when it wants");
	HOLDiplomacyRead::SetStance(Trio, 1, 0, Enemy);
	T.AssertFalse(HOLDiplomacyRead::CanLeaveRampart(Trio, 0, 1), "after the break: it stays there");
	T.AssertTrue(HOLDiplomacyRead::CanLeaveRampart(Trio, 1, 1), "the owner's own units come and go");
}

// HOLDamnedWarriorTests.cpp checks the C++ constants against UnitAbilities.csv and Effects.csv: here the same, through HOLUnitAbilityRead,
// against the script's table made from the same CSV (HOLUnitAbilityTable) and the real morale chain (Effects.csv); the cast and hit-cost vectors are in HOLUnitAbilityRules_Test.as (the script copy of the rules is gone).
void Test_UnitAbilityReadLibraryAgreesWithTable(FUnitTest& T)
{
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Rage = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::NecroticRage);
	T.AssertTrue(HOLUnitAbilityRead::GetRageUnitType().ToString() == Rage.UnitType, "the unit type that casts it");
	T.AssertEquals(Rage.CooldownMs, HOLUnitAbilityRead::GetRageCooldownMs(), "cool-down");
	T.AssertEquals(Rage.DurationMs, HOLUnitAbilityRead::GetRageDurationMs(), "duration");
	T.AssertEquals(Rage.HpCostPerHit, HOLUnitAbilityRead::GetRageHpCostPerHit(), "hit points lost per blow");
	T.AssertEquals(Rage.MinHp, HOLUnitAbilityRead::GetRageMinHp(), "refused under (MinHp)");
	T.AssertEquals(Rage.StopHp, HOLUnitAbilityRead::GetRageStopHp(), "ends at (StopHp)");

	int MoraleId = -1;
	for (int M = 0; M < HOLMoraleTable::Count; M++)
	{
		if (HOLMoraleTable::Row(M).Key == Rage.Effect)
			MoraleId = M;
	}
	T.AssertTrue(MoraleId >= 0, "the rage's effect is in the morale table");
	if (MoraleId >= 0)
	{
		HOLMorale::FHOLMoraleState State;
		HOLMorale::Apply(State, HOLMoraleTable::Effect(MoraleId));
		T.AssertEquals(100 + HOLUnitAbilityRead::GetRageDamagePercent(), HOLMorale::Multiplier(State, HOLMorale::LeverDealt), "damage dealt, in percent (Effects.csv)");
		T.AssertEquals(100 + HOLUnitAbilityRead::GetRageRatePercent(), HOLMorale::Multiplier(State, HOLMorale::LeverRate), "attack rate, in percent (Effects.csv)");
	}
	T.AssertEquals(800, HOLUnitAbilityRead::GetRageIntervalMs(1000), "25 % more blows: 1 000 ms -> 800 ms");
	T.AssertEquals(1, HOLUnitAbilityRead::GetRageIntervalMs(1), "never under 1 ms");

	T.AssertFalse(HOLUnitAbilityRead::CanCastRage(false, float(Rage.MinHp + 50), 0), "dead: refused");
}

// S1b: FHOLResources and FHOLMarketRow are visible from script (BlueprintType, fields BlueprintReadWrite). Visibility only, no rule:
// the market row's C++ defaults are the Standard row of Market.csv.
void Test_CoreTypesVisibleFromScript(FUnitTest& T)
{
	FHOLResources Cost;
	Cost.Food = 50;
	Cost.Gold = 25;
	T.AssertEquals(50, Cost.Food, "a cost's food");
	T.AssertEquals(0, Cost.Wood, "nothing else by default");
	FHOLMarketRow Row;
	T.AssertTrue(Row.FoodPrice == 80 && Row.WoodPrice == 80 && Row.StonePrice == 120 && Row.GoldPrice == 100, "prices at rest");
	T.AssertTrue(Row.FeePercent == 30 && Row.Step == 3 && Row.MinPrice == 20 && Row.MaxPrice == 300, "tax and limits");
	T.AssertTrue(Row.RecoverPoints == 1 && Row.RecoverEverySeconds == 10, "recovery");
}
