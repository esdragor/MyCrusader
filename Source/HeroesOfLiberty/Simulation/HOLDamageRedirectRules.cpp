// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDamageRedirectRules.h"

// NOT COMPILED (SAURON-1120f). STUB: redirects nothing, so the test is red; the next commit writes the rule.
HOLDamageRedirect::FResult HOLDamageRedirect::Redirect(float Damage, bool bAllyIsHero, int32 AllyId, int32 AllyXCm, int32 AllyYCm,
	TConstArrayView<FKnight> Knights, int32 RadiusCm, int32 PercentPerKnight, ECumulMode Mode, int32 CapPercent)
{
	FResult Result;
	Result.Kept = Damage;
	return Result;
}
