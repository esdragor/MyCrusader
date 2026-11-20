// MIG2-S3 pilot (D842 Q1 = A, ELROND-1117o): the per-player RPC actor written in script, model of the proven probe (HOLScriptRpcProbe.as). The server spawns one per controller,
// owned by it (C++ HOLScriptRpcPlayerBridge::SpawnAllFor, called from the game mode's PostLogin); each RPC of the player is declared here. ServerDiplomacyStance is the only entry of the stance order
// (D845, lot D: no C++ RPC and no cvar left; slot read from the owner: D288; HOLDiplomacy::SetStance checks the rest at the next step); the C++ RequestStance (console command hol.Diplomacy.Stance) reaches it through the C++ -> script route.
// This file lives in Script/Net: the plugin skips Script/Dev, Developers, Examples and Editor outside the interactive editor. NEEDS THE C++ LOT (HOLScriptRpcPlayerActor, HOLRpcBridge::).
// Proof: hol.Rpc.DiplomacyTarget (default -1; >= 0 on a client: it calls ServerDiplomacyStance(Target, hol.Rpc.DiplomacyStance) once) used by Tools\Scenarios\rpc_diplomacy_run.ps1.
const FConsoleVariable CVar_RpcDiplomacyTarget("hol.Rpc.DiplomacyTarget", -1);
const FConsoleVariable CVar_RpcDiplomacyStance("hol.Rpc.DiplomacyStance", 1);
const FConsoleVariable CVar_RpcProofRoute("hol.Rpc.ProofRoute", 0);
const FConsoleVariable CVar_RpcProofCheats("hol.Rpc.ProofCheats", 0); // ELROND-1118b: bits 1 FastBuild, 2 Population, 4 Technology, 8 Tier, 16 UnitSpeed, 32 GatherSpeed, 64 Wound, 128 DamageBuilding, 256 SpawnBuilding, 512 Defeat, 1024 FallForum, 2048 SkipForum, 4096 HoldPresence (Tools\Scenarios\rpc_cheats_run.ps1)

// D845, lot E: no cvar guards a script RPC any more (hol.Rpc.ScriptMatch, ScriptHero, ScriptSignal, ScriptChat are retired) and no C++ RPC is left for the player: every Server RPC (Match, Hero, Signals, Chat, Specialization, Duel,
// Diplomacy, Economy, the 14 cheats) and the Client RPCs ClientReceiveSignal / ClientReceiveChat are declared here, the C++ routes reach them through the script actor (RpcRoute: ... not sent when there is none).

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

// Why a vision order is not queued (ServerDiplomacyVision, D847 Q2 = B): 0 none (queue it), 1 the sender has no slot (spectator, out of range), 2 the target is out of range. bShare needs no check (any value is a boolean).
int VisionRefusal(int SenderSlot, int TargetSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (TargetSlot < 0 || TargetSlot >= HOLRpcSlots)
		return 2;
	return 0;
}

// Duel (ServerDuelChallenge / ServerDuelAnswer): why an order is not queued: 0 none, 1 the sender has no slot (spectator, out of range), 2 the challenged slot is out of range.
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

// Honor (CELEBORN-1120b): 3 the sender's faction has no gauge (bHasGauge: HOLFactionResource::HasResource, computed by the caller: the rule stays pure).
int CheatHonorRefusal(bool bAllowed, int SenderSlot, bool bHasGauge)
{
	const int Base = CheatRefusal(bAllowed, SenderSlot);
	if (Base != 0)
		return Base;
	if (!bHasGauge)
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

// The interface's call (HOLGameScreenScript.PanelClick): through the script actor of this controller, the only route of the stance order (D845, lot D). No C++ fallback: the C++ RequestStance (console command
// hol.Diplomacy.Stance) itself goes through the script actor, so a fallback to it would loop.
void RequestStance(AHOLPlayerController Controller, int TargetSlot, int Stance)
{
	if (Controller == nullptr)
		return;
	AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller));
	if (Rpc == nullptr)
	{
		Log(n"LogHOLMatch", f"RpcRoute: DiplomacyStance not sent: no script RPC actor");
		return;
	}
	Rpc.ServerDiplomacyStance(TargetSlot, Stance);
}

// The interface's call for the vision share (HOLGameScreenScript.PanelClick, D847 Q2 = B): same route as RequestStance, no C++ detour. No C++ fallback: the C++ RequestShareVision (console command
// hol.Diplomacy.Vision) itself goes through the script actor, so a fallback to it would loop.
void RequestVision(AHOLPlayerController Controller, int TargetSlot, bool bShare)
{
	if (Controller == nullptr)
		return;
	AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Controller));
	if (Rpc == nullptr)
	{
		Log(n"LogHOLMatch", f"RpcRoute: DiplomacyVision not sent: no script RPC actor");
		return;
	}
	Rpc.ServerDiplomacyVision(TargetSlot, bShare);
}

// ---- L2 orders, first 8 (A1, D846: the order RPCs of the unit selection, on this actor, their only entry since D848: the C++ controller keeps only the Do<X> bodies, called by the bridge).
// Pure refusals: the sender holds a seat (1: spectator or out of range) and names at least one unit (2). The 200 unit cap and the spectator rule stay HOLPlayerOrders::IsAccepted's (C++ Do<X> bodies, called by the bridge).
int UnitOrderRefusal(int SenderSlot, int UnitCount)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (UnitCount <= 0)
		return 2;
	return 0;
}

// A finite number: x - x is 0 only for a finite x (NaN and infinities give NaN).
bool IsFiniteNumber(float Value)
{
	return Value - Value == 0.0;
}

// Move / AttackMove: as UnitOrderRefusal, and 3 when one coordinate of the point is not finite.
int PointOrderRefusal(int SenderSlot, int UnitCount, float X, float Y, float Z)
{
	const int Base = UnitOrderRefusal(SenderSlot, UnitCount);
	if (Base != 0)
		return Base;
	if (!IsFiniteNumber(X) || !IsFiniteNumber(Y) || !IsFiniteNumber(Z))
		return 3;
	return 0;
}

// AttackBuilding: as UnitOrderRefusal, and 3 when no building is named (null object).
int BuildingOrderRefusal(int SenderSlot, int UnitCount, bool bHasBuilding)
{
	const int Base = UnitOrderRefusal(SenderSlot, UnitCount);
	if (Base != 0)
		return Base;
	if (!bHasBuilding)
		return 3;
	return 0;
}

// MoveInLine: as UnitOrderRefusal, and 3 when one coordinate of the two points (Start, End) is not finite.
int LineOrderRefusal(int SenderSlot, int UnitCount, float StartX, float StartY, float StartZ, float EndX, float EndY, float EndZ)
{
	const int Base = PointOrderRefusal(SenderSlot, UnitCount, StartX, StartY, StartZ);
	if (Base != 0)
		return Base;
	if (!IsFiniteNumber(EndX) || !IsFiniteNumber(EndY) || !IsFiniteNumber(EndZ))
		return 3;
	return 0;
}

// The unit ids as the bridge takes them (int32: uint32 is not BlueprintCallable); the C++ side casts them back to uint32.
TArray<int> OrderIds(const TArray<uint32>& UnitIds)
{
	TArray<int> Ids;
	for (int i = 0; i < UnitIds.Num(); i++)
		Ids.Add(int(UnitIds[i]));
	return Ids;
}

// ---- L2 orders, last 8 (A2, D846). Same codes as the first 8: 1 no seat, 2 no unit named, 3 a point not finite or no building, 4 (PlaceBuilding only) no type name.
// Orders on a building alone (UngarrisonAll, CancelBuilding): the seat (1) and the building (3); no unit list.
int BuildingOnlyRefusal(int SenderSlot, bool bHasBuilding)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
	if (!bHasBuilding)
		return 3;
	return 0;
}

// DropOff: units (2), a building (3) and a finite point (3), the seat first (1).
int DropOffRefusal(int SenderSlot, int UnitCount, bool bHasBuilding, float X, float Y, float Z)
{
	const int Base = BuildingOrderRefusal(SenderSlot, UnitCount, bHasBuilding);
	if (Base != 0)
		return Base;
	if (!IsFiniteNumber(X) || !IsFiniteNumber(Y) || !IsFiniteNumber(Z))
		return 3;
	return 0;
}

// PlaceBuilding: seat (1), finite point (3), and 4 when the type name is empty. NO builder is required: RequestPlaceBuilding sends the order even with an empty selection and HOLPlayerOrders::Place lays the foundation without them (the C++ route did, so must this one: UnitCount is kept in the signature and ignored).
int PlaceBuildingRefusal(int SenderSlot, int UnitCount, float X, float Y, float Z, bool bHasTypeName)
{
	const int Base = PointOrderRefusal(SenderSlot, 1, X, Y, Z);
	if (Base != 0)
		return Base;
	if (!bHasTypeName)
		return 4;
	return 0;
}

// SetRallyPoint: the seat (1), a building (3) and a finite point (3); no unit list, the second building (Site) may be null.
int RallyPointRefusal(int SenderSlot, bool bHasBuilding, float X, float Y, float Z)
{
	const int Base = BuildingOnlyRefusal(SenderSlot, bHasBuilding);
	if (Base != 0)
		return Base;
	if (!IsFiniteNumber(X) || !IsFiniteNumber(Y) || !IsFiniteNumber(Z))
		return 3;
	return 0;
}
// D848: the delay of a proof step written "@<seconds> <command>" (ProofRoute): the token "@12" is read from this table (no string parsing needed); -1 = not a delay token (an ordinary command).
float ProofStepDelay(const FString& Token)
{
	if (Token == "@2") return 2.0;
	if (Token == "@3") return 3.0;
	if (Token == "@4") return 4.0;
	if (Token == "@6") return 6.0;
	if (Token == "@8") return 8.0;
	if (Token == "@12") return 12.0;
	if (Token == "@15") return 15.0;
	if (Token == "@20") return 20.0;
	if (Token == "@25") return 25.0;
	if (Token == "@30") return 30.0;
	if (Token == "@40") return 40.0;
	if (Token == "@60") return 60.0;
	return -1.0;
}

// The command of a step "@12 hol.Units.Where 0" without its delay token ("hol.Units.Where 0"); the text itself when it is not a delayed step.
FString ProofStepCommand(const FString& Step)
{
	TArray<FString> Parts;
	Step.ParseIntoArray(Parts, " ");
	if (Parts.Num() < 2 || ProofStepDelay(Parts[0]) < 0.0)
		return Step;
	FString Result = Parts[1];
	for (int Index = 2; Index < Parts.Num(); Index++)
		Result = Result + " " + Parts[Index];
	return Result;
}

// The delay of a step (0 for an immediate one).
float ProofStepSeconds(const FString& Step)
{
	TArray<FString> Parts;
	Step.ParseIntoArray(Parts, " ");
	if (Parts.Num() < 2)
		return 0.0;
	const float Seconds = ProofStepDelay(Parts[0]);
	if (Seconds < 0.0)
		return 0.0;
	return Seconds;
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

	// D848: the delayed steps of the proof ("@<seconds> <command>", seconds after the shot): the commands, their due time on DeferredClock, and whether each one was run.
	TArray<FString> DeferredCommands;
	TArray<float> DeferredTimes;
	TArray<bool> DeferredDone;
	float DeferredClock = 0.0;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
		if (bRouteProved && DeferredCommands.Num() > 0 && !HasAuthority())
		{
			APlayerController DeferredController = Cast<APlayerController>(GetOwner());
			if (DeferredController != nullptr && DeferredController.IsLocalController())
			{
				DeferredClock += DeltaSeconds;
				for (int Slot = 0; Slot < DeferredCommands.Num(); Slot++)
				{
					if (!DeferredDone[Slot] && DeferredClock >= DeferredTimes[Slot])
					{
						DeferredDone[Slot] = true;
						const FString DueCommand = DeferredCommands[Slot];
						Log(n"LogHOLMatch", f"RpcRoute: the client runs '{DueCommand}' (delayed {DeferredTimes[Slot]} s)");
						System::ExecuteConsoleCommand(DueCommand, DeferredController);
					}
				}
			}
		}
		// Proof of the C++ -> script routes (Tools\Scenarios\rpc_routes_run.ps1): hol.Rpc.ProofRoute bits: 1 duel challenge, 2 pause, 4 spec cancel, 8 hero return, 16 cheat resources, 32 to 512 below, 1024 vision, 2048 hero ability, 4096 rage, 8192 watch unit (both select an idle worker by hol.Select.NextIdle: no screen projection needed), 16384 surrender (last), fired ONCE 4 s after the owner's first tick.
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
					// ELROND-1118b: P1 bits 32 hero talent (the C++ interface call RequestHeroTalent), 64 duel answer, 128 stance toward slot 1 (the interface call HOLRpc::RequestStance), 256 queue then cancel production (selects the main centre), 512 stance toward slot 1 by the console command hol.Diplomacy.Stance; P2: hol.Rpc.ProofCheats.
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
	// The interface's script call HOLRpc::RequestStance takes the script route directly; bit 512 below proves the console command's route (C++ RequestStance -> script actor, lot D).
	Log(n"LogHOLMatch", f"RpcRoute: the client runs 'HOLRpc::RequestStance 1 ally'");
	HOLRpc::RequestStance(RouteTalent, 1, 2);
}
if ((Routes & 512) != 0)
{
	// The loop below logs "RpcRoute: the client runs 'hol.Diplomacy.Stance 1 ally'" then runs the command: the C++ RequestStance now goes through the script actor's ServerDiplomacyStance (proof of the trap, D845 lot D).
	Commands.Add("hol.Diplomacy.Stance 1 ally");
}
if ((Routes & 256) != 0)
{
	Commands.Add("hol.Prod.Queue Peasant");
	Commands.Add("hol.Prod.Cancel 0");
}
// Lot E (D847): 1024 the vision share by the console command (C++ RequestShareVision -> script actor ServerDiplomacyVision), 2048 the hero ability CloseRanks (ServerHeroAbility), 4096 select an own unit then the rage (hol.Hero.Rage, C++ console command: ServerCastRage), 8192 select an own unit alone (BroadcastSelection -> ServerWatchUnit).
if ((Routes & 1024) != 0)
	Commands.Add("hol.Diplomacy.Vision 1 1");
if ((Routes & 2048) != 0)
	Commands.Add("hol.Hero.Cast CloseRanks");
if ((Routes & 4096) != 0)
{
	Commands.Add("hol.Select.NextIdle");
	Commands.Add("hol.Hero.Rage");
}
if ((Routes & 8192) != 0)
	Commands.Add("hol.Select.NextIdle");
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
// D848, the 16 L2 orders (CARTE-L2-PREUVES-1118.md), one bit each (bit 1073741824 is the last that fits the cvar's 31 bits). A step "@<seconds> <command>" runs that many seconds after the shot (the Tick above); the end reports are "@12 hol.Units.Where 0" and "@12 hol.Build.Report".
// P1 moves: 32768 Move, 65536 AttackMove, 131072 MoveInLine, 262144 Stop. The hol.Units.* commands need no selection (all the local player's units known here).
if ((Routes & 32768) != 0)
	Commands.Add("hol.Units.OrderNear 1500 0");
if ((Routes & 65536) != 0)
	Commands.Add("hol.Units.AttackMove -13300 0"); // absolute point 1500 cm in front of the main centre (-14800, 0): the old (1500, 0) was 16000 cm away, units still walking at +12 s
if ((Routes & 131072) != 0)
	Commands.Add("hol.Units.OrderLine -13300 -300 -13300 300"); // a 600 cm line close to the base (the old far line was still in transit at +12 s)
if ((Routes & 262144) != 0)
{
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Units.OrderNear 3000 0");
	Commands.Add("hol.Select.Stop");
}
// P2 combat: 524288 AttackUnit (the first unit of slot 1: ProofSpawn bit 4), 1048576 AttackBuilding, 2097152 HealUnit (ProofSpawn bit 1: the Monk), 4194304 ToggleDeploy (ProofSpawn bit 2 with hol.Siege.Deployable 1; cell 6 = HOLCommandGrid DeployCell).
if ((Routes & 524288) != 0)
{
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Select.Attack slot=1");
}
if ((Routes & 1048576) != 0)
{
	// The enemy main centre is ~30000 cm away (x 15200): speed x5 so the units arrive and strike within the report at +20 s.
	Commands.Add("hol.Cheat.UnitSpeed 5");
	Commands.Add("hol.Units.AttackBuilding");
	Commands.Add("@20 hol.Units.Where 0");
}
if ((Routes & 2097152) != 0)
{
	Commands.Add("hol.Cheat.Wound 50");
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Select.Heal slot=0");
}
if ((Routes & 4194304) != 0)
{
	Commands.Add("hol.Select.Type Mangonel");
	Commands.Add("hol.Select.OrderCell 6");
}
// P3 resources: 8388608 Gather, 16777216 DropOff (gather, then the drop-off at +25 s), 33554432 BuildUnits (a site first, then the units sent on it), 67108864 PlaceBuilding.
if ((Routes & 8388608) != 0)
{
	Commands.Add("hol.Cheat.GatherSpeed 5");
	Commands.Add("hol.Units.GatherOrder");
	Commands.Add("@30 hol.Units.Where 0");
	Commands.Add("@30 hol.Build.Report");
	Commands.Add("@60 hol.Build.Report");
}
if ((Routes & 16777216) != 0)
{
	Commands.Add("hol.Cheat.GatherSpeed 5");
	Commands.Add("hol.Units.GatherOrder");
	Commands.Add("@25 hol.Units.DropOff");
	Commands.Add("@40 hol.Units.Where 0");
	Commands.Add("@40 hol.Build.Report");
}
if ((Routes & 33554432) != 0)
{
	// The site first (no builder), replicated, THEN the units sent on it (@4). (-14800, 1500): ground between the LumberCamp (-14800, -1200) and the Mill (-14000, 2000); (-14000, 0) was "on a building".
	Commands.Add("hol.Cheat.Resources 100");
	Commands.Add("hol.Build.PlaceNear House 0 1500");
	Commands.Add("@4 hol.Select.Own");
	Commands.Add("@4 hol.Units.Build");
	Commands.Add("@12 hol.Build.Report");
	Commands.Add("@30 hol.Build.Report");
}
if ((Routes & 67108864) != 0)
{
	Commands.Add("hol.Cheat.Resources 100");
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Build.PlaceNear House -1500 0"); // a spot of its own (BuildUnits uses 0 1500)
}
// P4 buildings: 134217728 CancelBuilding (a site, selected by hol.Prod.Queue Peasant House: to be confirmed), 268435456 SetRallyPoint (the point, then the site), 536870912 Garrison (a Barracks from the client, then the garrison), 1073741824 UngarrisonAll (Garrison, then out at +25 s).
if ((Routes & 134217728) != 0)
{
	Commands.Add("hol.Cheat.Resources 100");
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Build.PlaceNear House 1500 -1800");
	// Real run 2026-11-18: no "Cancel request" line. hol.Prod.Queue Peasant House DID select a foundation ("Selection: House of slot 0 (own), foundation") but the "@4 hol.Select.Own" of the BuildUnits / Garrison bits ran before the cancel and replaced it by 6 units. The select and the cancel now run in the SAME step (@5), after the other bits' "@3" (rally site) and "@4" (Select.Own, Units.Build) steps.
	Commands.Add("@6 hol.Prod.Queue Peasant House"); // @6: the delay table of ProofStepSeconds has no 5 (an unknown delay ran the step raw, "Command not recognized")
	Commands.Add("@6 hol.Build.CancelSelected");
}
if ((Routes & 268435456) != 0)
{
	Commands.Add("hol.Prod.Rally 800 0");
	Commands.Add("hol.Cheat.Resources 100");
	Commands.Add("hol.Select.Own");
	Commands.Add("hol.Build.PlaceNear House 3000 0");
	Commands.Add("@3 hol.Prod.RallySite");
}
if ((Routes & (536870912 + 1073741824)) != 0)
{
	Commands.Add("hol.Cheat.SpawnBuilding Barracks 0 600 0");
	Commands.Add("@4 hol.Select.Own");
	Commands.Add("@4 hol.Units.Garrison");
	Commands.Add("@15 hol.Units.Where 0");
	Commands.Add("@15 hol.Build.Report");
}
if ((Routes & 1073741824) != 0)
{
	Commands.Add("@25 hol.Units.UngarrisonAll");
	Commands.Add("@30 hol.Units.Where 0");
}
// The end reports of the single-step bits (the effect is read on this client: the units are replicated here): the positions, the stock, the buildings and their progress.
if ((Routes & (32768 + 65536 + 131072 + 262144 + 524288 + 1048576 + 2097152 + 4194304)) != 0)
	Commands.Add("@12 hol.Units.Where 0");
if ((Routes & (33554432 + 67108864 + 134217728 + 268435456)) != 0)
	Commands.Add("@12 hol.Build.Report");
// Lot E P3: 16384 the surrender of the owner (hol.Victory.Surrender -> C++ RequestSurrender -> script ServerSurrender); LAST, the player is defeated after it: with delayed steps it is itself delayed past the last one (@60).
float LastDelay = 0.0;
for (FString Step : Commands)
{
	const float StepDelay = HOLRpc::ProofStepSeconds(Step);
	if (StepDelay > LastDelay)
		LastDelay = StepDelay;
}
if ((Routes & 16384) != 0)
{
	if (LastDelay > 0.0)
		Commands.Add("@60 hol.Victory.Surrender");
	else
		Commands.Add("hol.Victory.Surrender");
}
for (FString Command : Commands)
{
	const float Delay = HOLRpc::ProofStepSeconds(Command);
	if (Delay > 0.0)
	{
		DeferredCommands.Add(HOLRpc::ProofStepCommand(Command));
		DeferredTimes.Add(Delay);
		DeferredDone.Add(false);
		Log(n"LogHOLMatch", f"RpcRoute: the client will run '{Command}' after {Delay} s");
	}
	else
	{
		Log(n"LogHOLMatch", f"RpcRoute: the client runs '{Command}'");
		System::ExecuteConsoleCommand(Command, RouteController);
	}
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

	// Vision share (D847 Q2 = B; lot E: the only entry, no C++ RPC left). The bridge queues the order as Kind 1 (Vision); HOLDiplomacy checks the pair and the mode at the next step.
	UFUNCTION(Server)
	void ServerDiplomacyVision(int TargetSlot, bool bShare)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Shared = bShare ? 1 : 0;
		Log(n"LogHOLMatch", f"RpcDiplomacy: vision received by the server: sender slot {Slot} vision {Shared} toward slot {TargetSlot}");
		const int Refusal = HOLRpc::VisionRefusal(Slot, TargetSlot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcDiplomacy: vision refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		if (HOLRpcBridge::QueueDiplomacyOrder(this, 1, TargetSlot, Shared))
			Log(n"LogHOLMatch", f"Diplomacy order: slot {Slot} vision {Shared} toward slot {TargetSlot} queued");
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

	// Duel answer (kind Accept / Refuse, no other slot): same bridge call as the challenge.
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

	// ---- T3 Match lot. ServerRequestPause / ServerRequestResume / ServerSurrender have no guard and no C++ RPC left (D845, lots B and E).
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
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"Victory: surrender of slot {Slot} refused");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} surrenders");
		HOLRpcBridge::Surrender(this); // logs "Victory: surrender of slot N refused" itself when the game state refuses
	}

	// ---- T4 lot L1 (HALDIR-1117o). Specialization lot (D845: the only entry, no C++ RPC left, no cvar). The bridge calls AHOLPlayerController::DoStartSpecialization
	// / DoCancelSpecialization (the board's answer and the "Specialization: slot N ..." journal are the C++ ones).
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

	// ---- T4 lot L1 Hero lot. ServerReturnHero, ServerHeroTalent, ServerHeroAbility, ServerCastRage and ServerWatchUnit have no guard and no C++ RPC left (D845, lots B, D and E: the bridge calls the C++ DoReturnHero / DoHeroTalent / DoHeroAbility / DoCastRage / DoWatchUnit). Journals: "Hero return order ...", "Hero talent order ...", "Hero ability order ...",
	// "Rage order ..."; the acceptance lines "RpcHero: ability / rage / watch received by the server" prove the route. The C++ interface routes reach all five through the script actor.
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
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::HeroAbilityRefusal(Slot, Ability);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: ability refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcHero: ability received by the server: slot {Slot}");
		HOLRpcBridge::HeroAbility(this, Ability, Location.X, Location.Y, Location.Z);
	}

	UFUNCTION(Server)
	void ServerCastRage(TArray<uint32> UnitIds)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::RageRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: rage refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcHero: rage received by the server: slot {Slot} units {UnitIds.Num()}");
		TArray<int> Ids;
		for (int i = 0; i < UnitIds.Num(); i++)
			Ids.Add(int(UnitIds[i]));
		HOLRpcBridge::CastRage(this, Ids);
	}

	UFUNCTION(Server)
	void ServerWatchUnit(uint32 UnitId)
	{
		// The C++ DoWatchUnit only stores the id and sends the load of the player's OWN unit (D288 checked in SendWatchedLoad): no slot refusal here, as before; the slot is only read for the journal.
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcHero: watch received by the server: slot {Slot} unit {int(UnitId)}");
		HOLRpcBridge::WatchUnit(this, int(UnitId));
	}

	// ---- CHEATS A (ELROND-1117q): L4 group A, no guard and no C++ RPC left (D845, lots C and D). Each Server RPC refuses unless the cheats are active (R3.5: HOLRpcBridge::CheatRefused, which logs the
	// C++ line "Cheat refused: ...") and the sender has a seat (D288), then the bridge calls the C++ Do<X> (stock, cvars, tier, cap, journal: one source). FHOLResources travels as 4 ints.
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

	// Morceau 8 (NOT COMPILED): hol.Cheat.Specialization; the bridge calls AHOLPlayerController::DoCheatSpecialization (cheat guard, the board's rules, start then end at once).
	UFUNCTION(Server)
	void ServerCheatSpecialization(int Tier, int Option)
	{
		const int Sender = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::CheatRefusal(!HOLRpcBridge::CheatRefused(this, "specialization"), Sender);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: specialization refused ({Refusal}) for sender slot {Sender}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: specialization received by the server: sender slot {Sender}, tier {Tier} option {Option}");
		HOLRpcBridge::CheatSpecialization(this, Tier, Option);
	}

	UFUNCTION(Server)
	void ServerCheatUnitSpeed(float Scale)
	{
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

	// ---- CHEATS B (CELEBORN-1117q): T9 lot L4 cheat RPCs, no guard and no C++ RPC left (D845, lot D). Each asks the C++ IsCheatRefused through HOLRpcBridge::CheatsAllowed (R3.5), validates with the
	// HOLRpc::Cheat*Refusal functions, then the bridge calls the C++ Do<X> (the body and its "Cheat: ..." journal stay the single source). SpawnBuilding takes the type as an FString (the route fills it by CallEx).
	UFUNCTION(Server)
	void ServerCheatWound(int Percent)
	{
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
		const int Refusal = HOLRpc::CheatSlotRefusal(HOLRpcBridge::CheatsAllowed(this, "skip forum"), Slot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcCheat: skip forum refused ({Refusal}) slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcCheat: skip forum received by the server: slot {Slot}");
		HOLRpcBridge::CheatSkipForum(this, Slot);
	}

	// The C++ DoCheatHoldPresence has NO IsCheatRefused check (found by CELEBORN-1117q): the script RPC, now the only entry, is the one that refuses when cheats are off (R3.5).
	UFUNCTION(Server)
	void ServerCheatHoldPresence(int Slot, bool bOff, bool bAttackers, bool bDefenders, int AttackerSlot)
	{
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
	// its fan-out calls the viewers' Client RPC below through their script actor (no C++ Client RPC and no cvar left, lot E: RpcRoute: ... not sent when a viewer has none). The two Client RPCs are called BY THE SERVER on the owning client: this actor is owned and bAlwaysRelevant (PLAN-MIG2-S3 section 6).
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
	// ---- L2 orders, first 8 (A1, D846). The C++ RequestX of AHOLPlayerController routes here (HOLRpcScriptRoute); no C++ Server RPC is left (D848). Journal "RpcOrder: <name> received by the server: slot N ..."
	// then the pure refusal (HOLRpc::UnitOrderRefusal, PointOrderRefusal, BuildingOrderRefusal, LineOrderRefusal), then the bridge calls the controller's C++ Do<X> (the rules, HOLPlayerOrders::IsAccepted: cap 200, spectator).
	UFUNCTION(Server)
	void ServerMoveUnits(TArray<uint32> UnitIds, FVector_NetQuantize Target, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: move received by the server: slot {Slot} units {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::PointOrderRefusal(Slot, UnitIds.Num(), Target.X, Target.Y, Target.Z);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: move refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderMove(this, HOLRpc::OrderIds(UnitIds), Target.X, Target.Y, Target.Z, bQueue);
	}

	UFUNCTION(Server)
	void ServerAttackUnit(TArray<uint32> UnitIds, uint32 TargetUnitId, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: attack unit received by the server: slot {Slot} units {UnitIds.Num()} target {int(TargetUnitId)} queue {bQueue}");
		const int Refusal = HOLRpc::UnitOrderRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: attack unit refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderAttackUnit(this, HOLRpc::OrderIds(UnitIds), int(TargetUnitId), bQueue);
	}

	UFUNCTION(Server)
	void ServerHealUnit(TArray<uint32> UnitIds, uint32 TargetUnitId, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: heal unit received by the server: slot {Slot} units {UnitIds.Num()} target {int(TargetUnitId)} queue {bQueue}");
		const int Refusal = HOLRpc::UnitOrderRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: heal unit refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderHealUnit(this, HOLRpc::OrderIds(UnitIds), int(TargetUnitId), bQueue);
	}

	UFUNCTION(Server)
	void ServerAttackBuilding(TArray<uint32> UnitIds, AHOLBuilding Building, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: attack building received by the server: slot {Slot} units {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::BuildingOrderRefusal(Slot, UnitIds.Num(), Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: attack building refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderAttackBuilding(this, HOLRpc::OrderIds(UnitIds), Building, bQueue);
	}

	UFUNCTION(Server)
	void ServerAttackMove(TArray<uint32> UnitIds, FVector_NetQuantize Target, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: attack move received by the server: slot {Slot} units {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::PointOrderRefusal(Slot, UnitIds.Num(), Target.X, Target.Y, Target.Z);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: attack move refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderAttackMove(this, HOLRpc::OrderIds(UnitIds), Target.X, Target.Y, Target.Z, bQueue);
	}

	UFUNCTION(Server)
	void ServerStopUnits(TArray<uint32> UnitIds, bool bHold)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: stop received by the server: slot {Slot} units {UnitIds.Num()} hold {bHold}");
		const int Refusal = HOLRpc::UnitOrderRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: stop refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderStop(this, HOLRpc::OrderIds(UnitIds), bHold);
	}

	UFUNCTION(Server)
	void ServerToggleDeploy(TArray<uint32> UnitIds)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: toggle deploy received by the server: slot {Slot} units {UnitIds.Num()}");
		const int Refusal = HOLRpc::UnitOrderRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: toggle deploy refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderToggleDeploy(this, HOLRpc::OrderIds(UnitIds));
	}

	UFUNCTION(Server)
	void ServerMoveUnitsInLine(TArray<uint32> UnitIds, FVector_NetQuantize Start, FVector_NetQuantize End, bool bWidth, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int StartX = int(Start.X);
		const int StartY = int(Start.Y);
		const int EndX = int(End.X);
		const int EndY = int(End.Y);
		Log(n"LogHOLMatch", f"RpcOrder: move in line received by the server: slot {Slot} units {UnitIds.Num()} width {bWidth} queue {bQueue} start ({StartX}, {StartY}) end ({EndX}, {EndY})");
		const int Refusal = HOLRpc::LineOrderRefusal(Slot, UnitIds.Num(), Start.X, Start.Y, Start.Z, End.X, End.Y, End.Z);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: move in line refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderMoveInLine(this, HOLRpc::OrderIds(UnitIds), Start.X, Start.Y, Start.Z, End.X, End.Y, End.Z, bWidth, bQueue);
	}

	// ---- L2 orders, last 8 (A2, D846). Same parameters as the former C++ ServerX (retired, D848) (same route and journal as the first 8: "RpcOrder: <name> received by the server: slot N ...", the pure refusal, then the bridge).
	UFUNCTION(Server)
	void ServerGatherUnits(TArray<uint32> UnitIds, int SourceIndex, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: gather received by the server: slot {Slot} units {UnitIds.Num()} source {SourceIndex} queue {bQueue}");
		const int Refusal = HOLRpc::UnitOrderRefusal(Slot, UnitIds.Num());
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: gather refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderGather(this, HOLRpc::OrderIds(UnitIds), SourceIndex, bQueue);
	}

	UFUNCTION(Server)
	void ServerPlaceBuilding(FName TypeName, FVector_NetQuantize Point, TArray<uint32> UnitIds, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: place building received by the server: slot {Slot} type {TypeName} builders {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::PlaceBuildingRefusal(Slot, UnitIds.Num(), Point.X, Point.Y, Point.Z, TypeName != NAME_None);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: place building refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderPlaceBuilding(this, TypeName, Point.X, Point.Y, Point.Z, HOLRpc::OrderIds(UnitIds), bQueue);
	}

	UFUNCTION(Server)
	void ServerBuildUnits(TArray<uint32> UnitIds, AHOLBuilding Building, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: build units received by the server: slot {Slot} units {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::BuildingOrderRefusal(Slot, UnitIds.Num(), Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: build units refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderBuildUnits(this, HOLRpc::OrderIds(UnitIds), Building, bQueue);
	}

	UFUNCTION(Server)
	void ServerDropOffUnits(TArray<uint32> UnitIds, AHOLBuilding Building, FVector_NetQuantize Point, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: drop off received by the server: slot {Slot} units {UnitIds.Num()} queue {bQueue}");
		const int Refusal = HOLRpc::DropOffRefusal(Slot, UnitIds.Num(), Building != nullptr, Point.X, Point.Y, Point.Z);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: drop off refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderDropOff(this, HOLRpc::OrderIds(UnitIds), Building, Point.X, Point.Y, Point.Z, bQueue);
	}

	UFUNCTION(Server)
	void ServerGarrisonUnits(TArray<uint32> UnitIds, AHOLBuilding Building)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: garrison received by the server: slot {Slot} units {UnitIds.Num()}");
		const int Refusal = HOLRpc::BuildingOrderRefusal(Slot, UnitIds.Num(), Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: garrison refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderGarrison(this, HOLRpc::OrderIds(UnitIds), Building);
	}

	UFUNCTION(Server)
	void ServerUngarrisonAll(AHOLBuilding Building)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: ungarrison all received by the server: slot {Slot}");
		const int Refusal = HOLRpc::BuildingOnlyRefusal(Slot, Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: ungarrison all refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderUngarrisonAll(this, Building);
	}

	UFUNCTION(Server)
	void ServerCancelBuilding(AHOLBuilding Building)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: cancel building received by the server: slot {Slot}");
		const int Refusal = HOLRpc::BuildingOnlyRefusal(Slot, Building != nullptr);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: cancel building refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderCancelBuilding(this, Building);
	}

	UFUNCTION(Server)
	void ServerSetRallyPoint(AHOLBuilding Building, FVector_NetQuantize Point, int SourceIndex, AHOLBuilding Site, bool bQueue)
	{
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		Log(n"LogHOLMatch", f"RpcOrder: set rally point received by the server: slot {Slot} source {SourceIndex} queue {bQueue}");
		const int Refusal = HOLRpc::RallyPointRefusal(Slot, Building != nullptr, Point.X, Point.Y, Point.Z);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcOrder: set rally point refused ({Refusal}) for sender slot {Slot}");
			return;
		}
		HOLRpcBridge::OrderSetRallyPoint(this, Building, Point.X, Point.Y, Point.Z, SourceIndex, Site, bQueue);
	}
}
