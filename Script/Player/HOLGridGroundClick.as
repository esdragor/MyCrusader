// ELROND-1125mc (ENONCES-1125m G-3): the ground click of an armed grid mode (100 attack-move, 101 Banner, 102 Seal of Light), decided in script.
// The C++ controller keeps the click (SetArmedMode / ConsumeArmedClick, ELROND-1125ma); the script reads it and turns it into the order below, then sends it
// through the bindings the C++ already exposes (RequestCastUnitAbilityAt, RequestHeroAbilityAt, RequestAttackMoveUnits), like ConsumeFactionAimClick does for the powers.
// Called every frame by HOLGameScreenScript::Tick (ConsumeGridGroundClick).

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
	// The order a ground click gives in an armed mode (the mode and the caster are what the grid's verdict armed, HOLCommandGridAction::Decide).
	FHOLGridGroundOrder OnGroundClick(int Mode, int UnitId, const FVector& Point, const TArray<int>& SelectedUnitIds)
	{
		FHOLGridGroundOrder Order;
		int AttackMode = HOLCommandGridAction::GetAimAttackMove();
		int BannerMode = HOLCommandGridAction::GetAimBanner();
		int SealMode = HOLCommandGridAction::GetAimSeal();
		if (Mode == SealMode)
		{
			if (UnitId <= 0)
			{
				return Order;
			}
			Order.Kind = EHOLGridGroundKind::CastUnitAbility;
			Order.AbilityId = HOLUnitAbilityTable::LightSeal;
			Order.UnitId = UnitId;
		}
		else if (Mode == BannerMode)
		{
			Order.Kind = EHOLGridGroundKind::HeroAbility;
			Order.AbilityId = int(EHOLHeroAbility::Banner);
		}
		else if (Mode == AttackMode)
		{
			if (SelectedUnitIds.Num() == 0)
			{
				return Order;
			}
			Order.Kind = EHOLGridGroundKind::AttackMove;
			Order.UnitIds = SelectedUnitIds;
		}
		else
		{
			return Order;
		}
		Order.Point = Point;
		return Order;
	}

	// Sends the order through the C++ bindings; false when there is nothing to send.
	bool Execute(AHOLPlayerController Controller, const FHOLGridGroundOrder& Order)
	{
		if (Order.Kind == EHOLGridGroundKind::CastUnitAbility)
		{
			return HOLControllerCommand::RequestCastUnitAbilityAt(Controller, Order.AbilityId, Order.UnitId, Order.Point.X, Order.Point.Y, Order.Point.Z);
		}
		if (Order.Kind == EHOLGridGroundKind::HeroAbility)
		{
			return HOLControllerCommand::RequestHeroAbilityAt(Controller, Order.AbilityId, Order.Point.X, Order.Point.Y, Order.Point.Z);
		}
		if (Order.Kind == EHOLGridGroundKind::AttackMove)
		{
			return HOLControllerCommand::RequestAttackMoveUnits(Controller, Order.UnitIds, Order.Point.X, Order.Point.Y, false);
		}
		return false;
	}

	// The kept ground click of an armed grid mode (>= 100) becomes its order; the mode is then disarmed. True when a click was consumed.
	bool ConsumeGridGroundClick(AHOLPlayerController Controller)
	{
		FVector Point;
		int Mode = -1;
		int UnitId = Controller.GetArmedUnitId();
		if (!Controller.ConsumeArmedClick(Point, Mode))
		{
			return false;
		}
		TArray<int> Selected;
		HOLControllerCommand::OwnSelectedUnitIds(Controller, Selected);
		Execute(Controller, OnGroundClick(Mode, UnitId, Point, Selected));
		Controller.SetArmedMode(-1, 0.0, 0);
		return true;
	}
}
