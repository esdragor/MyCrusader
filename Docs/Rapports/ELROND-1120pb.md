# ELROND-1120pb : garnison UI (D813, Q-garrison-ui Q1 = C), cadre du batiment, script d'abord

APPELS_DECLARES: ~20 (arret a 25 vus ; heure d'arret dur 16:01)
HEURE_DEBUT/HEURE_FIN: 15:46 / voir fin de rapport
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 588d136 (stub ViewFor / PortraitName + 3 tests) ; assertit l'effet : oui (compteur, portraits, ordre de clic calcules par HOLGarrisonView) ; rouge NON VU a l'execution (pas de build ni quick_check, par consigne)
COMMITS: 2 (588d136 test seul, d8a35b9 regle + widget), dernier 15:49, merge_main_fait: voir fin
TESTS: aucun lance ; TESTS_SCRIPT_AJOUTES: 3 (Test_GarrisonFrameOwnFinishedBuildingDrawsTheGarrison, Test_GarrisonFrameOtherPlayersOrUnfinishedBuildingDrawsNothing, Test_GarrisonFramePortraitNamesAreDistinct) dans Script\UI\HOLGarrisonFrameRules_Test.as ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (rien d'affiche a l'ecran avant le lot C++)
QUESTIONS: Retours\Q-garrison-portrait.md (defaut C : identifiant d'unite en texte)
BUILDS_DEMANDES: 0 (le lot C++ ci-dessous en demandera un)
TYPES_A_EXPOSER: voir « Lot C++ » ci-dessous
DIAGNOSTICS_AS: non releves (pas d'editeur)

## Fait
- Script\UI\HOLGarrisonFrameRules.as : namespace HOLGarrisonFrameRules : ViewFor(Occupants, Slots, bOwn, bFinished) = HOLGarrisonView::Build pour un batiment a soi, fini, avec places, sinon vue vide (les occupants ne sont repliques qu'au proprietaire) ; PortraitName(Index) = « GarrisonPortrait<Index> ».
- Script\UI\HOLGarrisonFrame.as : UHOLGarrisonFrame (ligne du cadre : compteur + un bouton par occupant, pool qui grandit, handler par bouton comme HOLSelectionChips.as) ; ClickPortrait / ClickUngarrisonAll rangent l'ordre dans LastOrder. Non branche, non compile.
- Script\UI\HOLGarrisonFrameRules_Test.as : 3 tests.

## Ce qui existe deja dans main et se lit en script (verifie)
- AHOLBuilding::GetGarrisonOccupants (UFUNCTION BlueprintCallable, HOLBuilding.h:135, replique au proprietaire), HOLBuildingType::FindRow(TypeKey).GarrisonSlots (HOLBuildingTypeLibrary.h:22 ; places lisibles en script sans C++), HOLControllerCommand::RequestUngarrisonAllOf(Controller, Building) (HOLControllerCommandLibrary.h:185), UHOLScriptRpcPlayerActor::OrderUngarrisonUnit(RpcActor, UnitId) (HOLScriptRpcPlayerActor.h:233, BlueprintCallable), AHOLPlayerController::RequestUngarrisonUnit(uint32) (HOLPlayerController.h:388, PAS une UFUNCTION).

## Lot C++ minimal (a regrouper dans le lot de la seance ; non ecrit ici)
1. FHOLBuildingSelView (Source\HeroesOfLibertyUI\HUD\HOLSelectionReadLibrary.h:16-84) : ajouter `TArray<int32> GarrisonOccupants` (rempli dans GetBuildingSelectionView par Building->GetGarrisonOccupants) : c'est la seule lecture du batiment selectionne cote script, elle n'a ni pointeur de batiment ni occupants. Les places viennent du script (FindRow).
2. HOLControllerCommandLibrary : `RequestUngarrisonUnitOf(AHOLPlayerController*, int32 UnitId)` (une ligne : Controller->RequestUngarrisonUnit) ; et une lecture du batiment selectionne pour `RequestUngarrisonAllOf` (le controleur a GetOwnSelection()) : p. ex. `RequestUngarrisonAllSelected(Controller)`.
3. Case de grille « Sortir tous » + touche : la grille est en C++ (HOLCommandGrid, aucune ligne Garrison aujourd'hui ; GridPanel/HOLCommandGridPanel.as ne fait que dessiner les cases) : cellule a ajouter dans le lot D868 (cases de specialisation) pour ne pas toucher deux fois ce fichier.
4. Portrait : lecture client du type d'une unite par identifiant (Q-garrison-portrait.md) : absente.

## Branchement restant (script, apres le lot C++)
HOLSelection.as RefreshBuilding : creer un UHOLGarrisonFrame dans l'hote vertical du bloc (ChipsHost ou voisin), appeler Refresh(View.GarrisonOccupants, HOLBuildingType::FindRow(View.TypeKey).GarrisonSlots, View.bOwn, View.bFinished) ; envoyer LastOrder au controleur (RequestUngarrisonUnitOf / RequestUngarrisonAllSelected) ; hol.HUD.Click doit connaitre GarrisonPortrait<N>.

## Fichiers de suivi
Auto_Journey_File.md l.81, EtapeDefini.md l.93 (ELROND-1120g), FortificationsInProgress.md : mis a jour. Placeholders.txt : rien d'ajoute (aucun affichage visible avant le branchement ; le texte « identifiant d'unite » est provisoire une fois branche : ligne a ajouter alors).

NON VERIFIE : compilation du script (types UHorizontalBox / UButton / UTextBlock, SetContent, ConstructWidget, FName(f"...") copies du modele HOLSelectionChips.as sans etre compiles), rouge de 588d136, vert de d8a35b9, diagnostics AS.
ANOMALIES_PROCESSUS: aucune (IN_USE.txt est ignore par git, non commite).