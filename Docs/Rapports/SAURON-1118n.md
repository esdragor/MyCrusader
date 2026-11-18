# SAURON-1118n : MIG2-S4, tests de HOLSiegeTests.cpp portes en script

**NON COMPILE, AUCUN TEST LANCE.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche visual. Merge main non fait.

Commits (anglais, sans Co-Authored-By, pas de push) :
- `6ed4468` Siege: script tests for the rest + liaisons C++ en lecture seule (HOLSiegeCheckLibrary.h / .cpp) + `Script\Tests\HOLSiegeRemaining_Test.as`
- `b81e1ce` Tests: remove HOLSiegeTests.cpp (git rm, 269 lignes). Commit SEPARE : la session principale peut l'ecarter si le rouge/vert du script n'est pas vu.

BUILDS_DEMANDES: 1 (lot C++ minimal, a compiler par la session principale)
TESTS_CPP_RETIRES: 4 (Siege.Technologies, Siege.TechnologyHealth, Siege.Combat, Siege.Table) = le fichier entier
TESTS_SCRIPT_AJOUTES: 6 (Test_SiegeRemaining_ChooseTarget, _TechnologiesWorkshop, _TableWorkshop, _TableRam, _TableMangonel ; 5 fonctions dans HOLSiegeRemaining_Test.as, plus le test d'accord existant) -> en realite 5 fonctions Test_
LIGNES_CPP_SUPPRIMEES: 269 (HOLSiegeTests.cpp) ; lignes C++ ajoutees : ~149 (liaison)
TYPES_A_EXPOSER: FHOLSiegeUnitFigures (nouveau USTRUCT BlueprintType), FHOLBuildingTypeRow deja lisible (HOLBuildingType::FindRow, repere du 2026-11-16 perime)
TESTS_TAUTOLOGIQUES: 0 (valeurs lues dans les vraies tables Ram / Mangonel / SiegeWorkshop / Technologies, attendus en litteral)

## Liaisons ajoutees (raison C++ : liaison a exposer ; aucune regle changee)
HOLSiegeCheckLibrary : `ReadUnitFigures(FName, FHOLSiegeUnitFigures&)` (table UnitTypes via UHOLSimulationSettings, comme le test C++), `CountAreaRowsExcept(FName)`, `GetTileSize()`, `TechnologyChangesStat(FName, FName)` (UHOLTechnologySettings::FindTechnology), `ChooseSingleTarget(SeekMode, bCanAttackUnits, AttackRange, CandidatePosition)` (enveloppe de HOLCombat::ChooseTarget, un candidat). ChooseTarget est une fonction pure : portable, pas un processeur Mass.

## Table assertion C++ -> test script
Siege.Technologies
- table presente, 2 technologies a l'atelier -> Test_SiegeTechnologies (existant)
- MinTier == GetTier(engine), InPrototype -> Test_SiegeTechnologies (existant)
- batiment dans la table, MinTier >= Age, Produces contient Ram / Mangonel -> Test_SiegeRemaining_TechnologiesWorkshop (NEUF)
- GetHealthStat / GetDamageStat (5 assertions) -> Test_SiegeTechnologies (existant)
- Stats contient la stat lue -> Test_SiegeRemaining_TechnologiesWorkshop via TechnologyChangesStat (NEUF)
Ecart note : le test C++ charge le CSV `Tools\Data\Technologies.csv` ; le script lit la table reelle (asset importe du CSV, HOLAITechnology::ReadAITechnologyTable). Meme source de donnees, chemin different.

Siege.TechnologyHealth (400 / 400 / 440 / 480 / 400 negatif, 45, 54) -> Test_SiegeTechnologyHealth (existant). Ecart : le coup 45 / 54 passe par HOLAIArmy::HitDamage + AttackWithTechnology (pas HOLCombat::GetDamage direct) ; jugee equivalente (meme valeurs, meme regle C++ derriere AttackWithTechnology).

Siege.Combat
- ChooseTarget (3 assertions) -> Test_SiegeRemaining_ChooseTarget (NEUF)
- IsInArea (5 assertions) -> Test_SiegeCombatVectors (existant)

Siege.Table
- atelier : cout, temps, population -> Test_SiegeTableVectors (existant)
- atelier : IsBuildable, categorie, age, slot 4, emprise 3x3, PV 1500, Produces, Accepts, Walkable, vue -> Test_SiegeRemaining_TableWorkshop (NEUF)
- Ram : PV, vitesse, pop, cout, categorie Siege, Attack 0, temps, armure distance 60 -> Test_SiegeTableVectors (existant)
- Ram : attaque siege sur batiment, 60, 2.5 s, AreaRadius 0, armures >= 0 (siege, heros), vue -> Test_SiegeRemaining_TableRam (NEUF)
- Mangonel : PV, vitesse, pop, cout, categorie, 45, 5 s, 3000, temps, armure 60 -> Test_SiegeTableVectors (existant)
- Mangonel : AttackType siege, AreaRadius 250, attaque batiment 45 / 5 s / 3000, armures siege et heros, vue -> Test_SiegeRemaining_TableMangonel (NEUF)
- balayage "aucun autre type n'a de zone" -> CountAreaRowsExcept dans Test_SiegeRemaining_TableMangonel (NEUF)
- moteurs de l'atelier dans la table avec categorie Siege -> Test_SiegeRemaining_TableWorkshop (NEUF)

Aucune assertion ne reste en C++ : les quatre tests sont retires. Rien ne passe par un monde, Mass ou un processeur.

## Mutations pour voir rouge (une a la fois, remettre apres)
- ChooseTarget : `Simulation\HOLCombatRules.cpp`, ChooseTarget : ignorer `bCanAttackUnits` -> Test_SiegeRemaining_ChooseTarget rouge ; ou changer `300.0` du test en `30.0` n'est pas une mutation de la regle : preferer la premiere.
- Technologies : `Tools\Data\Technologies.csv` ligne ReinforcedRam, retirer la stat `RamHealth` de la colonne Stats (reimporter l'asset) -> TechnologiesWorkshop rouge ; ou `Siege\HOLSiegeRules.cpp` GetHealthStat renvoyer un autre nom.
- Atelier : `Tools\Data\BuildingTypes.csv` ligne SiegeWorkshop, MaxHealth 1500 -> 1400 (ou BuildGridSlot 4 -> 5, ou Produces inverse) -> TableWorkshop rouge.
- Ram : `Tools\Data\UnitTypes.csv` ligne Ram, BuildingAttack 60 -> 61 (ou BuildingAttackInterval 2.5 -> 3, AreaRadius 0 -> 10) -> TableRam rouge.
- Mangonel : `Tools\Data\UnitTypes.csv` ligne Mangonel, AreaRadius 250 -> 200 -> TableMangonel rouge ; mettre un AreaRadius de 100 sur une autre unite (ex. Spearman) -> "no other unit type has area damage" rouge.
- Tests existants jugés equivalents : memes mutations que ci-dessus sur les colonnes Health / Speed / Cost / RangedArmor de Ram et Mangonel (Test_SiegeTableVectors), HOLCombat::IsInArea `<=` -> `<` (Test_SiegeCombatVectors, bord a 250), HOLCombat::GetMaxHealth arrondi (Test_SiegeTechnologyHealth).
Les assertions neuves n'ont pas ete vues rouges (aucun lancement) : a faire par la session principale avant de garder `b81e1ce`.

## Suspects de compilation
- Cote C++ : `FHOLUnitTypeRow::HasArmorCategory`, `BuildingAttackType`, `AreaRadius`, `UHOLBuildingSettings::GetTileSize()` (double, converti en float), `UHOLTechnologySettings::FindTechnology` : tous utilises tels quels par l'ancien test, mais jamais compiles dans ce fichier ; includes ajoutes dans le .cpp : `Buildings/HOLBuildingTypes.h`, `Engine/DataTable.h`, `Simulation/HOLSimulationSettings.h`, `Technologies/HOLTechnologyTypes.h`. Build unity : aucun nom de namespace anonyme ajoute.
- Script : `HOLBuildingType::FindRow` rend la ligne avec `Produces` (TArray<FName>, `.Contains`), `Accepts` (TArray enum), `Category` (EHOLBuildingCategory : compare par `int()`), `Footprint` (FVector2D) : jamais lus ainsi par un test existant ; si `.Contains` ou l'enum EHOLBuildingCategory n'est pas lie, ajuster. `ChooseSingleTarget` retourne -1 pour INDEX_NONE. `Test_SiegeRemaining_*` sont des fonctions globales de noms uniques (pas de collision) ; resultats de methode affectes a des variables avant AssertEquals.
- Commande a lancer : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree, puis `quick_check -Parts tests` (scripts) : le total ne doit pas baisser (4 tests C++ retires, 5 tests script ajoutes, plus les 5 existants du port).
