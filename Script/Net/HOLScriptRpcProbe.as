// MIG-P1d (D830/D835; PROVEN 2026-11-17: this file lives in Script/Net because the plugin skips Script/Dev, Developers, Examples and Editor outside the interactive editor): can a Server and a Client RPC be written in Angelscript with Iris
// (net.Iris.UseIrisReplication=1)? The server runs hol.Dev.RpcProbe: each player controller gets a replicated script actor
// it owns (spawned by the C++ bridge HOLScriptRpcProbeBridge from the game mode PostLogin: a pure script world subsystem never ticked on the dedicated server); on the client, once the probe is its own and hol.Dev.RpcProbeSend is 1, it calls ServerPing (Server RPC), the server
// logs "RpcProbe: received by the server" and answers ClientPong (Client RPC), the client logs "RpcProbe: received by the client".
// Red: hol.Dev.RpcProbeSend 0 on the client, the server's line is absent.
const FConsoleVariable CVar_RpcProbeSend("hol.Dev.RpcProbeSend", 0);
// ELROND-1118f L2 S0 (D846): the 4 parameter-type combinations of the 16 L2 orders never seen in a script RPC. hol.Rpc.ProbeTypes 1 on the client: once, 4 Server RPCs, each logs on the server
// "RpcProbeTypes: <name> received: ..." with the value of EVERY parameter (Tools\Scenarios\rpc_probe_types_run.ps1). Red: hol.Rpc.ProbeTypes 0, no "RpcProbeTypes" line.
const FConsoleVariable CVar_RpcProbeTypes("hol.Rpc.ProbeTypes", 0);

class AHOLScriptRpcProbe : AHOLScriptRpcProbeActor
{
	default bReplicates = true;
	default bAlwaysRelevant = true;

	bool bSeen = false;
	bool bAsked = false;
	bool bTypesAsked = false;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
		if (HasAuthority())
			return;
		APlayerController Controller = Cast<APlayerController>(GetOwner());
		if (Controller == nullptr || !Controller.IsLocalController())
			return;
		if (!bSeen)
		{
			bSeen = true;
			Log(n"LogHOLMatch", f"RpcProbe: the client sees its probe {GetName()} (send={CVar_RpcProbeSend.GetInt()})");
		}
		if (!bTypesAsked && CVar_RpcProbeTypes.GetInt() != 0)
		{
			bTypesAsked = true;
			FireTypesProbe();
		}
		if (bAsked || CVar_RpcProbeSend.GetInt() == 0)
			return;
		bAsked = true;
		Log(n"LogHOLMatch", "RpcProbe: the client calls ServerPing");
		ServerPing("hello from the client");
	}

	// Client side: the 4 calls. Buildings: the first and the last AHOLBuilding the client knows, nullptr when there is none (the type is proved by the reception without a reflection error).
	void FireTypesProbe()
	{
		TArray<AHOLBuilding> Buildings;
		GetAllActorsOfClass(Buildings);
		AHOLBuilding BuildingA = nullptr;
		AHOLBuilding BuildingB = nullptr;
		if (Buildings.Num() > 0)
		{
			BuildingA = Buildings[0];
			BuildingB = Buildings[Buildings.Num() - 1];
		}
		Log(n"LogHOLMatch", f"RpcProbeTypes: the client knows {Buildings.Num()} buildings (nullptr is passed when 0)");

		FVector_NetQuantize Start;
		Start.X = 1001.0;
		Start.Y = 2.0;
		Start.Z = 3.0;
		FVector_NetQuantize End;
		End.X = 1004.0;
		End.Y = 5.0;
		End.Z = 6.0;
		FVector_NetQuantize PlacePoint;
		PlacePoint.X = 2001.0;
		PlacePoint.Y = 7.0;
		PlacePoint.Z = -8.0;
		FVector_NetQuantize RallyPoint;
		RallyPoint.X = 3001.0;
		RallyPoint.Y = 9.0;
		RallyPoint.Z = 10.0;
		FVector_NetQuantize DropPoint;
		DropPoint.X = 4001.0;
		DropPoint.Y = 11.0;
		DropPoint.Z = 12.0;

		TArray<uint32> Ids;
		Ids.Add(11);
		Ids.Add(22);
		Ids.Add(33);

		Log(n"LogHOLMatch", "RpcProbeTypes: the client calls ServerProbeMoveInLine");
		ServerProbeMoveInLine(Start, End);
		Log(n"LogHOLMatch", "RpcProbeTypes: the client calls ServerProbePlaceBuilding");
		ServerProbePlaceBuilding(n"ProbeHouse", PlacePoint, Ids);
		Log(n"LogHOLMatch", "RpcProbeTypes: the client calls ServerProbeSetRallyPoint");
		ServerProbeSetRallyPoint(BuildingA, BuildingB, RallyPoint, 42, true);
		Log(n"LogHOLMatch", "RpcProbeTypes: the client calls ServerProbeDropOff");
		ServerProbeDropOff(Ids, BuildingA, DropPoint);
	}

	FString ProbeNameOf(AActor Subject)
	{
		if (Subject == nullptr)
			return "null";
		return f"{Subject.GetName()}";
	}

	FString ProbeIdsText(const TArray<uint32>& UnitIds)
	{
		FString Text = "";
		for (int Index = 0; Index < UnitIds.Num(); Index++)
		{
			if (Index > 0)
				Text += ",";
			Text += f"{UnitIds[Index]}";
		}
		return Text;
	}

	// (a) two points (MoveInLine: Start, End).
	UFUNCTION(Server)
	void ServerProbeMoveInLine(FVector_NetQuantize Start, FVector_NetQuantize End)
	{
		Log(n"LogHOLMatch", f"RpcProbeTypes: MoveInLine received: Start=({Start.X}, {Start.Y}, {Start.Z}) End=({End.X}, {End.Y}, {End.Z})");
	}

	// (b) FName + point + ids (PlaceBuilding).
	UFUNCTION(Server)
	void ServerProbePlaceBuilding(FName TypeName, FVector_NetQuantize Point, const TArray<uint32>& UnitIds)
	{
		FString IdsText = ProbeIdsText(UnitIds);
		Log(n"LogHOLMatch", f"RpcProbeTypes: PlaceBuilding received: Name={TypeName} Point=({Point.X}, {Point.Y}, {Point.Z}) Ids=[{IdsText}] Count={UnitIds.Num()}");
	}

	// (c) two buildings + point + int + bool (SetRallyPoint).
	UFUNCTION(Server)
	void ServerProbeSetRallyPoint(AHOLBuilding BuildingA, AHOLBuilding BuildingB, FVector_NetQuantize Point, int Slot, bool bFlag)
	{
		FString NameA = ProbeNameOf(BuildingA);
		FString NameB = ProbeNameOf(BuildingB);
		FString FlagText = "false";
		if (bFlag)
			FlagText = "true";
		Log(n"LogHOLMatch", f"RpcProbeTypes: SetRallyPoint received: BuildingA={NameA} BuildingB={NameB} Point=({Point.X}, {Point.Y}, {Point.Z}) Int={Slot} Bool={FlagText}");
	}

	// (d) ids + building + point (DropOff / Garrison).
	UFUNCTION(Server)
	void ServerProbeDropOff(const TArray<uint32>& UnitIds, AHOLBuilding Building, FVector_NetQuantize Point)
	{
		FString IdsText = ProbeIdsText(UnitIds);
		FString BuildingName = ProbeNameOf(Building);
		Log(n"LogHOLMatch", f"RpcProbeTypes: DropOff received: Ids=[{IdsText}] Count={UnitIds.Num()} Building={BuildingName} Point=({Point.X}, {Point.Y}, {Point.Z})");
	}

	UFUNCTION(Server)
	void ServerPing(FString Text)
	{
		AActor Owner = GetOwner();
		FString OwnerName = "none";
		if (Owner != nullptr)
			OwnerName = f"{Owner.GetName()}";
		Log(n"LogHOLMatch", "RpcProbe: received by the server: '" + Text + "' from " + OwnerName);
		ClientPong(Text.Len());
	}

	UFUNCTION(Client)
	void ClientPong(int Length)
	{
		Log(n"LogHOLMatch", f"RpcProbe: received by the client: pong {Length}");
	}
}
