// The forum in the AI's rules (IA3.2): when its own main forum has fallen, its Peasants rebuild it (D418: they join the rebuilding, which they
// speed up); on its last life all of them go and the army fights for the ruin at any ratio (D426, the simple reading of the prototype: the
// weighing of the lives comes with IA4). Pure functions on the summary, tested in HOLAIForum_Test.as; the brain sends the orders.
struct FHOLAIForumData
{
	// Peasants sent to rebuild while the slot has more than one life left (it can give a life up and keep its economy going).
	int RebuildersSpare = 5;

	// The Peasants for the rebuilding are chosen at most this often (s).
	float OrderEvery = 4.f;

	// Nothing is built in the ruin (D352): a Peasant leaves his work only when he would reach the forum as the ruin ends (his distance over
	// PeasantSpeed (cm/s), plus ArriveMargin seconds for the detours), not to wait in front of it.
	float PeasantSpeed = 800.f;
	float ArriveMargin = 5.f;
}

// What the slot remembers of its rebuilding between two thoughts (the brain keeps one per slot).
struct FHOLAIForumMemory
{
	// The Peasants sent to rebuild the forum, while it is down.
	TArray<int> Sent;

	// Game time of the last choice (s).
	float LastOrder = -1000.f;
}

// The King in Regicide (IA3.4): he stays behind the forum, away from the enemy, and falls back there when enemy soldiers come near him.
struct FHOLAIKingData
{
	// His post: this far behind the main forum, on the side away from the enemy centre (cm).
	float PostDistance = 2500.f;

	// He is back at his post within this (cm).
	float PostRadius = 500.f;

	// An enemy soldier this near makes him fall back to his post (cm), when the post is on the safe side of him; he never runs into the
	// open: at his post, or with the enemy between him and it, he stays and the army comes (Threat counts his place as guarded).
	float DangerRadius = 1800.f;

	// His orders are given at most this often (s).
	float OrderEvery = 3.f;
}

// What the slot remembers of its King's orders between two thoughts.
struct FHOLAIKingMemory
{
	float LastOrder = -1000.f;
}

namespace HOLAIForum
{
	FHOLAIForumData DefaultData()
	{
		return FHOLAIForumData();
	}

	// The slot's own main forum is down (in ruin or rising again) and the slot has a centre to go to.
	bool OwnDown(const FHOLAISummary& Summary)
	{
		return Summary.bHasOwnForum && Summary.OwnForum.bDown && Summary.bHasCentre;
	}

	// The forum is down on its last life: the next fall is the defeat.
	bool Emergency(const FHOLAISummary& Summary)
	{
		return OwnDown(Summary) && Summary.OwnForum.Lives <= 1;
	}

	// The Peasants that start or join the rebuilding now (the ones nearest the forum first), remembered in Memory; empty while the forum
	// stands (the memory is cleared), within OrderEvery of the last choice, or when enough are sent already.
	TArray<int> RebuildOrders(FHOLAIForumMemory& Memory, const FHOLAISummary& Summary, const FHOLAIForumData& Data, float Now)
	{
		TArray<int> Ids;
		if (!OwnDown(Summary))
		{
			Memory.Sent.Empty();
			return Ids;
		}
		// The ones that died (or are not Peasants any more) are forgotten.
		for (int Index = Memory.Sent.Num() - 1; Index >= 0; Index--)
		{
			if (!Summary.PeasantIds.Contains(Memory.Sent[Index]))
			{
				Memory.Sent.RemoveAt(Index);
			}
		}
		if (Now - Memory.LastOrder < Data.OrderEvery)
		{
			return Ids;
		}
		const int Wanted = Emergency(Summary) ? Summary.Peasants.Num() : Math::Min(Data.RebuildersSpare, Summary.Peasants.Num());
		// The Peasants not sent yet, the nearest to the forum first (the lower identifier on a tie), up to Wanted in all.
		TArray<int> Left;
		for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
		{
			const float Distance = HOLAIArmy::FlatDistance(Summary.Peasants[Index].Location, Summary.OwnForum.Location);
			if (!Memory.Sent.Contains(Summary.Peasants[Index].Id) && Distance / Data.PeasantSpeed + Data.ArriveMargin >= Summary.OwnForum.RuinSecondsLeft)
			{
				Left.Add(Index);
			}
		}
		while (Memory.Sent.Num() < Wanted && Left.Num() > 0)
		{
			int Best = 0;
			for (int Index = 1; Index < Left.Num(); Index++)
			{
				const float Near = HOLAIArmy::FlatDistance(Summary.Peasants[Left[Index]].Location, Summary.OwnForum.Location);
				const float Far = HOLAIArmy::FlatDistance(Summary.Peasants[Left[Best]].Location, Summary.OwnForum.Location);
				if (Near < Far || (Near == Far && Summary.Peasants[Left[Index]].Id < Summary.Peasants[Left[Best]].Id))
				{
					Best = Index;
				}
			}
			const int Id = Summary.Peasants[Left[Best]].Id;
			Left.RemoveAt(Best);
			Memory.Sent.Add(Id);
			Ids.Add(Id);
		}
		if (Ids.Num() > 0)
		{
			Memory.LastOrder = Now;
		}
		return Ids;
	}

	// The summary without the Peasants busy with the rebuilding on the last life: they do not run from the enemy soldiers standing at the
	// ruin (the flight rule is given this summary).
	FHOLAISummary Calm(const FHOLAISummary& Summary, const FHOLAIForumMemory& Memory)
	{
		if (!Emergency(Summary))
		{
			return Summary;
		}
		FHOLAISummary Result = Summary;
		Result.Peasants.Empty();
		for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
		{
			if (!Memory.Sent.Contains(Summary.Peasants[Index].Id))
			{
				Result.Peasants.Add(Summary.Peasants[Index]);
			}
		}
		return Result;
	}

	// Whether an enemy forum lies fallen (in ruin or rising again) with a known place: the wave can go and hold it.
	bool EnemyRuin(const FHOLAISummary& Summary)
	{
		for (int Index = 0; Index < Summary.EnemyForums.Num(); Index++)
		{
			if (Summary.EnemyForums[Index].bDown && Summary.EnemyForums[Index].bHasLocation)
			{
				return true;
			}
		}
		return false;
	}

	// The place of the fallen enemy forum nearest to From (bFound false without one; From then).
	FVector RuinTarget(const FHOLAISummary& Summary, FVector From, bool& bFound)
	{
		bFound = false;
		FVector Best = From;
		for (int Index = 0; Index < Summary.EnemyForums.Num(); Index++)
		{
			const FHOLAIForum Forum = Summary.EnemyForums[Index];
			if (Forum.bDown && Forum.bHasLocation && (!bFound || HOLAIArmy::FlatDistance(Forum.Location, From) < HOLAIArmy::FlatDistance(Best, From)))
			{
				bFound = true;
				Best = Forum.Location;
			}
		}
		return Best;
	}

	// The King's post: behind the main forum, away from the enemy centre (the births side is the enemy's: the King starts in front).
	FVector KingPost(const FHOLAISummary& Summary, const FHOLAIKingData& Data)
	{
		FVector Away = FVector(1, 0, 0);
		if (Summary.bHasEnemyCentre)
		{
			FVector Line = Summary.CentreLocation - Summary.EnemyCentreLocation;
			Line.Z = 0;
			if (Line.Size() > 1.f)
			{
				Away = Line / Line.Size();
			}
		}
		return Summary.CentreLocation + Away * Data.PostDistance;
	}

	// Where the King goes now (Regicide only): to his post when an enemy soldier within DangerRadius is farther from it than he is, else to
	// his post when he stands idle off it; he stays when cornered.
	// False when he stays; Memory throttles the orders.
	bool KingOrder(FHOLAIKingMemory& Memory, const FHOLAISummary& Summary, const FHOLAIKingData& Data, float Now, FVector& Where)
	{
		if (!Summary.bRegicide || !Summary.bHasKing || !Summary.bHasCentre || Now - Memory.LastOrder < Data.OrderEvery)
		{
			return false;
		}
		const FVector Post = KingPost(Summary, Data);
		// An enemy soldier within the danger radius: he falls back to his post if the soldier is farther from it than he is, else he stays.
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (HOLAIArmy::IsThreatening(Enemy) && HOLAIArmy::FlatDistance(Enemy.Location, Summary.King.Location) <= Data.DangerRadius)
			{
				if (HOLAIArmy::FlatDistance(Summary.King.Location, Post) > Data.PostRadius
					&& HOLAIArmy::FlatDistance(Enemy.Location, Post) > HOLAIArmy::FlatDistance(Summary.King.Location, Post))
				{
					Where = Post;
					Memory.LastOrder = Now;
					return true;
				}
				return false;
			}
		}
		if (Summary.King.bIdle && HOLAIArmy::FlatDistance(Summary.King.Location, Post) > Data.PostRadius)
		{
			Where = Post;
			Memory.LastOrder = Now;
			return true;
		}
		return false;
	}

	// The enemy King, alive and known: where to send the wave in Regicide (bFound false without; From then).
	FVector KingTarget(const FHOLAISummary& Summary, FVector From, bool& bFound)
	{
		bFound = false;
		FVector Best = From;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (Enemy.Type == n"King" && Enemy.Health > 0 && (!bFound || HOLAIArmy::FlatDistance(Enemy.Location, From) < HOLAIArmy::FlatDistance(Best, From)))
			{
				bFound = true;
				Best = Enemy.Location;
			}
		}
		return Best;
	}
}
