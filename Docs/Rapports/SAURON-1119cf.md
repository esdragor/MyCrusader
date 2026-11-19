# SAURON-1119cf (17:34-17:36), branche ai-orders-world (depuis integ-19 ae64d0e), commit fde3017

TACHE: porter en script les cas "ordres seuls" de HOLAIOrdersTests.cpp (D853 point 4).

## Inventaire des 7 tests C++ (HOLAIOrdersTests.cpp)
| Test | Ligne | Naissance Mass (PlaceUnits = RequestUnits + ServeBirthRequests, l.48-49) | Decision |
|---|---|---|---|
| AI.Orders.Gather | 160 | oui | reste C++ (D854) |
| AI.Orders.Cap | 193 | oui | reste C++ |
| AI.Orders.AttackMove | 265 | oui | reste C++ |
| AI.Orders.AttackBuilding | 301 | oui (+ AHOLBuilding::Setup, RunStep des processeurs) | reste C++ |
| AI.Orders.AttackUnit | 341 | oui | reste C++ |
| AI.Orders.CombatFigures | 379 | NON (lecture de tables) | PORTE |
| AI.Orders.NonWorkers | 432 | oui | reste C++ |
(AI.Orders.Rule deja retire, 667d3ad.)

Les 6 autres tests exigent en plus RunStep des processeurs Mass (UHOLUnitOrderProcessor, UHOLGatherProcessor) et la lecture des fragments Mass (combat, activite) : hors portee des enveloppes actuelles meme sans la naissance. Aucune enveloppe C++ nouvelle n'a ete ecrite.

## Fait
`Script\Tests\HOLAIOrdersWorld_Test.as` : `Test_AIOrdersCombatFigures`, memes entrees et memes valeurs que le C++ (Spearman, ManAtArms, Archer, Crossbowman, Peasant, NoSuchUnit, House) via `HOLAIOrder::ReadAIUnitFigures` / `ReadAIBuildingFigures` (meme appel que `Script\AI\HOLAIFigures_Test.as`, contexte de monde du script). Toutes les assertions du C++ y sont (22).
Ecart : le "test world ready" du C++ (FWorld, qui ne servait que de porteur du UHOLUnitSubsystem) n'est pas reproduit ; le Spearman connu garantit que la table est atteinte. Le test C++ n'est PAS retire.
Le fichier definit un namespace `AIOrdersWorldTest` (IndexOf = TArray::FindIndex).

## Champs
BUILDS_DEMANDES: 0 / TESTS_CPP_RETIRES: 0 / TESTS_SCRIPT_AJOUTES: 1 (Test_AIOrdersCombatFigures) / LIGNES_CPP_SUPPRIMEES: 0 / LIGNES_MIGREES_PAR_LOT: 0 (copie, retrait apres rouge+vert) / TYPES_A_EXPOSER: aucun pour ce cas ; pour porter les 6 autres il faudrait des enveloppes de naissance (RequestUnits + ServeBirthRequests, D854: restent C++), RunStep des processeurs d'ordres et de recolte, lecture de FHOLCombatFragment (Mode, OrderedBuilding, bOrderedTarget, Target) et de l'activite (FHOLUnitView::Activity).

## Falsification a jouer (rouge VU, une ligne)
`Script\Tests\HOLAIOrdersWorld_Test.as:64` : `T.AssertEquals(50, Worker.CostFood, ...)` -> `T.AssertEquals(51, Worker.CostFood, ...)` doit donner rouge (message "Peasant: the cost as before"). Variante : ligne 46 `T.AssertFalse(Spear.bRanged` -> `T.AssertTrue(Spear.bRanged`.
Puis remettre, jouer le vert, et seulement ensuite retirer FHOLAIOrdersCombatFiguresTest du C++ (apres accord de la session principale).

## NON VERIFIE
Compilation du script, execution, et l'API exacte `IsValidIndex` / `FindIndex` sur TArray<FName> (FindIndex est utilise dans HOLAIFigures_Test.as ; IsValidIndex non vu ailleurs : si le compilateur la refuse, remplacer par `VsCavalry < Spear.BonusDamage.Num()`). Aucune execution possible dans l'agent.

## Anomalies
Aucune. `merge main` non fait. IN_USE.txt du worktree : entree `OCCUPE ... CPP: non`, sortie `libre`. Processus detaches : non lances (aucun processus demarre).
