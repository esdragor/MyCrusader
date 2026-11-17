# ELROND-1117o : pilote MIG2-S3, ServerDiplomacyStance dans un acteur script par joueur (2026-11-17, tiers, NON COMPILE)

Décision D842 : Q1 = A (acteur script par joueur, dossier `Script\Net\`), Q2 = A (pilote = UNE RPC). Worktree `HeroesOfLiberty-Elrond`, branche tiers, `git merge main` : déjà à jour.

## Commits (anglais, pas de push, pas de fusion)
- `19045e3` : `Script\Tests\HOLRpcDiplomacy_Test.as` (3 tests, 11 assertions), ROUGE d'abord : `HOLRpcStanceRefusal` n'existe pas encore (échec de compilation script jusqu'au commit suivant et au build).
- `02252a2` : le vert (non compilé) : C++ minimal + script + scénario réseau.

## Fichiers
- C++ (`Source\HeroesOfLiberty\Net\HOLScriptRpcPlayerActor.h/.cpp`, ~100 lignes) : `AHOLScriptRpcPlayerActor` (base abstraite, réplication et toujours pertinent, modèle de la sonde) ; `UHOLRpcBridgeLibrary` (script `HOLRpcBridge::`) : `FindPlayerActor(Controller)`, `GetSenderSlot(RpcActor)` (slot lu du propriétaire, D288), `QueueDiplomacyOrder(RpcActor, Kind, Autre, Valeur)` (enveloppe de `UHOLCommandSubsystem::IssueDiplomacyOrder`, `IssueDiplomacyOrder` n'étant pas visible du script) ; `HOLScriptRpcPlayerBridge::SpawnAllFor` (généralise le pont de la sonde : un acteur par classe script dérivée de la base, au `PostLogin`, 3 lignes dans `HeroesOfLibertyGameModeBase.cpp`). La sonde reste telle quelle.
- Script (`Script\Net\HOLRpcPlayer.as`) : `AHOLRpcPlayer` déclare `UFUNCTION(Server, Reliable) ServerDiplomacyStance(int, int)` ; règle pure `HOLRpcStanceRefusal(Sender, Cible, Stance)` (0 = empile ; 1 émetteur sans slot ou hors bornes ; 2 cible hors bornes ; 3 stance hors 0..2) ; journal identique au C++ (« Diplomacy order: slot N stance S toward slot T queued »). Côté client : `HOLRpcRequestStance` (global script) appelé par les 2 sites de `HOLGameScreenScript.as` (clic de stance, puce) : cvar `hol.Rpc.ScriptDiplomacy` 0 (défaut) = ancienne RPC C++ `RequestStance` ; 1 = l'acteur script, repli sur le C++ si l'acteur est introuvable. La vision (`ServerDiplomacyVision`) et toutes les autres RPC ne bougent pas.
- Preuve : cvars `hol.Rpc.DiplomacyTarget` (défaut -1 ; >= 0 sur un client : il appelle la RPC une fois) et `hol.Rpc.DiplomacyStance` ; `Tools\Scenarios\rpc_diplomacy_run.ps1` (serveur + client rouge + client vert).

## Non vérifié (rien n'est compilé ni lancé)
- `UFUNCTION(Server, Reliable)` en script (la sonde n'avait que `Server`) ; appel d'une fonction globale script depuis la méthode d'une classe déclarée plus bas dans le fichier ; `Cast<AHOLRpcPlayer>` du retour de `FindPlayerActor` ; sur la carte `Plain`, le client peut n'avoir aucun slot : la ligne attendue est alors « refused (1) » au lieu de « Diplomacy order ... queued » (la preuve réseau est la ligne « received by the server »).
- Retrait de `ServerDiplomacyStance` du contrôleur C++ : PAS fait (seulement après le vert réseau, puis lot C++ de la séance). Lignes C++ supprimées : 0 ; ajoutées ~100 ; script ajoutées ~85 (+ 25 de test).

## À lancer (session principale)
1. Build : `Build.bat HeroesOfLibertyEditor Win64 Development -Project=<worktree>\HeroesOfLiberty.uproject` (ou le lanceur habituel du lot C++), puis `quick_check -Parts tests` : le rouge de `HOLRpcDiplomacy_Test` devient vert.
2. Preuve réseau : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\rpc_diplomacy_run.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
3. Rouge VU du test : faire fausse une valeur attendue une fois, puis remettre.
