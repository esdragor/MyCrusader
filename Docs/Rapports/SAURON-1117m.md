# SAURON-1117m : MIG2-S3 T0 + T3 (script seul, NON COMPILE)

APPELS_DECLARES: 16 (arret 25 vus ; heure d'arret 18:03)
HEURE_DEBUT/HEURE_FIN: 17:47 / 17:50
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun quick_check)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: NON VU a l'execution (pas d'editeur). Commit unique 1081e17 (test + code, visual) ; assertit l'effet : non (fonctions pures de refus ; l'effet reseau est le rouge/vert 2 clients de la session principale)
COMMITS: 1, dernier a 17:49, merge_main_fait: non (visual = main fa1d9ab, rien a fusionner)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les valeurs de refus sont calculees par les fonctions testees)
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 6 (Test_RpcTypesProbeClassIsBorn dans HOLRpcTypes_Test.as ; Test_RpcDuelChallengeQueued, Test_RpcDuelChallengeRefused, Test_RpcDuelAnswer, Test_RpcMatchRefusal dans HOLRpcDuelMatch_Test.as ; la classe sonde de types porte 8 UFUNCTION(Server), une par type + 3 signatures reelles)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (les 5 RPC existent en script mais leurs appels de fin ne sont pas relies : rien n'est migre tant que le pont n'existe pas)
ESSAIS: aucun ; QUESTIONS: aucune

## FAIT
- T0 : `Script\Tests\HOLRpcTypes_Test.as`, classe `AHOLRpcTypesProbe` avec une `UFUNCTION(Server)` par type : `FVector_NetQuantize`, `TArray<uint32>`, `AHOLBuilding`, `FName`, `EHOLHeroAbility` (`UENUM(BlueprintType)`, `Simulation\HOLHeroAbilityRules.h`), plus les signatures reelles de `ServerPlaceBuilding`, `ServerHeroAbility`, `ServerMarketTrade`. Preuve = le module script compile ; sinon l'erreur de compilation donne la ligne du type fautif : commenter cette ligne et noter le type en TYPES_A_EXPOSER. Je n'ai pas pu lire le plugin Angelscript (introuvable dans l'arbre), donc aucun type n'est connu bon ou mauvais a ce stade.
- T3 : dans `Script\Net\HOLRpcPlayer.as`, meme acteur `AHOLRpcPlayer` : `ServerDuelChallenge(int)`, `ServerDuelAnswer(bool)`, `ServerRequestPause()`, `ServerRequestResume()`, `ServerSurrender()` (memes signatures que `HOLPlayerController.h` : 881, 883, 645, 647, 861). Slot de l'emetteur lu par `HOLRpcBridge::GetSenderSlot(this)` (jamais du client). Garde-fous `hol.Rpc.ScriptDuel` et `hol.Rpc.ScriptMatch` (defaut 0 : le corps sort immediatement, le C++ reste actif). Fonctions pures `HOLRpc::DuelChallengeRefusal`, `DuelAnswerRefusal`, `MatchRefusal` (1 = pas de slot / spectateur, 2 = cible hors bornes).
- Tests : `Script\Tests\HOLRpcDuelMatch_Test.as`.

## RESTE (important : le corps n'appelle PAS encore la regle de fin)
Les appels de fin ne sont pas visibles du script (methodes C++ sans UFUNCTION, enums sans UENUM / BlueprintType). Les corps valident et journalisent ("RpcDuel: received by the server ... (TODO bridge ...)"), mais ne mettent rien en file. Des TODO precis sont dans le code. Les lignes de journal C++ ("Duel order: slot N ... queued", "Victory: surrender of slot N refused") ne sont donc PAS reproduites a l'identique pour le succes (ce serait mentir) ; elles sont a poser au moment du pont.

TYPES_A_EXPOSER (lot C++ a faire, a ne pas decider ici ; modele : `UHOLRpcBridgeLibrary::QueueDiplomacyOrder`) :
- `HOLRpcBridge::QueueDuelOrder(AActor* RpcActor, int32 Kind, int32 OtherSlot)` : `UHOLCommandSubsystem::IssueDuelOrder(int32, EHOLDuelOrderKind, int32)` (methode nue ; `EHOLDuelOrderKind` est un `enum class` sans UENUM). Kind 0 Challenge / 1 Accept / 2 Refuse.
- `HOLRpcBridge::RequestPause(AActor*)` / `RequestResume(AActor*)` : `AHOLGameState::RequestPause(const AHOLPlayerState&, double Now)` (Now = `GetServerWorldTimeSeconds()`), methodes nues.
- `HOLRpcBridge::Surrender(AActor*)` : `AHOLGameState::DefeatSlot(Slot, EHOLDefeatReason::Surrender)` (`EHOLDefeatReason` est `UENUM()` sans BlueprintType) ; le pont doit garder la ligne "Victory: surrender of slot N refused".
- Routage de l'interface : `RequestDuelChallenge`, `RequestDuelAnswer`, `RequestPause`, `RequestResume`, `RequestSurrender` (`HOLPlayerController.cpp` 5058, 5158, 5166, 6343, 6349) appellent l'RPC C++ directement : sans C++ je n'ai pas pu ajouter l'aiguillage par cvar (comme `HOLRpc::RequestStance`). A faire dans le meme lot C++ (ou un helper script `HOLRpc::RequestDuelChallenge(Controller, Target)` appele par les panneaux script).
- Types de parametre de RPC (T0) : inconnu tant que le module n'a pas compile.

## Commande rouge / vert pour la session principale
Rouge : dans `Script\Net\HOLRpcPlayer.as`, remplacer le corps de `DuelChallengeRefusal`, `DuelAnswerRefusal`, `MatchRefusal` par `return 0;` ; `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Tests\quick_check.ps1 -Parts tests` : doivent echouer Test_RpcDuelChallengeRefused, Test_RpcDuelAnswer, Test_RpcMatchRefusal. Vert : `git -C C:\Users\baudo\Documents\HeroesOfLiberty-Sauron checkout -- Script/Net/HOLRpcPlayer.as` puis meme commande. Premier controle a faire : que le module compile (T0 : si une ligne de `HOLRpcTypes_Test.as` est refusee, c'est un type a exposer ; commenter la ligne et relancer).
Preuve reseau (apres le pont C++) : meme modele que `rpc_diplomacy_run.ps1`, avec `hol.Rpc.ScriptDuel 1` / `hol.Rpc.ScriptMatch 1`.

## NON VERIFIE
Compilation du script (aucun editeur) : syntaxe `f"..."` avec variable locale, `AHOLRpcTypesProbe::StaticClass()`, `const FString Answer = bAccept ? "accept" : "refuse"`, namespace `HOLRpc::` ajoute avant `RequestStance`. Aucun type de T0. Aucun test lance, aucun rouge vu a l'execution. Coherence "spectateur refuse" pour pause/resume : en C++ la regle de la partie decide (un joueur encore en jeu) ; ici le script refuse en plus tout emetteur sans slot (regle demandee par la tache).

PROCESSUS_DETACHES: aucun
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md`, ` M Auto_Journey_Jour.md` (pas de moi), ` M EtapeDefini.md`, `?? Docs/Priorite_Socle_Factions_2026-11-16.md` ; inchange hors docs voulus : oui (mes docs : EtapeDefini.md, Auto_Journey_File.md ligne MIG2-S3)
