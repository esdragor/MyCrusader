// MIG2-S3 pilot (D842 Q1 = A, ELROND-1117o): the per-player RPC actor written in script, model of the proven probe (HOLScriptRpcProbe.as). The server spawns one per controller,
// owned by it (C++ HOLScriptRpcPlayerBridge::SpawnAllFor, called from the game mode's PostLogin); each RPC of the player is declared here. Pilot: ONE RPC, ServerDiplomacyStance,
// the same body as the C++ AHOLPlayerController::ServerDiplomacyStance_Implementation (slot read from the owner: D288; HOLDiplomacy::SetStance checks the rest at the next step).
// This file lives in Script/Net: the plugin skips Script/Dev, Developers, Examples and Editor outside the interactive editor. NEEDS THE C++ LOT (HOLScriptRpcPlayerActor, HOLRpcBridge::).
// Switch hol.Rpc.ScriptDiplomacy (default 0): 0 = the interface keeps the old C++ RPC (RequestStance); 1 = it goes through this actor. The C++ RPC stays until the 2-client proof.
// Proof: hol.Rpc.DiplomacyTarget (default -1; >= 0 on a client: it calls ServerDiplomacyStance(Target, hol.Rpc.DiplomacyStance) once) used by Tools\Scenarios\rpc_diplomacy_run.ps1.
const FConsoleVariable CVar_RpcScriptDiplomacy("hol.Rpc.ScriptDiplomacy", 1);
const FConsoleVariable CVar_RpcDiplomacyTarget("hol.Rpc.DiplomacyTarget", -1);
const FConsoleVariable CVar_RpcDiplomacyStance("hol.Rpc.DiplomacyStance", 1);
const FConsoleVariable CVar_RpcProofRoute("hol.Rpc.ProofRoute", 0);
const FConsoleVariable CVar_RpcProofCheats("hol.Rpc.ProofCheats", 0); // ELROND-1118b: bits 1 FastBuild, 2 Population, 4 Technology, 8 Tier, 16 UnitSpeed, 32 GatherSpeed, 64 Wound, 128 DamageBuilding, 256 SpawnBuilding, 512 Defeat, 1024 FallForum, 2048 SkipForum, 4096 HoldPresence (Tools\Scenarios\rpc_cheats_run.ps1)

// T3 (SAURON-1117m): the Match (surrender) lot, guard model: 0 = the C++ RPC stays the only one (this script RPC logs and does nothing), 1 = the script RPC acts. The Duel cvar is retired (D845, lot B).
const FConsoleVariable CVar_RpcScriptMatch("hol.Rpc.ScriptMatch", 1);

// T4 lot L1 (HALDIR-1117o): the Hero RPCs (talent, ability, rage, watch unit), same guard model (the C++ RPC stays the only one at 0). ServerReturnHero, ServerStartSpecialization, ServerCancelSpecialization have no guard (D845, lot B).
const FConsoleVariable CVar_RpcScriptHero("hol.Rpc.ScriptHero", 1);

// T5 lot L1 (HALDIR-1117p): the Signals and Chat RPCs. ServerPostSignal / ServerPostChat have no guard and no C++ RPC left (D845, lot C). These two cvars are no longer read by this script: only the C++ fan-out
// (HOLPlayerController.cpp, CallEx) still reads them, to reach each viewer through its script actor's Client RPC (ClientReceiveSignal / ClientReceiveChat, still a C++ RPC with fallback, out of lot C).
const FConsoleVariable CVar_RpcScriptSignal("hol.Rpc.ScriptSignal", 1);
const FConsoleVariable CVar_RpcScriptChat("hol.Rpc.ScriptChat", 1);

// CHEATS A (ELROND-1117q): L4 group A, the cheat RPCs. Same guard model: default 0 = the C++ RPC stays the only one (these script RPCs return and do nothing), 1 = the script RPC acts. The Resources cvar is retired (D845, lot C).
const FConsoleVariable CVar_RpcScriptCheatFastBuild("hol.Rpc.ScriptCheatFastBuild", 0);
const FConsoleVariable CVar_RpcScriptCheatPopulation("hol.Rpc.ScriptCheatPopulation", 0);
const FConsoleVariable CVar_RpcScriptCheatTechnology("hol.Rpc.ScriptCheatTechnology", 0);
const FConsoleVariable CVar_RpcScriptCheatTier("hol.Rpc.ScriptCheatTier", 0);
const FConsoleVariable CVar_RpcScriptCheatUnitSpeed("hol.Rpc.ScriptCheatUnitSpeed", 0);
const FConsoleVariable CVar_RpcScriptCheatGatherSpeed("hol.Rpc.ScriptCheatGatherSpeed", 0);
// CHEATS B (CELEBORN-1117q, C++ NOT COMPILED): T9 lot L4, the cheat RPCs Wound, DamageBuilding, SpawnBuilding, Defeat, FallForum, SkipForum, HoldPresence. One guard each (default 0: the C++ RPC stays the only one).
const FConsoleVariable CVar_RpcScriptCheatWound("hol.Rpc.ScriptCheatWound", 0);
const FConsoleVariable CVar_RpcScriptCheatDamageBuilding("hol.Rpc.ScriptCheatDamageBuilding", 0);
const FConsoleVariable CVar_RpcScriptCheatSpawnBuilding("hol.Rpc.ScriptCheatSpawnBuilding", 0);
const FConsoleVariable CVar_RpcScriptCheatDefeat("hol.Rpc.ScriptCheatDefeat", 0);
const FConsoleVariable CVar_RpcScriptCheatFallForum("hol.Rpc.ScriptCheatFallForum", 0);
const FConsoleVariable CVar_RpcScriptCheatSkipForum("hol.Rpc.ScriptCheatSkipForum", 0);
const FConsoleVariable CVar_RpcScriptCheatHoldPresence("hol.Rpc.ScriptCheatHoldPresence", 0);

const int HOLRpcSlots = 8; // FHOLMatchConfig::MaxSlots
const int HOLRpcStanceAlly = 2; // HOLDiplomacy::Enemy 0 / Neutral 1 / Ally 2

namespace HOLRpc
{
// Why a stance order is not queued: 0 none (queue it), 1 the sender has no slot (spectator, out of range), 2 the target is out of range, 3 the stance is not Enemy / Neutral / Ally.
int StanceRefusal(int SenderSlot, int TargetSlot, int Stance)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (TargetSlot < 0 || TargetSlot >= HOLRpcSlots)
		return 2;
	if (Stance < 0 || Stance > HOLRpcStanceAlly)
		return 3;
	return 0;
}

// Duel (C++ ServerDuelChallenge / ServerDuelAnswer): why an order is not queued: 0 none, 1 the sender has no slot (spectator, out of range), 2 the challenged slot is out of range.
// The rest (hero alive, in range, a challenge already waiting) stays the duel rules' at the step's end (HOLDuel::CanChallenge), as for the C++ RPC.
int DuelChallengeRefusal(int SenderSlot, int TargetSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (TargetSlot < 0 || TargetSlot >= HOLRpcSlots)
		return 2;
	return 0;
}

int DuelAnswerRefusal(int SenderSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	return 0;
}

// EHOLDuelOrderKind as the int the bridge takes (enum class without UENUM): 0 Challenge, 1 Accept, 2 Refuse (HOLRpcBridge::QueueDuelOrder).
int DuelKindChallenge()
{
	return 0;
}

int DuelAnswerKind(bool bAccept)
{
	return bAccept ? 1 : 2;
}

// Match (ServerRequestPause / ServerRequestResume / ServerSurrender): 0 none, 1 the sender has no slot (a spectator cannot pause, resume or surrender). Who may pause, the cooldowns and
// the match-over case stay AHOLGameState::RequestPause / RequestResume / DefeatSlot's (they log their own refusal).
int MatchRefusal(int SenderSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	return 0;
}

// Specialization / Hero (T4 L1): 0 none, 1 the sender has no slot (spectator, out of range). The board (HOLSpecialization::Start: tier, Forum, option) and the hero rules decide the rest,
// as for the C++ RPC (its _Implementation is what the bridge calls). The ability adds: 2 not a cast ability (only CloseRanks and Banner are cast, never the Aura).
int SeatRefusal(int SenderSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	return 0;
}

int HeroAbilityRefusal(int SenderSlot, EHOLHeroAbility Ability)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (Ability != EHOLHeroAbility::CloseRanks && Ability != EHOLHeroAbility::Banner)
		return 2;
	return 0;
}

// Rage: 2 no unit named (the C++ RPC's caller sends nothing for an empty list; the server also drops it).
int RageRefusal(int SenderSlot, int UnitCount)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (UnitCount <= 0)
		return 2;
	return 0;
}

// Signals and chat (T5 L1). The board (HOLSignal::Post, HOLChat gate: length, rate, who reaches whom) stays the script board's, called by the C++ _Implementation; these only check what the RPC itself can know.
// SignalRefusal (Server): 0 none, 1 the sender has no slot (spectator, out of range: a spectator places no signal).
int SignalRefusal(int SenderSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	return 0;
}

// ClientReceiveSignal: 0 none, 1 the sender slot is out of range, 2 the kind is negative (the server only sends a posted signal).
int ClientSignalRefusal(int SenderSlot, int Kind)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (Kind < 0)
		return 2;
	return 0;
}

// ServerPostChat: 0 none, 1 negative channel, 2 empty text. A spectator talks too (D376): the sender's slot is NOT checked here (-1 is valid); the length, the rate and who reads it stay the board's.
int ChatRefusal(int Channel, const FString& Text)
{
	if (Channel < 0)
		return 1;
	if (Text.Len() == 0)
		return 2;
	return 0;
}

// ClientReceiveChat: 0 none, 1 the sender slot is neither -1 (spectator) nor in range, 2 the channel is negative.
int ClientChatRefusal(int SenderSlot, int Channel)
{
	if (SenderSlot < -1 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (Channel < 0)
		return 2;
	return 0;
}

// CHEATS A (ELROND-1117q, C++ NOT COMPILED): refusals of the L4 group A cheat RPCs. R3.5: a cheat is refused unless the cheats are active on the server (bAllowed = FHOLCheatRules::AreCheatsAllowed, read by
// the bridge); the sender is the owner's slot (D288: a seat is needed). 0 none, 1 cheats off, 2 no seat (spectator, out of range). The effect (stock, cvars, tier override, cap) stays the C++ _Implementation's.
int CheatRefusal(bool bAllowed, int SenderSlot)
{
	if (!bAllowed)
		return 1;
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 2;
	return 0;
}

// CHEATS B (CELEBORN-1117q): the cheat RPCs' pure refusals. bCheatsAllowed comes from HOLRpcBridge::CheatsAllowed (the C++ IsCheatRefused: R3.5, cheats off = refused, logs "Cheat refused: ...").
// 0 = pass to the C++ _Implementation (the board's own rules decide the rest and log their warning), else the reason. Cheats are checked first.
// Wound hits the sender's own units: 2 no slot (spectator). DamageBuilding: 2 negative amount (would heal).
int CheatWoundRefusal(bool bCheatsAllowed, int SenderSlot)
{
	if (!bCheatsAllowed)
		return 1;
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 2;
	return 0;
}

// Economy (T8 L3). The production decides the rest (owner, type, queue, stock, Market, lots: UHOLCommandSubsystem::IssueProductionOrder), as for the C++ RPC. These only check what the RPC itself can know.
// bHasBuilding = the Building pointer is not null (the caller computes it: the rules stay pure). 0 none, 1 the sender has no slot (spectator, out of range), 2 no building.
int EconomySeatRefusal(int SenderSlot, bool bHasBuilding)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (!bHasBuilding)
		return 2;
	return 0;
}

// Tier: 3 the target is not -2 (everyone), -1 (own slot) or a slot 0..7 (the C++ body ignores a slot outside, silently).
int CheatTierRefusal(bool bAllowed, int SenderSlot, int TargetSlot)
{
	const int Base = CheatRefusal(bAllowed, SenderSlot);
	if (Base != 0)
		return Base;
	if (TargetSlot < -2 || TargetSlot >= HOLRpcSlots)
		return 3;
	return 0;
}

// Technology: 3 no technology named (None). Whether it exists or is already researched stays the C++ body's (it logs a warning).
int CheatTechnologyRefusal(bool bAllowed, int SenderSlot, FName Technology)
{
	const int Base = CheatRefusal(bAllowed, SenderSlot);
	if (Base != 0)
		return Base;
	if (Technology == n"None")
		return 3;
	return 0;
}

int CheatDamageBuildingRefusal(bool bCheatsAllowed, int Amount)
{
	if (!bCheatsAllowed)
		return 1;
	if (Amount < 0)
		return 2;
	return 0;
}

// SpawnBuilding: 2 slot outside 0..255 (the C++ limit, any slot unchecked otherwise), 3 no type name.
int CheatSpawnBuildingRefusal(bool bCheatsAllowed, const FString& TypeName, int Slot)
{
	if (!bCheatsAllowed)
		return 1;
	if (Slot < 0 || Slot > 255)
		return 2;
	if (TypeName.Len() == 0)
		return 3;
	return 0;
}

// ServerQueueProduction: seat and building only (the type is checked by the production at its step).
int QueueProductionRefusal(int SenderSlot, bool bHasBuilding)
{
	return EconomySeatRefusal(SenderSlot, bHasBuilding);
}

// ServerCancelProduction: seat, building, then 3 a negative queue index.
int CancelProductionRefusal(int SenderSlot, bool bHasBuilding, int Index)
{
	const int Seat = EconomySeatRefusal(SenderSlot, bHasBuilding);
	if (Seat != 0)
		return Seat;
	if (Index < 0)
		return 3;
	return 0;
}

// UnitSpeed / GatherSpeed: 3 the scale is not finite (NaN, infinity: x - x is 0 only for a finite x); the C++ body clamps a finite scale to 0.1..10.
int CheatScaleRefusal(bool bAllowed, int SenderSlot, float Scale)
{
	const int Base = CheatRefusal(bAllowed, SenderSlot);
	if (Base != 0)
		return Base;
	if (!(Scale - Scale == 0.0))
		return 3;
	return 0;
}

// Defeat / FallForum / SkipForum: the slot named is one of the 8 seats (2 otherwise).
int CheatSlotRefusal(bool bCheatsAllowed, int Slot)
{
	if (!bCheatsAllowed)
		return 1;
	if (Slot < 0 || Slot >= HOLRpcSlots)
		return 2;
	return 0;
}

// HoldPresence: the Forum slot is a seat (2); unless bOff, held attackers name a seat as the attacker (3).
int CheatHoldPresenceRefusal(bool bCheatsAllowed, int Slot, bool bOff, bool bAttackers, bool bDefenders, int AttackerSlot)
{
	if (!bCheatsAllowed)
		return 1;
	if (Slot < 0 || Slot >= HOLRpcSlots)
		return 2;
	if (!bOff && bAttackers && (AttackerSlot < 0 || AttackerSlot >= HOLRpcSlots))
		return 3;
	return 0;
}

// ServerTribute: seat, building, then 3 the receiver out of range, 4 a negative resource, 5 an amount that is not positive.
int TributeRefusal(int SenderSlot, bool bHasBuilding, int Receiver, int Resource, int Amount)
{
	const int Seat = EconomySeatRefusal(SenderSlot, bHasBuilding);
	if (Seat != 0)
		return Seat;
	if (Receiver < 0 || Receiver >= HOLRpcSlots)
		return 3;
	if (Resource < 0)
		return 4;
	if (Amount <= 0)
		return 5;
	return 0;
}

// ServerMarketTrade: seat, building, then 3 a negative resource, 4 a lot count that is not positive.
int MarketTradeRefusal(int SenderSlot, bool bHasBuilding, int Resource, int Lots)
{
	const int Seat = EconomySeatRefusal(SenderSlot, bHasBuilding);
	if (Seat != 0)
		return Seat;
	if (Resource < 0)
		return 3;
	if (Lots <= 0)
		return 4;
	return 0;
}

// The interface's call (HOLGameScreenScript.PanelClick): through the script actor if hol.Rpc.ScriptDiplomacy is 1 and this controller has it, else the C++ RPC as before.
void RequestStance(AHOLPlayerController Controller, int TargetSlot, int Stance)
{
	if (Controller == nullptr)
		return;
	if (CVar_RpcScriptDiplomacy.GetInt() != 0)
	{
		AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller));
		if (Rpc != nullptr)
		{
			Rpc.ServerDiplomacyStance(TargetSlot, Stance);
			return;
		}
	}
	Controller.RequestStance(TargetSlot, Stance);
}
} // namespace HOLRpc

class AHOLRpcPlayer : AHOLScriptRpcPlayerActor
{
	default bReplicates = true;
	default bAlwaysRelevant = true;

	bool bProbed = false;

	// Client, owner only: the proof's trigger (hol.Rpc.DiplomacyTarget >= 0), once.
	float ProofClock = 0.0;
	bool bRouteProved = false;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
		// Proof of the C++ -> script routes (Tools\Scenarios\rpc_routes_run.ps1): hol.Rpc.ProofRoute bits: 1 duel challenge, 2 pause, 4 spec cancel, 8 hero return, 16 cheat resources, fired ONCE 4 s after the owner's first tick.
		if (!HasAuthority() && !bRouteProved && (CVar_RpcProofRoute.GetInt() > 0 || CVar_RpcProofCheats.GetInt() > 0))
		{
			APlayerController RouteController = Cast<APlayerController>(GetOwner());
			if (RouteController != nullptr && RouteController.IsLocalController())
			{
				ProofClock += DeltaSeconds;
				if (ProofClock >= 4.0)
				{
					bRouteProved = true;
					const int Routes = CVar_RpcProofRoute.GetInt();
					TArray<FString> Commands;
					if ((Routes & 1) != 0)
						Commands.Add("hol.Duel.Challenge 1");
					if ((Routes & 2) != 0)
						Commands.Add("hol.Match.Pause");
					if ((Routes & 4) != 0)
						Commands.Add("hol.Spec.Cancel");
					if ((Routes & 8) != 0)
						Commands.Add("hol.Hero.Return");
					if ((Routes & 16) != 0)
						Commands.Add("hol.Cheat.Resources 100");
					// ELROND-1118b: P1 bits 32 hero talent (the C++ interface call RequestHeroTalent), 64 duel answer, 128 stance toward slot 1, 256 queue then cancel production (selects the main centre); P2: hol.Rpc.ProofCheats.
AHOLPlayerController RouteTalent = Cast<AHOLPlayerController>(RouteController);
if ((Routes & 32) != 0 && RouteTalent != nullptr)
{
	Log(n"LogHOLMatch", f"RpcRoute: the client runs 'RequestHeroTalent 0'");
	RouteTalent.RequestHeroTalent(0);
}
if ((Routes & 64) != 0)
	Commands.Add("hol.Duel.Accept");
if ((Routes & 128) != 0 && RouteTalent != nullptr)
{
	// The console command hol.Diplomacy.Stance calls the C++ RequestStance (direct C++ RPC); only the interface's script call HOLRpc::RequestStance takes the script route.
	Log(n"LogHOLMatch", f"RpcRoute: the client runs 'HOLRpc::RequestStance 1 ally'");
	HOLRpc::RequestStance(RouteTalent, 1, 2);
}
if ((Routes & 256) != 0)
{
	Commands.Add("hol.Prod.Queue Peasant");
	Commands.Add("hol.Prod.Cancel 0");
}
const int Cheats = CVar_RpcProofCheats.GetInt();
if ((Cheats & 1) != 0)
	Commands.Add("hol.Cheat.FastBuild 1");
if ((Cheats & 2) != 0)
	Commands.Add("hol.Cheat.Population 5");
if ((Cheats & 4) != 0)
	Commands.Add("hol.Cheat.Technology MeleeAttackI");
if ((Cheats & 8) != 0)
	Commands.Add("hol.Cheat.Tier 1");
if ((Cheats & 16) != 0)
	Commands.Add("hol.Cheat.UnitSpeed 1");
if ((Cheats & 32) != 0)
	Commands.Add("hol.Cheat.GatherSpeed 1");
if ((Cheats & 64) != 0)
	Commands.Add("hol.Cheat.Wound 50");
if ((Cheats & 128) != 0)
	Commands.Add("hol.Cheat.DamageNear 1 0 0");
if ((Cheats & 256) != 0)
	Commands.Add("hol.Cheat.SpawnBuilding Barracks 0 3000 0");
if ((Cheats & 512) != 0)
	Commands.Add("hol.Victory.Defeat 7");
if ((Cheats & 1024) != 0)
	Commands.Add("hol.Victory.FallForum 7");
if ((Cheats & 2048) != 0)
	Commands.Add("hol.Victory.SkipRuin 7");
if ((Cheats & 4096) != 0)
	Commands.Add("hol.Victory.HoldPresence 7 off");
for (FString Command : Commands)
					{
						Log(n"LogHOLMatch", f"RpcRoute: the client runs '{Command}'");
						System::ExecuteConsoleCommand(Command, RouteController);
					}
				}
			}
		}
		if (HasAuthority() || bProbed || CVar_RpcDiplomacyTarget.GetInt() < 0)
			return;
		APlayerController Controller = Cast<APlayerController>(GetOwner());
		if (Controller == nullptr || !Controller.IsLocalController())
			return;
		bProbed = true;
		Log(n"LogHOLMatch", f"RpcDiplomacy: the client calls ServerDiplomacyStance(target {CVar_RpcDiplomacyTarget.GetInt()}, stance {CVar_RpcDiplomacyStance.GetInt()})");
		ServerDiplomacyStance(CVar_RpcDiplomacyTarget.GetInt(), CVar_RpcDiplomacyStance.GetInt());
	}

	UFUNCTION(Server)
	void ServerDiplomacyStance(int TargetSlot, int Stance)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcDiplomacy: received by the server: sender slot {Slot} stance {Stance} toward slot {TargetSlot}");
		const int Refusal = HOLRpc::StanceRefusal(Slot, TargetSlot, Stance);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcDiplomacy: refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		if (HOLRpcBridge::QueueDiplomacyOrder(this, 0, TargetSlot, Stance))
			Log(n"LogHOLMatch", f"Diplomacy order: slot {Slot} stance {Stance} toward slot {TargetSlot} queued");
	}

	// ---- T3 (SAURON-1117m) Duel lot (D845: the only entry of ServerDuelChallenge / ServerDuelAnswer, no C++ RPC left, no cvar). The slot comes from the owner (D288).
	// The last call, UHOLCommandSubsystem::IssueDuelOrder(Slot, EHOLDuelOrderKind::Challenge, TargetSlot), goes through HOLRpcBridge::QueueDuelOrder(this, Kind, OtherSlot) (HALDIR-1117n, C++ NOT COMPILED).
	UFUNCTION(Server)
	void ServerDuelChallenge(int TargetSlot)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::DuelChallengeRefusal(Slot, TargetSlot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcDuel: challenge refused ({Refusal}) for sender slot {Slot} toward slot {TargetSlot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} challenge at slot {TargetSlot}");
		// The bridge queues the order and logs the C++ line "Duel order: slot N challenge at slot M queued".
		HOLRpcBridge::QueueDuelOrder(this, HOLRpc::DuelKindChallenge(), TargetSlot);
	}

	// Body of ServerDuelAnswer_Implementation (kind Accept / Refuse, no other slot). Same TODO as the challenge.
	UFUNCTION(Server)
	void ServerDuelAnswer(bool bAccept)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::DuelAnswerRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcDuel: answer refused for sender slot {Slot}");
			return;
		}
		const FString Answer = bAccept ? "accept" : "refuse";
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} {Answer}");
		HOLRpcBridge::QueueDuelOrder(this, HOLRpc::DuelAnswerKind(bAccept), -1);
	}

	// ---- T3 Match lot. ServerRequestPause / ServerRequestResume have no guard and no C++ RPC left (D845); only ServerSurrender keeps the guard hol.Rpc.ScriptMatch (its C++ RPC stays, not proved).
	// The end calls go through HOLRpcBridge::RequestPause(this) / RequestResume(this) / Surrender(this) (owner's state, GetServerWorldTimeSeconds, EHOLDefeatReason::Surrender; HALDIR-1117n, C++ NOT COMPILED);
	// AHOLGameState logs its own "Pause: ..." refusals and the bridge keeps the line "Victory: surrender of slot N refused".
	UFUNCTION(Server)
	void ServerRequestPause()
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcMatch: pause refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} asks the pause");
		HOLRpcBridge::RequestPause(this);
	}

	UFUNCTION(Server)
	void ServerRequestResume()
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcMatch: resume refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} asks the resume");
		HOLRpcBridge::RequestResume(this);
	}

	UFUNCTION(Server)
	void ServerSurrender()
	{
		if (CVar_RpcScriptMatch.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"Victory: surrender of slot {Slot} refused");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} surrenders");
		HOLRpcBridge::Surrender(this); // logs "Victory: surrender of slot N refused" itself when the game state refuses
	}

	// ---- T4 lot L1 (HALDIR-1117o). Specialization lot (D845: the only entry, no C++ RPC left, no cvar). The bridge calls AHOLPlayerController::ServerStartSpecialization_Implementation
	// / ServerCancelSpecialization_Implementation (the board's answer and the "Specialization: slot N ..." journal are the C++ ones).
	UFUNCTION(Server)
	void ServerStartSpecialization(int Tier, int Option)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcSpec: start refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcSpec: received by the server: slot {Slot} starts tier {Tier} option {Option}");
		HOLRpcBridge::StartSpecialization(this, Tier, Option);
	}

	UFUNCTION(Server)
	void ServerCancelSpecialization()
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcSpec: cancel refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcSpec: received by the server: slot {Slot} cancels");
		HOLRpcBridge::CancelSpecialization(this);
	}

	// ---- T4 lot L1 Hero lot. ServerReturnHero has no guard (D845, no C++ RPC left); ServerHeroTalent / WatchUnit / HeroAbility / CastRage keep the guard hol.Rpc.ScriptHero (their C++ RPC stays). Same model: the bridge calls the C++ _Implementation (journals "Hero return order ...", "Hero talent order ...", "Hero ability order ...",
	// "Rage order ..."). The C++ interface route (cvar 1) covers ReturnHero, HeroTalent and WatchUnit; HeroAbility and CastRage (enum + vector, array) are not routed from C++ yet: call them from script.
	UFUNCTION(Server)
	void ServerReturnHero()
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: return refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcHero: return received by the server: slot {Slot}");
		HOLRpcBridge::ReturnHero(this);
	}

	UFUNCTION(Server)
	void ServerHeroTalent(int Family)
	{
		if (CVar_RpcScriptHero.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: talent refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcHero: talent received by the server: slot {Slot} family {Family}");
		HOLRpcBridge::HeroTalent(this, Family);
	}

	UFUNCTION(Server)
	void ServerHeroAbility(EHOLHeroAbility Ability, FVector_NetQuantize Location)
	{
		if (CVar_RpcScriptHero.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::HeroAbilityRefusal(Slot, Ability);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: ability refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::HeroAbility(this, Ability, Location.X, Location.Y, Location.Z);
	}

	UFUNCTION(Server)
	void ServerCastRage(const TArray<uint32>& UnitIds)
	{
		if (CVar_RpcScriptHero.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::RageRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: rage refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		TArray<int> Ids;
		for (int i = 0; i < UnitIds.Num(); i++)
			Ids.Add(int(UnitIds[i]));
		HOLRpcBridge::CastRage(this, Ids);
	}

	UFUNCTION(Server)
	void ServerWatchUnit(uint32 UnitId)
	{
		if (CVar_RpcScriptHero.GetInt() == 0)
			return;
		// The C++ body only stores the id and sends the load of the player's OWN unit (D288 checked in SendWatchedLoad): no slot refusal here, as in C++.
		HOLRpcBridge::WatchUnit(this, int(UnitId));
	}

	// ---- CHEATS A (ELROND-1117q, C++ NOT COMPILED): L4 group A. Guards hol.Rpc.ScriptCheat<Name> (default 0). Each Server RPC refuses unless the cheats are active (R3.5: HOLRpcBridge::CheatRefused, which logs the
	// C++ line "Cheat refused: ...") and the sender has a seat (D288), then the bridge calls the C++ _Implementation (stock, cvars, tier, cap, journal: one source). FHOLResources travels as 4 ints.
	UFUNCTION(Server)
	void ServerCheatResources(int Food, int Wood, int Stone, int Gold)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatRefusal(!HOLRpcBridge::CheatRefused(this, "resources"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: resources refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: resources received by the server: slot {Slot} food {Food}");
		HOLRpcBridge::CheatResources(this, Food, Wood, Stone, Gold);
	}

	UFUNCTION(Server)
	void ServerCheatFastBuild(bool bOn)
	{
		if (CVar_RpcScriptCheatFastBuild.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatRefusal(!HOLRpcBridge::CheatRefused(this, "fast build"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: fast build refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: fast build received by the server: slot {Slot}");
		HOLRpcBridge::CheatFastBuild(this, bOn);
	}

	UFUNCTION(Server)
	void ServerCheatPopulation(int Places)
	{
		if (CVar_RpcScriptCheatPopulation.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatRefusal(!HOLRpcBridge::CheatRefused(this, "population"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: population refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: population received by the server: slot {Slot}");
		HOLRpcBridge::CheatPopulation(this, Places);
	}

	UFUNCTION(Server)
	void ServerCheatTechnology(FName Technology)
	{
		if (CVar_RpcScriptCheatTechnology.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatTechnologyRefusal(!HOLRpcBridge::CheatRefused(this, "technology"), Slot, Technology);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: technology refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: technology received by the server: slot {Slot}");
		HOLRpcBridge::CheatTechnology(this, Technology);
	}

	UFUNCTION(Server)
	void ServerCheatTier(int Tier, int Slot)
	{
		if (CVar_RpcScriptCheatTier.GetInt() == 0)
			return;
		const int Sender = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatTierRefusal(!HOLRpcBridge::CheatRefused(this, "tier"), Sender, Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: tier refused ({Refusal}) for sender slot {Sender}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: tier received by the server: sender slot {Sender}");
		HOLRpcBridge::CheatTier(this, Tier, Slot);
	}

	UFUNCTION(Server)
	void ServerCheatUnitSpeed(float Scale)
	{
		if (CVar_RpcScriptCheatUnitSpeed.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatScaleRefusal(!HOLRpcBridge::CheatRefused(this, "unit speed"), Slot, Scale);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: unit speed refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: unit speed received by the server: slot {Slot}");
		HOLRpcBridge::CheatUnitSpeed(this, Scale);
	}

	UFUNCTION(Server)
	void ServerCheatGatherSpeed(float Scale)
	{
		if (CVar_RpcScriptCheatGatherSpeed.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatScaleRefusal(!HOLRpcBridge::CheatRefused(this, "gather speed"), Slot, Scale);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: gather speed refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: gather speed received by the server: slot {Slot}");
		HOLRpcBridge::CheatGatherSpeed(this, Scale);
	}

	// ---- CHEATS B (CELEBORN-1117q, C++ NOT COMPILED): T9 lot L4 cheat RPCs. Guards hol.Rpc.ScriptCheat<Name> (default 0). Each asks the C++ IsCheatRefused through HOLRpcBridge::CheatsAllowed (R3.5), validates with the
	// HOLRpc::Cheat*Refusal functions, then the bridge calls the C++ _Implementation (the body and its "Cheat: ..." journal stay the single source). SpawnBuilding takes the type as an FString (the route fills it by CallEx).
	UFUNCTION(Server)
	void ServerCheatWound(int Percent)
	{
		if (CVar_RpcScriptCheatWound.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatWoundRefusal(HOLRpcBridge::CheatsAllowed(this, "wound"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: wound refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: wound received by the server: slot {Slot}");
		HOLRpcBridge::CheatWound(this, Percent);
	}

	UFUNCTION(Server)
	void ServerCheatDamageBuilding(int Amount, int AttackType, FVector Point)
	{
		if (CVar_RpcScriptCheatDamageBuilding.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatDamageBuildingRefusal(HOLRpcBridge::CheatsAllowed(this, "damage building"), Amount);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: damage building refused ({Refusal})");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: damage building received by the server: amount {Amount}");
		HOLRpcBridge::CheatDamageBuilding(this, Point.X, Point.Y, Point.Z, Amount, AttackType);
	}

	UFUNCTION(Server)
	void ServerCheatSpawnBuilding(const FString& TypeName, int Slot, FVector Point)
	{
		if (CVar_RpcScriptCheatSpawnBuilding.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatSpawnBuildingRefusal(HOLRpcBridge::CheatsAllowed(this, "spawn building"), TypeName, Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: spawn building refused ({Refusal}) type {TypeName} slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: spawn building received by the server: type {TypeName} slot {Slot}");
		HOLRpcBridge::CheatSpawnBuilding(this, TypeName, Slot, Point.X, Point.Y, Point.Z);
	}

	UFUNCTION(Server)
	void ServerCheatDefeat(int Slot)
	{
		if (CVar_RpcScriptCheatDefeat.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatSlotRefusal(HOLRpcBridge::CheatsAllowed(this, "defeat"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: defeat refused ({Refusal}) slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: defeat received by the server: slot {Slot}");
		HOLRpcBridge::CheatDefeat(this, Slot);
	}

	UFUNCTION(Server)
	void ServerCheatFallForum(int Slot)
	{
		if (CVar_RpcScriptCheatFallForum.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatSlotRefusal(HOLRpcBridge::CheatsAllowed(this, "fall forum"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: fall forum refused ({Refusal}) slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: fall forum received by the server: slot {Slot}");
		HOLRpcBridge::CheatFallForum(this, Slot);
	}

	UFUNCTION(Server)
	void ServerCheatSkipForum(int Slot)
	{
		if (CVar_RpcScriptCheatSkipForum.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatSlotRefusal(HOLRpcBridge::CheatsAllowed(this, "skip forum"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: skip forum refused ({Refusal}) slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: skip forum received by the server: slot {Slot}");
		HOLRpcBridge::CheatSkipForum(this, Slot);
	}

	// The C++ ServerCheatHoldPresence_Implementation has NO IsCheatRefused check (found by CELEBORN-1117q): the script path refuses when cheats are off (stricter, R3.5).
	UFUNCTION(Server)
	void ServerCheatHoldPresence(int Slot, bool bOff, bool bAttackers, bool bDefenders, int AttackerSlot)
	{
		if (CVar_RpcScriptCheatHoldPresence.GetInt() == 0)
			return;
		const int Refusal = HOLRpc::CheatHoldPresenceRefusal(HOLRpcBridge::CheatsAllowed(this, "hold presence"), Slot, bOff, bAttackers, bDefenders, AttackerSlot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: hold presence refused ({Refusal}) slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: hold presence received by the server: slot {Slot}");
		HOLRpcBridge::CheatHoldPresence(this, Slot, bOff, bAttackers, bDefenders, AttackerSlot);
	}

	// ---- T5 lot L1 Signals and chat (HALDIR-1117p). ServerPostSignal / ServerPostChat: the only entry, no C++ RPC left, no guard (D845, lot C; the chat body keeps its ChatRefusal door, D376: a spectator may speak). The bridge calls the C++ _Implementation (board, fan-out, journal);
	// its fan-out calls the viewers' Client RPC below when hol.Rpc.ScriptSignal / ScriptChat is 1 (the Client RPC keep their C++ fallback). The two Client RPCs are called BY THE SERVER on the owning client: this actor is owned and bAlwaysRelevant (PLAN-MIG2-S3 section 6).
	UFUNCTION(Server)
	void ServerPostSignal(int Kind, FVector Location)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SignalRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcSignal: post refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcSignal: received by the server: slot {Slot} kind {Kind}");
		HOLRpcBridge::PostSignal(this, Kind, Location.X, Location.Y, Location.Z);
	}

	UFUNCTION(Client)
	void ClientReceiveSignal(int Sender, int Kind, FVector Location)
	{
		const int Refusal = HOLRpc::ClientSignalRefusal(Sender, Kind);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcSignal: receive refused ({Refusal}) sender {Sender} kind {Kind}");
			return;
		}
		HOLRpcBridge::ReceiveSignal(this, Sender, Kind, Location.X, Location.Y, Location.Z);
	}

	UFUNCTION(Server)
	void ServerPostChat(int Channel, const FString& Text, int TargetSlot)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::ChatRefusal(Channel, Text);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcChat: post refused ({Refusal}) for sender slot {Slot} channel {Channel}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcChat: received by the server: slot {Slot} channel {Channel}");
		HOLRpcBridge::PostChat(this, Channel, Text, TargetSlot);
	}

	UFUNCTION(Client)
	void ClientReceiveChat(int SenderSlot, int Channel, const FString& Text)
	{
		const int Refusal = HOLRpc::ClientChatRefusal(SenderSlot, Channel);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcChat: receive refused ({Refusal}) sender {SenderSlot} channel {Channel}");
			return;
		}
		HOLRpcBridge::ReceiveChat(this, SenderSlot, Channel, Text);
	}

	// ---- T8 lot L3 Economy (D845: the only entry of these 4 RPC, no C++ RPC left, no cvar). The bridge calls the C++ Do<Name> methods of the controller (builds the FHOLProductionCommand, IssueProductionOrder).
	// Building is passed as the AHOLBuilding object (replicated by net id), the type as an FName: no index needed.
	UFUNCTION(Server)
	void ServerQueueProduction(AHOLBuilding Building, FName TypeName)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::QueueProductionRefusal(Slot, Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcEconomy: queue refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcEconomy: queue received by the server: slot {Slot} type {TypeName}");
		HOLRpcBridge::QueueProduction(this, Building, TypeName);
	}

	UFUNCTION(Server)
	void ServerCancelProduction(AHOLBuilding Building, int Index)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CancelProductionRefusal(Slot, Building != nullptr, Index);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcEconomy: cancel refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcEconomy: cancel received by the server: slot {Slot} index {Index}");
		HOLRpcBridge::CancelProduction(this, Building, Index);
	}

	UFUNCTION(Server)
	void ServerTribute(AHOLBuilding Building, int Receiver, int Resource, int Amount)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::TributeRefusal(Slot, Building != nullptr, Receiver, Resource, Amount);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcEconomy: tribute refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcEconomy: tribute received by the server: slot {Slot} to {Receiver} resource {Resource} amount {Amount}");
		HOLRpcBridge::Tribute(this, Building, Receiver, Resource, Amount);
	}

	UFUNCTION(Server)
	void ServerMarketTrade(AHOLBuilding Building, int Resource, int Lots, bool bBuy)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::MarketTradeRefusal(Slot, Building != nullptr, Resource, Lots);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcEconomy: trade refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcEconomy: trade received by the server: slot {Slot} resource {Resource} lots {Lots} buy {bBuy}");
		HOLRpcBridge::MarketTrade(this, Building, Resource, Lots, bBuy);
	}
}
