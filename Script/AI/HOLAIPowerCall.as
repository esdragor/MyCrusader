// S4-a (ETAPE0-1123): the AI's call of its faction powers. A pure rule (no world, tested in Script/Tests/HOLAIPowerCall_Test.as): the spend rule of HOLAIPowerSpend with one reserve per AI level, no cheating (D688). Brain wiring (stock, book rows, the cast order) comes with S4-a's branch step, once the effects are coded (bEffectCoded).

struct FHOLAIPowerCallData
{
	// The reserve of faction resource the AI keeps, one value per AI level (index = level, the last value serves the levels past the table).
	TArray<int> ReserveByLevel;
}

namespace HOLAIPowerCall
{
	// The reserve of the level (D688: one value per level); a level past the table reads the last value, an empty table keeps nothing.
	int ReserveOf(const FHOLAIPowerCallData& Data, int Level)
	{
		if (Data.ReserveByLevel.Num() == 0)
		{
			return 0;
		}
		const int Index = Math::Clamp(Level, 0, Data.ReserveByLevel.Num() - 1);
		return Data.ReserveByLevel[Index];
	}

	// The index in Rows of the power to cast now, or -1: the spend rule (HOLAIPowerSpend::Choose, which asks the book's own state rule, so a
	// power whose effect is not coded is never cast: prepared without effect) with the reserve of the AI's level.
	int Choose(const TArray<FHOLPowerRow>& Rows, const TArray<int>& CooldownLeftMs, const TArray<bool>& Wanted, int Stock, int Tier, int Level, const FHOLAIPowerCallData& Data)
	{
		return HOLAIPowerSpend::Choose(Rows, CooldownLeftMs, Wanted, Stock, Tier, ReserveOf(Data, Level));
	}
}

// S4-a branch step: what the brain decides to cast now (the power's index in the book and the ground point), or nothing.
struct FHOLAIPowerOrder
{
	bool bCast = false;
	int PowerIndex = -1;
	FVector Point;
}

namespace HOLAIPowerCall
{
	// The AI's reserve per level (provisional values, one line to tune: D688, no cheating) and the share of full health under which a unit is "wounded".
	FHOLAIPowerCallData DefaultData()
	{
		FHOLAIPowerCallData Data;
		Data.ReserveByLevel.Add(0);
		Data.ReserveByLevel.Add(25);
		Data.ReserveByLevel.Add(50);
		Data.ReserveByLevel.Add(75);
		return Data;
	}

	// The point the AI casts from (GLORFINDEL-1123h). STUB (red step): the banner as is. Real partie: the AI never plants a banner, so it reads (0, 0), 14709 cm from its army.
	FVector OriginOf(const FIntPoint& Banner, const TArray<FVector>& Army)
	{
		return FVector(double(Banner.X), double(Banner.Y), 0.0);
	}

	int WoundedPercent()
	{
		return 70;
	}

	// The brain's whole decision. Rows/Cooldowns/Stock/Tier/Level as Choose; Wounded = the ground points of the slot's wounded units;
	// Origin and RangeCm = the cast origin and range the server checks (a point out of range is never chosen: the server would refuse it).
	// The Sacred Light (book key) is wanted when at least one wounded unit is in range, and is cast on the centre of those units.
	FHOLAIPowerOrder Plan(const TArray<FHOLPowerRow>& Rows, const TArray<int>& CooldownLeftMs, int Stock, int Tier, int Level, const FHOLAIPowerCallData& Data, const TArray<FVector>& Wounded, const FVector& Origin, int64 RangeCm)
	{
		FHOLAIPowerOrder Order;
		FVector Sum = FVector(0.0, 0.0, 0.0);
		int InRange = 0;
		for (int Index = 0; Index < Wounded.Num(); Index++)
		{
			const double DX = Wounded[Index].X - Origin.X;
			const double DY = Wounded[Index].Y - Origin.Y;
			if (Math::Sqrt(DX * DX + DY * DY) <= double(RangeCm))
			{
				Sum += Wounded[Index];
				InRange++;
			}
		}
		TArray<bool> Wanted;
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			Wanted.Add(Rows[Index].Key == "SacredLight" && InRange > 0);
		}
		const int Picked = Choose(Rows, CooldownLeftMs, Wanted, Stock, Tier, Level, Data);
		if (Picked < 0)
		{
			return Order;
		}
		Order.bCast = true;
		Order.PowerIndex = Picked;
		Order.Point = Sum / double(InRange);
		return Order;
	}
}