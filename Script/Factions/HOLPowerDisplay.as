// The DISPLAY line of one power box (GALADRIEL-1121sh): the player's text, separate from HOLPowerBook::Describe (the stable ASCII line of
// the tests and of hol.HUD.Report, which stays untouched). STUB of the red commit: the rule returns nothing yet.
struct FHOLPowerDisplay
{
	// Key of the text in the table (namespace "HOLFactionSlot"): PowerReady, PowerCooling, PowerShort, PowerTier, PowerSoon.
	FString TextKey;
	FString Name;
	int Cost = 0;
	// {2} of the pattern: the seconds left while cooling, the tier required while tier-locked, else 0.
	int Extra = 0;
}

namespace HOLPowerDisplay
{
	FHOLPowerDisplay Make(const FHOLPowerRow& Row, EHOLPowerState State, int CooldownLeftMs)
	{
		FHOLPowerDisplay Display;
		return Display;
	}

	FString Compose(const FString& Pattern, const FHOLPowerDisplay& Display)
	{
		return "";
	}
}
