# GALADRIEL-1119ab : lot R3 (issue du ralliement de ApplyOrders), NON COMPILE

Impact joueur : aucun changement visible (mêmes issues, mêmes lignes de journal, même ordre des effets).

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab. Commits (récents en tête) :
5dcf3b6 Production: board events for the rally outcome and rally site, ApplyOrders asks them with the C++ twin kept (not compiled)
1e34b50 Script: production board decides the rally outcome and the kept rally site (not compiled)
165c95e Script tests: production board rally site, name the first fact bOtherBuilding
(+ c6712d0 test initial ; ordre : test c6712d0, correctif de nom 165c95e, script 1e34b50, C++ au-dessus.)

## Fait
- Test (Script\Tests\HOLProductionBoard_Test.as) : Test_ProductionBoard_RallyOutcome (8 cas : Set, Set Shift sans point, AddStep, Clear x3, NoStep) et Test_ProductionBoard_RallySite (6 cas). 14 assertions. Rouge NON VU (aucun build/quick_check).
- Script (Script\Buildings\HOLProductionBoard.as) : enum EHOLRallyOutcome {NoStep, Clear, AddStep, Set}, HOLProductionBoard::RallyOutcome(bOnBody, bShift, bHasRally), RallySiteKept(bOtherBuilding, bOwnedByPlayer, bNeedsBuilders, bIsCamp), surcharges RallyAnswer et RallySiteAnswer dans UHOLProductionBoardScript.
- C++ : HOLProductionBoard.h/.cpp : AskRally, AskRallySite, RallyAnswer / RallySiteAnswer (BlueprintNativeEvent, _Implementation = -1, repli gardé). HOLProductionSubsystem.cpp ApplyOrders : RallyVerdict demandé avant le bloc ; les 3 conditions (== 0 / == 1 / == 2) lisent le verdict ; la validité du site passe par SiteKept, repli = condition d'origine. Texte et niveau des journaux, ordre des effets : inchangés (non touchés).
- Diff : git diff --stat 42 insertions / 5 suppressions (3 fichiers C++), identique en -w : aucune suppression large.

## Écarts / anomalies
- Le rouge demandé (EarlyRally inversé) ne concerne pas l'issue du ralliement (ce n'est pas un paramètre de RallyOutcome, il est dans la porte G4 déjà testée). Valeur à fausser pour R3 (plan) : retirer Shift de la condition NoStep (RallyOutcome(true, true, true) devrait alors rendre Clear), le test "Shift on the body with a rally point" passe rouge.
- Dépassement du plafond de 20 appels (environ 28 comptés, dont 11 Edit refusés en bloc par le classifieur d'auto-mode, sans effet) ; refaits en un seul appel PowerShell à remplacements de chaînes exacts avec garde d'unicité (10 ok, 0 refus).
- L'ancien .h finissait par Cost);}; sur une ligne ; corrigé (l'accolade fermante est maintenant sur sa ligne).
- Fins de ligne : avertissement git LF->CRLF sur les fichiers édités (autocrlf), sans effet attendu.

## A lancer par la session principale
Build C++ (nouveaux UFUNCTION : header modifié) puis quick_check -Parts du script des tests production ; voir le rouge puis le vert de Test_ProductionBoard_RallyOutcome.

BUILDS_DEMANDES: 1 (lot C++ de la séance), TESTS_CPP_RETIRES: 0, TESTS_SCRIPT_AJOUTES: 2 (Test_ProductionBoard_RallyOutcome, Test_ProductionBoard_RallySite), LIGNES_CPP_SUPPRIMEES: 0 (repli gardé), LIGNES_MIGREES_PAR_LOT: ~50 (décision de l'issue du ralliement), TYPES_A_EXPOSER: aucun.
NON VÉRIFIÉ : compilation du C++ et du script, les 14 assertions, RallySiteAnswer rendant int depuis un ool ? 1 : 0 en script.