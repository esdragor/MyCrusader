// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDamageRedirectRules.h"

#include "Simulation/HOLZoneEffectRules.h"

// NOT COMPILED (SAURON-1120f). See the header for what is decided and what is a parameter.
HOLDamageRedirect::FResult HOLDamageRedirect::Redirect(float Damage, bool bAllyIsHero, int32 AllyId, int32 AllyXCm, int32 AllyYCm,
	TConstArrayView<FKnight> Knights, int32 RadiusCm, int32 PercentPerKnight, ECumulMode Mode, int32 CapPercent)
{
	FResult Result;
	Result.Kept = Damage;
	const int32 Percent = FMath::Clamp(PercentPerKnight, 0, 100);
	if (Damage <= 0.f || bAllyIsHero || Percent == 0)
	{
		return Result;
	}

	// Knights that can redirect this blow: alive, not the struck unit itself, the ally inside the radius (edge included).
	TArray<const FKnight*, TInlineAllocator<8>> InRange;
	for (const FKnight& Knight : Knights)
	{
		if (Knight.bAlive && Knight.Id != AllyId && HOLZoneEffect::IsInZone(Knight.XCm, Knight.YCm, RadiusCm, AllyXCm, AllyYCm))
		{
			InRange.Add(&Knight);
		}
	}
	if (InRange.IsEmpty())
	{
		return Result;
	}

	float Redirected = 0.f;
	if (Mode == ECumulMode::Nearest)
	{
		const FKnight* Best = nullptr;
		int64 BestDistSq = 0;
		for (const FKnight* Knight : InRange)
		{
			const int64 Dx = static_cast<int64>(Knight->XCm) - AllyXCm;
			const int64 Dy = static_cast<int64>(Knight->YCm) - AllyYCm;
			const int64 DistSq = Dx * Dx + Dy * Dy;
			if (!Best || DistSq < BestDistSq || (DistSq == BestDistSq && Knight->Id < Best->Id))
			{
				Best = Knight;
				BestDistSq = DistSq;
			}
		}
		Redirected = Damage * static_cast<float>(Percent) / 100.f;
		Result.Shares.Add({ Best->Id, Redirected });
	}
	else
	{
		const int32 Count = InRange.Num();
		const int32 TotalPercent = FMath::Min(Percent * Count, FMath::Clamp(CapPercent, 0, 100));
		const float Each = Damage * static_cast<float>(TotalPercent) / 100.f / static_cast<float>(Count);
		for (const FKnight* Knight : InRange)
		{
			Result.Shares.Add({ Knight->Id, Each });
			Redirected += Each;
		}
	}
	Result.Kept = Damage - Redirected;
	return Result;
}
