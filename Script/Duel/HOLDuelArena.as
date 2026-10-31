// The duel's arena (DuelInProgress.md D5; GDD 10.2, D18, D120): a circle of short radius on the map, centred between the two
// heroes where the duel begins. Pure rules, whole centimetres, no world access: the match measures the heroes' positions,
// the rule says where the circle is and who stands inside it. The circle's drawing and the heroes' contact are the match's.
struct FHOLDuelArena
{
	int CenterX = 0;
	int CenterY = 0;
	int RadiusCm = 0;
}

struct FHOLDuelPoint
{
	int X = 0;
	int Y = 0;
}

namespace HOLDuel
{
	// The circle for a duel between the heroes at (AX, AY) and (BX, BY): centred on the middle of the two, the settings' radius.
	FHOLDuelArena MakeArena(const FHOLDuelSettings& S, int AX, int AY, int BX, int BY)
	{
		FHOLDuelArena Arena;
		Arena.CenterX = Math::IntegerDivisionTrunc(AX + BX, 2);
		Arena.CenterY = Math::IntegerDivisionTrunc(AY + BY, 2);
		Arena.RadiusCm = S.ArenaRadiusCm;
		return Arena;
	}

	// True when the point is inside the circle or on its edge.
	bool InArena(const FHOLDuelArena& Arena, int X, int Y)
	{
		const int64 DX = int64(X) - int64(Arena.CenterX);
		const int64 DY = int64(Y) - int64(Arena.CenterY);
		return DX * DX + DY * DY <= int64(Arena.RadiusCm) * int64(Arena.RadiusCm);
	}

	// "Short distance" (10.2): the two heroes fit together in one arena of the settings' radius.
	bool FitsInArena(const FHOLDuelSettings& S, int AX, int AY, int BX, int BY)
	{
		const int64 DX = int64(AX) - int64(BX);
		const int64 DY = int64(AY) - int64(BY);
		const int64 Diameter = int64(S.ArenaRadiusCm) * 2;
		return DX * DX + DY * DY <= Diameter * Diameter;
	}

	// D120: the nearest point of the circle (edge included) to a point; a point inside comes back unchanged.
	FHOLDuelPoint ClampToArena(const FHOLDuelArena& Arena, int X, int Y)
	{
		FHOLDuelPoint P;
		P.X = X;
		P.Y = Y;
		if (InArena(Arena, X, Y))
			return P;
		// Out: along the line from the centre, shortened to the radius (truncated toward the centre, so the point is inside).
		const float DX = float(X - Arena.CenterX);
		const float DY = float(Y - Arena.CenterY);
		const float Length = Math::Sqrt(DX * DX + DY * DY);
		const float Scale = float(Arena.RadiusCm) / Length;
		P.X = Arena.CenterX + Math::TruncToInt(DX * Scale);
		P.Y = Arena.CenterY + Math::TruncToInt(DY * Scale);
		return P;
	}

	// D120: the other duellist of an Active duel; -1 for a slot outside it, or a duel not Active.
	int AdversaryOf(const FHOLDuel& Duel, int Slot)
	{
		if (Duel.Phase != EHOLDuelPhase::Active)
			return -1;
		if (Slot == Duel.Challenger)
			return Duel.Target;
		if (Slot == Duel.Target)
			return Duel.Challenger;
		return -1;
	}

	int BookAdversaryOf(const FHOLDuelBook& Book, int Slot)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			const int Other = AdversaryOf(Book.Duels[Index], Slot);
			if (Other >= 0)
				return Other;
		}
		return -1;
	}

	// D120: the circle of the Active duel of this pair (set once, where the duel began). False when there is no such duel.
	bool BookSetArena(FHOLDuelBook& Book, int Challenger, int Target, const FHOLDuelArena& Arena)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Active && Book.Duels[Index].Challenger == Challenger && Book.Duels[Index].Target == Target)
			{
				Book.Duels[Index].bHasArena = true;
				Book.Duels[Index].Arena = Arena;
				return true;
			}
		}
		return false;
	}

	// D120: the circle of the Active duel the slot fights. False when the slot fights none.
	bool BookArenaOf(const FHOLDuelBook& Book, int Slot, FHOLDuelArena& Out)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].bHasArena && AdversaryOf(Book.Duels[Index], Slot) >= 0)
			{
				Out = Book.Duels[Index].Arena;
				return true;
			}
		}
		return false;
	}
}