# ELROND-1118d

## Lot D S2 fait (2026-11-18, commit 102d0f9 sur la branche tiers)

- no_cpp_rpc_gate.ps1 : $NoCppRpcLotD (15 noms, sans ServerDiplomacyVision) ajouté au défaut de Get-NoCppRpcLeft (rouge tant que HOLPlayerController.h garde les UFUNCTION).
- quick_check.ps1 : $RpcScriptCommands réduit à Hero, Match, Signal, Chat ; commentaire périmé (« C++ RPCs stay as the fallback ») corrigé. BOM EF BB BF conservé, CRLF conservé, ParseFile 0 erreur.
- rpc_cheats_run.ps1 : $cheatCvars et $on supprimés ; serveur sans -ExecCmds, client vert -ExecCmds="$proof" ; les 3 -ArgumentList de Start-Process vérifiés (guillemet fermant avant -PassThru). ParseFile 0 erreur. Non lancé (aucun build/éditeur).
- rpc_routes_*_run.ps1 non touchés (Match et Hero restent).
- À lancer après le build C++ lot D : quick_check -Parts values,net,netsig3 ; rpc_cheats_run.ps1 ; rpc_routes_seats_run.ps1 ; rpc_diplomacy_run.ps1.

## Lot D C2 fait (pont : HOLScriptRpcPlayerActor.cpp/.h, HOLCommandSubsystem.h)
- Commit 8fc4019 (worktree Elrond, branche tiers, non fusionne, non compile) : 22 lignes -, 22 lignes + sur 3 fichiers.
- 14 appels Controller->Server<X>_Implementation( remplaces par Controller->Do<X>( (HeroTalent + 13 triches), 1 occurrence chacun controlee par script, fins de ligne gardees (cpp/.h en LF, HOLCommandSubsystem.h en CRLF).
- QueueDiplomacyOrder non touche (seule source de la stance).
- Commentaires reformules : cpp 136, 200, 256 ; .h 74, 99, 126 ; HOLCommandSubsystem.h 255 et 272. Les commentaires .h 87 et 91 (HeroAbility, CastRage) et cpp 312/341 (signaux, economie) restent : ces RPC gardent leur _Implementation (hors lot D), les reformuler les rendrait faux.
- Suspect de compilation : les 14 Do<X> doivent exister dans HOLPlayerController.h (agent C1) avec les parametres actuels (FVector_NetQuantize, uint8 AttackType, FName, bOff/bAttackers/bDefenders/AttackerSlot) et le pont doit rester friend ; au moment de ce commit le .h du controleur (en cours chez C1) ne declarait pas encore les Do* (seul DoCheatResources existe).
## Lot D C1 fait (contrôleur : HOLPlayerController.h et .cpp)
- Commit bc0aca0 (worktree Elrond, branche tiers, non fusionné, NON COMPILÉ) : 115 lignes -, 71 lignes + sur 2 fichiers (.h CRLF, .cpp LF gardés).
- .h : 13 triches et HeroTalent passent de UFUNCTION(Server, Reliable) ServerX à DoX sans UFUNCTION (zone private, pont friend) ; ServerDiplomacyStance supprimée (ServerDiplomacyVision gardée, commentaire D5 reporté devant elle).
- .cpp : 14 corps renommés Do<X> (HeroTalent + 13 triches, journaux et IsCheatRefused gardés, pas d'IsCheatRefused ajouté à HoldPresence) ; corps ServerDiplomacyStance_Implementation supprimé ; 6 Request cheats CallCheat et 7 CallEx passent à nullptr + LogNotSent("Cheat<X>") ; RequestHeroTalent : Call(this, nullptr, ...) + LogNotSent("HeroTalent") ; RequestStance envoie par CallN(this, nullptr, "ServerDiplomacyStance", StanceArgs) puis LogNotSent("DiplomacyStance") (piège de la carte).
- LogNotSent de RequestPause / RequestResume : noms courts « RequestPause » / « RequestResume » (avant « ServerRequestPause » / « ServerRequestResume »).
- Commentaires périmés corrigés : l.64 (Economy), 4749-4753 (CHEATS A, CallCheat), 5221 et 5238 (« falls back to the C++ RPC »), commentaires « through the script actor when hol.Rpc.ScriptCheat<X> is 1 » retirés. Laissés (Signal / Chat hors lot) : « else the C++ RPC as before » aux ex-l.5527 et 5627.
- Grep final : aucun Server(HeroTalent|DiplomacyStance|Cheat[A-Za-z]+)_Implementation ni UFUNCTION(Server) de ces noms dans le .h / .cpp ; aucun autre appelant de ServerX dans Source hors le pont.
- Suspects de compilation : (1) RequestStance, StanceArgs est un tableau C const int32[2] passé à TArrayView<const int32> (même patron que StartSpecialization) ; (2) CallCheat / CallEx / Call / CallN avec cvar nullptr : acceptés d'après la carte (CallFull garde « CVarName && ! »), à confirmer au build ; (3) le pont (friend) doit appeler les 14 Do<X> avec les paramètres inchangés (FVector_NetQuantize, uint8 AttackType) ; (4) cvars hol.Rpc.ScriptCheat<X> et ScriptDiplomacy plus lus par le C++ (S1 les supprime du script).
- Reste : build groupé (C1 + C2) par la session principale, puis quick_check -Parts values,net,netsig3, rpc_cheats_run.ps1, rpc_routes_seats_run.ps1, rpc_diplomacy_run.ps1.

## Lot D S1 fait (agent S1, script, commit 533ff17 sur la branche tiers)
- Script\Net\HOLRpcPlayer.as : CVar_RpcScriptDiplomacy et les 13 CVar_RpcScriptCheat<X> supprimees (aucun lecteur restant dans Script\ ; Tools\ traite par S2) ; CVar_RpcScriptHero GARDEE (WatchUnit, HeroAbility, CastRage).
- 14 gardes retirees (HeroTalent + 13 triches) ; SeatRefusal, journaux "RpcCheat: ... received" et CheatsAllowed gardes.
- HOLRpc::RequestStance : plus de garde ni de repli Controller.RequestStance ; acteur present -> Rpc.ServerDiplomacyStance, sinon Log "RpcRoute: DiplomacyStance not sent: no script RPC actor".
- Preuve ProofRoute : bit 128 garde (HOLRpc::RequestStance), bit 512 ajoute (commande console "hol.Diplomacy.Stance 1 ally", ligne "RpcRoute: the client runs 'hol.Diplomacy.Stance 1 ally'" ecrite par la boucle des commandes).
- Commentaires perime reformules (en-tete, Hero, Specialization -> DoStart/DoCancelSpecialization, Cheats A/B -> Do<X>, HoldPresence) + 6 commentaires de Script\Tests\HOLRpc*_Test.as ("C++ RPC" -> "retired C++ RPC" / Do<X>).
- Rien compile ni lance (la DLL doit contenir C1+C2 : DoHeroTalent, DoCheat<X>, DoStartSpecialization, FindPlayerActor deja la). Lignes : 34 + / 75 - (7 fichiers).
- Restent des mentions "_Implementation" hors lot (HOLRpcPlayer.as l.80, 107, 146, 158 : HeroAbility/CastRage/Signal, cheats refusals) : non touchees.