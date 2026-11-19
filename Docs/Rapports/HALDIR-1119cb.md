# HALDIR-1119cb : retrait des replis C++ des 4 points S2 (HOLGameState.cpp)

NON COMPILÉ. Aucun build, aucun quick_check lancé (règle de séance a).

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `s2-nofallback`, depuis 28dfc85.
Commit : **6b549e4** `Match flow: remove the C++ fallbacks of the four S2 points in AHOLGameState ...` (1 fichier : `Source\HeroesOfLiberty\Match\HOLGameState.cpp`, +62 / -61). `Script\Match\HOLMatchFlow_Test.as` NON modifié (voir anomalies).
Heure début 16:56, fin ~16:59 ; ~13 appels d'outils vus.

## Retiré, par point (HOLGameState.cpp)
Modèle suivi : 9083320 (Production, `LogBoardAbsentOnce` + `continue`). Ici une fonction `LogMatchFlowBoardAbsentOnce(const TCHAR*)` dans un `namespace` anonyme juste avant `HandleForumFall` (ligne ~385), `static TSet<FString>`, une ligne par action et par processus.

- `DefeatSlot` (porte) : la chaîne `|| ... GetCamp(...) == INDEX_NONE` (6 lignes, ternaire `GateAnswer >= 0 ? ... : (...)`) est remplacée par `GateAnswer < 0` -> journal + `return false`, puis `GateAnswer == 0` -> `return false`.
- `HandleForumFall` (porte) : idem (ternaire de 4 lignes retiré).
- `HandleKingDeath` (porte) : idem (ternaire de 4 lignes retiré).
- `EvaluateMatchEnd` (verdict) : retirés `FHOLVictoryResult Result = HOLVictoryRules::Evaluate(...)` calculé d'avance, le premier bloc `Gone/Allied` (D373, ~19 lignes) qui remplissait `Result`, et le `UE_LOG(LogHOLMatch, Warning, "the script verdict (%d) differs from the C++ rule ...")` avec sa comparaison. Reste UN SEUL bloc `Gone/Allied` (celui qui calcule `AlliedMask` transmis au script), puis `Verdict = AskMatchEnd(...)` ; `Verdict < 0` -> journal + `return false` ; sinon `Result.bOver = Verdict < MatchNotOver`, `Result.WinnerMask = Result.bOver ? Verdict : 0` (struct `FHOLVictoryResult` gardé comme porteur pour `GetWinners()` et le journal `Victory: match over`).
- Les commentaires « kept until the script gate is proven ... Agreement » des 3 portes sont réécrits (« the script board's alone, proven by Test_MatchFlow... »).

## Gardé et pourquoi
- `HOLVictoryRules::Evaluate` (C++) : intact, toujours lu par le script via `HOLVictoryRead::Evaluate` (point 2). Seule la DUPLICATION dans le GameState (calcul d'avance, second bloc Gone/Allied, Warning d'écart) disparaît ; la règle des camps reste une seule source.
- `HOLVictoryRules::GetCamp` / `IsSlotDefeated` etc. : faits lus, passés en paramètres à la planche.
- HORS PÉRIMÈTRE, laissé : `HOLGameState.cpp` ~ ligne 1704-1705 (`AskEvaluateMatchEnd`, `EvaluateEnd >= 0 ? EvaluateEnd == 1 : Change.bFormed`) : repli du lot S1 (diplomatie), pas un des 4 points demandés. À retirer par le même geste dans un lot suivant.

## Point 1 : mondes de test sans planche
Réponse : AUCUN monde de test C++ n'est sans planche, donc le retrait des 4 points est sûr.
- `UHOLMatchFlowSubsystem` (`Source\HeroesOfLiberty\Match\HOLMatchFlowSubsystem.h:17-18`, `UCLASS(Abstract)`, `UWorldSubsystem`) : `ShouldCreateSubsystem` (`HOLMatchFlowSubsystem.cpp`, refuse seulement `CLASS_Abstract | CLASS_NewerVersionExists`) et `DoesSupportWorldType` (`WorldType == Game || PIE`, juste en dessous). UE instancie donc automatiquement la sous-classe script `UHOLMatchFlowBoardScript` (`Script\Match\HOLMatchFlowBoard.as:166`) dans tout monde `Game` ou `PIE` dès que le script est compilé ; `Find(World)` (`HOLMatchFlowSubsystem.cpp:13-14`) prend `GetSubsystemArrayCopy<...>()[0]`.
- Mondes des tests : sur 145 `UWorld::CreateWorld` du dossier `Source\HeroesOfLiberty\Tests\`, 144 sont `EWorldType::Game` (planche née) ; le seul `EWorldType::Inactive` est `HOLCampBridgeTests.cpp:80`, qui n'appelle aucun des 4 points (grep : aucune occurrence de `DefeatSlot|HandleForumFall|HandleKingDeath|EvaluateMatchEnd` dans ce fichier).
- Tests C++ qui appellent les 4 points directement (tous en monde `Game`, `CreateWorld` en tête de test) :
  - `DefeatSlot` : `HOLVictoryTests.cpp:131,133,140,141,217` (monde l.117) ; `HOLAIArmyTests.cpp:270` (monde l.186) ; `HOLDiplomacyAlliesTests.cpp:375` (l.106) ; `HOLDiplomacyBranchTests.cpp:153,163,164,170` (l.54) ; `HOLHeroReturnTests.cpp:305,812` (l.78) ; `HOLSignalSpectatorTests.cpp:66,67` (l.30) ; `HOLGameStateTestLibrary.cpp:32-35` (pont appelé par les tests script, monde `Game` de `HOLTestWorldLibrary`).
  - `HandleForumFall` : `HOLAIForumTests.cpp:136` (l.106) ; `HOLHeroReturnTests.cpp:289,605,799,960`.
  - `HandleKingDeath` : `HOLRegicideTests.cpp:225,229,236,243,247` (monde l.196).
  - `EvaluateMatchEnd` : appelé seulement depuis `DefeatSlot` (`HOLGameState.cpp:703`) et la diplomatie (`:1705`) ; donc couvert par `Victory.*`, `Diplomacy*`.
  - Via le code de production : Disconnect (`HOLGameState.cpp:1199`), `HOLScriptRpcPlayerActor.cpp:95` (Surrender), `HOLPlayerController.cpp:3592` (cheat), `HOLSiteSubsystem.cpp:428` (SacredSites), `HOLCombatProcessor.cpp:652` (roi), `HOLConstructionSubsystem.cpp:581` (forum).
- Les preuves déjà vues (falsification de `DefeatGateOpen` fait tomber `HeroesOfLiberty.Victory.GameState`) confirment que la planche naît dans ces mondes.
- Risque résiduel : un monde `Game` où le script n'est PAS compilé (erreur de compilation du `.as`) : toutes ces actions sont refusées, journal `Error`, et les tests ci-dessus tombent rouges (comportement voulu, même précédent que `Pause: refused ... no script match flow board`).

## Comportement sans planche
Ligne de journal exacte (une fois par action et par processus, catégorie `LogHOLMatch`, niveau Error) :
`Match flow: <Action> refused, no script match flow board (HOLMatchFlowBoard.as not compiled)`
avec `<Action>` = `DefeatSlot` | `HandleForumFall` | `HandleKingDeath` | `EvaluateMatchEnd`. Valeur de retour : `false` (aucune défaite, aucune chute, aucun roi mort, aucune fin de partie).

## Preuves à rejouer (session principale)
1. Build C++ (HOLGameState.cpp seul) : `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Haldir`.
2. `quick_check -Parts tests` (au moins : `HeroesOfLiberty.Victory.*`, `Regicide.*`, `Diplomacy*`, `HeroReturn*`, `AIForum*`, `AIArmy*`, `SignalSpectator*`, scripts `Test_MatchFlow*`) : attendu 1779/1779 inchangé (aucun test retiré ni ajouté).
3. Parties : net, victory, salon, disconnect (la défaite `Disconnected` passe par `DefeatSlot`), un regicide (`hol.Cheat` ou partie) : même journal qu'avant, et AUCUNE ligne `Match flow: ... refused`.
4. Recherche dans les journaux : plus aucune ligne `Victory: the script verdict (%d) differs` (le Warning n'existe plus, c'est normal).
5. Falsification à refaire : `DefeatGateOpen` faussée doit encore faire tomber `Victory.GameState` + `Test_MatchFlowDefeatGate` ; `MatchEndVerdict` faussée doit faire tomber `Victory.GameState` (avant : seul le Warning le disait, la règle C++ n'était pas choisie au-dessus du script) ; `FallGateOpen`/`KingGateOpen` faussées doivent faire tomber `Regicide.*` / `HeroReturn*`. Test négatif de planche absente (manuel, optionnel) : renommer un instant `HOLMatchFlowBoard.as` doit donner les 4 lignes d'erreur et des tests rouges.

## NON VÉRIFIÉ
- Compilation (aucun build) : `TSet<FString>` supposé disponible via les includes existants de HOLGameState.cpp (CoreMinimal), `LogHOLMatch` déjà utilisé avant le namespace anonyme ; `FHOLVictoryResult Result;` construit par défaut (struct à valeurs par défaut vues dans HOLVictoryRules.h).
- Tous les tests : aucun lancé. Équivalence de `EvaluateMatchEnd` : le résultat est celui du script, inchangé pour les mondes avec planche (le Warning prouvait l'accord ; l'accord 256 masques est couvert par `Test_MatchFlowMatchEndVerdictAgreement`).
- Aucun monde de test sans planche : prouvé par lecture (types de monde), pas à l'exécution.

## Anomalies
- Commentaires de test périmés dans `Script\Match\HOLMatchFlow_Test.as` (lignes ~177 et ~254 : « two copies of the gate while the C++ inline refusal stays », et vers 273 pour `KingGateAgreement`) : mon remplacement n'a rien trouvé (texte différent à l'octet près, non diagnostiqué faute d'appels), fichier NON modifié. Les tests `*GateAgreement` restent valables : ils comparent à une chaîne `||` ÉCRITE DANS LE TEST, pas au C++ ; seul le commentaire est faux, et le nom « Agreement » reste un oracle de régression. À corriger par la session principale si voulu.
- Repli S1 restant ligne ~1705 (`AskEvaluateMatchEnd` ... `: Change.bFormed`), hors périmètre.
- `Content\Maps\Crown.umap` non suivi, non touché.
- Aucun `*InProgress.md`, `EtapeDefini.md`, `Auto_Journey_File.md`, `IN_USE.txt` édité (règle b). Pas de merge main, pas de push.

APPELS_DECLARES: 13 ; BUILDS_DEMANDES: 1 (HOLGameState.cpp) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~61 brutes (net +1 avec le helper) ; TYPES_A_EXPOSER: aucun ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: aucun lancé.
