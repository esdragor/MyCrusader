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

// T4 lot L1 (HALDIR-1117o, C++ NOT COMPILED): the Specialization (start, cancel) and Hero (return, talent, ability, rage, watch unit) RPCs, same guard model (default 0: the C++ RPC stays the only one).
const FConsoleVariable CVar_RpcScriptSpecialization("hol.Rpc.ScriptSpecialization", 0);
const FConsoleVariable CVar_RpcScriptHero("hol.Rpc.ScriptHero", 0);

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
	// The last call, UHOLCommandSubsystem::IssueDuelOrder(Slot, EHOLDuelOrderKind::Challenge, TargetSlot), goes through HOLRpcBridge::QueueDuelOrder(this, Kind, OtherSlot) (HALDIR-1117n, C++ NOT COMPILED).
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
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} challenge at slot {TargetSlot}");
		// The bridge queues the order and logs the C++ line "Duel order: slot N challenge at slot M queued".
		HOLRpcBridge::QueueDuelOrder(this, HOLRpc::DuelKindChallenge(), TargetSlot);
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
		Log(n"LogHOLMatch", f"RpcDuel: received by the server: slot {Slot} {Answer}");
		HOLRpcBridge::QueueDuelOrder(this, HOLRpc::DuelAnswerKind(bAccept), -1);
	}

	// ---- T3 Match lot. Guard hol.Rpc.ScriptMatch (default 0: the C++ RPC stays active). Bodies of ServerRequestPause / ServerRequestResume / ServerSurrender _Implementation.
	// The end calls go through HOLRpcBridge::RequestPause(this) / RequestResume(this) / Surrender(this) (owner's state, GetServerWorldTimeSeconds, EHOLDefeatReason::Surrender; HALDIR-1117n, C++ NOT COMPILED);
	// AHOLGameState logs its own "Pause: ..." refusals and the bridge keeps the line "Victory: surrender of slot N refused".
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
		Log(n"LogHOLMatch", f"RpcMatch: received by the server: slot {Slot} asks the pause");
		HOLRpcBridge::RequestPause(this);
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

	// ---- T4 lot L1 (HALDIR-1117o, C++ NOT COMPILED). Specialization lot, guard hol.Rpc.ScriptSpecialization (default 0). The bridge calls AHOLPlayerController::ServerStartSpecialization_Implementation
	// / ServerCancelSpecialization_Implementation (the board's answer and the "Specialization: slot N ..." journal are the C++ ones).
	UFUNCTION(Server)
	void ServerStartSpecialization(int Tier, int Option)
	{
		if (CVar_RpcScriptSpecialization.GetInt() == 0)
			return;
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
		if (CVar_RpcScriptSpecialization.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcSpec: cancel refused for sender slot {Slot}");
			return;
		}
		Log(n"LogHOLMatch", f"RpcSpec: received by the server: slot {Slot} cancels");
		HOLRpcBridge::CancelSpecialization(this);
	}

	// ---- T4 lot L1 Hero lot, guard hol.Rpc.ScriptHero (default 0). Same model: the bridge calls the C++ _Implementation (journals "Hero return order ...", "Hero talent order ...", "Hero ability order ...",
	// "Rage order ..."). The C++ interface route (cvar 1) covers ReturnHero, HeroTalent and WatchUnit; HeroAbility and CastRage (enum + vector, array) are not routed from C++ yet: call them from script.
	UFUNCTION(Server)
	void ServerReturnHero()
	{
		if (CVar_RpcScriptHero.GetInt() == 0)
			return;
		const int Slot = HOLRpcBridge::GetSenderSlot(this);
		if (HOLRpc::SeatRefusal(Slot) != 0)
		{
			Log(n"LogHOLMatch", f"RpcHero: return refused for sender slot {Slot}");
			return;
		}
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
		HOLRpcBridge::HeroAbility(this, Ability, Location);
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
	}}