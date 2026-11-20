# CELEBORN-1120b : hol.Cheat.Honor (lot du socle, morceau 2), 2026-11-20

Joueur : la triche d'Honneur n'est PAS encore jouable ; les regles (bornes 0..100, refus, effet sur l'etat, commande tapee) sont ecrites et testees en script, il manque une lecture de la faction du siege pour brancher le RPC.

```
APPELS_DECLARES: 23 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 09:07 / 09:11
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: ba87011 + assertit l'effet : oui (Test_HonorCheatEffect lit AHOLGameState::GetFactionResource apres HOLHonorCheat::Apply) ; rouge NON VU a l'execution (aucun quick_check lance)
COMMITS: 2 (ba87011 test + stubs, e22eb60 regles), dernier a 09:10, merge_main_fait: non (consigne : aucun build, DLL non verifiee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune (plafond 100 / plancher 0 = D859 et D688, codes dans HOLHonorCheat::HonorCap)
```

## FAIT (branche duel-rewards, worktree HeroesOfLiberty-Celeborn, NON COMPILE)

- Garde de triche : `HOLRpcBridge::CheatRefused(this, "...")` est exposee au script (`HOLScriptRpcPlayerActor.h:102`), `GetSenderSlot` aussi (`:49`). `SetFactionResource` / `GetFactionResource` sont des UFUNCTION (`HOLGameState.h:466-470`). Donc la triche tient en script.
- `HOLRpc::CheatHonorRefusal(bAllowed, SenderSlot, bHasGauge)` dans `Script\Net\HOLRpcPlayer.as` (1 triches coupees, 2 pas d'emplacement, 3 faction sans jauge ; la base est `CheatRefusal`).
- `HOLFactionResource::CheatTarget(Requested, Gauge)` dans `Script\Factions\HOLFactionResourceRules.as` : `Math::Clamp(Requested, Floor, Cap)`.
- `HOLHonorCheat::HonorCap()` (100) et `HOLHonorCheat::Apply(State, Slot, Faction, Requested)` dans le NOUVEAU `Script\Factions\HOLHonorCheat.as` : refuse (-1) sans etat, slot < 0 ou faction sans jauge (`HasResource`, l'Aube seule) ; sinon `SetFactionResource` de la valeur bornee.
- Console : `hol.Cheat.Honor <Valeur>` ajoute a `HOLControllerCommandsC2.as` (`Names()` passe de 12 a 13, plan `ServerCheatHonor`, usage `Usage: hol.Cheat.Honor <Value>`, fonction + `FConsoleCommand`). Le test `Test_ControllerCommandsC2EachNameOnceAndRouted` passe de 12 a 13 dans le commit rouge (le nom est ajoute dans le commit vert).
- Tests : `Script\Tests\HOLHonorCheat_Test.as` (`Test_HonorCheatRefusal`, `Test_HonorCheatTargetIsBounded`, `Test_HonorCheatEffect`) ; `HOLControllerCommandsC2_Test.as` (`Test_ControllerCommandsC2CheatHonor`, nom des commandes 13). TESTS_SCRIPT_AJOUTES: 4 (+1 compte modifie).

## RESTE / TYPES_A_EXPOSER (C++ pour le lot groupe)

BUILDS_DEMANDES: 0 par moi, 1 pour le lot.

TYPES_A_EXPOSER: la faction d'un emplacement n'est lisible du script par aucune voie sure : `AHOLGameState::GetLaunchConfig()` n'est pas expose (`HOLDiplomacyReadLibrary.cpp:124-135` ne donne qu'un texte localise, inutilisable pour une regle) ; `HOLFactionRules::GetSlotFactionKey(World, Slot)` (`HOLFactionRules.h:119`) est C++ pur. A ajouter (une des deux) :
1. `UFUNCTION(BlueprintPure) EHOLFaction AHOLGameState::GetSlotFaction(int32 Slot) const` (lit `GetLaunchConfig().Slots[Slot].Faction`, `DawnOrder` par defaut comme `GetFactionKey`), ou
2. `static int32 UHOLRpcBridgeLibrary::GetSenderFaction(AActor* RpcActor)`.

Une fois exposee, le RPC se ferme en script seul (a coller dans `HOLRpcPlayer.as` apres `ServerCheatSpecialization`, plus une branche `Send` du plan `ServerCheatHonor` dans `HOLControllerCommandsC2.as`) :

```
UFUNCTION(Server)
void ServerCheatHonor(int Value)
{
	const int Sender = HOLRpcBridge::GetSenderSlot(this);
	const bool bGauge = HOLFactionResource::HasResource(<GetSlotFaction(Sender)>);
	const int Refusal = HOLRpc::CheatHonorRefusal(!HOLRpcBridge::CheatRefused(this, "honor"), Sender, bGauge);
	if (Refusal != 0) { Log(n"LogHOLMatch", f"RpcCheat: honor refused ({Refusal}) for sender slot {Sender}"); return; }
	TArray<AHOLGameState> States; GetAllActorsOfClass(States);
	if (States.Num() > 0) HOLHonorCheat::Apply(States[0], Sender, <faction>, Value);
}
```

Branche `Send` : `else if (Plan.Route == "ServerCheatHonor") Rpc.ServerCheatHonor(Plan.Ints[0]);` avant le `else LogNotSent`. Tant que ce n'est pas fait, la commande est acceptee et journalise `RpcRoute: ServerCheatHonor not sent` (Error), rien n'est ecrit. Pas de `[slot]` (le plan du lot proposait `<valeur> [slot]`) : un seul argument, a trancher si un slot est voulu.

## NON VERIFIE

Rien n'est compile ni lance : `HOLHonorCheat_Test.as`, les stubs et le correctif n'ont jamais passe l'analyseur Angelscript ; rouge non vu. Points a surveiller a la premiere compilation : `Math::Clamp` sur int, `HOLRpc::CheatRefusal` visible depuis `CheatHonorRefusal` (meme namespace, meme fichier), l'appel `HOLHonorCheat::Apply(...)` sans variable dans le test (valeur de retour ignoree), `HOLFactionResource` appele depuis `HOLRpc` (autre fichier, namespace : visible).

## ANOMALIES

- `EAU` : aucun. Les .as neufs sont en LF (git avertit du passage en CRLF), `eol_check` non lance.
- Placeholders.txt : non touche (aucun provisoire ajoute).

```
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress, Auto_Journey_File, EtapeDefini + fichiers deja modifies par d'autres sessions) : oui
```
