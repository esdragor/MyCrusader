// The duel's arena (DuelInProgress.md D5; GDD 10.2, D18, D120): a circle of short radius on the map, centred between the two
// heroes where the duel begins. Pure rules, whole centimetres, no world access: the match measures the heroes' positions,
// the rule says where the circle is and who stands inside it. The circle's drawing and the heroes' contact are the match's.
struct FHOLDuelArena
{
	int CenterX = 0;
	int CenterY = 0;
	int RadiusCm = 0;
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
}
