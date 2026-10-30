// The forum in the AI's rules (IA3.2): when its own main forum has fallen, its Peasants rebuild it (D418: they join the rebuilding, which they
// speed up); on its last life all of them go and the army fights for the ruin at any ratio (D426, the simple reading of the prototype: the
// weighing of the lives comes with IA4). Pure functions on the summary, tested in HOLAIForum_Test.as; the brain sends the orders.
struct FHOLAIForumData
{
	// Peasants sent to rebuild while the slot has more than one life left (it can give a life up and keep its economy going).
	int RebuildersSpare = 5;

	// The Peasants for the rebuilding are chosen at most this often (s).
	float OrderEvery = 4.f;
}

// What the slot remembers of its rebuilding between two thoughts (the brain keeps one per slot).
struct FHOLAIForumMemory
{
	// The Peasants sent to rebuild the forum, while it is down.
	TArray<int> Sent;

	// Game time of the last choice (s).
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
			if (!Memory.Sent.Contains(Summary.Peasants[Index].Id))
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
}
