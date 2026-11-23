// S4-a (ETAPE0-1123): the AI's call of its faction powers. STUB (red): returns "nothing to cast"; the rule comes in the next commit.

struct FHOLAIPowerCallData
{
	// The reserve of faction resource the AI keeps, one value per AI level (index = level, the last value serves the levels past the table).
	TArray<int> ReserveByLevel;
}

namespace HOLAIPowerCall
{
	int Choose(const TArray<FHOLPowerRow>& Rows, const TArray<int>& CooldownLeftMs, const TArray<bool>& Wanted, int Stock, int Tier, int Level, const FHOLAIPowerCallData& Data)
	{
		return -1;
	}
}
