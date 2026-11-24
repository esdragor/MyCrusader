// FARAMIR-1124b (S2-c, D404): Sacred Light takes the malus off the caster's camp where the zone lands. Pure rule HOLFactionPowerServer::ClearMalusOrderOf, the
// ONE source PutDown hands to HOLMorale::ClearMalusAround (the processor lays the order: HeroesOfLiberty.Morale.ClearMalusAround*). Values are computed.

// The order is centred on the pose (the aimed point, truncated to cm), covers the zone, and belongs to the caster's camp.
void Test_SacredLightClearOrderIsCentredOnThePoseWithTheZoneRadius(FUnitTest& T)
{
	const FHOLSacredLightClearOrder Order = HOLFactionPowerServer::ClearMalusOrderOf(1, FVector(1600.7, 700.2, 30.0));
	T.AssertEquals(Order.CenterXCm, 1600, "the clearing is centred on the zone's X");
	T.AssertEquals(Order.CenterYCm, 700, "the clearing is centred on the zone's Y");
	T.AssertEquals(Order.RadiusCm, HOLZoneEffect::GetSacredLightRadiusCm(), "the clearing covers exactly the zone");
	T.AssertTrue(Order.RadiusCm > 0, "the zone has a radius");
	T.AssertEquals(Order.Slot, 1, "the clearing is for the caster's camp");
}

// Another caster, another pose: the order follows both (no constant answer).
void Test_SacredLightClearOrderFollowsTheCaster(FUnitTest& T)
{
	const FHOLSacredLightClearOrder First = HOLFactionPowerServer::ClearMalusOrderOf(0, FVector(-300.0, 4000.0, 0.0));
	const FHOLSacredLightClearOrder Second = HOLFactionPowerServer::ClearMalusOrderOf(1, FVector(5000.0, -90.0, 0.0));
	T.AssertEquals(First.Slot, 0, "slot 0 casts for camp 0");
	T.AssertEquals(Second.Slot, 1, "slot 1 casts for camp 1");
	T.AssertEquals(First.CenterXCm, -300, "negative X is kept");
	T.AssertEquals(Second.CenterYCm, -90, "negative Y is kept");
}