# ELROND-1116m — migration D830/D835, jumeaux UnitTier et Technology (worktree HeroesOfLiberty-Elrond, branche `tiers`, sans C++)

APPELS_DECLARES: 40 (au moment du rapport ; arrêt à 45)
HEURE_DEBUT/HEURE_FIN: 20:11 / 20:31
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, `Source\` non touché)   LAUNCHCHECK_DEBUT: sans C++ (DLL 19:14 donnée à jour par l'énoncé)
TEST_ROUGE: vu à 20:21 : quick_check tests 1326/1330, rouges nommés `Angelscript.UnitTests.Tests.HOLUnitTierRules_Test.Test_UnitTierAgreesOnEveryRowOfTheTable` (rouge RÉEL) et `...HOLTechnologyRules_Test.Test_TechnologyRulesVectors` (valeur attendue faussée une fois) ; rouge non commité (même tour), vert commité ba50275 / df294d2 ; assertit l'effet : oui (vraie chaîne C++ par les bibliothèques `HOLUnitTierRules` / `HOLTechnologyCheck`)
TEST_ROUGE_VU: oui
COMMITS: 5 (ba50275, df294d2, merge main 39f8edd, 7347007, 983ce8e), dernier à 20:31, merge_main_fait: oui (main e435ca9)
TESTS: quick_check -Parts tests : entrée 1326/1328 (20:17), rouge 1326/1330 (20:21), vert 1328/1330 (20:25) ; seuls rouges = `Match.StartingHero.NomadFirstForum`, `.NomadReplicated` (D831, connus, sur main) ; total passe de 1328 à 1330 (ne baisse pas) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (lots de tests seulement) ; LOGIQUE SEULEMENT
QUESTIONS: aucune fiche ; deux sens de bascule laissés à l'utilisateur (R9) dans « Lot C++ de fin de séance »
EDITS_REFUSES: 1 (Edit « not read » sur HOLUnitTierRules_Test.as, remplacé par un ajout en fin de fichier)
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (la suppression appartient à la bascule)
TESTS_SCRIPT_AJOUTES: 2 (Test_UnitTierAgreesOnEveryRowOfTheTable, Test_TechnologyRulesVectors)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: UnitTier : 0 ligne de test C++ (aucun test C++ pur à porter ; accord étendu : +69 lignes de test, .as +4/-3) ; Technology : 49 lignes C++ (HOLTechnologyTableTests.cpp l. 283-331, part disponibilité de Technologies.Rules) portées en 33 lignes de script
DURÉE DES LOTS: UnitTier 20:12 -> 20:25 (13 min, dont 2 quick_check de ~3,5 min communs aux deux lots) ; Technology 20:19 -> 20:25 (6 min, même quick_check) ; écritures (file, journaux, rapport) 20:26 -> 20:33
TYPES_A_EXPOSER: aucun pour ces lots (bibliothèques déjà exposées). Pour porter le reste de Technologies.Rules : FHOLTechnologyRow (lecture de la table), HOLTechnology::MakeCost, GetBonusPercent, ScaleByPercent, GetAttackStat, GetArmorStat, ArmorCoversAttack, DescribeBlock, FHOLTechnologyState, HOLProduction::TryEnqueue/Cancel/Advance, FHOLProductionItem, FHOLProductionCost, EHOLAttackType, EHOLArmorCategory. Pour UnitTier sens « script seul » : un pont serveur -> script (socle de l'étape 1), FHOLUnitTypeRow.MinTier.

## Écart réel trouvé (UnitTier)
L'accord étendu aux 16 lignes de `Tools\Data\UnitTypes.csv` a vu ROUGE pour de vrai : la table tapée du script (`HOLUnitTier::MinTierOf`) oubliait le **Zombie (0)** et le **Damned Warrior (1)** des Légions, que la colonne MinTier du C++ porte. Effet en jeu : `Script\AI\HOLAIArmy.as` l. 610 lisait -1 et croyait le Damned Warrior ouvert à tout palier alors que le serveur (`HOLProductionSubsystem.cpp` l. 446-448) le refuse au palier 0. Correctif : les deux lignes ajoutées au `.as` (miroir de la colonne, qui est la source déclarée dans l'en-tête du fichier). Écart restant, à trancher avec la bascule : héros, Roi, LightInfantry, LightInfantryNanite = NotListed côté script, palier 0 côté colonne ; sans effet aujourd'hui (les appelants C++ ne lisent que `== TierTooLow`), le test l'assertit.

## Registre des tests
| Test C++ | Test script en regard | Statut |
|---|---|---|
| (aucun test C++ pur de la règle UnitTier) | Test_UnitTierScriptAndSimulationRulesAgree (10 unités) + **Test_UnitTierAgreesOnEveryRowOfTheTable** (16 lignes, paliers 0-3, bâtiment, stock, type hors table) | accord vert |
| HeroesOfLiberty.Production.UnitTier (HOLUnitTierProductionTests.cpp) | — | reste en C++ (crée un monde, production serveur) |
| HeroesOfLiberty.Technologies.Rules, part disponibilité (l. 283-331, 15 assertions dont `DescribeBlock` « soon ») | **Test_TechnologyRulesVectors** (14 vecteurs, chacun sur les 2 copies) + Test_TechnologyScriptAndSimulationRulesAgree (1296 cas) | porté ; le mot « soon » reste en C++ (DescribeBlock non exposé) |
| HeroesOfLiberty.Technologies.Rules, reste (coût, file de 5, bonus T5/T6, armures F2, FHOLTechnologyState) | — | reste en C++ (aucune bibliothèque ; voir TYPES_A_EXPOSER) |
| HeroesOfLiberty.Technologies.Table, .Playable | — | restent en C++ (DataTable importée, types de bâtiments) |
| (script seulement) Test_TechnologyAvailable, Hero, BuildingAndCost, Chain, ReasonOrder, Queued, NotImplemented | — | inchangés, verts |
| (script seulement) Test_UnitTierTable (+2 assertions Zombie, Damned Warrior), OpensSoldiers, ReasonOrder, AgreesWithTheSiegeRule, Monotone | — | verts |

## Appelants relevés
- UnitTier C++ : `Buildings\HOLProductionSubsystem.cpp` l. 446-448 (serveur, à chaque ordre), `HeroesOfLibertyUI\HUD\HOLGameScreen.cpp` l. 2279-2280 (Slate, grille) ; script : `Script\AI\HOLAIArmy.as` l. 610 ; test monde `HOLUnitTierProductionTests.cpp`.
- Technology `HOLTechnology::Check` : `HOLProductionSubsystem.cpp` l. 408 (serveur), `Player\HOLCommandGrid.cpp` l. 337 (grille) ; autres `HOLTechnology::` (bonus) : `HOLCombatProcessor.cpp` (9) et `HOLGatherProcessor.cpp` (4) = Mass, `HOLPlayerController.cpp` (7), `HOLGameScreen.cpp` (12) ; script : `Script\AI\HOLAITech.as` l. 144.

## Bascules proposées (non tranchées, R9)
Lignes `[ ]` MIG-CPP-UnitTier et MIG-CPP-Technology ajoutées sous « Lot C++ de fin de séance » d'`Auto_Journey_File.md` (sens proposé : le C++ reste, la copie script passe par la bibliothèque déjà exposée — côté script sans build ; sens inverse décrit).

FAIT / RESTE: MIG-J-UnitTier [x] ba50275 ; MIG-J-Technology [x] df294d2 ; branche `tiers` à jour avec main (39f8edd puis 983ce8e), **branche prête** (pas de fusion dans main) ; IN_USE « libre » ; disponible pour le lot C++ + fusion groupée de 21:50. Reste : décision du sens des deux jumeaux (utilisateur), portage du reste de Technologies.Rules (types à exposer).
NON VÉRIFIÉ : le message exact des assertions rouges (seul le nom du test rouge a été lu dans quick_check) ; aucune partie autre que `tests` lancée (pas de `net`, script seulement).
PROCESSUS_DETACHES: aucun (stray_processes -Stop, 20:31)
ANOMALIES_PROCESSUS: `git fetch` (lancé par erreur dans un appel groupé) a bloqué l'appel au-delà de 120 s : tâche arrêtée (TaskStop), commits déjà faits, merge refait seul sans fetch.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M EtapeDefini.md (mes lignes + autres agents), ?? rapports et docs d'étape 0 ; inchangée hors docs voulus : oui
