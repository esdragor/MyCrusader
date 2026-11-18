// MIG2-S3 T4 lot L1 (HALDIR-1117o): the pure rules of the script RPCs of the Specialization lot (ServerStartSpecialization, ServerCancelSpecialization) and the Hero lot (ServerReturnHero,
// ServerHeroTalent, ServerHeroAbility, ServerCastRage, ServerWatchUnit) of Script/Net/HOLRpcPlayer.as. The sender is the owner's slot (D288); a spectator is refused; the board and the hero rules
// decide the rest at the step's end, as the retired C++ RPC did. RED FIRST for the main session: replace each body by `return 0;` (stub), run quick_check -Parts tests, then restore.

void Test_RpcSeatRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::SeatRefusal(0), "slot 0 starts a specialization or calls the hero: queued");
	T.AssertEquals(0, HOLRpc::SeatRefusal(7), "slot 7 (the last) is in range");
	T.AssertEquals(1, HOLRpc::SeatRefusal(-1), "a spectator (no slot) is refused");
	T.AssertEquals(1, HOLRpc::SeatRefusal(8), "a sender past the 8 slots is refused");
}

void Test_RpcHeroAbilityRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::HeroAbilityRefusal(0, EHOLHeroAbility::CloseRanks), "the cry is cast");
	T.AssertEquals(0, HOLRpc::HeroAbilityRefusal(3, EHOLHeroAbility::Banner), "the banner is cast");
	T.AssertEquals(2, HOLRpc::HeroAbilityRefusal(0, EHOLHeroAbility::Aura), "the aura is passive: never cast");
	T.AssertEquals(1, HOLRpc::HeroAbilityRefusal(-1, EHOLHeroAbility::Banner), "a spectator cannot cast");
	T.AssertEquals(1, HOLRpc::HeroAbilityRefusal(-1, EHOLHeroAbility::Aura), "the sender is checked first");
}

void Test_RpcRageRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::RageRefusal(0, 3), "3 units named: queued");
	T.AssertEquals(2, HOLRpc::RageRefusal(0, 0), "no unit named: refused");
	T.AssertEquals(1, HOLRpc::RageRefusal(-1, 3), "a spectator cannot cast the rage");
	T.AssertEquals(1, HOLRpc::RageRefusal(9, 0), "the sender is checked first");
}