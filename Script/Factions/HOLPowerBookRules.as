// The state of one button of a faction's power book (A6a, priority 1 of the faction base): pure rule, no world, no widget.
// Generic: a row is a name, a cost, a cool-down, a tier and a flag "the effect is coded"; no power of any faction is named
// here (costs, cool-downs and the number of powers are tuned in play, never decided here).
// Refusal order when several things are wrong at once: Soon > TierLocked > Cooling > ShortOfResource.
enum EHOLPowerState
{
	Ready,
	Cooling,
	ShortOfResource,
	TierLocked,
	Soon
}

struct FHOLPowerRow
{
	FString Key;
	int Cost = 0;
	int CooldownMs = 0;
	int MinTier = 0;
	// False while the power has no coded effect: the book shows it greyed "Soon" (D601).
	bool bEffectCoded = false;
}

namespace HOLPowerBook
{
	// The state of the button from the row, the stock of the resource, the cool-down left (ms) and the faction's tier.
	EHOLPowerState StateOf(const FHOLPowerRow& Row, int Stock, int CooldownLeftMs, int Tier)
	{
		if (!Row.bEffectCoded)
		{
			return EHOLPowerState::Soon;
		}
		if (Tier < Row.MinTier)
		{
			return EHOLPowerState::TierLocked;
		}
		if (CooldownLeftMs > 0)
		{
			return EHOLPowerState::Cooling;
		}
		if (Stock < Row.Cost)
		{
			return EHOLPowerState::ShortOfResource;
		}
		return EHOLPowerState::Ready;
	}

	// The stable line of the tests and of the logs: "<key> cost <n> ready", "... cooling <s> s", "... short", "... tier <n>", "... soon".
	FString Describe(const FHOLPowerRow& Row, EHOLPowerState State, int CooldownLeftMs)
	{
		FString Head = f"{Row.Key} cost {Row.Cost}";
		if (State == EHOLPowerState::Ready)
		{
			return f"{Head} ready";
		}
		if (State == EHOLPowerState::Cooling)
		{
			int Seconds = (CooldownLeftMs + 999) / 1000;
			return f"{Head} cooling {Seconds} s";
		}
		if (State == EHOLPowerState::ShortOfResource)
		{
			return f"{Head} short";
		}
		if (State == EHOLPowerState::TierLocked)
		{
			return f"{Head} tier {Row.MinTier}";
		}
		return f"{Head} soon";
	}
}