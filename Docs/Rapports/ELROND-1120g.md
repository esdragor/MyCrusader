# ELROND-1120g : garnison UI, regle pure HOLGarrisonView (F7 lot 4a, D813 Q-garrison-ui Q1 = C)

NON COMPILE (script seul, aucun build, aucun quick_check : le rouge n'a PAS ete vu a l'execution).

APPELS_DECLARES: ~14 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 11:24:59 / 11:3x
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7f9dc0a (test + bouchon) ; assertit l'effet : oui (compteur, portraits, case, ordre calcules par la regle) ; rouge NON VU (pas de quick_check dans l'agent)
COMMITS: 2 (7f9dc0a test + bouchon, 0a55fab correctif), dernier 11:26, merge_main_fait: oui (main ff668a9, avance rapide, premier geste)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune (Q-garrison-ui deja repondue : Q1 = C)
BUILDS_DEMANDES: 0 pour cette sous-etape ; commande a lancer par la session principale (script) : quick_check -Parts tests (ou la partie script) sur le worktree Elrond ; attendu : Test_GarrisonView* (4) verts apres 0a55fab, rouges sur 7f9dc0a.
TESTS_SCRIPT_AJOUTES: 4 (Test_GarrisonViewCounterAndPortraits, Test_GarrisonViewEmptyHidesUngarrisonAll, Test_GarrisonViewPortraitClickOrdersThatUnit, Test_GarrisonViewUngarrisonAllKey) dans Script\Garrison\HOLGarrisonView_Test.as
TYPES_A_EXPOSER: aucun pour la regle ; pour la liaison a venir (voir ci-dessous) : une lecture client des occupants (TArray<int32> d'identifiants d'unite) et un ordre « cette unite sort » (UHOLScriptRpcPlayerActor).

## Ce qui existe (lu)
- Serveur, ordre « sortir tous » : HOLPlayerOrders::UngarrisonAll (HOLPlayerOrders.h:92), HOLPlayerOrdersBoard::AskUngarrisonAll (HOLPlayerOrdersBoard.h:49) / UngarrisonAllAnswer (:103), UHOLCommandSubsystem::IssueUngarrisonAllOrder (HOLCommandSubsystem.h:250), UHOLScriptRpcPlayerActor::OrderUngarrisonAll (HOLScriptRpcPlayerActor.h:230, RPC du client), AHOLPlayerController::RequestUngarrisonAll (HOLPlayerController.h:381, « pour le bouton a venir », Q-garrison-ui Q1), console hol.Units.UngarrisonAll (Script\Player\HOLControllerCommandsC9a.as:256-273).
- Entree : RequestGarrisonUnits (HOLPlayerController.h:379), regle d'admission en script (Script\Garrison\HOLGarrisonRules.as, HOLGarrisonBoardScript.as).
- Places : GarrisonSlots par type (UHOLGarrisonSubsystem, HOLGarrisonSubsystem.h:32).

## Ce qui manque
1. Ordre « CETTE unite sort » : aucun IssueUngarrisonUnit / Ungarrison(UnitId) (seulement UngarrisonAll). C++ (file de commandes serveur, RPC).
2. Lecture des occupants par le client : aucun accesseur client (UHOLGarrisonSubsystem applique les ordres cote serveur, « Nothing on a client », HOLGarrisonSubsystem.h:25). A confirmer : les occupants en garnison sont-ils replicables (HOLNetUnitStateNetSerializer.cpp, HOLUnitReplicationRules.h mentionnent Garrison) ; sinon un compteur et la liste d'identifiants repliques par batiment.
3. Widget : cadre du batiment (portraits cliquables + compteur) et case de grille « Sortir tous » avec touche ; Script\UI\HOLCommandGridPanel.as (grille) et HOLGameScreenScript.as (cadre) sont les points d'accroche ; aucune ligne « Garrison » dans Source\HeroesOfLibertyUI hors le libelle de technologie.

## A ecrire ensuite (decrit, non ecrit)
- C++ (lot groupe, raison : liaison a exposer + RPC) : UHOLCommandSubsystem::IssueUngarrisonUnitOrder(PlayerIndex, Building, UnitId) ; HOLPlayerOrders::UngarrisonUnit ; UHOLScriptRpcPlayerActor::OrderUngarrisonUnit(AActor*, Building, int32 UnitId) ; AHOLPlayerController::RequestUngarrisonUnit ; lecture client : UHOLGarrisonReadLibrary::OccupantsOf(Building) -> TArray<int32>.
- Script : le widget lit OccupantsOf + GarrisonSlots, appelle HOLGarrisonView::Build, dessine View.CounterText, un portrait par View.PortraitUnitIds (clic -> OrderForPortraitClick), la case « Sortir tous » si View.bShowUngarrisonAll (touche -> OrderForUngarrisonAll), envoie Kind/UnitId au controleur.

## Divers
- Noms : types a l'echelle du fichier FHOLGarrisonView / FHOLGarrisonOrder ; fonctions dans namespace HOLGarrisonView (fichier Script\Garrison\, pas un dossier saute par le plugin). Identifiants d'unite en int script (uint32 cote C++ : meme largeur, a surveiller au-dela de 2^31).
- Fichiers de suivi : FortificationsInProgress.md (ligne 227), Auto_Journey_File.md (ligne 72, [~]), EtapeDefini.md (apres ELROND-1120e/f), IN_USE.txt libre. Placeholders.txt : rien d'ajoute (aucun provisoire visible).

FAIT / RESTE : voir ci-dessus.
NON VERIFIE : compilation du script, rouge sur 7f9dc0a, vert sur 0a55fab ; l'absence d'accesseur client des occupants (deduite de greps sur Garrison/Occupant/Ungarrison, pas d'une lecture complete de HOLUnitReplication*).
ANOMALIES_PROCESSUS: aucune