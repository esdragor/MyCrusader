// Port of HOLOrderChainTests.cpp (HeroesOfLiberty.Orders.Chains, .BuildingQueue) against UHOLOrderChainReadLibrary (L17-09, ELROND-1117c, D830/D839).
// Same vectors, names adapted. NOT ported (the library does not expose them): EndBuildingTarget (a template over TWeakObjectPtr), so every
// assertion that follows an EndBuildingTarget call stays in the C++ test HeroesOfLiberty.Orders.BuildingQueue.

// One unit as the order processor sees it: its current order and its queue, advanced as StartQueuedMoves does.
struct FOCUnitModel
{
	bool bWalking = false;
	bool bOrderedTarget = false;
	bool bDroppingOff = false;
	int TargetId = 0;
	int OrderedId = 0;
	TArray<FHOLOrderChainStep> Queue;
}

bool OCIsOver(const FOCUnitModel& Unit)
{
	return HOLOrderChainRead::IsCurrentOrderOver(Unit.bWalking, Unit.bOrderedTarget, Unit.OrderedId != 0, false, Unit.bDroppingOff);
}

bool OCMaySeek(const FOCUnitModel& Unit, bool bAttackMove)
{
	return HOLOrderChainRead::MaySeek(Unit.bWalking, bAttackMove, Unit.bOrderedTarget, Unit.OrderedId != 0, false);
}

bool OCAdvance(FOCUnitModel& Unit)
{
	if (!OCIsOver(Unit) || Unit.Queue.Num() == 0)
	{
		return false;
	}
	FHOLOrderChainStep Next = Unit.Queue[0];
	Unit.Queue.RemoveAt(0);
	if (Next.Kind == EHOLOrderChainStepKind::Attack)
	{
		Unit.bOrderedTarget = true;
	}
	else if (Next.Kind == EHOLOrderChainStepKind::AttackBuilding)
	{
		Unit.TargetId = Next.TargetId;
		Unit.OrderedId = Next.TargetId;
	}
	else if (Next.Kind == EHOLOrderChainStepKind::Deposit)
	{
		Unit.bDroppingOff = true;
	}
	else
	{
		Unit.bWalking = true;
	}
	return true;
}

FHOLOrderChainUnitSteps OCUnit(int Id, const TArray<FHOLOrderChainStep>& Steps)
{
	FHOLOrderChainUnitSteps Unit;
	Unit.UnitId = Id;
	Unit.Steps = Steps;
	return Unit;
}

TArray<FHOLOrderChainStep> OCSteps1(FHOLOrderChainStep A)
{
	TArray<FHOLOrderChainStep> Steps;
	Steps.Add(A);
	return Steps;
}

// A group sharing a queue: one chain, its units in order.
void Test_OrderChains_Group(FUnitTest& T)
{
	FHOLOrderChainStep MoveA = HOLOrderChainRead::MakeMove(FVector(1000, 0, 0), false);
	FHOLOrderChainStep MoveB = HOLOrderChainRead::MakeMove(FVector(2000, 500, 0), false);
	FHOLOrderChainStep AttackMoveC = HOLOrderChainRead::MakeMove(FVector(3000, 0, 0), true);
	TArray<FHOLOrderChainStep> Three;
	Three.Add(MoveA);
	Three.Add(MoveB);
	Three.Add(AttackMoveC);
	TArray<FHOLOrderChainUnitSteps> Units;
	for (int Id = 1; Id <= 4; ++Id)
	{
		Units.Add(OCUnit(Id, Three));
	}
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(1, Chains.Num(), "Group: one chain");
	if (Chains.Num() == 1)
	{
		T.AssertEquals(4, Chains[0].UnitIds.Num(), "Group: 4 units");
		T.AssertEquals(3, Chains[0].Steps.Num(), "Group: 3 steps");
	}
}

// Peasants with different queues: one chain each, units without steps left out, order stable.
void Test_OrderChains_Peasants(FUnitTest& T)
{
	TArray<FHOLOrderChainUnitSteps> Units;
	TArray<int> Ids;
	Ids.Add(10);
	Ids.Add(11);
	Ids.Add(12);
	Ids.Add(13);
	TArray<int> Sources;
	Sources.Add(3);
	Sources.Add(0);
	Sources.Add(4);
	Sources.Add(3);
	for (int Index = 0; Index < 4; ++Index)
	{
		TArray<FHOLOrderChainStep> Steps;
		if (Index != 1)
		{
			Steps.Add(HOLOrderChainRead::MakeBuild(7));
			Steps.Add(HOLOrderChainRead::MakeGather(Sources[Index]));
		}
		Units.Add(OCUnit(Ids[Index], Steps));
	}
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(2, Chains.Num(), "Peasants: two chains");
	if (Chains.Num() == 2)
	{
		T.AssertEquals(2, Chains[0].UnitIds.Num(), "Peasants: first chain has two units");
		if (Chains[0].UnitIds.Num() == 2)
		{
			T.AssertEquals(10, Chains[0].UnitIds[0], "Peasants: first chain #10");
			T.AssertEquals(13, Chains[0].UnitIds[1], "Peasants: first chain #13");
		}
		T.AssertEquals(1, Chains[1].UnitIds.Num(), "Peasants: second chain has one unit");
		if (Chains[1].UnitIds.Num() == 1)
		{
			T.AssertEquals(12, Chains[1].UnitIds[0], "Peasants: second chain #12");
		}
	}
}

// A mix, in order: move, attack, attack-move, build, gather. Then move and attack-move to the same point: two chains.
void Test_OrderChains_Mix(FUnitTest& T)
{
	FHOLOrderChainStep MoveA = HOLOrderChainRead::MakeMove(FVector(1000, 0, 0), false);
	TArray<FHOLOrderChainStep> Mix;
	Mix.Add(MoveA);
	Mix.Add(HOLOrderChainRead::MakeAttack(42));
	Mix.Add(HOLOrderChainRead::MakeMove(FVector(3000, 0, 0), true));
	Mix.Add(HOLOrderChainRead::MakeBuild(9));
	Mix.Add(HOLOrderChainRead::MakeGather(5));
	TArray<FHOLOrderChainUnitSteps> Units;
	Units.Add(OCUnit(1, Mix));
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(1, Chains.Num(), "Mix: one chain");
	if (Chains.Num() == 1 && Chains[0].Steps.Num() == 5)
	{
		TArray<EHOLOrderChainStepKind> Expected;
		Expected.Add(EHOLOrderChainStepKind::Move);
		Expected.Add(EHOLOrderChainStepKind::Attack);
		Expected.Add(EHOLOrderChainStepKind::AttackMove);
		Expected.Add(EHOLOrderChainStepKind::Build);
		Expected.Add(EHOLOrderChainStepKind::Gather);
		for (int Index = 0; Index < 5; ++Index)
		{
			T.AssertTrue(Chains[0].Steps[Index].Kind == Expected[Index], "Mix: step " + Index + " is " + HOLOrderChainRead::KindName(Expected[Index]));
		}
		T.AssertEquals(42, Chains[0].Steps[1].TargetId, "Mix: attack target");
		T.AssertEquals(5, Chains[0].Steps[4].SourceIndex, "Mix: gather source");
	}
	else
	{
		T.AssertTrue(false, "Mix: 5 steps expected");
	}
	TArray<FHOLOrderChainUnitSteps> Two;
	Two.Add(OCUnit(1, OCSteps1(MoveA)));
	Two.Add(OCUnit(2, OCSteps1(HOLOrderChainRead::MakeMove(FVector(1000, 0, 0), true))));
	T.AssertEquals(2, HOLOrderChainRead::MakeChains(Two).Num(), "Move and attack-move to the same point: two chains");
}

// A rally (D328): units born together share one step to their point; a move to the same point is another chain.
void Test_OrderChains_Rally(FUnitTest& T)
{
	FHOLOrderChainStep Rally = HOLOrderChainRead::MakeRally(FVector(500, 200, 0));
	TArray<FHOLOrderChainUnitSteps> Units;
	Units.Add(OCUnit(3, OCSteps1(Rally)));
	Units.Add(OCUnit(4, OCSteps1(Rally)));
	Units.Add(OCUnit(5, OCSteps1(HOLOrderChainRead::MakeMove(FVector(500, 200, 0), false))));
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(2, Chains.Num(), "Rally: two chains");
	if (Chains.Num() == 2)
	{
		T.AssertEquals(2, Chains[0].UnitIds.Num(), "Rally: born together share it (2 units)");
		if (Chains[0].UnitIds.Num() == 2)
		{
			T.AssertEquals(3, Chains[0].UnitIds[0], "Rally: first #3");
			T.AssertEquals(4, Chains[0].UnitIds[1], "Rally: second #4");
		}
		T.AssertTrue(Chains[0].Steps[0].Kind == EHOLOrderChainStepKind::Rally, "Rally: its kind");
		T.AssertTrue(HOLOrderChainRead::KindName(EHOLOrderChainStepKind::Rally) == "Rally", "Rally: its name");
	}
}

// The bound: at most MaxSteps steps; queues alike up to the bound share a chain.
void Test_OrderChains_Bound(FUnitTest& T)
{
	int MaxSteps = HOLOrderChainRead::GetMaxSteps();
	TArray<FHOLOrderChainStep> Long;
	for (int Index = 0; Index < MaxSteps + 5; ++Index)
	{
		Long.Add(HOLOrderChainRead::MakeMove(FVector(100.0 * Index, 0, 0), false));
	}
	TArray<FHOLOrderChainStep> Longer = Long;
	Longer.Add(HOLOrderChainRead::MakeGather(1));
	TArray<FHOLOrderChainUnitSteps> Units;
	Units.Add(OCUnit(1, Long));
	Units.Add(OCUnit(2, Longer));
	TArray<FHOLOrderChain> Chains = HOLOrderChainRead::MakeChains(Units);
	T.AssertEquals(1, Chains.Num(), "Bound: one chain");
	if (Chains.Num() == 1)
	{
		T.AssertEquals(MaxSteps, Chains[0].Steps.Num(), "Bound: MaxSteps steps");
	}
}

// BuildingQueue, a building attack queued after a move: it waits for the walk, then strikes (the part before EndBuildingTarget).
void Test_OrderBuildingQueue_AfterMove(FUnitTest& T)
{
	FOCUnitModel Unit;
	Unit.bWalking = true;
	Unit.Queue.Add(HOLOrderChainRead::MakeAttackBuilding(7));
	T.AssertFalse(OCAdvance(Unit), "After a move: waits while it walks");
	Unit.bWalking = false;
	T.AssertTrue(OCAdvance(Unit), "After a move: starts at the arrival");
	T.AssertEquals(7, Unit.TargetId, "After a move: strikes building 7");
	T.AssertFalse(OCIsOver(Unit), "After a move: its order goes on while the building stands");
}

// Two buildings queued (Shift): never dropped for an enemy unit meanwhile (D345) (the part before EndBuildingTarget).
void Test_OrderBuildingQueue_TwoBuildings(FUnitTest& T)
{
	FOCUnitModel Unit;
	Unit.Queue.Add(HOLOrderChainRead::MakeAttackBuilding(1));
	Unit.Queue.Add(HOLOrderChainRead::MakeAttackBuilding(2));
	T.AssertTrue(OCAdvance(Unit), "Two buildings: the first starts");
	T.AssertFalse(OCMaySeek(Unit, false), "Two buildings: the first never dropped for an enemy unit");
	T.AssertFalse(OCAdvance(Unit), "Two buildings: the second waits");
	T.AssertEquals(1, Unit.OrderedId, "Two buildings: still on the first");
}

// Shift on a building after an attack on a unit: it waits for the target death.
void Test_OrderBuildingQueue_AfterUnitAttack(FUnitTest& T)
{
	FOCUnitModel Unit;
	Unit.bOrderedTarget = true;
	Unit.Queue.Add(HOLOrderChainRead::MakeAttackBuilding(5));
	T.AssertFalse(OCAdvance(Unit), "After a unit attack: waits while the target lives");
	Unit.bOrderedTarget = false;
	T.AssertTrue(OCAdvance(Unit), "After a unit attack: starts at the target death");
	T.AssertEquals(5, Unit.OrderedId, "After a unit attack: strikes building 5");
}

// A building attack without Shift, then a move with Shift: the move waits for the fall (the part before EndBuildingTarget).
void Test_OrderBuildingQueue_ThenMove(FUnitTest& T)
{
	FOCUnitModel Unit;
	Unit.TargetId = 3;
	Unit.OrderedId = 3;
	Unit.Queue.Add(HOLOrderChainRead::MakeMove(FVector(100, 0, 0), false));
	T.AssertFalse(OCAdvance(Unit), "Then a move: waits while the building stands");
}

// The drop (D345): only a building taken on by itself is dropped for an enemy unit; an ordered one never (before EndBuildingTarget).
void Test_OrderBuildingQueue_Drop(FUnitTest& T)
{
	FOCUnitModel Ordered;
	Ordered.TargetId = 4;
	Ordered.OrderedId = 4;
	Ordered.Queue.Add(HOLOrderChainRead::MakeMove(FVector(100, 0, 0), false));
	T.AssertFalse(OCMaySeek(Ordered, false), "Ordered: never dropped for an enemy unit");
	T.AssertFalse(OCAdvance(Ordered), "Ordered: its queue waits while it stands");
	T.AssertEquals(4, Ordered.TargetId, "Ordered: still struck");
	FOCUnitModel Rest;
	Rest.TargetId = 9;
	T.AssertTrue(OCMaySeek(Rest, false), "At rest: its own building dropped for an enemy unit");
	FOCUnitModel AttackMove;
	AttackMove.bWalking = true;
	AttackMove.TargetId = 9;
	T.AssertTrue(OCMaySeek(AttackMove, true), "Attack-move: its own building dropped for an enemy unit");
	T.AssertFalse(OCMaySeek(AttackMove, false), "A plain walk: nobody taken on");
}

// Its chain step (D328): red as an attack, to the building; not the same step as a build of the same id.
void Test_OrderBuildingQueue_Step(FUnitTest& T)
{
	FHOLOrderChainStep Step = HOLOrderChainRead::MakeAttackBuilding(11);
	T.AssertTrue(HOLOrderChainRead::KindName(Step.Kind) == "AttackBuilding", "Step: its name");
	T.AssertEquals(11, Step.TargetId, "Step: its building");
	TArray<FHOLOrderChainUnitSteps> Units;
	Units.Add(OCUnit(1, OCSteps1(Step)));
	Units.Add(OCUnit(2, OCSteps1(Step)));
	Units.Add(OCUnit(3, OCSteps1(HOLOrderChainRead::MakeBuild(11))));
	T.AssertEquals(2, HOLOrderChainRead::MakeChains(Units).Num(), "Step: two chains (attack, build)");
}

// Shift: a move, a drop-off, a move (D485, EtapeDefini L.5b): the drop-off waits for the first walk, the last move for the unload.
void Test_OrderBuildingQueue_Deposit(FUnitTest& T)
{
	FOCUnitModel Unit;
	Unit.bWalking = true;
	Unit.Queue.Add(HOLOrderChainRead::MakeDeposit(4));
	Unit.Queue.Add(HOLOrderChainRead::MakeMove(FVector(100, 0, 0), false));
	T.AssertFalse(OCAdvance(Unit), "Drop-off: waits while it walks");
	Unit.bWalking = false;
	T.AssertTrue(OCAdvance(Unit), "Drop-off: starts at the arrival");
	T.AssertTrue(Unit.bDroppingOff, "Drop-off: on its way to unload");
	T.AssertFalse(OCAdvance(Unit), "Drop-off: the next move waits for the unload");
	Unit.bDroppingOff = false;
	T.AssertTrue(OCAdvance(Unit), "Drop-off: unloaded, the next move starts");
	T.AssertEquals(0, Unit.Queue.Num(), "Drop-off: queue empty");
	T.AssertTrue(HOLOrderChainRead::KindName(EHOLOrderChainStepKind::Deposit) == "Deposit", "Drop-off: its step name");
}
