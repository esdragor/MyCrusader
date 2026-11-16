# FARAMIR-1116m : MIG-J-Siege-a puis MIG-J-Siege-b (migration D830, sans C++), worktree Faramir, branche `fog`

APPELS_DECLARES: 45 (arrêt à 45 vus par moi ; dont 1 appel perdu : un sous-agent « ok » lancé par erreur pendant une attente)
HEURE_DEBUT/HEURE_FIN: 20:11 / 20:36
PREMIER_BUILD: sans C++   BUILD: aucun (BUILDS_DEMANDES: 0)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: Siege-a vu à 20:23 : tests 1327/1332 (Test_SiegeFindEngine, Test_SiegeTechnologies, Test_SiegeTechnologyHealth rouges contre bouchons qui compilent) ; Siege-b vu à 20:31 : tests 1330/1334 (Test_SiegeCombatVectors contre bouchons, Test_SiegeTableVectors avec « Ram: 400 health » faussé à 401 une fois). Rouge non commité (bouchons remplacés avant le commit vert, L15-5). Assertit l'effet : oui (valeurs calculées par les règles du script sur les lignes RÉELLES des tables lues par HOLAIOrder::ReadAIUnitFigures / ReadAIBuildingFigures / HOLAITechnology::ReadAITechnologyTable).
COMMITS: 3 + 1 fusion, dernier à 20:35 (seuil 30 min), merge_main_fait: oui (3a2c2e8, main 832f0d6, docs seulement)
TESTS: quick_check -Parts tests : référence 20:19 = 1327/1329 ; après Siege-a 20:27 = 1330/1332 ; après Siege-b 20:34 = 1332/1334. Seuls rouges : HeroesOfLiberty.Match.StartingHero.NomadFirstForum et .NomadReplicated (D831, connus, déjà dans main ; « REGRESSION » au niveau de la partie, cf. known_reds.txt). Aucun rouge venu du WIP fog (BackKeyReplaysOptionSetting vert). TESTS_TAUTOLOGIQUES: 0.
ESSAIS: aucun (lot de règles et de tests, LOGIQUE SEULEMENT).
QUESTIONS: aucune (le sens du jumeau est laissé à l'utilisateur, R9 : ligne de bascule dans « Lot C++ de fin de séance »).
FAIT / RESTE:
- MIG-J-Siege-a f5af17c : HOLSiegeRules.as gagne FindEngine, DescribeBlock, HealthStatOf, DamageStatOf, HealthWithTechnology, AttackWithTechnology ; 3 tests portés.
- MIG-J-Siege-b 5f55490 : HOLSiegeRules.as gagne TakesUnitsOn, IsInArea ; 2 tests portés.
- Journal 1a33673 (FogInProgress.md). Auto_Journey_File.md : 2 lignes [x] + ligne de bascule Siege ; EtapeDefini.md : 1 ligne.
- Reste : décision du sens du jumeau Siege (R9) puis bascule C++ éventuelle (lot MIG-CPP) ; exposition des types listés ci-dessous si l'on veut retirer HOLSiegeTests.cpp en entier.
NON VÉRIFIÉ: les tests script ne couvrent que le niveau 1 (HOLHeroXP::GetLevelStat n'est pas en script) ; HOLAIArmy::HitDamage (copie script) retire l'armure de mêlée pour une attaque de siège alors que HOLCombat::GetDamage retire l'armure de siège : le vecteur Mangonel -> Spearman concorde (les deux à 0) mais la copie diverge pour une cible à armure de siège non nulle (écart noté, non corrigé : hors lot) ; cherry-pick sur main non essayé.
PROCESSUS_DETACHES: voir la sortie ajoutée en fin de fichier.
ANOMALIES_PROCESSUS: 1er quick_check de référence perdu (redirection Out-File verrouillée, rien lancé), relancé ; 1 appel gaspillé (sous-agent d'attente).
GIT_STATUS_PRINCIPALE: voir la sortie ajoutée en fin de fichier (changements voulus : Auto_Journey_File.md, EtapeDefini.md, ce rapport ; non commités par moi).

## Champs de migration
BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (aucun : la suppression appartient à la bascule)
TESTS_SCRIPT_AJOUTES: 5 (Test_SiegeFindEngine, Test_SiegeTechnologyHealth, Test_SiegeTechnologies, Test_SiegeCombatVectors, Test_SiegeTableVectors)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: Siege-a 150 (règles ~70 + tests 80) ; Siege-b 95 (règles 16 + tests 79) ; total 245 lignes de script ajoutées
DURÉE PAR LOT: Siege-a 16 min (20:11 -> 20:27, dont ~8 min de lecture et 2 quick_check de 3,5 min) ; Siege-b 7 min (20:27 -> 20:34, écrit pendant les quick_check de Siege-a ; 2 quick_check)
SHA À CHERRY-PICKER (dans l'ordre, sur main) : f5af17c, 5f55490, 1a33673 (journal). Ne PAS prendre de79413 / 87999d5 (WIP D827/D828) ni les fusions.
TYPES_A_EXPOSER (pour porter le reste de HOLSiegeTests.cpp) :
- FHOLAITechnology : Stats (les statistiques qu'une technologie change).
- Fiche de bâtiment lisible du script (FHOLAITypeFigures ne porte que coût, temps, population) : Age, BuildGridSlot, Footprint, MaxHealth, Category, Produces, Accepts, Walkable, SightRadius, IsBuildable.
- FHOLAITypeFigures (unités) : AttackType, BuildingAttackType, BuildingAttack, BuildingAttackInterval, BuildingAttackRange, AreaRadius, SightRadius, SiegeArmor, HeroArmor ; liste des noms de lignes de la table d'unités (balayage « seul le Mangonneau a une zone »).
- HOLHeroXP::GetLevelStat (part du niveau dans les PV et l'attaque).
- HOLCombat::ChooseTarget / FSeeker : NON proposé (règle de Mass, reste en C++).

## Registre nom à nom (C++ HOLSiegeTests.cpp / script Script\Siege\HOLSiegeRules_Test.as)
| Test C++ | Test(s) script en regard | Ce qui reste en C++ et pourquoi |
|---|---|---|
| HeroesOfLiberty.Siege.Tiers | Test_SiegeEngineTiers, Test_SiegePrototypeSet, Test_SiegeCheckOneReason, Test_SiegeCheckReasonOrder (existants, mêmes vecteurs) + Test_SiegeFindEngine (neuf : FindEngine x6, DescribeBlock x5) | rien : couvert en entier |
| HeroesOfLiberty.Siege.TechnologyHealth | Test_SiegeTechnologyHealth (ligne Ram réelle 400 ; 0/+10/+20/-10 % -> 400/440/480/400 ; Mangonneau sur Lancier 45 puis 54 avec +20 %) | le niveau (GetLevelStat) ; HOLCombat::GetMaxHealth / GetDamage sont des règles de combat de Mass, pas de HOLSiege |
| HeroesOfLiberty.Siege.Technologies | Test_SiegeTechnologies (table réelle : 2 technologies à l'atelier, MinTier = palier de l'engin, engins du prototype, noms des statistiques) | Stats de la ligne contient la statistique lue ; l'atelier existe dans la table des bâtiments, son âge, ses produits (types non exposés) |
| HeroesOfLiberty.Siege.Combat | Test_SiegeCombatVectors (TakesUnitsOn sur les lignes réelles Lancier / Bélier / Mangonneau ; IsInArea x5, mêmes vecteurs) | ChooseTarget (attaque-déplacement, tenir) : HOLCombat, Mass |
| HeroesOfLiberty.Siege.Table | Test_SiegeTableVectors (atelier : coût, temps, population ; Bélier et Mangonneau : PV, vitesse, places, coût, catégorie Siege seule, attaque, intervalle, portée, temps, armure à distance 60, armures >= 0) | voir TYPES_A_EXPOSER (âge, case, emprise, PV et produits de l'atelier ; attaque sur bâtiment, zone, vue, armures de siège et de héros, balayage des zones) |
| HeroesOfLiberty.Tiers.Cheat (HOLTierCheatTests.cpp, monde) | aucun | test de monde (AHOLGameState::GetTier), reste |

## Appelants relevés (jumeau HOLSiege)
- C++ : HOLCombatProcessor.cpp l. 738, 769, 880 (GetDamageStat, Mass) ; HOLUnitSubsystem.cpp l. 1331, 1359 (GetHealthStat) ; HOLUnitReceptionSubsystem.cpp l. 624 (GetHealthStat) ; HOLProductionSubsystem.cpp l. 430-438 (FindEngine, Check, DescribeBlock) ; HOLTierCheatTests.cpp (Check) ; HOLGameScreen.cpp l. 2277 (commentaire seulement) ; HOLCombatProcessor.cpp l. 1221, 1604, 1839 (bCanAttackUnits = Attack > 0, la règle de TakesUnitsOn).
- Script : AI\HOLAISiege.as l. 47, 55 (Check) ; Tests\HOLUnitTierRules_Test.as (accord Check / UnitTier).
- Sens : non tranché (R9) ; ligne `[ ]` « bascule Siege » proposée dans « Lot C++ de fin de séance » d'Auto_Journey_File.md (sens A du plan : le C++ reste, appelé par Mass ; sens B : production par le script).
