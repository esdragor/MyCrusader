// The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1). STUB written with the red test (HOLUnitTierRules_Test.as): the signatures, no
// behaviour yet.
namespace HOLUnitTier
{
	const int Available = 0;
	const int NotListed = 1;
	const int TierTooLow = 2;
	const int NoBuilding = 3;
	const int CannotPay = 4;
	const int UnknownTier = -1;

	int MinTierOf(const FString& UnitType)
	{
		return UnknownTier;
	}

	int Check(const FString& UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		return Available;
	}

	bool IsAvailable(const FString& UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		return Check(UnitType, Tier, bBuildingFinished, bCanPay) == Available;
	}
}
