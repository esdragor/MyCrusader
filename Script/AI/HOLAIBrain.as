// The AI slots' brain (IA0.4, IA1.5, IA1.6, IA1.7): the C++ base (UHOLAIBrainSubsystem) calls Think for each AI slot every
// hol.AI.ThinkSteps steps, on the server only. Think reads the summary, asks the opening rules (HOLAIOpening.as, pure,
// tested in script) what to build, queue and gather, and sends the orders (UHOLAIOrderLibrary, HOLAIOrder::): the next
// building (the houses before the cap, lumber camp, mill, the camps where a massif is cut, the farm block when the berries
// run low) with one Peasant to build it, a Peasant queued at the centre with
// its rally on the resource that lacks Peasants, and the idle Peasants spread over the resources by their needs
// (HOLAINeeds.as: what the slot wants to spend over the minute, a source used up sending its Peasants on).
// The builders a slot has sent to sites not finished yet (IA1.7): not sent again, so that several sites go up at once.
struct FHOLAIBuilders
{
	TArray<int> Ids;
}

// The military buildings of a slot whose rally was set (IA2.4), and the point it was set to (sent again when the point moves).
struct FHOLAIRallied
{
	TArray<FVector> Buildings;
	FVector Point;
}

class UHOLAIBrain : UHOLAIBrainSubsystem
{
	// Thoughts this brain ran, all slots together.
	int NumScriptThoughts = 0;

	// The source each slot's centre rally was last set on (the order is sent again only when it changes).
	TMap<int, int> RallySources;

	// The farm block each slot is building (IA1.7: the spots of FindAIFarmBlock), and the builders it has sent to sites.
	TMap<int, FHOLAIFarmPlan> FarmPlans;

	// What each slot's Peasants' flight remembers (IA2.6f).
	TMap<int, FHOLAIFlight> Flights;
	TMap<int, FHOLAIBuilders> SentBuilders;

	// The Peasants each slot has sent to rebuild its fallen forum (IA3.2), and when its King was last ordered (IA3.4).
	TMap<int, FHOLAIForumMemory> Rebuilds;
	TMap<int, FHOLAIKingMemory> Kings;

	// What each slot's relay detachment remembers (R6, RQ3 = B, D589).
	TMap<int, FHOLAIRelayMemory> RelayMemories;

	// The main talent family each slot drew at its first thought (T6, D424).
	TMap<int, int> TalentMains;

	// The duel (D4, D9): the game second each slot first saw a challenge waiting for it (its reaction time runs from there), and the
	// game second of its last challenge sent (it asks again only after DuelAskEvery).
	TMap<int, float> DuelSeen;
	TMap<int, float> DuelAsked;

	// The last "forum attacked" situation written in the journal, per slot (IA4, D579).
	TMap<int, FHOLAIForumLogged> ForumLogged;

	// The slots whose level was written in the journal (once, at the first army thought).
	TMap<int, bool> LevelLogged;

	// What each slot's feint (Normal) and snipes (Hard) remember (IA4, D579).
	TMap<int, FHOLAIFeintMemory> Feints;
	TMap<int, FHOLAISnipeMemory> Snipes;

	// The level each slot plays now, to write its change (Evolving passes from one to the next).
	TMap<int, EHOLAIDifficulty> LevelsNow;

	// The rally each slot's military buildings were last given.
	TMap<int, FHOLAIRallied> Rallied;

	// What each slot's defence remembers (IA2.5: defending or at home, the soldiers just sent back to the assembly point), and the orders
	// it has given (regroup orders, defence orders: the status line shows them).
	TMap<int, FHOLAIDefence> Defences;
	TMap<int, int> RegroupCounts;
	TMap<int, int> DefenceCounts;

	// The attack wave of each slot (IA2.6: home, out or back, the soldiers it launched, the cooldown).
	TMap<int, FHOLAIWave> Waves;

	// The step of each slot's last Market trade (M6).
	TMap<int, int64> MarketSteps;

	// What each slot's farm block stood like (IAQ-S-2, D621), to tell when its mill or a farm is destroyed.
	TMap<int, FHOLAIBlockSeen> BlockSeens;

	// Since when the idle Peasants of each slot have had no safe source (IAQ-S, D619).
	TMap<int, FHOLAIStranded> Strandeds;

	// The step of the last status line per slot.
	TMap<int, int64> LastStatusStep;

	// What each slot remembers of its sites under way, to build again the ones whose builder left (HOLAIOpening::ResumeSite).
	TMap<int, FHOLAISiteMemory> SiteMemories;

	// The last reason a building could not be placed, per slot (written again only when it changes).
	TMap<int, FString> LastBuildReason;

	UFUNCTION(BlueprintOverride)
	bool Think(int PlayerIndex)
	{
		NumScriptThoughts++;
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(PlayerIndex);
		FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
		ReadFigures(Data);
		const int64 Step = GetStepIndex();
		Data.GameSeconds = float(Step) / 20.f;
		LogStatus(PlayerIndex, Step, Summary);
		const int NumIdle = Summary.IdlePeasants.Num();
		TArray<int> Free = Summary.IdlePeasants;

		// 0. The forum fallen (IA3.2): Peasants go and rebuild it (all of them on the last life), and stay out of the ordinary work.
		FHOLAIForumMemory Rebuild;
		Rebuilds.Find(PlayerIndex, Rebuild);
		const TArray<int> Rebuilders = HOLAIForum::RebuildOrders(Rebuild, Summary, HOLAIForum::DefaultData(), Data.GameSeconds);
		Rebuilds.Add(PlayerIndex, Rebuild);
		if (Rebuilders.Num() > 0)
		{
			if (HOLAIOrder::IssueAIBuildOrder(PlayerIndex, Summary.OwnForum.Location, Rebuilders))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Rebuilders.Num()} Peasants rebuild the forum ({Summary.OwnForum.Lives} lives left)");
			}
			else
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: rebuild order refused");
			}
		}
		for (int Index = 0; Index < Rebuild.Sent.Num(); Index++)
		{
			Free.Remove(Rebuild.Sent[Index]);
		}

		// 1. The next building (house, camp, mill, farm block: HOLAIOpening::ChooseBuild), placed by the rules of its type, one
		// Peasant sent to build it, not one already sent to another site.
		FHOLAIFarmPlan Plan;
		FarmPlans.Find(PlayerIndex, Plan);
		FHOLAIBuilders Sent;
		SentBuilders.Find(PlayerIndex, Sent);
		if (HOLAIOpening::SitesUnderWay(Summary) == 0)
		{
			Sent.Ids.Empty();
		}

		// 0b. A site whose builder left it (ran from a soldier) is built again by another Peasant.
		FHOLAISiteMemory Sites;
		SiteMemories.Find(PlayerIndex, Sites);
		FVector ResumedSite;
		const int Resumer = HOLAIOpening::ResumeSite(Sites, Summary, Sent.Ids, Data.GameSeconds, ResumedSite);
		SiteMemories.Add(PlayerIndex, Sites);
		if (Resumer >= 0)
		{
			TArray<int> One;
			One.Add(Resumer);
			if (HOLAIOrder::IssueAIBuildOrder(PlayerIndex, ResumedSite, One))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Peasant {Resumer} builds the abandoned site at ({ResumedSite.X}, {ResumedSite.Y}) again");
				Free.Remove(Resumer);
				Sent.Ids.Add(Resumer);
			}
		}
		// 0c. IAQ-S-2 (D621): the mill or a farm destroyed is built again near the refuge (40 m behind the centre), at the normal cost.
		Plan = RelocateDestroyed(PlayerIndex, Step, Summary, Data, Plan);
		FarmPlans.Add(PlayerIndex, Plan);
		FHOLAIBuildChoice Build = HOLAIOpening::ChooseBuild(Summary, Data, Plan, Sent.Ids);
		if (Build.bPlanFarms)
		{
			Plan = PlanFarmBlock(PlayerIndex, Summary, Data);
			if (Plan.bActive)
			{
				FarmPlans.Add(PlayerIndex, Plan);
				Build = HOLAIOpening::ChooseBuild(Summary, Data, Plan, Sent.Ids);
			}
		}
		if (Build.bBuild)
		{
			FVector Spot = Build.Spot;
			bool bFound = true;
			if (!Build.bAtSpot)
			{
				bFound = HOLAIOrder::FindAIPlacement(PlayerIndex, Build.Type, Build.Anchor, Data.Rings, Spot);
			}
			if (!bFound)
			{
				LogBuildOnce(PlayerIndex, Step, f"no spot for {Build.Type}");
			}
			else
			{
				TArray<int> Builders;
				Builders.Add(Build.Builder);
				if (HOLAIOrder::IssueAIPlaceOrder(PlayerIndex, Build.Type, Spot, Builders))
				{
					Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: build {Build.Type} at ({Spot.X}, {Spot.Y}) by #{Build.Builder}");
					Free.Remove(Build.Builder);
					Sent.Ids.Add(Build.Builder);
				}
				else
				{
					LogBuildOnce(PlayerIndex, Step, f"build {Build.Type} refused");
					if (Build.bAtSpot)
					{
						// The planned spot is not free any more (another building came): the block is planned again.
						FarmPlans.Remove(PlayerIndex);
					}
				}
			}
		}
		SentBuilders.Add(PlayerIndex, Sent);

		// 2. A Peasant queued at the centre, new ones sent to the resource that lacks Peasants.
		FHOLAIProduceChoice Produce = HOLAIOpening::ChooseProduction(Summary, Data);
		if (Produce.bProduce)
		{
			int LastRally = -2;
			RallySources.Find(PlayerIndex, LastRally);
			if (Produce.RallyResource >= 0 && LastRally != Produce.RallySource)
			{
				if (HOLAIOrder::IssueAISetRallyOrder(PlayerIndex, Produce.Centre, Produce.RallyPoint, Produce.RallySource))
				{
					RallySources.Add(PlayerIndex, Produce.RallySource);
				}
			}
			if (HOLAIOrder::IssueAIProduceOrder(PlayerIndex, Produce.Centre, n"Peasant"))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: queue a Peasant, rally on source #{Produce.RallySource}");
			}
		}

		// 3. The military buildings (IA2.4): a unit queued in each that can take one, their soldiers' rally on a point toward the enemy.
		ThinkMilitary(PlayerIndex, Step, Summary, Data, Produce.bProduce ? 1 : 0);

		// 3b. The army (IA2.5): the base defence when enemy soldiers come near, otherwise the regrouping at the assembly point.
		ThinkArmy(PlayerIndex, Step, Summary, Data);

		// 3b2. The Market (M6, MK-Q3 provisional): with a finished one, a surplus buys what is missing (HOLAIMarket::Choose).
		ThinkMarket(PlayerIndex, Step, Summary, Data);

		// 3c. The King (IA3.4, Regicide): behind the forum, away from the enemy soldiers that come near him.
		FHOLAIKingMemory KingMemory;
		Kings.Find(PlayerIndex, KingMemory);
		FVector KingWhere;
		if (HOLAIForum::KingOrder(KingMemory, Summary, FHOLAIKingData(), Data.GameSeconds, KingWhere))
		{
			TArray<int> KingIds;
			KingIds.Add(Summary.King.Id);
			if (HOLAIOrder::IssueAIMoveOrder(PlayerIndex, KingIds, KingWhere))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: King goes to ({KingWhere.X}, {KingWhere.Y})");
			}
		}
		Kings.Add(PlayerIndex, KingMemory);

		// 3d. The hero's talent points (T6, D424): spent on the main family drawn at the start, in order.
		ThinkTalent(PlayerIndex, Step);

		// 3e. The duel (D4, D9): the answer to a challenge received, or a challenge when a fight is near.
		ThinkDuel(PlayerIndex, Step, Summary, Data);

		// 4. The idle Peasants (the builder left out) spread by the needs, one gather order per resource.
		FHOLAIStranded Stranded;
		Strandeds.Find(PlayerIndex, Stranded);
		if (NumIdle == 0)
		{
			Stranded.Since = -1.f;
			Strandeds.Add(PlayerIndex, Stranded);
			return true;
		}
		// IAQ-S (D619): with every source under an enemy soldier the Peasants wait 30 s, then go to the source farthest from any soldier.
		const bool bWaited = Stranded.Since >= 0.f;
		TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::StrandedGroups(Stranded, Summary, Data, Free, Data.GameSeconds);
		Strandeds.Add(PlayerIndex, Stranded);
		if (bWaited && Groups.Num() > 0 && Stranded.Since == Data.GameSeconds)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: every source is under an enemy soldier, {Free.Num()} idle Peasants leave after 30 s for the source farthest from them");
		}
		if (Groups.Num() == 0)
		{
			if (Free.Num() > 0)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> no source");
			}
			return true;
		}
		// One line per decision (IA0.7 and quick_check read them): "AI slot 1 step 40: 3 idle -> gather Food #12" for the
		// first group, then "AI slot 1 step 40: split Wood #20 x2" for each other.
		for (int Index = 0; Index < Groups.Num(); Index++)
		{
			const FHOLAIGatherGroup Group = Groups[Index];
			const bool bQueued = HOLAIOrder::IssueAIGatherOrder(PlayerIndex, Group.Peasants, Group.SourceIndex);
			const FString Refused = bQueued ? "" : " refused";
			if (Index == 0)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {NumIdle} idle -> gather {Group.ResourceName} #{Group.SourceIndex}{Refused}");
			}
			else
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: split {Group.ResourceName} #{Group.SourceIndex} x{Group.Peasants.Num()}{Refused}");
			}
		}
		return true;
	}

	// The hero's point, if one is in reserve (HOLAITalent::Choose): the order of the talent of the main family, the same order a player's button sends.
	// The main family is drawn once per slot from its index (HOLAITalent::DrawMain; the prototype has one family with effects, so the roll is moot).
	void ThinkTalent(int PlayerIndex, int64 Step)
	{
		const int Count = HOLHero::GetHeroTalentCount(PlayerIndex);
		if (Count < 0)
		{
			return;
		}
		int Main = HOLAITalent::FamilyNone;
		if (!TalentMains.Find(PlayerIndex, Main))
		{
			Main = HOLAITalent::DrawMain(PlayerIndex);
			TalentMains.Add(PlayerIndex, Main);
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: talent profile main family {Main}");
		}
		TArray<int> Picks;
		for (int Index = 0; Index < Count; Index++)
		{
			Picks.Add(HOLHero::GetHeroTalentFamily(PlayerIndex, Index));
		}
		const int Family = HOLAITalent::Choose(HOLHero::GetHeroLevel(PlayerIndex), Picks, Main);
		if (Family != HOLAITalent::FamilyNone && HOLAIOrder::IssueAITalentOrder(PlayerIndex, Family))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: talent of family {Family} in row {HOLTalent::FirstRow + Count}");
		}
	}

	// The percent of a hero's health (his slot's sheet), 100 when his maximum is unknown.
	int HeroHealthPercent(int Slot)
	{
		const int MaxHealth = HOLHero::GetHeroMaxHealth(Slot);
		if (MaxHealth <= 0)
			return 100;
		return Math::Clamp(Math::IntegerDivisionTrunc(HOLHero::GetHeroHealth(Slot) * 100, MaxHealth), 0, 100);
	}

	// What the AI sees of a duel with the enemy hero of EnemySlot standing at EnemyWhere: the two heroes' level and health, and the moment
	// (its soldiers within 15 m of him, some of them fighting). The kit is not in the game yet (D6): counted ready.
	FHOLAIDuelView DuelViewOf(int PlayerIndex, int EnemySlot, const FVector& EnemyWhere, const FHOLAISummary& Summary)
	{
		FHOLAIDuelView View;
		View.MyLevel = Math::Max(1, HOLHero::GetHeroLevel(PlayerIndex));
		View.EnemyLevel = Math::Max(1, HOLHero::GetHeroLevel(EnemySlot));
		View.MyHealthPercent = HeroHealthPercent(PlayerIndex);
		View.EnemyHealthPercent = HeroHealthPercent(EnemySlot);
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			FVector Away = Summary.Army[Index].Location - EnemyWhere;
			Away.Z = 0;
			if (Away.Size() <= 1500.0)
			{
				View.TroopsNearMe++;
				View.bBattleNear = View.bBattleNear || Summary.Army[Index].TargetId != -1;
			}
		}
		View.bMyHeroInDanger = View.bBattleNear && View.MyHealthPercent <= 30;
		return View;
	}

	// The duel (D4, D9): a challenge waiting for the slot is answered after the AI's reaction time (HOLAIDuel::Answer); with none, and
	// an enemy hero within the arena's reach of its soldiers, it may challenge (HOLAIDuel::Challenge). The orders are those a player's
	// commands send; the rules of the book refuse what is not allowed.
	void ThinkDuel(int PlayerIndex, int64 Step, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		const int Code = HOLHero::GetHeroDuelCode(PlayerIndex);
		const FHOLAIDuelSettings Settings = HOLAIDuel::Preset(Summary.Level);
		// The error of its estimate, drawn from the slot and the second (the rules are pure: the draw is here), -100..100.
		const int Noise = ((PlayerIndex * 37 + int(Data.GameSeconds) * 53) % 201) - 100;
		if (Code == 2)
		{
			float Seen = Data.GameSeconds;
			if (!DuelSeen.Find(PlayerIndex, Seen))
			{
				Seen = Data.GameSeconds;
				DuelSeen.Add(PlayerIndex, Seen);
			}
			if (Data.GameSeconds - Seen < float(Settings.ReactionMs) / 1000.f)
				return;
			const int Other = HOLHero::GetHeroDuelOpponent(PlayerIndex);
			FVector Where = FVector::ZeroVector;
			for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
			{
				if (Summary.Enemies[Index].OwnerSlot == Other && Summary.Enemies[Index].Type == n"Hero")
					Where = Summary.Enemies[Index].Location;
			}
			const FHOLAIDuelDecision Decision = HOLAIDuel::Answer(Settings, DuelViewOf(PlayerIndex, Other, Where, Summary), Noise);
			if (HOLAIOrder::IssueAIDuelAnswer(PlayerIndex, Decision.bYes))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: " + HOLAIDuel::Describe("answer", Decision));
				DuelSeen.Remove(PlayerIndex);
			}
			return;
		}
		DuelSeen.Remove(PlayerIndex);
		float Asked = -1000.f;
		DuelAsked.Find(PlayerIndex, Asked);
		if (Code != 0 || HOLHero::GetHeroState(PlayerIndex) != EHOLHeroState::Alive || Data.GameSeconds - Asked < 20.f)
			return;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (Enemy.Type != n"Hero" || HOLHero::GetHeroState(Enemy.OwnerSlot) != EHOLHeroState::Alive)
				continue;
			// Within the arena's reach of some soldier of its own (its hero marches with them).
			bool bNear = false;
			for (int Each = 0; Each < Summary.Army.Num() && !bNear; Each++)
			{
				FVector Away = Summary.Army[Each].Location - Enemy.Location;
				Away.Z = 0;
				bNear = Away.Size() <= 1200.0;
			}
			if (!bNear)
				continue;
			const FHOLAIDuelDecision Decision = HOLAIDuel::Challenge(Settings, DuelViewOf(PlayerIndex, Enemy.OwnerSlot, Enemy.Location, Summary), true, Noise);
			if (Decision.bYes && HOLAIOrder::IssueAIDuelChallenge(PlayerIndex, Enemy.OwnerSlot))
			{
				DuelAsked.Add(PlayerIndex, Data.GameSeconds);
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: challenge slot {Enemy.OwnerSlot}: " + HOLAIDuel::Describe("challenge", Decision));
				return;
			}
		}
	}

	// The orders of HOLAIOpening::ChooseMilitary: the unit queued in each building, the rally set once per building (again when the point moves).
	void ThinkMilitary(int PlayerIndex, int64 Step, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data, int Reserved)
	{
		const FHOLAIMilitaryChoice Military = HOLAIOpening::ChooseMilitary(Summary, Data, Reserved);
		if (Military.Orders.Num() == 0)
		{
			return;
		}
		FHOLAIRallied Memory;
		Rallied.Find(PlayerIndex, Memory);
		FVector Moved = Memory.Point - Military.RallyPoint;
		Moved.Z = 0;
		if (Moved.Size() > 100.0)
		{
			Memory.Buildings.Empty();
			Memory.Point = Military.RallyPoint;
		}
		for (int Index = 0; Index < Military.Orders.Num(); Index++)
		{
			const FHOLAIMilitaryOrder Order = Military.Orders[Index];
			bool bKnown = false;
			for (int Each = 0; Each < Memory.Buildings.Num(); Each++)
			{
				FVector Away = Memory.Buildings[Each] - Order.Location;
				Away.Z = 0;
				bKnown = bKnown || Away.Size() < 100.0;
			}
			if (!bKnown && HOLAIOrder::IssueAISetRallyOrder(PlayerIndex, Order.Location, Military.RallyPoint, -1))
			{
				Memory.Buildings.Add(Order.Location);
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: rally of {Order.Building} on ({Military.RallyPoint.X}, {Military.RallyPoint.Y})");
			}
			if (Order.bQueue && HOLAIOrder::IssueAIProduceOrder(PlayerIndex, Order.Location, Order.Unit))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: queue a {Order.Unit} at {Order.Building}");
			}
		}
		Memory.Point = Military.RallyPoint;
		Rallied.Add(PlayerIndex, Memory);
	}

	// The Market (M6, MK-Q3): one trade at a time, at most one every MarketGapSteps steps (20 s). The brain does not build the Market
	// yet (the opening's steps and the AI's reaching tier II come with the answer to MK-Q3): this serves a slot that has a finished one.
	// The numbers of the rule are HOLAIMarket's (MinDifficulty is the answer to MK-Q3 = C: 2, Hard only; Easy and Normal never touch the Market).
	void ThinkMarket(int PlayerIndex, int64 Step, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FVector MarketAt;
		bool bReady = false;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			const FHOLAIBuilding Each = Summary.Buildings[Index];
			if (Each.TypeName == n"Market" && Each.State == EHOLConstructionState::Finished)
			{
				MarketAt = Each.Location;
				bReady = true;
				break;
			}
		}
		if (!bReady)
		{
			return;
		}
		int64 Last = -100000;
		MarketSteps.Find(PlayerIndex, Last);
		if (Step - Last < 400)
		{
			return;
		}
		TArray<int> Prices;
		if (!HOLAIOrder::ReadAIMarketBook(Prices))
		{
			return;
		}
		FHOLMarketBook Book;
		Book.Prices = Prices;
		const TArray<float> Spend = HOLAINeeds::SpendOf(Summary, Data);
		TArray<int> Stock;
		TArray<int> Wanted;
		for (int Resource = 0; Resource < 4; Resource++)
		{
			Stock.Add(HOLAIOpening::PartOf(Summary, Resource).Stock);
			Wanted.Add(int(Spend[Resource]));
		}
		const FHOLAIMarketChoice Choice = HOLAIMarket::Choose(HOLMarket::DefaultSettings(), Book, FHOLAIMarketSettings(), Stock, Wanted, true, int(Summary.Level));
		if (!Choice.bTrade)
		{
			return;
		}
		MarketSteps.Add(PlayerIndex, Step);
		if (HOLAIOrder::IssueAIMarketOrder(PlayerIndex, MarketAt, Choice.Resource, Choice.Lots, Choice.bBuy))
		{
			const FString Side = Choice.bBuy ? "buy" : "sell";
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Market {Side} {Choice.Lots} lots of resource {Choice.Resource} ({Choice.Reason})");
		}
	}

	// The army's orders (IA2.5, IA2.6). The assembly point is the soldiers' rally point. When the rules want the defence (enemy soldiers
	// within the defence radius of the base, the army not far behind them), every soldier attack-moves toward the threat, the orders sent
	// again every few seconds; it ends after a quiet delay. At home, the idle soldiers far from the point attack-move back to it (a fight
	// on the way). The attack wave (HOLAIArmy::UpdateWave) goes out when the rules allow and the army is ahead, and falls back to the
	// point (a move, not an attack-move) when the ratio near it collapses; while it is out the defence and the regrouping order the
	// soldiers at home only, unless the threat is strong (AttackRecallCount enemies): then the wave is called back.
	void ThinkArmy(int PlayerIndex, int64 Step, const FHOLAISummary& FullSummary, const FHOLAIOpeningData& Data)
	{
		const float Now = Data.GameSeconds;
		FHOLAIDefence State;
		Defences.Find(PlayerIndex, State);
		FHOLAIWave Wave;
		Waves.Find(PlayerIndex, Wave);
		// The relays (R6, RQ3 = B, D589: hol.AI.RelayMode, default B): a detachment (or the whole army) goes to take them; its soldiers are left out
		// of the army the other rules see below (defence, regrouping, waves) while it is away.
		const FHOLAISummary Summary = ThinkRelays(PlayerIndex, Step, FullSummary, Data, Wave, State);
		const FVector Assembly = HOLAIOpening::RallyPointOf(Summary, Data);
		const FHOLAIThreat Whole = HOLAIArmy::Threat(Summary, Data.Army);
		const FHOLAISummary Rest = HOLAIArmy::WithoutWave(Summary, Wave);
		const FHOLAIThreat Threat = HOLAIArmy::Threat(Rest, Data.Army);
		const bool bWas = State.bDefending;
		// IA4 (D579): the forum attacked is weighed by the level; every situation is written down (lives, ratio, decision); Easy sends the army whatever the ratio.
		// The slot's level (D422) is read from its config into the summary (C++ Read, D579).
		const EHOLAIDifficulty Level = HOLAILives::Played(Summary.Level, Now, Data.Army.Lives);
		if (!LevelLogged.Contains(PlayerIndex))
		{
			LevelLogged.Add(PlayerIndex, true);
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: level {HOLAIForum::LevelName(Summary.Level)}");
		}
		EHOLAIDifficulty Played = Level;
		if (!LevelsNow.Find(PlayerIndex, Played) || Played != Level)
		{
			LevelsNow.Add(PlayerIndex, Level);
			if (Summary.Level == EHOLAIDifficulty::Evolving)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: evolving, plays {HOLAIForum::LevelName(Level)} now");
			}
		}
		const FHOLAIForumSituation Situation = HOLAIForum::Judge(Rest, Data.Army, Threat, Level);
		FHOLAIForumLogged Logged;
		ForumLogged.Find(PlayerIndex, Logged);
		if (HOLAIForum::ShouldLog(Logged, Situation))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {HOLAIForum::Describe(Situation, Level)}");
		}
		ForumLogged.Add(PlayerIndex, Logged);
		// The feint (Normal): falls back, goes round, then the rules meet the attackers; the life let go (Hard): the army is held back.
		FHOLAIFeintMemory Feint;
		Feints.Find(PlayerIndex, Feint);
		const FHOLAISummary Soldiers = HOLAIArmy::WithoutWave(Summary, Wave);
		const FHOLAIFeintStep FeintNow = HOLAILives::FeintStep(Feint, Soldiers, Data.Army, Threat, Situation, Now);
		Feints.Add(PlayerIndex, Feint);
		if (FeintNow.bPhaseChanged)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: feint, phase {int(Feint.Phase)} (1 back, 2 round, 3 strike), {Soldiers.Army.Num()} soldiers");
		}
		if (FeintNow.MoveIds.Num() > 0 && !HOLAIOrder::IssueAIMoveOrder(PlayerIndex, FeintNow.MoveIds, FeintNow.Where))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: feint order refused");
		}
		const bool bHeld = HOLAILives::ArmyHeld(Situation, FeintNow);
		HOLAIArmy::UpdateDefence(State, (HOLAIArmy::ShouldDefend(Rest, Data.Army, Threat, Assembly) && !bHeld) || HOLAIForum::DefendAnyway(Situation, Threat), Now, Data.Army);
		FHOLAISnipeMemory SnipeMemory;
		Snipes.Find(PlayerIndex, SnipeMemory);
		const TArray<FHOLAISnipe> Sniped = HOLAILives::SnipeOrders(SnipeMemory, Soldiers, Data.Army, Situation, Now);
		Snipes.Add(PlayerIndex, SnipeMemory);
		for (int Each = 0; Each < Sniped.Num(); Each++)
		{
			TArray<int> One;
			One.Add(Sniped[Each].SoldierId);
			if (HOLAIOrder::IssueAIAttackUnitOrder(PlayerIndex, One, Sniped[Each].TargetId))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: soldier {Sniped[Each].SoldierId} snipes enemy {Sniped[Each].TargetId}");
			}
		}
		if (State.bDefending != bWas)
		{
			const FString Change = State.bDefending ? "defence begins" : "defence ends, home";
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Change} ({Threat.Count} enemies, ratio {Threat.Ratio})");
		}

		// The wave: launched, kept on its way, called back or over.
		FHOLAIForumMemory Rebuild;
		Rebuilds.Find(PlayerIndex, Rebuild);
		// Hard (D579): the life is let go and the army is enough to crush the attacker's home: the wave goes now, and is not called back by the
		// enemies at our base (it went on purpose).
		if (Wave.Phase == EHOLAIWavePhase::Home)
		{
			Wave.bPunishing = false;
		}
		Wave.bForce = Situation.Call == EHOLAILifeCall::Let && Wave.Phase == EHOLAIWavePhase::Home;
		const bool bRecall = HOLAIForum::WaveRecalled(Summary, Data.Army, Whole, Situation) && !Wave.bPunishing;
		const FHOLAIWaveStep Wave_ = HOLAIArmy::UpdateWave(Wave, Summary, Data.Army, State.bDefending, bRecall, Assembly, Now);
		if (Wave_.bLaunched)
		{
			if (Wave.bPunishing)
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: punishes the attacker's home, the life is let go");
			}
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: attack launched with {Wave_.Left} soldiers, ratio {Wave_.Ratio}, toward ({Wave_.AttackTarget.X}, {Wave_.AttackTarget.Y})");
		}
		if (Wave_.bRetreat)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: retreat: ratio {Wave_.Ratio}, {Wave_.Left} left");
		}
		if (Wave_.bOver || Wave_.bHome)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: wave over, home");
		}
		if (Wave_.AttackIds.Num() > 0 && !HOLAIOrder::IssueAIAttackMoveOrder(PlayerIndex, Wave_.AttackIds, Wave_.AttackTarget))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: wave order refused");
		}
		if (Wave_.MoveIds.Num() > 0 && !HOLAIOrder::IssueAIMoveOrder(PlayerIndex, Wave_.MoveIds, Assembly))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: retreat order refused");
		}
		Waves.Add(PlayerIndex, Wave);

		// The soldiers at home (the whole army when no wave is out).
		const FHOLAISummary Home = HOLAIArmy::WithoutWave(Summary, Wave);
		if (State.bDefending)
		{
			const TArray<int> Ids = HOLAIArmy::DefenceOrders(State, Home, Data.Army, Threat, Now);
			if (Ids.Num() > 0 && HOLAIOrder::IssueAIAttackMoveOrder(PlayerIndex, Ids, Threat.Location))
			{
				int Count = 0;
				DefenceCounts.Find(PlayerIndex, Count);
				DefenceCounts.Add(PlayerIndex, Count + 1);
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: defend with {Ids.Num()} soldiers toward ({Threat.Location.X}, {Threat.Location.Y})");
			}
		}
		else
		{
			// (Not while the army is held back, D579: the feint's retreat and the soldiers kept out of the fight are not sent back to the assembly
			// point, which stands by the attackers: found in a real game, the regroup order undid the feint's first order in the same thought.)
			const TArray<int> Ids = HOLAILives::RegroupUnlessHeld(State, Home, Data.Army, Assembly, bHeld, Now);
			if (Ids.Num() > 0 && HOLAIOrder::IssueAIAttackMoveOrder(PlayerIndex, Ids, Assembly))
			{
				int Count = 0;
				RegroupCounts.Find(PlayerIndex, Count);
				RegroupCounts.Add(PlayerIndex, Count + 1);
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: regroup {Ids.Num()} soldiers to ({Assembly.X}, {Assembly.Y})");
			}
		}
		Defences.Add(PlayerIndex, State);

		// The Peasants near an enemy soldier run to the centre (IA2.6f); once there they are idle and the brain sends them to work again.
		FHOLAIFlight Flight;
		Flights.Find(PlayerIndex, Flight);
		// A Peasant that wins its duel against a weakened soldier near it finishes it off instead (IA4, D572).
		const FHOLAISummary CalmSummary = HOLAIForum::Calm(Summary, Rebuild);
		const TArray<FHOLAIStrike> Strikes = HOLAIArmy::StrikeOrders(Flight, CalmSummary, Data.Army, Now);
		for (int Each = 0; Each < Strikes.Num(); Each++)
		{
			TArray<int> One;
			One.Add(Strikes[Each].PeasantId);
			if (HOLAIOrder::IssueAIAttackUnitOrder(PlayerIndex, One, Strikes[Each].TargetId))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Peasant {Strikes[Each].PeasantId} finishes soldier {Strikes[Each].TargetId} off");
			}
		}
		const TArray<int> Runners = HOLAIArmy::FleeOrders(Flight, CalmSummary, Data.Army, Now);
		Flights.Add(PlayerIndex, Flight);
		// Normal that knows it loses keeps its economy: the Peasants run to the refuge, not to the attacked centre (HOLAILives::FleeTarget).
		const FVector FleeTo = HOLAILives::FleeTarget(Summary, Data.Army, Situation, Threat);
		if (Runners.Num() > 0 && Summary.bHasCentre && HOLAIOrder::IssueAIMoveOrder(PlayerIndex, Runners, FleeTo))
		{
			const FString Where = (FleeTo - Summary.CentreLocation).Size() < 1.f ? "the centre" : "the refuge";
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Runners.Num()} Peasants run to {Where}");
		}
	}

	// The AI and the relays (R6): see Script/AI/HOLAIRelay.as. Returns the summary without the detached soldiers.
	FHOLAISummary ThinkRelays(int PlayerIndex, int64 Step, const FHOLAISummary& FullSummary, const FHOLAIOpeningData& Data, const FHOLAIWave& Wave, const FHOLAIDefence& State)
	{
		FHOLAIRelayMemory Memory;
		RelayMemories.Find(PlayerIndex, Memory);
		Memory.Slot = PlayerIndex;
		const int Mode = HOLRelayRules::GetAIRelayMode();
		TArray<FHOLAIRelay> Relays;
		if (Mode != HOLAIRelay::ModeIgnore || Memory.Ids.Num() > 0)
		{
			HOLRelayRules::ReadAIRelays(Relays);
		}
		if (Relays.Num() == 0 && Memory.Ids.Num() == 0)
		{
			return FullSummary;
		}
		const float Now = Data.GameSeconds;
		const EHOLAIDifficulty Level = HOLAILives::Played(FullSummary.Level, Now, Data.Army.Lives);
		const bool bAttacked = HOLAIArmy::Threat(FullSummary, Data.Army).bThreatened || State.bDefending;
		const FVector From = FullSummary.bHasCentre ? FullSummary.CentreLocation : FVector(0, 0, 0);
		const FHOLAIRelayStep Result = HOLAIRelay::Step(Memory, Mode, Level, HOLAIArmy::WithoutWave(FullSummary, Wave), Relays, bAttacked, Now, From, FHOLAIRelayData());
		RelayMemories.Add(PlayerIndex, Memory);
		if (Result.Reason.Len() > 0)
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: relay: {Result.Reason}");
		}
		if (Result.MoveIds.Num() > 0 && !HOLAIOrder::IssueAIAttackMoveOrder(PlayerIndex, Result.MoveIds, Result.Where))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: relay order refused");
		}
		return HOLAIRelay::Without(FullSummary, Memory);
	}
	// The farm block around the slot's centre: the mill and the farms where they fit (FindAIFarmBlock); not active when it
	// does not fit within the rings.
	// The mill or the farms of the planned block that stood and are gone (BlockLoss) get new spots around the refuge (FindAIFarmBlock there);
	// the opening is told that the slot had a mill and has none (the next one goes near the refuge too). Nothing destroyed: the plan as it was.
	FHOLAIFarmPlan RelocateDestroyed(int PlayerIndex, int64 Step, const FHOLAISummary& Summary, FHOLAIOpeningData& Data, FHOLAIFarmPlan Planned)
	{
		FHOLAIFarmPlan Plan = Planned;
		FHOLAIBlockSeen Seen;
		BlockSeens.Find(PlayerIndex, Seen);
		const FHOLAIBlockLoss Loss = HOLAIOpening::BlockLoss(Seen, Summary, Plan);
		Data.bMillLost = Seen.bAnyMill && !HOLAIOpening::HasBuilding(Summary, n"Mill");
		if (Loss.bMill || Loss.Farms.Num() > 0)
		{
			FVector NewMill;
			TArray<FVector> NewFarms;
			if (HOLAIOrder::FindAIFarmBlock(PlayerIndex, HOLAIOpening::RefugeCentreOf(Summary, Data), Data.FarmRings, NewMill, NewFarms))
			{
				HOLAIOpening::RelocateLost(Plan, Seen, Loss, NewMill, NewFarms);
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: farm block destroyed (mill {Loss.bMill}, {Loss.Farms.Num()} farms), built again near the refuge ({NewMill.X}, {NewMill.Y})");
			}
			else
			{
				LogBuildOnce(PlayerIndex, Step, "no room near the refuge for the farm block");
			}
		}
		BlockSeens.Add(PlayerIndex, Seen);
		return Plan;
	}

	FHOLAIFarmPlan PlanFarmBlock(int PlayerIndex, const FHOLAISummary& Summary, const FHOLAIOpeningData& Data)
	{
		FHOLAIFarmPlan Plan;
		FVector Mill;
		TArray<FVector> Farms;
		if (!HOLAIOrder::FindAIFarmBlock(PlayerIndex, Summary.CentreLocation, Data.FarmRings, Mill, Farms))
		{
			LogBuildOnce(PlayerIndex, GetStepIndex(), "no room for the farm block");
			return Plan;
		}
		Plan.bActive = true;
		Plan.Mill = Mill;
		Plan.Farms = Farms;
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {GetStepIndex()}: farm block planned, mill at ({Mill.X}, {Mill.Y}), {Farms.Num()} farms");
		return Plan;
	}

	// The costs and rates of the data from the type tables (UnitTypes.csv, BuildingTypes.csv), over the values DefaultData holds as
	// a fallback: nothing of the tables is copied in the script any more (IA1.6).
	void ReadFigures(FHOLAIOpeningData& Data)
	{
		FHOLAITypeFigures Figures;
		if (HOLAIOrder::ReadAIUnitFigures(n"Peasant", Figures))
		{
			Data.Peasant = Figures;
		}
		if (HOLAIOrder::ReadAIBuildingFigures(n"House", Figures))
		{
			Data.House = Figures;
		}
		if (HOLAIOrder::ReadAIBuildingFigures(n"Farm", Figures))
		{
			Data.Farm = Figures;
		}
		if (HOLAIOrder::ReadAIBuildingFigures(n"MiningCamp", Figures))
		{
			Data.MiningCamp = Figures;
		}
		for (int Index = 0; Index < Data.Steps.Num(); Index++)
		{
			if (HOLAIOrder::ReadAIBuildingFigures(Data.Steps[Index].Type, Figures))
			{
				Data.Steps[Index].WoodCost = Figures.CostWood;
			}
		}
		for (int Index = 0; Index < Data.ArmySteps.Num(); Index++)
		{
			if (HOLAIOrder::ReadAIBuildingFigures(Data.ArmySteps[Index].Type, Figures))
			{
				Data.ArmySteps[Index].WoodCost = Figures.CostWood;
			}
		}
		HOLAIArmy::LoadFigures(Data.Army);
	}

	// One line every 100 steps (5 s) per slot for the tests of the AI games (quick_check aiduel reads it): population, the queue
	// of the centre, the four stocks, the Peasants and the buildings that matter.
	void LogStatus(int PlayerIndex, int64 Step, const FHOLAISummary& Summary)
	{
		int64 Last = -1000;
		LastStatusStep.Find(PlayerIndex, Last);
		if (Step - Last < 100)
		{
			return;
		}
		LastStatusStep.Add(PlayerIndex, Step);
		int Queued = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			Queued += Summary.Buildings[Index].Queue.Num();
		}
		const int Houses = HOLAIOpening::CountBuildings(Summary, n"House");
		const int Camps = HOLAIOpening::CountBuildings(Summary, n"LumberCamp");
		const int Mills = HOLAIOpening::CountBuildings(Summary, n"Mill");
		const int Farms = HOLAIOpening::CountBuildings(Summary, n"Farm");
		const int Spears = CountOfType(Summary, n"Spearman");
		const int Men = CountOfType(Summary, n"ManAtArms");
		const int Archers = CountOfType(Summary, n"Archer");
		const int Crossbows = CountOfType(Summary, n"Crossbowman");
		const int Barracks = HOLAIOpening::CountBuildings(Summary, n"Barracks");
		const int Ranges = HOLAIOpening::CountBuildings(Summary, n"ArcheryRange");
		const int BarracksQueue = QueueOf(Summary, n"Barracks");
		const int RangeQueue = QueueOf(Summary, n"ArcheryRange");
		FHOLAIDefence Defence;
		Defences.Find(PlayerIndex, Defence);
		int Regroups = 0;
		RegroupCounts.Find(PlayerIndex, Regroups);
		int Meetings = 0;
		DefenceCounts.Find(PlayerIndex, Meetings);
		const FString Stance = Defence.bDefending ? "defend" : "home";
		FHOLAIWave Wave;
		Waves.Find(PlayerIndex, Wave);
		const FString WaveState = Wave.Phase == EHOLAIWavePhase::Out ? "out" : (Wave.Phase == EHOLAIWavePhase::Back ? "back" : "off");
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: status pop {Summary.PopulationUsed}/{Summary.PopulationCap} queued {Queued} stock {Summary.Food.Stock} {Summary.Wood.Stock} {Summary.Stone.Stock} {Summary.Gold.Stock} peasants {Summary.PeasantIds.Num()} houses {Houses} camps {Camps} mills {Mills} farms {Farms} army S{Spears} M{Men} A{Archers} C{Crossbows} barracks {Barracks} q{BarracksQueue} range {Ranges} q{RangeQueue} {Stance} regroups {Regroups} defences {Meetings} wave {WaveState} size {HOLAIArmy::WaveSize(Wave, Summary)}");
	}

	// The soldiers of a type alive (the army of the summary: those of the slot that do not gather).
	int CountOfType(const FHOLAISummary& Summary, FName Type)
	{
		int Count = 0;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			Count += Summary.Army[Index].Type == Type ? 1 : 0;
		}
		return Count;
	}

	// The units waiting in the queues of the buildings of a type (the one in production included).
	int QueueOf(const FHOLAISummary& Summary, FName Type)
	{
		int Count = 0;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == Type)
			{
				Count += Summary.Buildings[Index].Queue.Num();
			}
		}
		return Count;
	}

	// Writes the reason unless it is the one just written for this slot (the thought comes every few steps).
	void LogBuildOnce(int PlayerIndex, int64 Step, FString Reason)
	{
		FString Last;
		if (LastBuildReason.Find(PlayerIndex, Last) && Last == Reason)
		{
			return;
		}
		LastBuildReason.Add(PlayerIndex, Reason);
		Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Reason}");
	}
}
