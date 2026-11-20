// One box of the faction slot's power book (A6a): the power's key, the state the book's rule (HOLPowerBook::StateOf) gives it,
// the cool-down left, and the book's stable line. No label and no icon here: those are the game's text table and art.
struct FHOLFactionPowerBox
{
	FString Key;
	EHOLPowerState State = EHOLPowerState::Soon;
	int CooldownLeftMs = 0;
	// The book's line: "<key> cost <n> ready|cooling <s> s|short|tier <n>|soon" (HOLPowerBook::Describe).
	FString Describe;
}

// The faction slot of the HUD as a view (GDD 15, D201: one slot above the hero's block; D823: the permanent specialization
// button shares this same slot): what it shows, from what it knows (pure, no world, no widget). Position, size, icons and the
// texts shown to the player are not decided here. Describe is the stable ASCII line of the tests and of a future hol.HUD.Report.
struct FHOLFactionSlotView
{
	// False: the slot is empty (no gauge and no specialization button to show).
	bool bVisible = false;
	// The faction has a resource gauge (HOLFactionResource::HasResource).
	bool bHasGauge = false;
	int Value = 0;
	int Cap = 0;
	float Fraction = 0.0;
	// The gauge's text, "30 / 100"; empty without a gauge.
	FString GaugeText;
	// One box per power, in the order of the book's rows (empty without a gauge: the powers spend the resource).
	TArray<FHOLFactionPowerBox> Powers;
	// The faction shows the permanent specialization button, and it is lit (a tier is open).
	bool bHasSpecializationButton = false;
	bool bSpecializationLit = false;
	// "slot empty", "slot gauge 30 / 100 powers a cost 10 ready; b cost 50 short spec lit", "slot spec dark".
	FString Describe;
}

namespace HOLFactionSlot
{
	// The view from: Faction, its gauge Resource, the power book's Rows with the cool-down left (ms) of each (same order; a
	// missing entry reads 0), the faction's Tier, bHasSpecializationButton (the faction has a button to show in the slot) and
	// bTierOpen (a tier's specialization is open to choose: the button is lit).
	FHOLFactionSlotView Make(EHOLFaction Faction, FHOLFactionResource Resource, TArray<FHOLPowerRow> Rows, TArray<int> CooldownLeftMs,
		int Tier, bool bHasSpecializationButton, bool bTierOpen)
	{
		FHOLFactionSlotView View;
		View.bHasGauge = HOLFactionResource::HasResource(Faction);
		View.bHasSpecializationButton = bHasSpecializationButton;
		if (!View.bHasGauge && !bHasSpecializationButton)
		{
			View.Describe = "slot empty";
			return View;
		}
		View.bVisible = true;
		View.bSpecializationLit = bHasSpecializationButton && bTierOpen;
		FString Line = "slot";
		if (View.bHasGauge)
		{
			View.Value = Resource.Value;
			View.Cap = Resource.Cap;
			View.Fraction = Resource.Cap > 0 ? float(Resource.Value) / float(Resource.Cap) : 0.0;
			View.GaugeText = f"{Resource.Value} / {Resource.Cap}";
			FString PowersLine = "";
			for (int Index = 0; Index < Rows.Num(); Index++)
			{
				const int Left = Index < CooldownLeftMs.Num() ? CooldownLeftMs[Index] : 0;
				FHOLFactionPowerBox Box;
				Box.Key = Rows[Index].Key;
				Box.CooldownLeftMs = Left;
				Box.State = HOLPowerBook::StateOf(Rows[Index], Resource.Value, Left, Tier);
				Box.Describe = HOLPowerBook::Describe(Rows[Index], Box.State, Left);
				View.Powers.Add(Box);
				if (Index > 0)
					PowersLine += "; ";
				PowersLine += Box.Describe;
			}
			if (Rows.Num() == 0)
				PowersLine = "none";
			Line += f" gauge {View.GaugeText} powers {PowersLine}";
		}
		if (bHasSpecializationButton)
		{
			Line += View.bSpecializationLit ? " spec lit" : " spec dark";
		}
		View.Describe = Line;
		return View;
	}
}

// The faction slot of a player from the game state (CELEBORN-1120b gauge, HOLDawnValues cap, HOLDawnPowers rows; GALADRIEL-1120d/e).
namespace HOLFactionSlot
{
	// The cool-down left (ms) of a power: its end time in the state (0 = ready) minus the server's time Now; 0 once over, never negative.
	int CooldownLeft(int EndMs, int NowMs)
	{
		if (EndMs <= 0 || EndMs <= NowMs)
			return 0;
		return EndMs - NowMs;
	}

	// The cool-down left of the slot's first Count powers, from the state's end times (GetPowerCooldownEndMs), at the server's time NowMs.
	TArray<int> CooldownsLeft(AHOLGameState State, int Slot, int Count, int NowMs)
	{
		TArray<int> Result;
		for (int Index = 0; Index < Count; Index++)
			Result.Add(CooldownLeft(State.GetPowerCooldownEndMs(Slot, Index), NowMs));
		return Result;
	}

	// The slot's view: the gauge is the state's value for the slot held in [0, HonorCap]; the rows are the Dawn's book (the only gauge
	// faction for now), cool-downs from the state's PowerCooldownEndMs (ELROND-1120c) minus the server's time, Tier 0 (no tier is read here yet).
	// An unknown slot (spectator) or no state: the empty slot.
	FHOLFactionSlotView FromGameState(AHOLGameState State, int Slot, bool bHasSpecializationButton, bool bTierOpen)
	{
		const int NowMs = State != nullptr ? int(State.GetServerWorldTimeSeconds() * 1000.0) : 0;
		return FromGameStateAt(State, Slot, NowMs, bHasSpecializationButton, bTierOpen);
	}

	// The same at the server's time NowMs (ms): the cool-down of each box is the state's end time minus NowMs.
	FHOLFactionSlotView FromGameStateAt(AHOLGameState State, int Slot, int NowMs, bool bHasSpecializationButton, bool bTierOpen)
	{
		if (State == nullptr || Slot < 0)
		{
			FHOLFactionSlotView Empty;
			Empty.Describe = "slot empty";
			return Empty;
		}
		// The faction of the slot as the state has it (GetSlotFaction, an EHOLFaction index): no more reading of the lobby's French text.
		const EHOLFaction Faction = EHOLFaction(State.GetSlotFaction(Slot));
		const FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Slot), 0, HOLDawnValues::HonorCap());
		TArray<FHOLPowerRow> Rows;
		if (HOLFactionResource::HasResource(Faction))
			Rows = HOLDawnPowers::Rows();
		TArray<int> Cooldowns = CooldownsLeft(State, Slot, Rows.Num(), NowMs);
		return Make(Faction, Gauge, Rows, Cooldowns, 0, bHasSpecializationButton, bTierOpen);
	}
}

// The click on the faction slot's power boxes (GALADRIEL-1120f). A click on a ready box enters the aim mode (AimIndex = the box); a click
// on the ground while aiming casts that power there (the caller sends ServerCastFactionPower(Slot, CastIndex, Point), the server decides
// again, HOLFactionPowerCast::Decide); Escape or the same box again cancels; a box that is not ready (cooling, short of gauge, tier,
// soon = no coded effect) refuses and the aim does not change. Pure: the view in, the next aim and the action out.
enum EHOLFactionClickKind
{
	// A click on the box number BoxIndex of the slot.
	Box,
	// A left click on the ground (the caller has the point).
	Ground,
	// The Escape key.
	Escape,
}

enum EHOLFactionClickAction
{
	// Not for the slot (no aim, no box): the click goes on to the game as it did.
	Nothing,
	// The aim mode is entered or switched to AimIndex.
	Aim,
	// The aim is over with a cast: send the power CastIndex at the clicked point.
	Launch,
	// The aim is over without a cast.
	Cancel,
	// A box that cannot be used now was clicked.
	Refuse,
}

struct FHOLFactionClickResult
{
	EHOLFactionClickAction Action = EHOLFactionClickAction::Nothing;
	// The aim after the click (-1 = none).
	int AimIndex = -1;
	// The power to cast when Action is Launch, else -1.
	int CastIndex = -1;
	// "nothing", "aim 0", "cast 0", "cancel", "refuse cooling" (the book's state word).
	FString Describe;
}

namespace HOLFactionSlot
{
	// STUB (red first): never does anything.
	FHOLFactionClickResult ClickPower(FHOLFactionSlotView View, int AimIndex, EHOLFactionClickKind Kind, int BoxIndex)
	{
		FHOLFactionClickResult Result;
		Result.AimIndex = AimIndex;
		Result.Describe = "nothing";
		return Result;
	}
}