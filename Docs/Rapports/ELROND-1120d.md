NON COMPILÉ (C++ écrit : HOLFormationRules.h, HOLFormationReadLibrary.h/.cpp, HOLSelectionMarkers.cpp ; le test script ne compile qu'après le build qui expose FindBlockCentres)

# ELROND-1120d : BUG order_lines_multi_blocs (une ligne de mouvement par bloc)

Impact joueur : une sélection d'unités de blocs éloignés trace maintenant une ligne par bloc, depuis le centre de chaque bloc, au lieu d'une seule depuis le milieu de nulle part.

APPELS_DECLARES: ~22 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:30:09 / 10:35 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: 5658626 (test + stub) + assertit l'effet : oui (nombre de lignes et points de départ calculés par la règle FindPackets, pas de valeur recopiée). Rouge à VOIR à l'exécution : build puis quick_check -Parts tests ; attendu rouge : Test_OrderLines_TwoBlocksTwoStarts, Test_OrderLines_InterleavedSelectionStillGroupsByBlock, Test_OrderLines_BlockAndLoneUnit (le stub rend 1 centre) ; verts sur le stub : OneBlockOneStart, NoUnitsNoStart.
COMMITS: 2 (5658626 test + stub, 904bd5e correctif), dernier à 10:34, merge_main_fait: non (integ-26 fusionné en premier geste, comme demandé)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (les points de départ sont comparés à la moyenne des positions des unités du bloc, calculée dans le test ; la règle passe par HOLFormation::FindPackets)
ESSAIS: aucun (rien compilé). À rejouer par l'intégrateur sur le build fusionné : deux groupes à 30 m, sélection des deux, clic droit au sol, deux lignes.
QUESTIONS: aucune ; valeur codée : un bloc = un paquet de FindPackets au pas HOLFormation::BlockJoinDistance (450 cm, le même que l'ordre de déplacement) ; une unité seule est un bloc d'une unité, sa propre ligne (le processeur d'ordres la traite en retardataire, le BUG ne dit rien).

## Où est la règle (source unique)
- Dessin et calcul des points de départ : C++, AHOLSelectionMarkers::UpdateOrderLines (Source\HeroesOfLiberty\Player\HOLSelectionMarkers.cpp, ~l. 1237-1284). Aucune règle script des lignes (grep OrderLines dans Script\ : seulement des commandes de test hol.Select.OrderLineGround).
- Cause : une ligne par FHOLNetOrderChain, départ = centre de TOUTES les unités debout de la chaîne ; un clic droit sur une sélection éparse crée une chaîne unique.
- La notion de bloc est elle aussi en C++ : HOLFormation::FindPackets (HOLFormationRules, exposé HOLFormationRead::FindPackets) ; le processeur d'ordres (HOLUnitOrderProcessor.cpp:1861-1883) en tire BlockOfUnit.

## Fait
- HOLFormation::FindBlockCentres(Locations, LinkDistance, OutCentres) dans HOLFormationRules.h (inline, une seule source, pas de jumeau script) : un centre par paquet, plus grand d'abord ; exposé en UFUNCTION par UHOLFormationReadLibrary::FindBlockCentres (HOLFormationRead::FindBlockCentres en script).
- UpdateOrderLines : pour chaque chaîne, les unités debout -> FindBlockCentres -> une polyligne (mêmes étapes, mêmes couleurs, mêmes points) par bloc ; le `continue` « Count == 0 » devient une boucle vide.
- Script\Tests\HOLOrderLines_Test.as : 5 tests (deux blocs -> deux lignes aux centres de leur bloc et aucun au milieu ; un bloc -> une ligne ; sélection entrelacée A1 B1 A2 B2 -> deux ; bloc + unité seule -> deux, la seconde sur l'unité ; aucune unité -> aucune).

## Reste
- Build (lot C++ de la séance) : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests`. Le test dit rouge sur 5658626 seul : si on veut le rouge VU, lancer le build sur 5658626 avant 904bd5e (sinon seul le vert est vu).
- Coût image : FindPackets est quadratique en unités d'une chaîne, une fois par chaîne sélectionnée et par image ; à mesurer si une chaîne dépasse quelques centaines d'unités (sinon mettre le résultat en cache par chaîne).

BUILDS_DEMANDES: 1
TESTS_SCRIPT_AJOUTES: 5 (Test_OrderLines_TwoBlocksTwoStarts, Test_OrderLines_OneBlockOneStart, Test_OrderLines_InterleavedSelectionStillGroupsByBlock, Test_OrderLines_BlockAndLoneUnit, Test_OrderLines_NoUnitsNoStart)
TYPES_A_EXPOSER: aucun (HOLFormationRead::FindBlockCentres est ajouté par ce lot C++ ; TArray<FVector>& en sortie, comme FindPackets avec TArray<int32>&)

NON VÉRIFIÉ: tout (rien compilé) ; que le script lie FindBlockCentres avec TArray<FVector>& en sortie ; que FindPackets rend 0 paquet pour une liste vide (court-circuité par IsEmpty) ; le rendu réel des lignes.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune (regex PowerShell `$` avec CRLF a raté une première insertion ; refaite par chaîne exacte, EDITS_REFUSES: 0)
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File.md, EtapeDefini.md, InterfaceInProgress.md modifiés par ma ligne ; reste = autres sessions) : oui
