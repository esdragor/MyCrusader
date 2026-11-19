// C2 (SAURON-1119s, UNCOMPILED): the hero, duel, diplomacy, signal, chat and specialization console commands in script (HOLControllerCommandsC2.as). Each command is registered once under its `hol.*` name and
// asks the right RPC (Server<X> of Script\Net\HOLRpcPlayer.as) with the arguments the C++ command passed; refused arguments give the C++ line and send nothing.
// RED to see (once, then put back): in HOLControllerCommandsC2.as StanceOf, return 2 for "neutral" instead of 1 -> Test_ControllerCommandsC2DiplomacyStance fails on "neutral is stance 1".
// RED to see for the spectator guard (once, then put back): in HOLControllerCommandsC2.as ShouldSend, change `return Route == "ServerPostChat";` to `return true;` -> Test_ControllerCommandsC2SpectatorSendsNothing fails on "a spectator sends nothing".

TArray<FString> C2Args(const FString& A = "", const FString& B = "", const FString& C = "", const FString& D = "", const FString& E = "")
{
	TArray<FString> Result;
	if (!A.IsEmpty()) Result.Add(A);
	if (!B.IsEmpty()) Result.Add(B);
	if (!C.IsEmpty()) Result.Add(C);
	if (!D.IsEmpty()) Result.Add(D);
	if (!E.IsEmpty()) Result.Add(E);
	return Result;
}

void Test_ControllerCommandsC2EachNameOnceAndRouted(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC2::Names();
	T.AssertEquals(12, Names.Num(), "12 hero, duel, diplomacy, signal, chat and specialization commands (11 ported, plus hol.Cheat.Specialization)");
	TArray<FString> Seen;
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		T.AssertTrue(Name.StartsWith("hol."), f"{Name}: a hol.* command");
		T.AssertFalse(Seen.Contains(Name), f"{Name}: registered once");
		Seen.Add(Name);
		FHOLCheatPlan Plan = HOLControllerCommandsC2::PlanOf(Name, C2Args("all", "1", "x"));
		T.AssertTrue(Plan.bValid || !Plan.Usage.IsEmpty(), f"{Name}: known to the planner");
		if (Plan.bValid)
			T.AssertTrue(Plan.Route.StartsWith("Server"), f"{Name}: routes to a Server RPC");
	}
	FHOLCheatPlan Unknown = HOLControllerCommandsC2::PlanOf("hol.Hero.Nothing", C2Args());
	T.AssertFalse(Unknown.bValid, "an unknown name plans nothing");
	T.AssertTrue(Unknown.Usage.IsEmpty(), "an unknown name logs nothing");
}

void Test_ControllerCommandsC2HeroAndDuel(FUnitTest& T)
{
	FHOLCheatPlan Return = HOLControllerCommandsC2::PlanOf("hol.Hero.Return", C2Args());
	T.AssertEquals("ServerReturnHero", Return.Route, "Return routes to ServerReturnHero");
	FHOLCheatPlan Cry = HOLControllerCommandsC2::PlanOf("hol.Hero.Cast", C2Args("closeranks"));
	T.AssertEquals("ServerHeroAbility", Cry.Route, "Cast routes to ServerHeroAbility");
	T.AssertEquals(0, Cry.Ints[0], "CloseRanks is ability 0 (any case)");
	FHOLCheatPlan Banner = HOLControllerCommandsC2::PlanOf("hol.Hero.Cast", C2Args("Banner", "1200", "-300.5"));
	T.AssertEquals(1, Banner.Ints[0], "Banner is ability 1");
	T.AssertTrue(Math::IsNearlyEqual(Banner.Dx, 1200.0), "Banner X 1200");
	T.AssertTrue(Math::IsNearlyEqual(Banner.Dy, -300.5), "Banner Y -300.5");
	FHOLCheatPlan BannerShort = HOLControllerCommandsC2::PlanOf("hol.Hero.Cast", C2Args("Banner", "1200"));
	T.AssertFalse(BannerShort.bValid, "Banner without Y is refused");
	T.AssertEquals("hol.Hero.Cast: CloseRanks, or Banner X Y", BannerShort.Usage, "Cast usage line");
	FHOLCheatPlan Challenge = HOLControllerCommandsC2::PlanOf("hol.Duel.Challenge", C2Args("3"));
	T.AssertEquals("ServerDuelChallenge", Challenge.Route, "Challenge route");
	T.AssertEquals(3, Challenge.Ints[0], "Challenge slot 3");
	FHOLCheatPlan NoTarget = HOLControllerCommandsC2::PlanOf("hol.Duel.Challenge", C2Args());
	T.AssertEquals("hol.Duel.Challenge <TargetSlot>", NoTarget.Usage, "Challenge usage line (no 'Usage:' prefix, as the C++)");
	FHOLCheatPlan Accept = HOLControllerCommandsC2::PlanOf("hol.Duel.Accept", C2Args());
	T.AssertEquals("ServerDuelAnswer", Accept.Route, "Accept route");
	T.AssertEquals(1, Accept.Ints[0], "Accept answers yes");
	FHOLCheatPlan Refuse = HOLControllerCommandsC2::PlanOf("hol.Duel.Refuse", C2Args());
	T.AssertEquals("ServerDuelAnswer", Refuse.Route, "Refuse route");
	T.AssertEquals(0, Refuse.Ints[0], "Refuse answers no");
}

void Test_ControllerCommandsC2DiplomacyStance(FUnitTest& T)
{
	FHOLCheatPlan Ally = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Stance", C2Args("2", "ALLY"));
	T.AssertEquals("ServerDiplomacyStance", Ally.Route, "Stance route");
	T.AssertEquals(2, Ally.Ints[0], "slot 2");
	T.AssertEquals(2, Ally.Ints[1], "ally is stance 2");
	FHOLCheatPlan Neutral = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Stance", C2Args("2", "neutral"));
	T.AssertEquals(1, Neutral.Ints[1], "neutral is stance 1");
	FHOLCheatPlan Enemy = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Stance", C2Args("2", "enemy"));
	T.AssertEquals(0, Enemy.Ints[1], "enemy is stance 0");
	FHOLCheatPlan Other = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Stance", C2Args("2", "friend"));
	T.AssertEquals(0, Other.Ints[1], "an unknown word is enemy, as the C++");
	FHOLCheatPlan Short = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Stance", C2Args("2"));
	T.AssertEquals("Usage: hol.Diplomacy.Stance <Slot> <ally|neutral|enemy>", Short.Usage, "Stance usage line");
	FHOLCheatPlan Share = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Vision", C2Args("1", "1"));
	T.AssertEquals("ServerDiplomacyVision", Share.Route, "Vision route");
	T.AssertEquals(1, Share.Ints[1], "Vision 1 shares");
	FHOLCheatPlan Stop = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Vision", C2Args("1", "0"));
	T.AssertEquals(0, Stop.Ints[1], "Vision 0 stops");
	FHOLCheatPlan VisionShort = HOLControllerCommandsC2::PlanOf("hol.Diplomacy.Vision", C2Args("1"));
	T.AssertEquals("Usage: hol.Diplomacy.Vision <Slot> <0|1>", VisionShort.Usage, "Vision usage line");
}

void Test_ControllerCommandsC2SignalAndChat(FUnitTest& T)
{
	FHOLCheatPlan Post = HOLControllerCommandsC2::PlanOf("hol.Signal.Post", C2Args("4", "100", "-250"));
	T.AssertEquals("ServerPostSignal", Post.Route, "Signal route");
	T.AssertEquals(4, Post.Ints[0], "kind 4");
	T.AssertTrue(Math::IsNearlyEqual(Post.Dx, 100.0), "X 100");
	T.AssertTrue(Math::IsNearlyEqual(Post.Dy, -250.0), "Y -250");
	FHOLCheatPlan PostShort = HOLControllerCommandsC2::PlanOf("hol.Signal.Post", C2Args("4", "100"));
	T.AssertEquals("Usage: hol.Signal.Post <Kind> <X> <Y>", PostShort.Usage, "Signal usage line");
	TArray<FString> AllWords;
	AllWords.Add("all"); AllWords.Add("hello"); AllWords.Add("there");
	FHOLCheatPlan All = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", AllWords);
	T.AssertEquals("ServerPostChat", All.Route, "Chat route");
	T.AssertEquals(0, All.Ints[0], "channel all is 0");
	T.AssertEquals(-1, All.Ints[1], "no target outside private");
	T.AssertEquals("hello there", All.Name, "the words are joined with a space");
	TArray<FString> PrivateWords;
	PrivateWords.Add("PRIVATE"); PrivateWords.Add("3"); PrivateWords.Add("psst");
	FHOLCheatPlan Private = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", PrivateWords);
	T.AssertEquals(2, Private.Ints[0], "channel private is 2");
	T.AssertEquals(3, Private.Ints[1], "private target slot 3");
	T.AssertEquals("psst", Private.Name, "private text starts after the slot");
	FHOLCheatPlan Allies = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", C2Args("allies", "go"));
	T.AssertEquals(1, Allies.Ints[0], "channel allies is 1");
	FHOLCheatPlan NoText = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", C2Args("all"));
	T.AssertFalse(NoText.bValid, "a channel with no text is refused");
	T.AssertEquals("Usage: hol.Chat.Say <all|allies|private> [<TargetSlot>] <text>", NoText.Usage, "Chat usage line");
	FHOLCheatPlan PrivateNoText = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", C2Args("private", "3"));
	T.AssertFalse(PrivateNoText.bValid, "private with a slot and no text is refused");
	FHOLCheatPlan BadChannel = HOLControllerCommandsC2::PlanOf("hol.Chat.Say", C2Args("everyone", "hi"));
	T.AssertFalse(BadChannel.bValid, "an unknown channel is refused");
}

void Test_ControllerCommandsC2Specialization(FUnitTest& T)
{
	FHOLCheatPlan Start = HOLControllerCommandsC2::PlanOf("hol.Spec.Start", C2Args("2", "1"));
	T.AssertEquals("ServerStartSpecialization", Start.Route, "Spec.Start route");
	T.AssertEquals(2, Start.Ints[0], "tier 2");
	T.AssertEquals(1, Start.Ints[1], "option 1");
	FHOLCheatPlan Short = HOLControllerCommandsC2::PlanOf("hol.Spec.Start", C2Args("2"));
	T.AssertEquals("Usage: hol.Spec.Start <Tier> <Option>", Short.Usage, "Spec.Start usage line");
	FHOLCheatPlan Cancel = HOLControllerCommandsC2::PlanOf("hol.Spec.Cancel", C2Args());
	T.AssertEquals("ServerCancelSpecialization", Cancel.Route, "Spec.Cancel route");
}

// Morceau 8 (CELEBORN-1119n3, NOT COMPILED): hol.Cheat.Specialization <Tier 1..3> <Option> asks ServerCheatSpecialization; a tier outside 1..3 or a missing word is refused with the usage line.
// RED to see on the first commit: the plan is a stub (always refused), so the route, the integers and the accepted tiers all fail.
void Test_ControllerCommandsC2CheatSpecialization(FUnitTest& T)
{
	const FString CheatUsage = "Usage: hol.Cheat.Specialization <Tier 1..3> <Option>";
	FHOLCheatPlan Start = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("2", "1"));
	T.AssertTrue(Start.bValid, "Cheat.Specialization 2 1 is accepted");
	T.AssertEquals("ServerCheatSpecialization", Start.Route, "Cheat.Specialization route");
	T.AssertEquals(2, Start.Ints.Num(), "two integers");
	const int StartTier = Start.Ints.Num() > 0 ? Start.Ints[0] : -99;
	const int StartOption = Start.Ints.Num() > 1 ? Start.Ints[1] : -99;
	T.AssertEquals(2, StartTier, "tier 2");
	T.AssertEquals(1, StartOption, "option 1");
	FHOLCheatPlan First = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("1", "2"));
	T.AssertTrue(First.bValid, "tier 1 is accepted");
	FHOLCheatPlan Third = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("3", "1"));
	T.AssertTrue(Third.bValid, "tier 3 is accepted");
	FHOLCheatPlan Short = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("2"));
	T.AssertFalse(Short.bValid, "a missing option is refused");
	T.AssertEquals(CheatUsage, Short.Usage, "Cheat.Specialization usage line");
	FHOLCheatPlan Zero = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("0", "1"));
	T.AssertFalse(Zero.bValid, "tier 0 is refused");
	T.AssertEquals(CheatUsage, Zero.Usage, "tier 0 gives the usage line");
	FHOLCheatPlan Four = HOLControllerCommandsC2::PlanOf("hol.Cheat.Specialization", C2Args("4", "1"));
	T.AssertFalse(Four.bValid, "tier 4 is refused");
	T.AssertEquals(CheatUsage, Four.Usage, "tier 4 gives the usage line");
}

// SAURON-1119w: a pure spectator sends nothing (the C++ Request<X> returned silently), for every command but the chat; a player sends all of them.
void Test_ControllerCommandsC2SpectatorSendsNothing(FUnitTest& T)
{
	TArray<FString> Names = HOLControllerCommandsC2::Names();
	for (int I = 0; I < Names.Num(); I++)
	{
		FString Name = Names[I];
		FHOLCheatPlan Plan = HOLControllerCommandsC2::PlanOf(Name, C2Args("all", "1", "x"));
		if (!Plan.bValid)
			continue;
		bool bPlayerSends = HOLControllerCommandsC2::ShouldSend(false, Plan.Route);
		T.AssertTrue(bPlayerSends, f"{Name}: a player sends");
		bool bSpectatorSends = HOLControllerCommandsC2::ShouldSend(true, Plan.Route);
		if (Plan.Route == "ServerPostChat")
			T.AssertTrue(bSpectatorSends, f"{Name}: a spectator talks, the server decides (D376)");
		else
			T.AssertFalse(bSpectatorSends, f"{Name}: a spectator sends nothing");
	}
	T.AssertFalse(HOLControllerCommandsC2::ShouldSend(true, "ServerStartSpecialization"), "hol.Spec.Start: a spectator sends nothing");
	T.AssertFalse(HOLControllerCommandsC2::ShouldSend(true, "ServerCancelSpecialization"), "hol.Spec.Cancel: a spectator sends nothing");
}