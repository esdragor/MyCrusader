// L2 proof spawn (D848, ELROND-1118g): the SERVER places the units and the building the L2 order proofs need (a monk to heal, a deployable engine, soldiers of the other slot, a building with garrison places).
// A world subsystem (UScriptWorldSubsystem, ticked on every machine, like UHOLRpcOrderPilot): it does nothing unless the cvar hol.Rpc.ProofSpawn is not 0 (0 = inert, the default) AND the cheats are allowed on this
// server (the same gate as the cheat RPCs: HOLRpcBridge::CheatsAllowed, the C++ IsCheatRefused = FHOLCheatRules::AreCheatsAllowed, i.e. -HOLCheats on the dedicated server's command line; refused = the C++ line "Cheat refused: proof spawn").
// Set it on the SERVER only (-ExecCmds="hol.Rpc.ProofSpawn <bits>"). Bits: 1 a Monk of slot 0, 2 a Mangonel (the deployable engine of the table) of slot 0, 4 three Spearman of slot 1 (a target for the attack orders),
// 8 a Barracks of slot 0 (hol.Cheat.SpawnBuilding). Fired ONCE, 5 s after the first RPC actor with authority exists (the server then has a seated player). This file lives in Script/Net (never Dev, Developers, Examples, Editor).
// Caveats (the report ELROND-1118g, "Preuves L2 script fait"): the Mangonel is born deployed or packed according to hol.Siege.Deployable (set it to 1 in the server's -ExecCmds BEFORE this fires, deploy.txt); hol.Cheat.SpawnBuilding
// works through the LOCAL controllers (none on a dedicated server: the bit 8 only acts on a listen server or in solo; the dedicated proof places the building from the client, ProofRoute bit 536870912); no row of
// Tools\Data\BuildingTypes.csv has GarrisonSlots > 0 today.

const FConsoleVariable CVar_RpcProofSpawn("hol.Rpc.ProofSpawn", 0);

class UHOLRpcProofSpawn : UScriptWorldSubsystem
{
	// Seconds since the first RPC actor with authority was seen; -1 until then.
	float Clock = -1.0;

	// Fired once (or refused once: the refusal is logged by the C++ and nothing is retried).
	bool bDone = false;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		if (bDone)
			return;
		const int Bits = CVar_RpcProofSpawn.GetInt();
		if (Bits <= 0)
			return;
		TArray<AHOLRpcPlayer> Players;
		GetAllActorsOfClass(Players);
		AHOLRpcPlayer Authority = nullptr;
		for (AHOLRpcPlayer Player : Players)
		{
			if (Player != nullptr && Player.HasAuthority())
			{
				Authority = Player;
				break;
			}
		}
		if (Authority == nullptr)
			return;
		if (Clock < 0.0)
			Clock = 0.0;
		Clock += DeltaTime;
		if (Clock < 5.0)
			return;
		bDone = true;
		if (!HOLRpcBridge::CheatsAllowed(Authority, "proof spawn"))
			return;
		APlayerController Owner = Cast<APlayerController>(Authority.GetOwner());
		TArray<FString> Commands;
		// hol.Units.Spawn <n> <x> <y> <slot> <Type> <radius> (HOLUnitSubsystem.cpp): absolute ground point in cm, the slot 0 start is near the origin on Plain (deploy.txt: the enemy House at 5300 0).
		if ((Bits & 1) != 0)
			Commands.Add("hol.Units.Spawn 1 300 300 0 Monk 100");
		if ((Bits & 2) != 0)
			Commands.Add("hol.Units.Spawn 1 400 -300 0 Mangonel 100");
		if ((Bits & 4) != 0)
			Commands.Add("hol.Units.Spawn 3 1800 0 1 Spearman 150");
		if ((Bits & 8) != 0)
			Commands.Add("hol.Cheat.SpawnBuilding Barracks 0 600 0");
		for (FString Command : Commands)
		{
			Log(n"LogHOLMatch", f"RpcProofSpawn: the server runs '{Command}'");
			System::ExecuteConsoleCommand(Command, Owner);
		}
	}
}
