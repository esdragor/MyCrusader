// MIG2-S3 T0 (SAURON-1117m): which types can be the parameter of a UFUNCTION(Server) written in script (the L1 Heroes, L2 Orders and L3 Economy lots need them). ONE line per type:
// the probe class below declares one Server RPC per type. If a type is not declarable the script module fails to compile and the error line names it: comment that line out, note the type in
// TYPES_A_EXPOSER, and the other lines keep their proof. The class is never spawned (no network here); declaring it is the proof, the test only checks that it was born.
// Not in Script/Dev, Developers, Examples, Editor (the plugin skips them): this file is in Script/Tests.
class AHOLRpcTypesProbe : AActor
{
	UFUNCTION(Server)
	void ServerTypeVector(FVector_NetQuantize Point) {}

	UFUNCTION(Server)
	void ServerTypeUnitIds(const TArray<uint32>& UnitIds) {}

	UFUNCTION(Server)
	void ServerTypeBuilding(AHOLBuilding Building) {}

	UFUNCTION(Server)
	void ServerTypeName(FName TypeName) {}

	UFUNCTION(Server)
	void ServerTypeAbility(EHOLHeroAbility Ability) {}

	// The real signatures of the L2 / L1 Heroes lots, all at once (what a migrated RPC will really declare).
	UFUNCTION(Server)
	void ServerTypePlaceBuilding(FName TypeName, FVector_NetQuantize Point, const TArray<uint32>& UnitIds, bool bQueue) {}

	UFUNCTION(Server)
	void ServerTypeHeroAbility(EHOLHeroAbility Ability, FVector_NetQuantize Location) {}

	UFUNCTION(Server)
	void ServerTypeMarketTrade(AHOLBuilding Building, int Resource, int Lots, bool bBuy) {}
}

void Test_RpcTypesProbeClassIsBorn(FUnitTest& T)
{
	UClass ProbeClass = AHOLRpcTypesProbe::StaticClass();
	T.AssertTrue(ProbeClass != nullptr, "the probe class with one Server RPC per parameter type exists (FVector_NetQuantize, TArray<uint32>, AHOLBuilding, FName, EHOLHeroAbility)");
}
