# GLORFINDEL-1120pc : lot C++ garnison (D813), NON COMPILE

APPELS_DECLARES: ~35 vus (DEPASSE le seuil de 25 : tache elargie en cours de route par la session principale, point 4 ajoute ; arret par l'heure 16:07 respecte)
HEURE_DEBUT/HEURE_FIN: 15:51 / 15:55
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: 14b8786 (en-tetes + bouchons + test C++ HeroesOfLiberty.Garrison.SelectionView) ; assertit l'effet : oui (ids d'occupants et types calcules par la vraie chaine : ordre de garnison, pas du step, liste du batiment, HOLUnitView) ; rouge NON VU (pas de build)
COMMITS: 2 (14b8786 test seul, 65ebacd corps), dernier 15:54, merge_main_fait: oui (avant le rapport, sans conflit visible)
TESTS: aucun lance ; TESTS_SCRIPT_AJOUTES: 0 ; test C++ ajoute : HeroesOfLiberty.Garrison.SelectionView (Source\HeroesOfLibertyUI\Tests\HOLGarrisonSelectionTests.cpp) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune neuve (Q-garrison-portrait repondue A par l'utilisateur ; Retours\Q-garrison-portrait.md archive par main)
BUILDS_DEMANDES: 1 (lot C++ de la session)
TYPES_A_EXPOSER: voir ci-dessous
DIAGNOSTICS_AS: sans objet (aucun .as touche)

## Fait (points 1, 2, 4 ; PAS le point 3)
En-tetes touches (un seul commit, 14b8786) : Source\HeroesOfLibertyUI\HUD\HOLSelectionReadLibrary.h, Source\HeroesOfLiberty\Player\HOLControllerCommandLibrary.h (+ forward declaration `class AHOLBuilding;` dans le premier). Recompilation large : HOLControllerCommandLibrary.h est inclus par le module principal.
Corps (65ebacd) : HOLSelectionReadLibrary.cpp (include HOLUnitView.h, GetUnitTypeKey, ReadGarrisonOccupants, appel depuis GetBuildingSelectionView), HOLControllerCommandLibrary.cpp.

## Noms exposes pour le branchement (script)
- `FHOLBuildingSelView::GarrisonOccupants` : TArray<int> (ids d'unite, ordre trie) ; `FHOLBuildingSelView::GarrisonOccupantTypes` : TArray<FString> parallele (meme longueur ; chaine vide si le client ne connait pas l'unite). Remplis par `HOLSelectionRead::GetBuildingSelectionView(View)` (vide pour un batiment qui n'est pas au joueur : replique au proprietaire seul).
- `HOLSelectionRead::GetUnitTypeKey(int UnitId) -> FString` (UFUNCTION BlueprintPure, WorldContext implicite comme les autres de HOLSelectionRead) : cle de type d'une unite par identifiant (HOLUnitView::Find) ; vide si inconnue ou id <= 0.
- `HOLControllerCommand::RequestUngarrisonUnitOf(AHOLPlayerController Controller, int UnitId) -> bool` (une unite sort) ; `HOLControllerCommand::RequestUngarrisonAllSelected(AHOLPlayerController Controller) -> bool` (tout sortir du batiment propre selectionne seul ; GetOwnSelection()). `RequestUngarrisonAllOf(Controller, Building)` existait deja.
- Icone : le script derive l'icone de GarrisonOccupantTypes[i] comme les puces (HOLSelectionChips.as) ; Placeholders.txt si l'icone manque.

## Reste
Point 3 (case de grille Sortir tous, lot D868) ; branchement HOLSelection.as RefreshBuilding (Elrond, integ-25) ; build + quick_check cpp (Garrison.*) par la session principale.

NON VERIFIE : toute la compilation (UHOLSelectionReadLibrary::ReadGarrisonOccupants non-UFUNCTION public statique avec const AHOLBuilding*, include des tests dans le module UI : Mass via HOLUnitView.h comme HOLGameScreen.cpp), le rouge de 14b8786, le vert de 65ebacd, ET surtout que HOLUnitView::Find trouve une unite garnisonnee sur un CLIENT (non lu : s'il l'ignore, GarrisonOccupantTypes sera vide en jeu reseau et le test C++ solo/serveur passera quand meme ; alors lire le type depuis un registre replique). RequestUngarrisonUnitOf / AllSelected : l'effet reseau n'est pas teste (seulement le refus sans controleur) ; l'ordre serveur est couvert par HeroesOfLiberty.Garrison.UngarrisonUnit.
ANOMALIES_PROCESSUS: aucune. Le seuil de 25 appels a ete depasse (voir APPELS_DECLARES).
