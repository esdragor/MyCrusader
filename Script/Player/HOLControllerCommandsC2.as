// C2 (SAURON-1119s, D851 step 5, UNCOMPILED): the hero, duel, diplomacy, signal, chat and specialization console commands of AHOLPlayerController.cpp, now script. Same names `hol.*`, same arguments, same
// effect: each one calls the script RPC actor of every local controller (AHOLRpcPlayer.Server<X>, Script\Net\HOLRpcPlayer.as), as the C++ Request<X> did through CallN / CallEx; same "Usage: ..." lines.
// Same pattern as HOLControllerCommands.as (C1); its parsers are reused (HOLControllerCommands::ParseInt / ParseReal / IsNumber), the plan struct too (FHOLCheatPlan: Route, Ints, Name = the chat text, Dx / Dy = X / Y).
// Journal levels kept (SAURON-1119v): the C++ "Usage" and "unknown" lines were UE_LOG(LogHOLMatch, Warning) -> HOLScriptLog::LogWarning; the "RpcRoute: ... not sent" line was Error -> HOLScriptLog::LogError (LogNotSent below).
// Spectator (SAURON-1119w): the C++ Request<X> returned silently for a pure spectator (IsSpectator); ShouldSend below gives that back (HOLControllerCommand::IsSpectatorController, nothing sent, no line), the chat excepted as in the C++.
// NOT here (they read C++ state with no script binding, see the report): hol.Hero.Rage, hol.Market.Trade, hol.Tribute, hol.Prod.*, the *.Report and Tribute.Notice, hol.Diplomacy.Force / ForceVision.

namespace HOLControllerCommandsC2
{
	TArray<FString> Names()
	{
		TArray<FString> Result;
		Result.Add("hol.Hero.Return"); Result.Add("hol.Hero.Cast");
		Result.Add("hol.Duel.Challenge"); Result.Add("hol.Duel.Accept"); Result.Add("hol.Duel.Refuse");
		Result.Add("hol.Diplomacy.Stance"); Result.Add("hol.Diplomacy.Vision");
		Result.Add("hol.Signal.Post"); Result.Add("hol.Chat.Say");
		Result.Add("hol.Spec.Start"); Result.Add("hol.Spec.Cancel"); Result.Add("hol.Cheat.Specialization"); Result.Add("hol.Cheat.Honor");
		return Result;
	}

	FHOLCheatPlan Refused(const FString& Usage)
	{
		FHOLCheatPlan Plan;
		Plan.Usage = Usage;
		return Plan;
	}

	FHOLCheatPlan Accepted(const FString& Route)
	{
		FHOLCheatPlan Plan;
		Plan.bValid = true;
		Plan.Route = Route;
		return Plan;
	}

	// ally 2, neutral 1, enemy 0 (HOLDiplomacy::Ally / Neutral / Enemy): anything else than ally or neutral is enemy, the name compared in lowercase.
	int StanceOf(const FString& Name)
	{
		const FString Lower = Name.ToLower();
		if (Lower == "ally")
			return 2;
		if (Lower == "neutral")
			return 1;
		return 0;
	}

	// UHOLChatSubsystem::ParseChannel: all 0, allies 1, private 2, -1 (INDEX_NONE) for any other name.
	int ChannelOf(const FString& Name)
	{
		const FString Lower = Name.ToLower();
		if (Lower == "all")
			return 0;
		if (Lower == "allies")
			return 1;
		if (Lower == "private")
			return 2;
		return -1;
	}

	// The plan of one command by its name; bValid false with Usage empty is an unknown command, with a Usage the refused arguments (the line the C++ command logged, at Warning level).
	FHOLCheatPlan PlanOf(const FString& Command, const TArray<FString>& Args)
	{
		if (Command == "hol.Hero.Return")
			return Accepted("ServerReturnHero");
		if (Command == "hol.Hero.Cast")
		{
			// CloseRanks, or Banner X Y (cm): Ints[0] the ability (EHOLHeroAbility: CloseRanks, Banner by name in Send), Dx / Dy the banner's place (0 for the cry).
			const bool bBanner = Args.Num() >= 3 && Args[0].ToLower() == "banner";
			if (!bBanner && !(Args.Num() >= 1 && Args[0].ToLower() == "closeranks"))
				return Refused("hol.Hero.Cast: CloseRanks, or Banner X Y");
			FHOLCheatPlan Plan = Accepted("ServerHeroAbility");
			Plan.Ints.Add(bBanner ? 1 : 0);
			Plan.Dx = bBanner ? HOLControllerCommands::ParseReal(Args[1]) : 0.0;
			Plan.Dy = bBanner ? HOLControllerCommands::ParseReal(Args[2]) : 0.0;
			return Plan;
		}
		if (Command == "hol.Duel.Challenge")
		{
			if (Args.Num() < 1)
				return Refused("hol.Duel.Challenge <TargetSlot>");
			FHOLCheatPlan Plan = Accepted("ServerDuelChallenge");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			return Plan;
		}
		if (Command == "hol.Duel.Accept" || Command == "hol.Duel.Refuse")
		{
			FHOLCheatPlan Plan = Accepted("ServerDuelAnswer");
			Plan.Ints.Add(Command == "hol.Duel.Accept" ? 1 : 0);
			return Plan;
		}
		if (Command == "hol.Diplomacy.Stance")
		{
			if (Args.Num() < 2)
				return Refused("Usage: hol.Diplomacy.Stance <Slot> <ally|neutral|enemy>");
			FHOLCheatPlan Plan = Accepted("ServerDiplomacyStance");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(StanceOf(Args[1]));
			return Plan;
		}
		if (Command == "hol.Diplomacy.Vision")
		{
			if (Args.Num() < 2)
				return Refused("Usage: hol.Diplomacy.Vision <Slot> <0|1>");
			FHOLCheatPlan Plan = Accepted("ServerDiplomacyVision");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]) != 0 ? 1 : 0);
			return Plan;
		}
		if (Command == "hol.Signal.Post")
		{
			if (Args.Num() < 3)
				return Refused("Usage: hol.Signal.Post <Kind> <X> <Y>");
			FHOLCheatPlan Plan = Accepted("ServerPostSignal");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Dx = HOLControllerCommands::ParseReal(Args[1]);
			Plan.Dy = HOLControllerCommands::ParseReal(Args[2]);
			return Plan;
		}
		if (Command == "hol.Chat.Say")
		{
			const int Channel = Args.Num() > 0 ? ChannelOf(Args[0]) : -1;
			const int First = Channel == 2 ? 2 : 1;
			if (Channel == -1 || Args.Num() <= First)
				return Refused("Usage: hol.Chat.Say <all|allies|private> [<TargetSlot>] <text>");
			FHOLCheatPlan Plan = Accepted("ServerPostChat");
			Plan.Ints.Add(Channel);
			Plan.Ints.Add(Channel == 2 ? HOLControllerCommands::ParseInt(Args[1]) : -1);
			FString Text = "";
			for (int I = First; I < Args.Num(); I++)
			{
				if (I > First)
					Text += " ";
				Text += Args[I];
			}
			Plan.Name = Text;
			return Plan;
		}
		if (Command == "hol.Spec.Start")
		{
			if (Args.Num() < 2)
				return Refused("Usage: hol.Spec.Start <Tier> <Option>");
			FHOLCheatPlan Plan = Accepted("ServerStartSpecialization");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]));
			return Plan;
		}
		if (Command == "hol.Cheat.Specialization")
		{
			const FString CheatUsage = "Usage: hol.Cheat.Specialization <Tier 1..3> <Option>";
			if (Args.Num() < 2)
				return Refused(CheatUsage);
			const int CheatTier = HOLControllerCommands::ParseInt(Args[0]);
			if (CheatTier < 1 || CheatTier > 3)
				return Refused(CheatUsage);
			FHOLCheatPlan Plan = Accepted("ServerCheatSpecialization");
			Plan.Ints.Add(CheatTier);
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[1]));
			return Plan;
		}
		if (Command == "hol.Cheat.Honor")
		{
			// <Value>: the gauge is set to it, held between the floor and the cap by the server (HOLFactionResource::CheatTarget); the Dawn only (CELEBORN-1120b).
			if (Args.Num() < 1)
				return Refused("Usage: hol.Cheat.Honor <Value>");
			FHOLCheatPlan Plan = Accepted("ServerCheatHonor");
			Plan.Ints.Add(HOLControllerCommands::ParseInt(Args[0]));
			return Plan;
		}
		if (Command == "hol.Spec.Cancel")
			return Accepted("ServerCancelSpecialization");
		return FHOLCheatPlan();
	}

	// The line of C++ LogNotSent (UE_LOG Error "RpcRoute: %s not sent: no script RPC actor / function"); the line is Error as the C++ (HOLScriptLog::LogError), the function name is the RPC's.
	void LogNotSent(const FString& Route)
	{
		HOLScriptLog::LogError(n"LogHOLMatch", f"RpcRoute: {Route} not sent: no script RPC actor / function");
	}

	void Send(AHOLPlayerController Controller, const FHOLCheatPlan& Plan)
	{
		AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller));
		if (Rpc == nullptr)
		{
			LogNotSent(Plan.Route);
			return;
		}
		if (Plan.Route == "ServerReturnHero")
			Rpc.ServerReturnHero();
		else if (Plan.Route == "ServerHeroAbility")
		{
			FVector_NetQuantize AbilityLocation;
			AbilityLocation.X = Plan.Dx;
			AbilityLocation.Y = Plan.Dy;
			AbilityLocation.Z = 0.0;
			Rpc.ServerHeroAbility(Plan.Ints[0] == 1 ? EHOLHeroAbility::Banner : EHOLHeroAbility::CloseRanks, AbilityLocation);
		}
		else if (Plan.Route == "ServerDuelChallenge")
			Rpc.ServerDuelChallenge(Plan.Ints[0]);
		else if (Plan.Route == "ServerDuelAnswer")
			Rpc.ServerDuelAnswer(Plan.Ints[0] != 0);
		else if (Plan.Route == "ServerDiplomacyStance")
			Rpc.ServerDiplomacyStance(Plan.Ints[0], Plan.Ints[1]);
		else if (Plan.Route == "ServerDiplomacyVision")
			Rpc.ServerDiplomacyVision(Plan.Ints[0], Plan.Ints[1] != 0);
		else if (Plan.Route == "ServerPostSignal")
			Rpc.ServerPostSignal(Plan.Ints[0], FVector(Plan.Dx, Plan.Dy, 0.0));
		else if (Plan.Route == "ServerPostChat")
			Rpc.ServerPostChat(Plan.Ints[0], Plan.Name, Plan.Ints[1]);
		else if (Plan.Route == "ServerStartSpecialization")
			Rpc.ServerStartSpecialization(Plan.Ints[0], Plan.Ints[1]);
		else if (Plan.Route == "ServerCancelSpecialization")
			Rpc.ServerCancelSpecialization();
		else if (Plan.Route == "ServerCheatSpecialization")
			Rpc.ServerCheatSpecialization(Plan.Ints[0], Plan.Ints[1]);
		else
			LogNotSent(Plan.Route);
	}

	// The spectator guard of the C++ Request<X> (SAURON-1119w): a pure spectator (AHOLPlayerController::IsSpectator) sent NOTHING and logged nothing for the hero, duel, diplomacy, signal and specialization routes;
	// only the chat had no guard ("a pure spectator talks too, with the spectators, D376: the server decides"). Pure, so a test can call it without a world.
	bool ShouldSend(bool bSpectator, const FString& Route)
	{
		if (!bSpectator)
			return true;
		return Route == "ServerPostChat";
	}

	// The command as typed: refused arguments log their usage line (Warning), else every local controller sends the plan (GetLocalControllers of the C++).
	void Run(const FString& Command, const TArray<FString>& Args)
	{
		const FHOLCheatPlan Plan = PlanOf(Command, Args);
		if (!Plan.bValid)
		{
			if (!Plan.Usage.IsEmpty())
				HOLScriptLog::LogWarning(n"LogHOLMatch", Plan.Usage);
			return;
		}
		TArray<AHOLPlayerController> Controllers;
		GetAllActorsOfClass(Controllers);
		for (AHOLPlayerController Controller : Controllers)
		{
			if (Controller.IsLocalController() && ShouldSend(HOLControllerCommand::IsSpectatorController(Controller), Plan.Route))
				Send(Controller, Plan);
		}
	}
}

void HOLHeroReturnCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Hero.Return", Args); }
void HOLHeroCastCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Hero.Cast", Args); }
void HOLDuelChallengeCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Duel.Challenge", Args); }
void HOLDuelAcceptCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Duel.Accept", Args); }
void HOLDuelRefuseCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Duel.Refuse", Args); }
void HOLDiplomacyStanceCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Diplomacy.Stance", Args); }
void HOLDiplomacyVisionCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Diplomacy.Vision", Args); }
void HOLSignalPostCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Signal.Post", Args); }
void HOLChatSayCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Chat.Say", Args); }
void HOLSpecStartCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Spec.Start", Args); }
void HOLSpecCancelCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Spec.Cancel", Args); }
void HOLCheatSpecializationCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Cheat.Specialization", Args); }
void HOLCheatHonorCommand(const TArray<FString>& Args) { HOLControllerCommandsC2::Run("hol.Cheat.Honor", Args); }

const FConsoleCommand HOLHeroReturnConsole("hol.Hero.Return", n"HOLHeroReturnCommand");
const FConsoleCommand HOLHeroCastConsole("hol.Hero.Cast", n"HOLHeroCastCommand");
const FConsoleCommand HOLDuelChallengeConsole("hol.Duel.Challenge", n"HOLDuelChallengeCommand");
const FConsoleCommand HOLDuelAcceptConsole("hol.Duel.Accept", n"HOLDuelAcceptCommand");
const FConsoleCommand HOLDuelRefuseConsole("hol.Duel.Refuse", n"HOLDuelRefuseCommand");
const FConsoleCommand HOLDiplomacyStanceConsole("hol.Diplomacy.Stance", n"HOLDiplomacyStanceCommand");
const FConsoleCommand HOLDiplomacyVisionConsole("hol.Diplomacy.Vision", n"HOLDiplomacyVisionCommand");
const FConsoleCommand HOLSignalPostConsole("hol.Signal.Post", n"HOLSignalPostCommand");
const FConsoleCommand HOLChatSayConsole("hol.Chat.Say", n"HOLChatSayCommand");
const FConsoleCommand HOLSpecStartConsole("hol.Spec.Start", n"HOLSpecStartCommand");
const FConsoleCommand HOLSpecCancelConsole("hol.Spec.Cancel", n"HOLSpecCancelCommand");
const FConsoleCommand HOLCheatSpecializationConsole("hol.Cheat.Specialization", n"HOLCheatSpecializationCommand");
const FConsoleCommand HOLCheatHonorConsole("hol.Cheat.Honor", n"HOLCheatHonorCommand");
