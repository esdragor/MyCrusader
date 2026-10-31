// The common part of the duel kit (DuelInProgress.md D6; GDD 10.3, D14, D118, D119): posture and parry, pure rules, whole
// numbers (percent, milliseconds). The GDD says "values to be tuned in tests": the preset is a starting point in one place.
// No world access: the match tells it the time that goes by and the blows; it answers the damage that lands.
enum EHOLDuelPosture
{
	Offensive,
	Balanced,
	Defensive
}

struct FHOLDuelKitSettings
{
	// The damage the posture deals, percent of the reference (Balanced is the reference, D118).
	int OffensiveDamagePercent = 0;
	int BalancedDamagePercent = 0;
	int DefensiveDamagePercent = 0;

	// How long the parry window stays open once the parry is pressed, by posture (offensive: shorter; defensive: wider, D118).
	int OffensiveParryWindowMs = 0;
	int BalancedParryWindowMs = 0;
	int DefensiveParryWindowMs = 0;

	// What a parry takes off an announced strong attack (D119: "reduces strongly, without cancelling"), percent.
	int ParryReducePercent = 0;

	// Recovery before the parry can be pressed again (D119: short).
	int ParryCooldownMs = 0;

	// The strong attack's announcement before it lands (10.3: about 0.5 to 1 s).
	int StrongAnnounceMs = 0;
}

// One duellist's kit state: his posture and his parry.
struct FHOLDuelKit
{
	EHOLDuelPosture Posture = EHOLDuelPosture::Balanced;
	int ParryWindowLeftMs = 0;
	int ParryCooldownLeftMs = 0;
}

namespace HOLDuelKit
{
	// The preset: a starting point for the tests (GDD: "values to be tuned in tests"), in one place.
	FHOLDuelKitSettings Preset()
	{
		FHOLDuelKitSettings S;
		S.OffensiveDamagePercent = 125;
		S.BalancedDamagePercent = 100;
		S.DefensiveDamagePercent = 75;
		S.OffensiveParryWindowMs = 300;
		S.BalancedParryWindowMs = 450;
		S.DefensiveParryWindowMs = 600;
		S.ParryReducePercent = 70;
		S.ParryCooldownMs = 4000;
		S.StrongAnnounceMs = 750;
		return S;
	}

	int DamagePercentOf(const FHOLDuelKitSettings& S, EHOLDuelPosture Posture)
	{
		if (Posture == EHOLDuelPosture::Offensive)
			return S.OffensiveDamagePercent;
		if (Posture == EHOLDuelPosture::Defensive)
			return S.DefensiveDamagePercent;
		return S.BalancedDamagePercent;
	}

	int ParryWindowMsOf(const FHOLDuelKitSettings& S, EHOLDuelPosture Posture)
	{
		if (Posture == EHOLDuelPosture::Offensive)
			return S.OffensiveParryWindowMs;
		if (Posture == EHOLDuelPosture::Defensive)
			return S.DefensiveParryWindowMs;
		return S.BalancedParryWindowMs;
	}

	// Posture changes at once, at any time (D118: one click).
	FHOLDuelKit SetPosture(const FHOLDuelKit& Kit, EHOLDuelPosture Posture)
	{
		FHOLDuelKit Next = Kit;
		Next.Posture = Posture;
		return Next;
	}

	// The parry pressed: the window opens for the posture's time if the parry is ready; else the kit is returned unchanged.
	FHOLDuelKit PressParry(const FHOLDuelKitSettings& S, const FHOLDuelKit& Kit)
	{
		FHOLDuelKit Next = Kit;
		if (Kit.ParryCooldownLeftMs > 0)
			return Next;
		Next.ParryWindowLeftMs = ParryWindowMsOf(S, Kit.Posture);
		Next.ParryCooldownLeftMs = S.ParryCooldownMs;
		return Next;
	}

	// Time goes by: the window and the cooldown run down.
	FHOLDuelKit Tick(const FHOLDuelKit& Kit, int DeltaMs)
	{
		FHOLDuelKit Next = Kit;
		Next.ParryWindowLeftMs = Math::Max(0, Kit.ParryWindowLeftMs - DeltaMs);
		Next.ParryCooldownLeftMs = Math::Max(0, Kit.ParryCooldownLeftMs - DeltaMs);
		return Next;
	}

	// An announced strong attack of BaseDamage lands on this duellist: the damage taken. An open window takes ParryReducePercent
	// off and is spent (the parry worked); without one the whole damage lands.
	int TakeStrongHit(const FHOLDuelKitSettings& S, FHOLDuelKit& Kit, int BaseDamage)
	{
		if (Kit.ParryWindowLeftMs <= 0)
			return BaseDamage;
		Kit.ParryWindowLeftMs = 0;
		return BaseDamage - Math::IntegerDivisionTrunc(BaseDamage * S.ParryReducePercent, 100);
	}

	// The damage a blow of BaseDamage deals from this posture.
	int DealDamage(const FHOLDuelKitSettings& S, const FHOLDuelKit& Kit, int BaseDamage)
	{
		return Math::IntegerDivisionTrunc(BaseDamage * DamagePercentOf(S, Kit.Posture), 100);
	}
}