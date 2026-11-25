// ELROND-1125mc (ENONCES-1125m G-3): the ground click of an armed grid mode (100 attack-move, 101 Banner, 102 Seal of Light), decided in script.
// The C++ controller keeps the click (SetArmedMode / ConsumeArmedClick, ELROND-1125ma); the script reads it and turns it into the order below, then sends it
// through the bindings the C++ already exposes (RequestCastUnitAbilityAt, RequestHeroAbilityAt, RequestAttackMoveUnits), like ConsumeFactionAimClick does for the powers.
// STUB (RED): OnGroundClick answers None for everything; the correction follows in the next commit.

enum EHOLGridGroundKind
{
	None,
	CastUnitAbility,
	HeroAbility,
	AttackMove
}

struct FHOLGridGroundOrder
{
	EHOLGridGroundKind Kind = EHOLGridGroundKind::None;
	// CastUnitAbility: the HOLUnitAbilityTable id; HeroAbility: the EHOLHeroAbility as int.
	int AbilityId = 0;
	// CastUnitAbility: the casting unit.
	int UnitId = 0;
	FVector Point;
	// AttackMove: the selected units.
	TArray<int> UnitIds;
}

namespace HOLGridGroundClick
{
	FHOLGridGroundOrder OnGroundClick(int Mode, int UnitId, const FVector& Point, const TArray<int>& SelectedUnitIds)
	{
		FHOLGridGroundOrder Order;
		return Order;
	}
}
