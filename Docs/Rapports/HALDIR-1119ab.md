# HALDIR-1119ab : retrait du repli C++ des 16 ordres du joueur - NON COMPILE

Impact joueur : aucun attendu (memes ordres, memes refus, memes journaux, `bChain` toujours porte par les emetteurs `Issue*Player`) ; seul changement : sans planche script, l'ordre est REFUSE et une ligne d'erreur le dit, au lieu de tourner sur la copie C++.
Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `orders-nofallback` (issue de `net-playtest`). Aucun build, aucun quick_check, aucun jeu. EtapeDefini.md et Auto_Journey_File.md non touches. Rien pousse, pas de merge.

## Commits
- `ccbe38a` P1 : IsAccepted, Move, Attack, AttackBuilding, AttackMove, Stop, ToggleDeploy.
- `cb28d0f` P2 : Gather, Build, DropOff, Place, QueueProduction, MarketTrade, SetRally, Garrison, UngarrisonAll (+ commentaires de `HOLPlayerOrdersBoard.h/.cpp` mis a jour).

## Ordres dont le repli est retire : 16 sur 16
Aucun ordre garde : pour chacun, le corps de repli ne faisait rien de plus que la regle script (compare ligne a ligne a `Script\Commands\HOLPlayerOrders.as`). Points verifies : les refus (slot, 0..200 unites, batiment nul, `TakesGarrisonOrder`, `OwnsBuilding`) et le choix de l'emetteur sont tous dans la regle script ; `Place` (rule IsAccepted(0) + builders vides au-dela de 200), `QueueProduction`/`MarketTrade`/`SetRally` (IsAccepted(0) + batiment non nul) identiques.

## Ce qui reste en C++ (effets / faits du monde)
- `ToBridgeIds`, `SendNonWorkersTo` (HOLUnitView), `bSource` + `SourceLocation` (UHOLSourceSubsystem), `bDeposit` (type de batiment), `bSourceStanding`, la ligne de journal "Drop-off order of slot..." et "Order of slot ... do not work walk to ..." : inchanges.
- Gather / Build / DropOff : `World.GetSubsystem<UHOLCommandSubsystem>()` (non nul, pour ouvrir `FPlayerOrderScope` avant `SendNonWorkersTo`, comme avant) ; le portillon qui precedait la marche des soldats (slot + nombre d'unites) est maintenant `UHOLPlayerOrdersBoard::AskIsAccepted` (donc la regle script, plus `GetQueue`/`IsAcceptedCpp`). Build : si `Building` est nul, la marche des soldats est sautee et la planche refuse (avant : refus C++ avant la marche) ; meme resultat.
- Nouveau (anonyme, `HOLPlayerOrders.cpp`) : `Verdict(World, Ordre, Reponse)` (-1 => `false` + `LogBoardAbsent`) et `LogBoardAbsent` : `UE_LOG(LogHOLMatch, Error, "PlayerOrders: %s refused, script board absent")`, une fois par couple (fonction, monde) via `TMap<FName, FObjectKey>` sous `FCriticalSection` (IsAccepted : monde nul, donc une fois par processus ; une nouvelle partie = nouveau monde = une nouvelle ligne).
- Supprimes : `IsAcceptedCpp`, `GetQueue`, les 16 corps de repli.

## Chiffres
- LIGNES_CPP_SUPPRIMEES: 257 lignes supprimees (253 `HOLPlayerOrders.cpp` + 1 `HOLPlayerOrdersBoard.cpp` + 3 `HOLPlayerOrdersBoard.h` ; `git diff --numstat net-playtest..HEAD`: +65 / -257, net -192 ; dont corps de repli seul : 253 supprimees, 62 ajoutees dans `HOLPlayerOrders.cpp`, helpers inclus).
- BUILDS_DEMANDES: 1 (C++ touche : HOLPlayerOrders.cpp, commentaires de la planche) a lancer par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Haldir.
- TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0 (voir ci-dessous). TEST_ROUGE: non vu (aucun build autorise). TESTS_TAUTOLOGIQUES: 0. TYPES_A_EXPOSER: inchange (FHOLUnitView + HOLUnitView::Find, FHOLBuildingTypeRow::Accepts / TakesDeliveries / GetType, UHOLSourceSubsystem::GetSources).
- LIGNES_MIGREES_PAR_LOT: 0 (retrait de doublon). PROCESSUS_DETACHES: non lance (aucun processus demarre). EDITS_REFUSES: 0 (3 essais de script de decoupe corriges avant commit : motif `\t}\n` trouve dans `\t\t}\n`, repare ; rien n'est dans l'historique).

## A signaler (tests C++ qui comparaient au repli supprime)
1. `Source\HeroesOfLiberty\Tests\HOLAIOrdersTests.cpp:159-163` : appelle `HOLPlayerOrders::IsAccepted(...)` SANS monde de jeu. Avant : repli C++. Maintenant : `AskIsAcceptedAnywhere` cherche une planche dans les mondes du moteur ; sans elle les 3 `TestTrue` (159-161) tombent. Equivalent script deja present (`HOLPlayerOrders_Test.as`, `HOLPlayerOrdersBoard_Test.as`). A traiter par la session principale apres le build : soit donner un monde avec planche au test, soit le retirer au profit du test script (rouge puis vert). Non supprime.
2. Les autres tests C++ d'ordres (GarrisonOrder, Gather, AISlot, AIOrders, StartingHero, DeployButton, DiplomacyBranch) passent un monde : ils tournent par la planche comme avant ; ils exigent la planche compilee dans le monde de test (deja le cas de `net-playtest`, 1760/1760).
3. Pas de test script « planche absente = refus » : le -1 vient de la liaison C++ (classe script absente), aucune fonction pure script ne s'y prete. Verification possible a la main : lancer sans la DLL script et lire "PlayerOrders: <Ordre> refused, script board absent" une seule fois par ordre.

## Risques
- Hors fil de jeu, `UHOLPlayerOrdersBoard::Find` rend null (IsInGameThread) : l'ordre est maintenant refuse (avant : repli C++ execute). Appelants relus (rapport P2) : tous sur le fil de jeu (RPC controleur, IA `HOLAIOrders.cpp`, `HOLVolcanoSubsystem.cpp:395` Move). A confirmer au journal : aucune ligne "script board absent" pendant `-Parts aiserver,netai`.
- Compilation : `FObjectKey` (`UObject/ObjectKey.h` inclus), `FCriticalSection`/`FScopeLock` (HAL, inclus par CoreMinimal) ; `Verdict` prend `const UWorld*` ; `&World` (UWorld*) convertit. `FHOLBuildingTypeRow`, `HOLCommandSubsystem` toujours inclus.

## Preuves a rejouer (apres build)
1. `Tools\Tests\quick_check.ps1 -Parts tests` : attendu 1760/1760 sauf `HOLAIOrdersTests` (IsAccepted sans monde) a corriger comme ci-dessus.
2. `Tools\Scenarios\rpc_orders_p1p2_run.ps1` et `rpc_orders_p3p4_run.ps1` : VERDICT PASS attendu.
3. `-Parts net,aiserver,netai` ; journal serveur : ligne "Drop-off order of slot" identique, AUCUNE ligne "script board absent".
4. Rouge de chaine : fausser `Answer()` de `HOLPlayerOrdersBoard.as` (`bDone ? 0 : 1`) : les tests C++ d'ordres tombent comme avant (la regle reste la seule source).
5. Placeholders.txt : aucun provisoire ajoute ni retire.
