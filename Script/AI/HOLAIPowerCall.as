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

// GLORFINDEL-1123pa (D891): the AI plants a banner before it casts (the cast origin is the banner, as for the player; the army-centre fallback of
// HOLFactionPowerServer::CastOriginOf stays while none is planted). A pure rule, tested in script. The ORDER to plant is not reachable from script:
// UHOLHeroSubsystem::PlantBanner and UHOLCommandSubsystem::IssueHeroAbilityOrder are not UFUNCTIONs (TYPES_A_EXPOSER), so the brain only logs the decision.
struct FHOLAIBannerOrder
{
	bool bPlant = false;
	FVector Point;
}

namespace HOLAIPowerCall
{
	// GLORFINDEL-1123pd: PlanBanner fed with the hero's REAL place (HOLFactionPowerServer::HeroPlaceOf): no hero found, or his ability not ready, plants nothing.
	FHOLAIBannerOrder PlanBannerOf(const TArray<FVector>& Wounded, bool bHeroKnown, const FVector& HeroLocation, bool bAbilityReady, bool bBannerStanding)
	{
		return PlanBanner(Wounded, HeroLocation, bHeroKnown && bAbilityReady, bBannerStanding);
	}

	// Plant when a hero able to (alive, level, cool-down: bHeroCanPlant) has no banner standing and some of the army is wounded (the same trigger as the cast).
	// The point is the centre of the wounded units, brought back along the line from the hero to the nearest point he may plant at (the reach is the hero
	// rules' own, HOLHeroAbilityRead::CanPlantBanner, found by halving: no copied constant), so the server never refuses it for distance.
	FHOLAIBannerOrder PlanBanner(const TArray<FVector>& Wounded, const FVector& HeroLocation, bool bHeroCanPlant, bool bBannerStanding)
	{
		FHOLAIBannerOrder Order;
		if (!bHeroCanPlant || bBannerStanding || Wounded.Num() == 0)
		{
			return Order;
		}
		FVector Sum = FVector(0.0, 0.0, 0.0);
		for (int Index = 0; Index < Wounded.Num(); Index++)
		{
			Sum += Wounded[Index];
		}
		const FVector Centre = FVector(Sum.X / double(Wounded.Num()), Sum.Y / double(Wounded.Num()), 0.0);
		const double DX = Centre.X - HeroLocation.X;
		const double DY = Centre.Y - HeroLocation.Y;
		Order.bPlant = true;
		Order.Point = Centre;
		if (!HOLHeroAbilityRead::CanPlantBanner(float(Math::Sqrt(DX * DX + DY * DY))))
		{
			double Near = 0.0;
			double Far = 1.0;
			for (int Step = 0; Step < 24; Step++)
			{
				const double Mid = (Near + Far) * 0.5;
				if (HOLHeroAbilityRead::CanPlantBanner(float(Mid * Math::Sqrt(DX * DX + DY * DY))))
				{
					Near = Mid;
				}
				else
				{
					Far = Mid;
				}
			}
			Order.Point = FVector(HeroLocation.X + DX * Near, HeroLocation.Y + DY * Near, 0.0);
		}
		return Order;
	}
}
