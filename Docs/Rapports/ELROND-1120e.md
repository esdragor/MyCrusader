# ELROND-1120e : socle des factions, morceau 7 (specialisation par faction + 3e case de grille)

Script seul, AUCUN build, AUCUN test lance : tout est NON VERIFIE a l'execution (les scripts n'ont pas ete compiles par le plugin).

APPELS_DECLARES: 18 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:37 / 10:41 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (script seul, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: beac365 (test + stubs qui compilent, prevus rouges : SetSeatFaction sans effet, GetCommandWithOptions ignore le compte) ; rouge NON VU a l'execution ; assertit l'effet : oui (option de la 3e voie acceptee puis appliquee `Chosen[1] == 2` ; case 3 = commande Specialize option 2)
COMMITS: 2 (beac365 test + stubs, 5bb28d4 correctif), dernier a 10:40, merge_main_fait: oui (e529932, premier geste, main 37bf476)
TESTS: aucun lance. Nouveaux : Test_SpecializationBoardSeatFactionOptions, Test_SpecializationThirdGridCellForThreeOptions (Script\Tests\HOLSpecializationRules_Test.as) ; TESTS_TAUTOLOGIQUES: 0 (les comptes viennent de HOLSpecialization::OptionCountOf, jamais recopies ; l'effet passe par Board.StartSpecialization -> Start -> OptionCount de l'etat, puis Advance et ViewSpecialization)
ESSAIS: aucun
QUESTIONS: Retours\Q-specialization-troisieme-case.md (la 3e case, cellule 8, est aussi la 1re case de recherche du Forum : elle la masque tant qu'un palier est ouvert ; defaut code A = cases contiguees 6,7,8 ; recommandation B = 5,6,7 pour les Templiers). 1 question.

## Reponse a la question du lot : le morceau 7 n'est-il plus que du script ?
NON, en deux moities :
- Moitie siege (le tableau) : SCRIPT SEUL. `AHOLGameState::GetSlotFaction(int32)` (HOLGameState.h:488) est lisible depuis le script ; `SetSeatFaction` n'a pas besoin d'etre un evenement C++ : c'est une methode script de `UHOLSpecializationBoardScript`.
- Moitie grille : le contexte de la grille est construit en C++ pour le client (`HOLPlayerController.cpp:2226` -> `HOLCommandGrid::SetSpecialization` -> `FHOLGridContext` -> `HOLCommandGridBoard.cpp:77` MakeContextView -> `UHOLCommandGridBoardScript::GetCommandAnswer`), et le script n'a ni le numero de siege local ni la faction dans ce contexte (aucun `GetPlayerIndex` appele depuis le script). Il faut donc UN champ de plus qui traverse le C++ (voir lot ci-dessous). La regle est faite et testee a part : `HOLCommandGridRules::GetCommandWithOptions(Context, Cell, OptionCount)` ; `GetCommand(Context, Cell)` l'appelle encore avec la constante 2.

## FAIT
- `Script\Specialization\HOLSpecializationBoardScript.as` : `SeatFactions`, `SeatFactionOf(Slot)` (explicite sinon `GetSlotFaction` ; Aube sans monde ou siege < 0), `SetSeatFaction(Slot, Faction)` (reconstruit l'etat du siege), `StateOf` cree l'etat par `NewStateFor(faction du siege)` au lieu de `NewState(DefaultOptionCount)`. Un siege de l'Aube garde 2 options ; Templiers 3 ; une autre faction : ses propres lignes n'existent pas encore (`OptionCountOf` donne 2), aucune valeur de design inventee.
- `Script\Player\HOLCommandGridRules.as` : `GetCommandWithOptions` ; la borne haute des cases de specialisation est `GetSpecializationCell(OptionCount)`.
## RESTE (lot C++ groupe)
BUILDS_DEMANDES: 1 (le lot C++ de la seance, avec ce qui suit ; rien d'autre n'est compile par moi)
TYPES_A_EXPOSER / C++ a ecrire :
- `FHOLSpecializationView.OptionCount` (HOLSpecializationView.h, defaut 2, dans `operator==`) rempli par `ViewSpecialization` du script (`State.OptionCount`), copie par la replication existante vers `AHOLPlayerState` ;
- `FHOLGridContext.SpecializationOptions` (HOLCommandGrid.h:178-180, defaut 2) rempli par `HOLCommandGrid::SetSpecialization` (HOLCommandGrid.cpp:84) ; meme champ dans `FHOLGridContextView` (HOLCommandGridReadLibrary.h:126-130), `MakeContextView` (HOLCommandGridBoard.cpp:77), `ToRule` (HOLCommandGridReadLibrary.cpp:33) et `ToView` (HOLScriptScreen.cpp:91) ; `GetCommand` du script passe alors `Context.SpecializationOptions` a `GetCommandWithOptions` ;
- `HOLCommandGrid.h:322` `SpecializationOptions = 2` et `GetSpecializationCell` (Clamp `NumCells - SpecializationOptions`) : a lire avec le compte ;
- libelle/rapport de l'option 2 : `GetSpecializationName` (HOLGameNames.cpp:448-453) donnerait « Voie B » pour l'option 2 ; `GetReportName` (HOLCommandGrid.cpp:96-103) ecrit B pour toute option != 0 ; libelles des commanderies : Q-specialization-ui (fichier absent de Retours\ aujourd'hui, archive ?) ;
- l'appelant de `SetSeatFaction` a l'initialisation des sieges (GameMode, avec `SetSeatNomad`) : inutile tant que `SeatFactionOf` lit `GetSlotFaction` au premier acces ; a ajouter seulement si le premier acces peut preceder la configuration de lancement ;
- `ServerStartSpecialization` : la validation d'option passe par le script (`Start` -> `BadOption`) ; verifier qu'aucun controle C++ ne coupe a 2.

## NON VERIFIE
Compilation des deux fichiers script et des deux tests ; `NewObject(GetTransientPackage(), UHOLSpecializationBoardScript)` (sous-systeme de monde instancie sans monde, `GetWorld()` nul garde par `SeatFactionOf`) ; appel d'une surcharge `BlueprintOverride` d'un evenement C++ `protected` depuis un test ; rouge de beac365 jamais vu ; replication de la faction en reseau (le siege lit le GameState cote serveur seulement).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress, Auto_Journey_File, EtapeDefini, FortificationsInProgress, SpecializationInProgress modifies par plusieurs sessions ; nouveaux rapports et Q) : oui, aucun code ecrit dans la copie principale
