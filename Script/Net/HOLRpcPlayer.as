// MIG2-S3 pilot (D842 Q1 = A, ELROND-1117o): the per-player RPC actor written in script, model of the proven probe (HOLScriptRpcProbe.as). The server spawns one per controller,
// owned by it (C++ HOLScriptRpcPlayerBridge::SpawnAllFor, called from the game mode's PostLogin); each RPC of the player is declared here. Pilot: ONE RPC, ServerDiplomacyStance,
// the same body as the C++ AHOLPlayerController::ServerDiplomacyStance_Implementation (slot read from the owner: D288; HOLDiplomacy::SetStance checks the rest at the next step).
// This file lives in Script/Net: the plugin skips Script/Dev, Developers, Examples and Editor outside the interactive editor. NEEDS THE C++ LOT (HOLScriptRpcPlayerActor, HOLRpcBridge::).
// Switch hol.Rpc.ScriptDiplomacy (default 0): 0 = the interface keeps the old C++ RPC (RequestStance); 1 = it goes through this actor. The C++ RPC stays until the 2-client proof.
// Proof: hol.Rpc.DiplomacyTarget (default -1; >= 0 on a client: it calls ServerDiplomacyStance(Target, hol.Rpc.DiplomacyStance) once) used by Tools\Scenarios\rpc_diplomacy_run.ps1.
const FConsoleVariable CVar_RpcScriptDiplomacy("hol.Rpc.ScriptDiplomacy", 0);
const FConsoleVariable CVar_RpcDiplomacyTarget("hol.Rpc.DiplomacyTarget", -1);
const FConsoleVariable CVar_RpcDiplomacyStance("hol.Rpc.DiplomacyStance", 1);

// T3 (SAURON-1117m): the Duel and Match (pause, resume, surrender) lots, same guard model: 0 = the C++ RPC stays the only one (these script RPCs log and do nothing), 1 = the script RPC acts.
const FConsoleVariable CVar_RpcScriptDuel("hol.Rpc.ScriptDuel", 0);
const FConsoleVariable CVar_RpcScriptMatch("hol.Rpc.ScriptMatch", 0);

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

// Match (ServerRequestPause / ServerRequestResume / ServerSurrender): 0 none, 1 the sender has no slot (a spectator cannot pause, resume or surrender). Who may pause, the cooldowns and
// the match-over case stay AHOLGameState::RequestPause / RequestResume / DefeatSlot's (they log their own refusal).
int MatchRefusal(int SenderSlot)
{
	if (SenderSlot < 0 || SenderSlot >= HOLRpcSlots)
		return 1;
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
	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
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

	// ---- T3 (SAURON-1117m) Duel lot. Guard hol.Rpc.ScriptDuel (default 0: the C++ RPC stays active). Body of AHOLPlayerController::ServerDuelChallenge_Implementation: the slot comes from the owner (D288).
	// TODO(TYPES_A_EXPOSER): the last call, UHOLCommandSubsystem::IssueDuelOrder(Slot, EHOLDuelOrderKind::Challenge, TargetSlot), is not visible from script (plain C++ method, enum without UENUM):
	// a HOLRpcBridge::QueueDuelOrder(this, Kind, OtherSlot) in UHOLRpcBridgeLibrary (like QueueDiplomacyOrder) is needed; until then this body validates and logs but queues nothing.
	UFUNCTION(Server)
	void ServerDuelChallenge(int TargetSlot)
	{
		if (CVar_RpcScriptDuel.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		const int Refusal = HOLRpc::DuelChallengeRefusal(Slot, TargetSlot);
		if (Refusal != 0)
		{
			Log(n"LogHOLMatch", f"RpcDuel: challenge refused ({Refusal}) for sender slot {Slot} toward slot {TargetSlot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} challenge at slot {TargetSlot} (TODO bridge: IssueDuelOrder not yet called)");
		// TODO: HOLRpcBridge::QueueDuelOrder(this, 0, TargetSlot) then Log "Duel order: slot {Slot} challenge at slot {TargetSlot} queued" (the C++ line).
	}

	// Body of ServerDuelAnswer_Implementation (kind Accept / Refuse, no other slot). Same TODO as the challenge.
	UFUNCTION(Server)
	void ServerDuelAnswer(bool bAccept)
	{
		if (CVar_RpcScriptDuel.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::DuelAnswerRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcDuel: answer refused for sender slot {Slot}");
			return;
		}
		const FString Answer = bAccept ? "accept" : "refuse";
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} {Answer} (TODO bridge: IssueDuelOrder not yet called)");
		// TODO: HOLRpcBridge::QueueDuelOrder(this, bAccept ? 1 : 2, -1) then Log "Duel order: slot {Slot} accept|refuse queued" (the C++ line).
	}

	// ---- T3 Match lot. Guard hol.Rpc.ScriptMatch (default 0: the C++ RPC stays active). Bodies of ServerRequestPause / ServerRequestResume / ServerSurrender _Implementation.
	// TODO(TYPES_A_EXPOSER): AHOLGameState::RequestPause(const AHOLPlayerState&, double), RequestResume(...) and DefeatSlot(int32, EHOLDefeatReason) are plain C++ methods (no UFUNCTION; EHOLDefeatReason
	// is a UENUM without BlueprintType): a bridge HOLRpcBridge::RequestPause(this) / RequestResume(this) / Surrender(this) (owner's state, GetServerWorldTimeSeconds, EHOLDefeatReason::Surrender)
	// is needed; the C++ logs "Pause: ..." and "Victory: surrender of slot N refused" on its own refusals, so the bridge keeps those lines.
	UFUNCTION(Server)
	void ServerRequestPause()
	{
		if (CVar_RpcScriptMatch.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcMatch: pause refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} asks the pause (TODO bridge: AHOLGameState::RequestPause not yet called)");
	}

	UFUNCTION(Server)
	void ServerRequestResume()
	{
		if (CVar_RpcScriptMatch.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::MatchRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcMatch: resume refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} asks the resume (TODO bridge: AHOLGameState::RequestResume not yet called)");
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
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} surrenders (TODO bridge: AHOLGameState::DefeatSlot(Slot, Surrender) not yet called)");
	}
}