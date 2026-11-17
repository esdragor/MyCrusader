// MIG-P1d (D830/D835, throwaway branch try-as-rpc, never merged): can a Server and a Client RPC be written in Angelscript with Iris
// (net.Iris.UseIrisReplication=1)? The server runs hol.Dev.RpcProbe: each player controller gets a replicated script actor
// it owns (spawned by the C++ bridge HOLScriptRpcProbeBridge from the game mode PostLogin: a pure script world subsystem never ticked on the dedicated server); on the client, once the probe is its own and hol.Dev.RpcProbeSend is 1, it calls ServerPing (Server RPC), the server
// logs "RpcProbe: received by the server" and answers ClientPong (Client RPC), the client logs "RpcProbe: received by the client".
// Red: hol.Dev.RpcProbeSend 0 on the client, the server's line is absent.
const FConsoleVariable CVar_RpcProbeSend("hol.Dev.RpcProbeSend", 0);

class AHOLScriptRpcProbe : AActor
{
	default bReplicates = true;
	default bAlwaysRelevant = true;

	bool bSeen = false;
	bool bAsked = false;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaSeconds)
	{
		if (HasAuthority() || bAsked)
			return;
		APlayerController Controller = Cast<APlayerController>(GetOwner());
		if (Controller == nullptr || !Controller.IsLocalController())
			return;
		if (!bSeen)
		{
			bSeen = true;
			Log(n"LogHOLMatch", f"RpcProbe: the client sees its probe {GetName()} (send={CVar_RpcProbeSend.GetInt()})");
		}
		if (CVar_RpcProbeSend.GetInt() == 0)
			return;
		bAsked = true;
		Log(n"LogHOLMatch", "RpcProbe: the client calls ServerPing");
		ServerPing("hello from the client");
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
