// Sacred sites, pure rule (S1, HOLSiteRules.as; GDD 14.3 Sacred sites, D429 to D431, D697 to D703, SacredSitesInProgress.md). Four slots; every slot
// its own team unless a test says otherwise. The units come through HOLSite::AddUnit (the classification of the rule: who takes, who freezes,
// who does nothing), and the steps through HOLSite::Step, so each case goes through the whole chain, never counts typed by hand. The capture time
// and the recession are the rule's provisional constants, read from it (a test does not retype a value).

TArray<int> SiteTeams(int T0 = 0, int T1 = 1, int T2 = 2, int T3 = 3)
{
	TArray<int> Teams;
	Teams.Add(T0);
	Teams.Add(T1);
	Teams.Add(T2);
	Teams.Add(T3);
	return Teams;
}

// What stands in the zone: the counts the rule reads, filled by AddUnit, one unit at a time.
struct FSiteZone
{
	TArray<int> Takers;
	TArray<int> Disturbers;
}

FSiteZone EmptyZone()
{
	FSiteZone Zone;
	HOLSite::ResetCounts(Zone.Takers, Zone.Disturbers, 4);
	return Zone;
}

// A unit able to take (a holy unit, the Monk, or the hero) and a soldier who cannot, and a worker (the Peasant): the three kinds of the rule.
void AddHoly(FSiteZone& Zone, int Slot, int Count = 1)
{
	for (int N = 0; N < Count; N++)
		HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, Slot, true, false);
}

void AddSoldier(FSiteZone& Zone, int Slot, int Count = 1)
{
	for (int N = 0; N < Count; N++)
		HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, Slot, false, false);
}

void AddPeasant(FSiteZone& Zone, int Slot, int Count = 1)
{
	for (int N = 0; N < Count; N++)
		HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, Slot, false, true);
}

HOLSite::FHOLSiteState SiteRun(const HOLSite::FHOLSiteState& From, const FSiteZone& Zone, const TArray<int>& Teams, int Steps)
{
	HOLSite::FHOLSiteState State = From;
	for (int Step = 0; Step < Steps; Step++)
	{
		State = HOLSite::Step(State, Zone.Takers, Zone.Disturbers, Teams);
	}
	return State;
}

// The site is nobody's at the start; a holy unit alone in the zone fills the count and takes it when the count is full, not a step before.
void Test_SiteTakenByAHolyUnit(FUnitTest& T)
{
	HOLSite::FHOLSiteState State;
	T.AssertEquals(HOLSite::None, State.Owner, "a site laid by the map belongs to nobody");
	FSiteZone Zone = EmptyZone();
	AddHoly(Zone, 1);
	State = SiteRun(State, Zone, SiteTeams(), HOLSite::HoldSteps() - 1);
	T.AssertEquals(HOLSite::None, State.Owner, "one step short: still nobody's");
	T.AssertEquals(HOLSite::HoldSteps() - 1, State.Held, "the count shows it");
	T.AssertEquals(1, State.Taker, "slot 1 is filling it");
	State = SiteRun(State, Zone, SiteTeams(), 1);
	T.AssertEquals(1, State.Owner, "the count full: slot 1's");
	T.AssertEquals(0, State.Held, "the count starts again");
	T.AssertEquals(HOLSite::None, State.Taker, "nobody is filling it");
	T.AssertEquals(HOLSite::HoldSeconds * HOLHero::GetSimulationRateHz(), HOLSite::HoldSteps(), "the capture time in steps follows the simulation rate");
}

// A soldier alone does not take the site, however long he stays (D698: only a holy unit or the hero takes).
void Test_SiteNotTakenBySoldiers(FUnitTest& T)
{
	FSiteZone Zone = EmptyZone();
	AddSoldier(Zone, 1, 6);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Zone, SiteTeams(), HOLSite::HoldSteps() * 2);
	T.AssertEquals(HOLSite::None, State.Owner, "six Spearmen alone: nobody's");
	T.AssertEquals(0, State.Held, "no count");
	T.AssertEquals(HOLSite::None, State.Taker, "nobody is filling it");
}

// The worker does nothing: a Peasant, even a crowd of them, neither takes nor freezes nor defends (D698).
void Test_SitePeasantsDoNothing(FUnitTest& T)
{
	FSiteZone Zone = EmptyZone();
	AddPeasant(Zone, 1, 10);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Zone, SiteTeams(), HOLSite::HoldSteps() * 2);
	T.AssertEquals(HOLSite::None, State.Owner, "ten Peasants alone: they do not take it");
	T.AssertEquals(0, State.Held, "no count");
	// An adverse Peasant freezes nothing: the taker's count rises with a crowd of enemy Peasants around.
	FSiteZone Crowd = EmptyZone();
	AddHoly(Crowd, 1);
	AddPeasant(Crowd, 2, 10);
	HOLSite::FHOLSiteState Taken = SiteRun(HOLSite::FHOLSiteState(), Crowd, SiteTeams(), 100);
	T.AssertEquals(100, Taken.Held, "adverse Peasants freeze nothing: the count rises");
	// The owner's Peasants (and his ally's) defend nothing: the site is taken from its owner with them around.
	HOLSite::FHOLSiteState Owned;
	Owned.Owner = 0;
	FSiteZone Defended = EmptyZone();
	AddHoly(Defended, 1);
	AddPeasant(Defended, 0, 10);
	AddPeasant(Defended, 3, 10);
	Owned = SiteRun(Owned, Defended, SiteTeams(0, 1, 2, 0), HOLSite::HoldSteps());
	T.AssertEquals(1, Owned.Owner, "the owner's Peasants and his ally's do not defend: slot 1 takes it");
}

// The hero takes, alone, as a holy unit does (the hero is a taker whatever the faction).
void Test_SiteTakenByTheHeroAlone(FUnitTest& T)
{
	FSiteZone Zone = EmptyZone();
	AddHoly(Zone, 2);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Zone, SiteTeams(), HOLSite::HoldSteps());
	T.AssertEquals(2, State.Owner, "slot 2's hero alone takes it");
}

// An adverse soldier freezes the count under way, the capture and a count already held; the taker leaving does not undo what an adverse soldier freezes.
void Test_SiteFrozenByAnAdverseSoldier(FUnitTest& T)
{
	FSiteZone Taking = EmptyZone();
	AddHoly(Taking, 1);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Taking, SiteTeams(), 100);
	T.AssertEquals(100, State.Held, "alone: the count rises");
	FSiteZone Both = EmptyZone();
	AddHoly(Both, 1);
	AddSoldier(Both, 2);
	State = SiteRun(State, Both, SiteTeams(), 200);
	T.AssertEquals(100, State.Held, "an adverse Spearman in the zone: frozen");
	T.AssertEquals(HOLSite::None, State.Owner, "nobody has it");
	// The Spearman alone, the taker gone: the adverse soldier still freezes the count (it does not recede under his eyes).
	FSiteZone Soldier = EmptyZone();
	AddSoldier(Soldier, 2);
	State = SiteRun(State, Soldier, SiteTeams(), 200);
	T.AssertEquals(100, State.Held, "the taker gone, an adverse soldier there: still frozen");
	// The soldier of the taker's own side does not freeze his count: it recedes.
	FSiteZone Own = EmptyZone();
	AddSoldier(Own, 1);
	State = SiteRun(State, Own, SiteTeams(), 10);
	T.AssertEquals(100 - 10 * HOLSite::DecaySteps, State.Held, "the taker's own soldier does not freeze: the count recedes");
	// The soldier freezes the taker alone too: the soldier first, the holy unit comes, no count.
	FSiteZone Late = EmptyZone();
	AddHoly(Late, 1);
	AddSoldier(Late, 3);
	HOLSite::FHOLSiteState Blocked = SiteRun(HOLSite::FHOLSiteState(), Late, SiteTeams(), HOLSite::HoldSteps() * 2);
	T.AssertEquals(0, Blocked.Held, "an adverse soldier there from the start: the count never rises");
	T.AssertEquals(HOLSite::None, Blocked.Owner, "nobody takes it");
}

// The owner freezes the count with any unit that is no worker, his ally likewise (one camp: D703); when the defender leaves the count goes on.
void Test_SiteDefendedByItsOwnerAndHisAlly(FUnitTest& T)
{
	HOLSite::FHOLSiteState State;
	State.Owner = 0;
	FSiteZone Taking = EmptyZone();
	AddHoly(Taking, 1);
	State = SiteRun(State, Taking, SiteTeams(), 100);
	T.AssertEquals(100, State.Held, "an enemy alone: the count rises");
	FSiteZone Held = EmptyZone();
	AddHoly(Held, 1);
	AddSoldier(Held, 0);
	State = SiteRun(State, Held, SiteTeams(), 200);
	T.AssertEquals(100, State.Held, "one soldier of the owner in the zone: frozen");
	T.AssertEquals(0, State.Owner, "still the owner's");
	State = SiteRun(State, Taking, SiteTeams(), 10);
	T.AssertEquals(110, State.Held, "the defender gone: it goes on");
	// The owner's ally: a soldier of slot 3 (same team as slot 0) freezes it, and his holy unit takes nothing.
	FSiteZone Allied = EmptyZone();
	AddHoly(Allied, 1);
	AddSoldier(Allied, 3);
	HOLSite::FHOLSiteState Ally;
	Ally.Owner = 0;
	Ally = SiteRun(Ally, Allied, SiteTeams(0, 1, 2, 0), HOLSite::HoldSteps() * 2);
	T.AssertEquals(0, Ally.Owner, "an ally of the owner there: nothing taken");
	T.AssertEquals(0, Ally.Held, "no count");
	FSiteZone AllyHoly = EmptyZone();
	AddHoly(AllyHoly, 3, 4);
	HOLSite::FHOLSiteState Mine;
	Mine.Owner = 0;
	Mine = SiteRun(Mine, AllyHoly, SiteTeams(0, 1, 2, 0), HOLSite::HoldSteps() * 2);
	T.AssertEquals(0, Mine.Owner, "the owner's ally is no enemy: he takes nothing from him");
	T.AssertEquals(0, Mine.Held, "no count");
}

// Two adverse camps in the zone freeze each other, even with a holy unit each; the same camp spread over two slots counts for one (D703).
void Test_SiteTwoAdverseCampsFreeze(FUnitTest& T)
{
	FSiteZone Two = EmptyZone();
	AddHoly(Two, 1, 2);
	AddHoly(Two, 2, 2);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Two, SiteTeams(), 500);
	T.AssertEquals(0, State.Held, "two camps, neither alone: the count does not rise");
	T.AssertEquals(HOLSite::None, State.Owner, "nobody takes it");
	// One camp over two slots (1 and 3 allied), the other camp's slot 2 between them in the order: still two camps while slot 2 is there.
	FSiteZone Interleaved = EmptyZone();
	AddHoly(Interleaved, 1);
	AddSoldier(Interleaved, 2);
	AddHoly(Interleaved, 3);
	State = SiteRun(HOLSite::FHOLSiteState(), Interleaved, SiteTeams(0, 1, 2, 1), 500);
	T.AssertEquals(0, State.Held, "slots 1 and 3 are one camp, slot 2 another: frozen");
	// Slots 1 and 3 alone, one camp: the count rises, the most numerous of the camp is the taker, the lowest slot on a tie.
	FSiteZone Camp = EmptyZone();
	AddHoly(Camp, 1);
	AddHoly(Camp, 3, 3);
	State = SiteRun(HOLSite::FHOLSiteState(), Camp, SiteTeams(0, 1, 2, 1), 1);
	T.AssertEquals(3, State.Taker, "the most numerous of the camp is the taker");
	FSiteZone Tie = EmptyZone();
	AddHoly(Tie, 1, 2);
	AddHoly(Tie, 3, 2);
	State = SiteRun(HOLSite::FHOLSiteState(), Tie, SiteTeams(0, 1, 2, 1), 1);
	T.AssertEquals(1, State.Taker, "the lowest slot on a tie");
	State = SiteRun(State, Tie, SiteTeams(0, 1, 2, 1), HOLSite::HoldSteps());
	T.AssertEquals(1, State.Owner, "the camp's taker owns it when the count is full");
}

// With nobody there the count recedes to 0, never below, and the taker is forgotten at 0; the site's owner is not lost (D698).
void Test_SiteRecedesAndStaysWithItsOwner(FUnitTest& T)
{
	FSiteZone Taking = EmptyZone();
	AddHoly(Taking, 1);
	HOLSite::FHOLSiteState State = SiteRun(HOLSite::FHOLSiteState(), Taking, SiteTeams(), 100);
	State = SiteRun(State, EmptyZone(), SiteTeams(), 10);
	T.AssertEquals(100 - 10 * HOLSite::DecaySteps, State.Held, "the count recedes");
	T.AssertEquals(1, State.Taker, "its taker is remembered while the count lasts");
	State = SiteRun(State, EmptyZone(), SiteTeams(), 100);
	T.AssertEquals(0, State.Held, "down to 0, never below");
	T.AssertEquals(HOLSite::None, State.Taker, "nobody is filling it any more");
	// Only Peasants there: as nobody (they hold nothing back).
	State = SiteRun(State, Taking, SiteTeams(), 100);
	FSiteZone Workers = EmptyZone();
	AddPeasant(Workers, 2, 5);
	State = SiteRun(State, Workers, SiteTeams(), 10);
	T.AssertEquals(100 - 10 * HOLSite::DecaySteps, State.Held, "Peasants only: the count recedes");
	// The taker having left after the capture, the site stays his, however long.
	HOLSite::FHOLSiteState Owned = SiteRun(HOLSite::FHOLSiteState(), Taking, SiteTeams(), HOLSite::HoldSteps());
	T.AssertEquals(1, Owned.Owner, "taken");
	Owned = SiteRun(Owned, EmptyZone(), SiteTeams(), HOLSite::HoldSteps() * 3);
	T.AssertEquals(1, Owned.Owner, "the taker gone for three times the capture time: still his");
	Owned = SiteRun(Owned, Taking, SiteTeams(), HOLSite::HoldSteps() * 2);
	T.AssertEquals(1, Owned.Owner, "its owner's own units there: nothing to take");
	T.AssertEquals(0, Owned.Held, "no count");
}

// Another camp takes it from its owner the same way (the owner away), a Neutral slot (its own team, D401) is an adversary like another.
void Test_SiteRetakenAndNeutralIsAdverse(FUnitTest& T)
{
	HOLSite::FHOLSiteState State;
	State.Owner = 0;
	FSiteZone Zone = EmptyZone();
	AddHoly(Zone, 2);
	State = SiteRun(State, Zone, SiteTeams(), HOLSite::HoldSteps());
	T.AssertEquals(2, State.Owner, "retaken by slot 2");
	// Slot 3 is at peace with everyone (Diplomacy, D401) but of no team with the owner: its holy unit takes it, its soldier freezes.
	HOLSite::FHOLSiteState Neutral;
	Neutral.Owner = 0;
	FSiteZone Mixed = EmptyZone();
	AddHoly(Mixed, 1);
	AddSoldier(Mixed, 3);
	Neutral = SiteRun(Neutral, Mixed, SiteTeams(0, 1, 2, 3), 300);
	T.AssertEquals(0, Neutral.Held, "a neutral soldier is an adversary of the taker: frozen");
	FSiteZone Alone = EmptyZone();
	AddHoly(Alone, 3);
	Neutral = SiteRun(Neutral, Alone, SiteTeams(0, 1, 2, 3), HOLSite::HoldSteps());
	T.AssertEquals(3, Neutral.Owner, "a neutral slot's holy unit takes it like another");
}

// The classification of a unit: only a holy unit or the hero takes, a worker counts for nothing, a slot out of the zone's arrays is ignored.
void Test_SiteCountsOfTheUnits(FUnitTest& T)
{
	FSiteZone Zone = EmptyZone();
	AddHoly(Zone, 1, 2);
	AddSoldier(Zone, 1, 3);
	AddPeasant(Zone, 1, 4);
	T.AssertEquals(2, Zone.Takers[1], "takers: the holy units");
	T.AssertEquals(5, Zone.Disturbers[1], "disturbers: every unit that is no worker, the takers with them");
	T.AssertEquals(0, Zone.Takers[0], "nobody at slot 0");
	HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, 9, true, false);
	HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, -1, true, false);
	T.AssertEquals(4, Zone.Takers.Num(), "a slot out of the arrays is ignored");
	T.AssertEquals(2, Zone.Takers[1], "and changes nothing");
}

// Q2 = B (Retours\Q-sites-rule-edges.md, answered 2026-11-05): a taker of ANOTHER camp who arrives alone while a count was begun by a first
// camp starts again from zero (he does not inherit the effort); a taker of the SAME camp (an ally's slot) goes on from the count.
void Test_SiteTakerOfAnotherCampStartsFromZero(FUnitTest& T)
{
	FSiteZone First = EmptyZone();
	AddHoly(First, 1);
	HOLSite::FHOLSiteState Begun = SiteRun(HOLSite::FHOLSiteState(), First, SiteTeams(), 100);
	T.AssertEquals(100, Begun.Held, "the first camp has 100 steps");
	// Another camp's holy unit alone (the first one gone): the count starts again, it is his own.
	FSiteZone Second = EmptyZone();
	AddHoly(Second, 2);
	HOLSite::FHOLSiteState State = SiteRun(Begun, Second, SiteTeams(), 10);
	T.AssertEquals(10, State.Held, "another camp's taker starts from zero, he inherits nothing");
	T.AssertEquals(2, State.Taker, "he is the one filling it");
	// His count is his: it is full after the whole capture time from his arrival, not before.
	State = SiteRun(Begun, Second, SiteTeams(), HOLSite::HoldSteps() - 1);
	T.AssertEquals(HOLSite::None, State.Owner, "not taken one step before his own full count");
	State = SiteRun(Begun, Second, SiteTeams(), HOLSite::HoldSteps());
	T.AssertEquals(2, State.Owner, "taken after the whole capture time from his arrival");
	// An ally of the first taker (slot 3, same team as slot 1) goes on from the count: the same camp.
	FSiteZone Ally = EmptyZone();
	AddHoly(Ally, 3);
	State = SiteRun(Begun, Ally, SiteTeams(0, 1, 2, 1), 10);
	T.AssertEquals(110, State.Held, "a taker of the same camp goes on from the count");
	T.AssertEquals(3, State.Taker, "and is shown as the taker");
}

// The units of one presence digit for a slot: 0 nobody, 1 one holy unit, 2 a soldier and a worker, 3 two holy units and a soldier.
void SiteAddPresence(TArray<int>& UnitSlots, TArray<int>& CanTake, TArray<int>& IsPeasant, int Slot, int Digit)
{
	int Holy = 0;
	int Soldiers = 0;
	int Workers = 0;
	if (Digit == 1)
		Holy = 1;
	else if (Digit == 2)
	{
		Soldiers = 1;
		Workers = 1;
	}
	else if (Digit == 3)
	{
		Holy = 2;
		Soldiers = 1;
	}
	for (int N = 0; N < Holy; N++)
	{
		UnitSlots.Add(Slot);
		CanTake.Add(1);
		IsPeasant.Add(0);
	}
	for (int N = 0; N < Soldiers; N++)
	{
		UnitSlots.Add(Slot);
		CanTake.Add(0);
		IsPeasant.Add(0);
	}
	for (int N = 0; N < Workers; N++)
	{
		UnitSlots.Add(Slot);
		CanTake.Add(0);
		IsPeasant.Add(1);
	}
}

TArray<int> SiteArrangement(int Index)
{
	if (Index == 1)
		return SiteTeams(0, 0, 1, 1);
	if (Index == 2)
		return SiteTeams(0, 1, 0, 1);
	return SiteTeams();
}

// The simulation's copy of the rules (C++, HOLSite::Step and HOLSite::AddUnit) gives the same site and the same counts as the script's on every
// case: each owner, count and taker of the count, each presence of the four slots (nobody, a holy unit, a soldier and a worker, two holy units
// and a soldier), three arrangements of the teams. The units go through the AddUnit of each side, never counts typed by hand.
void Test_SiteScriptAndSimulationRulesAgree(FUnitTest& T)
{
	TArray<int> Owners;
	Owners.Add(-1);
	Owners.Add(0);
	Owners.Add(2);
	TArray<int> Helds;
	Helds.Add(0);
	Helds.Add(1);
	Helds.Add(100);
	Helds.Add(HOLSite::HoldSteps() - 1);
	TArray<int> Takers;
	Takers.Add(1);
	Takers.Add(3);
	int Cases = 0;
	int Differences = 0;
	int CountDifferences = 0;
	int Risen = 0;
	for (int O = 0; O < Owners.Num(); O++)
	{
		for (int H = 0; H < Helds.Num(); H++)
		{
			for (int K = 0; K < (Helds[H] > 0 ? 2 : 1); K++)
			{
				for (int A = 0; A < 3; A++)
				{
					for (int Presence = 0; Presence < 256; Presence++)
					{
						TArray<int> UnitSlots;
						TArray<int> CanTake;
						TArray<int> IsPeasant;
						int Rest = Presence;
						for (int Slot = 0; Slot < 4; Slot++)
						{
							SiteAddPresence(UnitSlots, CanTake, IsPeasant, Slot, Rest % 4);
							Rest = Rest / 4;
						}
						FSiteZone Zone = EmptyZone();
						for (int Unit = 0; Unit < UnitSlots.Num(); Unit++)
							HOLSite::AddUnit(Zone.Takers, Zone.Disturbers, UnitSlots[Unit], CanTake[Unit] != 0, IsPeasant[Unit] != 0);
						HOLSite::FHOLSiteState State;
						State.Owner = Owners[O];
						State.Taker = Helds[H] > 0 ? Takers[K] : -1;
						State.Held = Helds[H];
						const HOLSite::FHOLSiteState Script = HOLSite::Step(State, Zone.Takers, Zone.Disturbers, SiteArrangement(A));
						int OwnerOut = -9;
						int TakerOut = -9;
						int HeldOut = -9;
						HOLSiteRules::StepSite(State.Owner, State.Taker, State.Held, UnitSlots, CanTake, IsPeasant, SiteArrangement(A), HOLSite::HoldSteps(), HOLSite::DecaySteps, OwnerOut, TakerOut, HeldOut);
						TArray<int> SimTakers;
						TArray<int> SimDisturbers;
						HOLSiteRules::CountSiteUnits(4, UnitSlots, CanTake, IsPeasant, SimTakers, SimDisturbers);
						Cases++;
						if (Script.Owner != OwnerOut || Script.Taker != TakerOut || Script.Held != HeldOut)
							Differences++;
						if (SimTakers.Num() != 4 || SimDisturbers.Num() != 4)
							CountDifferences++;
						else
						{
							for (int Slot = 0; Slot < 4; Slot++)
							{
								if (SimTakers[Slot] != Zone.Takers[Slot] || SimDisturbers[Slot] != Zone.Disturbers[Slot])
									CountDifferences++;
							}
						}
						if (Script.Held > State.Held)
							Risen++;
					}
				}
			}
		}
	}
	T.AssertEquals(3 * 7 * 3 * 256, Cases, "every case compared");
	T.AssertEquals(0, CountDifferences, "AddUnit makes the same counts on both sides");
	T.AssertEquals(0, Differences, "the script and the simulation agree on every case");
	T.AssertTrue(Risen > 0, "the cases do raise the count (the comparison is not made of frozen sites only)");
}

// The provisional values of the simulation (C++) are the script's.
void Test_SiteConstantsAgreeWithTheSimulation(FUnitTest& T)
{
	T.AssertEquals(HOLSite::HoldSeconds, HOLSiteRules::GetHoldSeconds(), "the capture time");
	T.AssertEquals(HOLSite::DecaySteps, HOLSiteRules::GetDecaySteps(), "the recession");
}

// At most two ints in a list; -100 stands for "no more".
TArray<int> SiteInts(int A = -100, int B = -100)
{
	TArray<int> List;
	if (A != -100)
		List.Add(A);
	if (B != -100)
		List.Add(B);
	return List;
}

// The owners of the sites the victory cases walk through: none, one, two of one slot, two of two slots (one camp or two by the teams), an unowned site, a
// slot out of the teams, and the last two sites (slots 2 and 3).
TArray<int> VictoryOwnerSet(int Index)
{
	if (Index == 1)
		return SiteInts(0);
	if (Index == 2)
		return SiteInts(0, 1);
	if (Index == 3)
		return SiteInts(0, 0);
	if (Index == 4)
		return SiteInts(1, 0);
	if (Index == 5)
		return SiteInts(-1, 0);
	if (Index == 6)
		return SiteInts(9);
	if (Index == 7)
		return SiteInts(2, 3);
	if (Index == 8)
		return SiteInts(2, 2);
	return TArray<int>();
}

TArray<int> VictoryContestSet(int Index)
{
	if (Index == 1)
		return SiteInts(0, 0);
	if (Index == 2)
		return SiteInts(0, 1);
	if (Index == 3)
		return SiteInts(1);
	return TArray<int>();
}

// The teams of the slots: four teams, two pairs, and two with a slot of team -1 (the first, the second): no camp has team -1.
TArray<int> VictoryTeamSet(int Index)
{
	if (Index == 1)
		return SiteTeams(0, 0, 1, 1);
	if (Index == 2)
		return SiteTeams(-1, -1, 1, 1);
	if (Index == 3)
		return SiteTeams(0, -1, 1, 1);
	return SiteTeams();
}

bool SameInts(const TArray<int>& A, const TArray<int>& B)
{
	if (A.Num() != B.Num())
		return false;
	for (int Index = 0; Index < A.Num(); Index++)
	{
		if (A[Index] != B[Index])
			return false;
	}
	return true;
}

// The simulation's victory count (C++, HOLSite::VictoryStep) gives the same state as the script's (HOLSiteVictory::Step) on every case: leader, steps
// held (from nothing to one short of the full count), winner, owners at the last step (none, the same, others), owners of the sites, contested sites,
// teams (with a team -1: the script has no camp then, and so has the simulation). The state out is compared whole, the owners it keeps included.
void Test_SiteVictoryScriptAndSimulationAgree(FUnitTest& T)
{
	const int Steps = HOLSiteVictory::VictorySteps();
	TArray<int> Leaders;
	Leaders.Add(-1);
	Leaders.Add(0);
	Leaders.Add(1);
	TArray<int> Helds;
	Helds.Add(0);
	Helds.Add(5);
	Helds.Add(Steps - 2);
	Helds.Add(Steps - 1);
	TArray<int> Winners;
	Winners.Add(-1);
	Winners.Add(0);
	int Cases = 0;
	int Differences = 0;
	int Counting = 0;
	int Won = 0;
	for (int L = 0; L < Leaders.Num(); L++)
	{
		for (int H = 0; H < Helds.Num(); H++)
		{
			for (int W = 0; W < Winners.Num(); W++)
			{
				for (int Last = 0; Last < 3; Last++)
				{
					for (int Owner = 0; Owner < 9; Owner++)
					{
						for (int Contest = 0; Contest < 4; Contest++)
						{
							for (int Team = 0; Team < 4; Team++)
							{
								HOLSiteVictory::FHOLSiteVictoryState State;
								State.Leader = Leaders[L];
								State.Held = Helds[H];
								State.Winner = Winners[W];
								State.LastOwners = Last == 0 ? TArray<int>() : (Last == 1 ? VictoryOwnerSet(Owner) : SiteInts(3, 3));
								const HOLSiteVictory::FHOLSiteVictoryState Script = HOLSiteVictory::Step(State, VictoryOwnerSet(Owner), VictoryContestSet(Contest), VictoryTeamSet(Team));
								int LeaderOut = -9;
								int HeldOut = -9;
								int WinnerOut = -9;
								TArray<int> LastOut;
								HOLSiteRules::VictoryStepSite(State.Leader, State.Held, State.Winner, State.LastOwners, VictoryOwnerSet(Owner), VictoryContestSet(Contest), VictoryTeamSet(Team), Steps, LeaderOut, HeldOut, WinnerOut, LastOut);
								Cases++;
								if (Script.Leader != LeaderOut || Script.Held != HeldOut || Script.Winner != WinnerOut || !SameInts(Script.LastOwners, LastOut))
									Differences++;
								if (Script.Held > State.Held)
									Counting++;
								if (Script.Winner >= 0 && State.Winner < 0)
									Won++;
							}
						}
					}
				}
			}
		}
	}
	T.AssertEquals(3 * 4 * 2 * 3 * 9 * 4 * 4, Cases, "every case compared");
	T.AssertEquals(0, Differences, "the script and the simulation agree on every case");
	T.AssertTrue(Counting > 0, "the cases do raise the count");
	T.AssertTrue(Won > 0, "the cases do end the count with a winner");
}
