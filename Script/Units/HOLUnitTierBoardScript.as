// The server's unit tier board (MIG-CPP-UnitTier, D836 point 3), the script side of UHOLUnitTierSubsystem: the C++ part (the production
// subsystem, each queued order) gives the unit type, the player's tier and whether the building is finished and the stock pays; the rule is
// HOLUnitTierRules.as (HOLUnitTier::Check, the script is the source). Server and solo only. Tested by HeroesOfLiberty.UnitTier.ServerBoard.
class UHOLUnitTierBoardScript : UHOLUnitTierSubsystem
{
	UFUNCTION(BlueprintOverride)
	int CheckUnitTier(FName UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		return HOLUnitTier::Check(UnitType.ToString(), Tier, bBuildingFinished, bCanPay);
	}
}
