# HALDIR-1117n : MIG2-S3 ponts Duel / Partie  -- NON COMPILE (C++ jamais compile, pas de test lance)

APPELS_DECLARES: ~17 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 18:05 / 18:08
PREMIER_BUILD: sans build (consigne : machine saturee)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lance (DLL perimee, pas de build)
TEST_ROUGE: aucun (pas d'execution possible) ; assertit l'effet : non (test de fonction pure)
COMMITS: 1 (efb7685 sur maps, 18:07), merge_main_fait: non (DLL perimee)
TESTS: aucun lance ; 1 test script ajoute Test_RpcDuelKinds (HOLRpcDuelMatch_Test.as) ; TESTS_TAUTOLOGIQUES: 0 declare mais le test est des constantes de numerotation (a verifier par la session principale)
ESSAIS: aucun
QUESTIONS: aucune (defaut hol.Rpc.ScriptDuel / ScriptMatch = 0 inchange)

FAIT :
- Le pont existait deja : `UHOLRpcBridgeLibrary` est dans `Source\HeroesOfLiberty\Net\HOLScriptRpcPlayerActor.h/.cpp` (FindPlayerActor, GetSenderSlot, QueueDiplomacyOrder). Une 2e classe du meme nom dans Player\ aurait casse UHT : j'ai AJOUTE les statiques a la classe existante, pas de nouveau fichier : `QueueDuelOrder(AActor*, int32 Kind 0 Challenge/1 Accept/2 Refuse, int32 OtherSlot)` (journal "Duel order: slot N ... queued" identique au C++), `RequestPause(AActor*)`, `RequestResume(AActor*)` (GetServerWorldTimeSeconds), `Surrender(AActor*)` (DefeatSlot(Slot, Surrender), journal "Victory: surrender of slot N refused"). Include `Match/HOLGameState.h` ajoute.
- `Script\Net\HOLRpcPlayer.as` : les 5 TODO remplaces par les appels `HOLRpcBridge::QueueDuelOrder / RequestPause / RequestResume / Surrender` ; 2 fonctions pures `HOLRpc::DuelKindChallenge()` et `DuelAnswerKind(bool)`.
- `HOLPlayerController.cpp` : namespace anonyme-style unique `HOLRpcScriptRoute_Haldir1117n` (IsOn par FindConsoleVariable, lecture seule : la cvar est declaree par le script, pas de double enregistrement ; Call par nom via FindFunction + ProcessEvent, un argument int/bool) ; RequestSurrender, RequestDuelChallenge, RequestDuelAnswer, RequestPause, RequestResume passent par l'acteur script quand la cvar = 1, sinon (cvar 0, pas d'acteur, pas de fonction) l'RPC C++ comme avant. Aucune RPC C++ supprimee.

RESTE / A FAIRE PAR LA SESSION PRINCIPALE :
- Build : `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests` (Test_RpcDuelKinds + HOLRpcDuelMatch_Test) ; rouge a faire : `DuelAnswerKind` -> `return 0;` puis restaurer.
- Essai 2 clients avec hol.Rpc.ScriptDuel=1 / ScriptMatch=1 (defier, repondre, pause, reprise, abandon) et lire "Duel order: ... queued" / "Victory: surrender ... refused".

NON VÉRIFIÉ : tout le C++ (compilation) ; le type de parametre des RPC script (FIntProperty / FBoolProperty attendus par `Call` : si Angelscript genere un autre type de propriete, le parametre ne sera pas rempli et vaudra 0/false, a verifier au premier essai) ; que `ProcessEvent` sur une fonction Server de l'acteur script emette bien le RPC reseau depuis un client ; l'acces `IConsoleVariable::GetInt` const.
PROCESSUS_DETACHES: aucun lance (aucun processus demarre)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relance ; modifs voulues : EtapeDefini.md, Auto_Journey_File.md (docs), Docs\Rapports\HALDIR-1117n.md
