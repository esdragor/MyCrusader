# HALDIR-1117c : MIG-P1d-C++ (pont serveur pour la sonde RPC en script), 2026-11-17

**NON COMPILE : le C++ et les scripts n'ont pas ete compiles, la sonde n'a pas ete rejouee. Syntaxe relue seulement.**

APPELS_DECLARES: ~17 (arret 15 min ou 25 appels)
HEURE_DEBUT/HEURE_FIN: 09:13 / ~09:22
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: pas atteint (la sonde n'a pas tourne) ; assertit l'effet : sans objet
COMMITS: 2 sur la branche maps (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir), merge_main_fait: non (base = main 962c37d, rien a fusionner) ; branche prete, pas de push
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0

## Commits
- `0f806ea` C++ : `Source/HeroesOfLiberty/Dev/HOLScriptRpcProbeBridge.h/.cpp` (namespace `HOLScriptRpcProbeBridge`, `FindScriptClass()` et `SpawnFor(APlayerController*)`) + 1 appel dans `AHeroesOfLibertyGameModeBase::PostLogin` (juste apres `Super::PostLogin`) + 1 include. `FindScriptClass` parcourt les UClass (comme `UHOLScriptScreen::FindScriptClass`), nom `HOLScriptRpcProbe` (ou `AHOLScriptRpcProbe`), enfant d'AActor, sans CLASS_Abstract / NewerVersionExists / Deprecated. `SpawnFor` : serveur seulement (pas NM_Client), `SpawnActor` avec `Params.Owner = Controller`, journal `LogHOLMatch: RpcProbe: the C++ bridge spawned <nom> for <controleur>`. Classe script absente : ne fait rien.
- `adad731` scripts : `Script/Dev/HOLScriptRpcProbe.as` recupere de try-as-rpc par `git checkout try-as-rpc -- ` (sans fusion), le sous-systeme `UHOLScriptRpcProbeSpawner` retire (une seule source : le pont C++ cree la sonde) ; `Tools/Scenarios/rpc_probe_run.ps1` (le `rpc_probe_run2.ps1` du scratchpad de GLORFINDEL, rendu durable et parametre par `-Root`).
- `Content/Maps/Crown.umap` non commite : non touche.

## Rejouer la sonde (apres build, par la session principale)
```
powershell -NoProfile -Command "& 'C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1' -LiveCoding=0 -Wait"
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\rpc_probe_run.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
```
Le lanceur (fenetres cachees, -nullrhi, port 7988, journaux dans `%TEMP%\rpc_probe\`) : serveur dedie /Game/Maps/Plain (Iris), client 1 rouge (`hol.Dev.RpcProbeSend 0`), client 2 vert (`-ExecCmds="hol.Dev.RpcProbeSend 1"`). Attendu :
- ligne `server up: True`, `server spawned: True` (journal serveur : `RpcProbe: the C++ bridge spawned ...`) ;
- ROUGE : `RED CHECK ... received by the server present = False` (client 1, envoi a 0) ;
- VERT : `green server received: True` (`RpcProbe: received by the server: 'hello from the client' from <PlayerController>`) et `green client pong: True` (`RpcProbe: received by the client: pong 21`).
- Reponse a ecrire dans la file : OUI si les deux VERT, NON si `green server received: False` alors que `client sees probe` est vrai (RPC script non transmise) ; si `server spawned: False` : le pont n'a pas trouve la classe (nom de classe script a verifier : regarder si le journal dit autre chose que `HOLScriptRpcProbe`).

FAIT / RESTE:
- FAIT : pont C++, hook PostLogin, scripts de sonde, lanceur durable, ligne de la file mise a jour.
- RESTE : build, rejeu rouge puis vert, reponse OUI/NON ; ensuite retirer le pont et la sonde (code jetable de Dev) ou le generaliser si OUI.
NON VERIFIE: compilation C++ (include `Dev/HOLScriptRpcProbeBridge.h` depuis `Source/HeroesOfLiberty`, `LogHOLMatch` via `HeroesOfLibertyLog.h`, `FActorSpawnParameters` via `Engine/World.h`) ; compilation du script (le fichier est identique a try-as-rpc moins le spawner) ; le nom du UClass d'une classe script (`HOLScriptRpcProbe` sans le A : deux formes testees) ; que l'acteur script se replique avec Iris ; que l'Owner donne a la creation permette le Server RPC ; le lanceur `rpc_probe_run.ps1` (analyse de syntaxe OK, jamais execute ; ses motifs d'attente ont change : plus de `spawner ticks`).
PROCESSUS_DETACHES: aucun lance
ANOMALIES_PROCESSUS: une commande python lancee par erreur a bloque 120 s en arriere-plan (sans effet, aucun fichier touche). EDITS_REFUSES: 2 (Edit sans Read, remplace par PowerShell).
GIT_STATUS_PRINCIPALE: non relu en fin (docs seulement ecrits : ce rapport, la ligne MIG-P1d-C++ de Auto_Journey_File.md).

## Champs de migration
BUILDS_DEMANDES: 1 (lot C++ : le pont, `Tools\build.ps1 -LiveCoding=0 -Wait` sur le worktree Haldir)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 0 (sonde jetable, pas un test de quick_check)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0
TYPES_A_EXPOSER: aucun nouveau (le pont C++ cree une classe script par nom)
