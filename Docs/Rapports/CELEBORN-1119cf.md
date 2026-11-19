# CELEBORN-1119cf : port script de Victory.GameState (HOLVictoryTests.cpp)

NON COMPILE. Aucun build, aucun quick_check, aucun editeur. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `test-world-8` (base integ-19 ae64d0e). Commit `e1d4f63` (3 fichiers, +~190), pas de merge main, pas de push.

APPELS_DECLARES: ~11 (arret 17:49 ou 25 appels)

## Tests de HOLVictoryTests.cpp (Grep, 15 tests)
Sans naissance Mass : Victory.GameState (l.112-149, seul sans monde de Mass ni composant). Avec RequestUnits/ServeBirthRequests : PlaceUnit (aide l.68), Collapse, Forum.Fall?, Forum.Builders et ceux qui appellent PlaceUnit/RequestUnits (l.192-214, 293, 818-824). Les autres Forum.* (Hold.Chain, Hold.Presence, PresenceTeams, Block, Hold.Display, Look, Appearance, PlayersTable) lisent unites/composants/apparence : non evalues un a un (temps), laisses en C++. Choix : GameState, le plus simple (12 assertions, aucune dependance Mass ni composant).

## Test script : `Script\Tests\HOLVictoryWorld_Test.as` (neuf), `Test_VictoryGameState`
Memes entrees que le C++ : config Human + AI, equipes 0, autres slots fermes (= SpawnGameState(World,1,1), qui appelle InitVictory sur MakeDefault dont Team = 0). Memes 13 verifications : slot ferme (4) ne perd pas ; pas fini au lancement ; slot 1 abandonne (vrai) ; slot 1 defait ; raison Surrender ; slot 0 non defait ; fini ; gagnants == {0} (Num==1 puis [0]==0) ; IsWinner(0) && !IsWinner(1) ; plus personne ne perd apres la fin ; jamais deux fois ; 1 defaite annoncee ; 1 fin annoncee. Ajout : Num()==1 explicite avant [0].
Non porte : rien de ce test. Tests laisses en C++ : Collapse (RequestUnits Mass + ServeBirthRequests, GetUnitView), Forum.Fall / Builders (naissances), le reste Forum.* (unites, composants, apparence, journal) ; les Victory.Rules sont deja dans HOLVictory_Test.as. AUCUN test C++ retire.

## Enveloppes C++ ajoutees (liaison, D839 ; HOLTestWorldLibrary.h/.cpp ; AHOLGameState n'a aucune de ces methodes en UFUNCTION)
- Classe `UHOLVictoryEventCounter` (UObject, UCLASS) : `Bind(State)` (C++), `GetDefeats()`, `GetEnds()` (UFUNCTION) ; compte OnSlotDefeated / OnMatchOver (delegues C++ simples, non exposables autrement).
- `DefeatGameStateSlot(State, Slot, EHOLDefeatReason)` -> DefeatSlot ; `IsGameStateSlotDefeated` ; `GetGameStateDefeatReason` ; `IsGameStateMatchOver` ; `GetGameStateWinners` (TArray<int32>) ; `IsGameStateWinner` ; `CountVictoryEvents(State)`.
- Includes ajoutes : header `Match/HOLGameState.h` (EHOLDefeatReason en parametre d'UFUNCTION) ; cpp `UObject/Package.h` (hors bloc WITH_DEV : GetTransientPackage). Les corps ne sont PAS sous WITH_DEV (ils n'ont besoin d'aucun composant). CRLF conserve.

## FALSIFICATION a jouer (apres build du lot, hot reload du script)
`Script\Tests\HOLVictoryWorld_Test.as` ligne 54 : `T.AssertEquals(1, Defeats, "one defeat told");` -> `T.AssertEquals(2, Defeats, "one defeat told");` : attendu rouge `Test_VictoryGameState` seul (le jeu annonce 1 defaite). Variante plus forte (regle du jeu) : ligne 27 `HOLTestWorld::DefeatGameStateSlot(State, 4, EHOLDefeatReason::Cheat)` -> `(State, 1, ...)` : rouge ("a closed slot cannot lose" ; slot 1 defait trop tot). Remettre puis revoir le vert avant tout retrait du test C++ `Victory.GameState` (HOLVictoryTests.cpp l.112-149, lot L2 ailleurs).

## Commande pour la session principale
`C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` (C++ de liaison change), puis `quick_check -Parts tests` : attendu +1 test (Test_VictoryGameState), total C++ inchange.

## Champs migration
BUILDS_DEMANDES: 1 (liaison). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 1 (Test_VictoryGameState). LIGNES_CPP_SUPPRIMEES: 0. LIGNES_MIGREES_PAR_LOT: ~38 (C++ 112-149). TYPES_A_EXPOSER: aucun de plus pour ce test ; pour Collapse : naissance Mass, GetUnitView, populations. TESTS_TAUTOLOGIQUES: 0 (les valeurs viennent de la vraie AHOLGameState). EDITS_REFUSES: 0. PROCESSUS_DETACHES: aucun lance.

## NON VERIFIE
Tout : compilation C++ (UHT : `EHOLDefeatReason` param/retour d'UFUNCTION statique, UCLASS compteur dans le header de bibliotheque, `AddUObject` sur FOnSlotDefeated/FOnMatchOver, parametres inutilises de HandleDefeated), binding script (nom de classe `UHOLVictoryEventCounter`, `TArray<int>` retourne depuis `TArray<int32>`, comparaison `Reason == EHOLDefeatReason::Surrender`, appel de `Counter.GetDefeats()` assigne a une variable locale comme prescrit par le brief). Hypotheses : Team par defaut 0 dans FHOLMatchSlot (`int32 Team = 0`, vu) ; SpawnGameState ferme les slots non occupes (vu). Le compteur n'est gardien que par la reference script (GC improbable pendant le test). Les autres Forum.* n'ont pas ete classes un par un.
## Anomalies
`IN_USE.txt` non suivi par git (non commite). Fichiers non suivis `Tools\Scenarios\c28_*`, `c31_*` non touches. La liste 'Forum.Fall?' ci-dessus : Forum.Fall contient des naissances (l.305+, RequestUnits l.293 appartient a Collapse) non verifie ligne a ligne.