// STUB (red commit): final signatures, wrong returns. The real view follows in the next commit.
struct FHOLFactionPowerBox
{
	FString Key;
	EHOLPowerState State = EHOLPowerState::Soon;
	int CooldownLeftMs = 0;
	FString Describe;
}

struct FHOLFactionSlotView
{
	bool bVisible = false;
	bool bHasGauge = false;
	int Value = 0;
	int Cap = 0;
	float Fraction = 0.0;
	FString GaugeText;
	TArray<FHOLFactionPowerBox> Powers;
	bool bHasSpecializationButton = false;
	bool bSpecializationLit = false;
	FString Describe;
}

namespace HOLFactionSlot
{
	FHOLFactionSlotView Make(FString Faction, FHOLFactionResource Resource, TArray<FHOLPowerRow> Rows, TArray<int> CooldownLeftMs,
		int Tier, bool bHasSpecializationButton, bool bTierOpen)
	{
		FHOLFactionSlotView View;
		View.bVisible = true;
		View.Describe = "stub";
		return View;
	}
}
