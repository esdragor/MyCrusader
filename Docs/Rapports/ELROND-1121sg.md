# ELROND-1121sg : TowerWood tire (colonnes d'attaque)

RESULTAT : RAPPORT SANS CODE. Le tir de bâtiment n'existe pas ; les colonnes d'attaque exigent du C++ (lot groupé). Aucun fichier modifié dans le worktree Elrond, aucun commit.

## Constats (par lecture)
- `Tools\Data\BuildingTypes.csv` : ligne TowerWood (L17) présente (800 x 800, 800 PV, 100 bois, 20 s, grille 7, vision 4800, 3 places) ; aucune colonne d'attaque dans l'en-tête (22 colonnes, dernière GarrisonSlots). Aucune ligne « Tour de pierre » (`Placeholders.txt` : « Tour de guet en bois [à venir] », « Tour de pierre [à venir] »).
- `Source\HeroesOfLiberty\Buildings\HOLBuildingTypes.h` : la structure de ligne n'a que GarrisonSlots (l.100) et SightRadius (l.167) ; aucun champ d'attaque. Ajouter une colonne au CSV sans champ C++ correspondant casserait l'import (make_building_types.py lit les colonnes par nom) : ne pas toucher au CSV avant le lot C++.
- Tir de bâtiment : `HOLCombatProcessor.cpp` ne traite que des unités (`FFighter`) comme tireuses ; les bâtiments n'y sont que des CIBLES (`StepBuildings`, l.1388, flèche sur bâtiment l.1430-1446, `BuildingAttack` = dégâts d'une unité CONTRE un bâtiment). Aucune tour de pierre, aucun HOLArrow lancé par un bâtiment. Donc pas de « tir existant » à brancher : conformément à la consigne, rien codé. Le tir de tour vit dans HOLCombatProcessor.cpp (interdit : Celeborn y travaille) : à faire après lui.
- Tests : aucun test C++ ou script ne compte les colonnes de BuildingTypes. `HOLCommandGrid_Test.as:524` compte 15 types constructibles (TowerWood inclus), `HOLGarrisonOrderTests.cpp` fabrique sa propre ligne de tour (GarrisonSlots = 2) ; ajouter des colonnes ne les change pas. Le test à écrire plus tard : « une tour de bois finie tire sur un ennemi à portée » (C++, simulation Mass, D838). `Tools\Tests\quick_check.ps1` : rien ne nomme TowerWood ni ne compte les bâtiments qui tirent. Les lignes L18-d / L19-4 citées n'existent pas dans `Auto_Journey_File.md` (grep vide).
- `regen_tables.ps1 -Root <worktree>` (usage lu) : à lancer seul, après le lot C++ et le build ; pas lancé ici (pas de build, binaire non vérifié).

## Rouges attendus nom par nom
Aucun (rien modifié).

## Lot C++ à prévoir (une session, un build)
1. `FHOLBuildingRow` : champs `BuildingAttack` (int), `BuildingAttackInterval` (s), `BuildingAttackRange` (cm), défauts 0 (= ne tire pas) ; colonnes en fin de `BuildingTypes.csv` ; make_building_types.py à vérifier ; régénérer le .uasset (regen_tables.ps1).
2. Tir de bâtiment : dans HOLCombatProcessor (après le travail de Celeborn), un bâtiment fini avec BuildingAttack > 0 cherche un ennemi vivant à portée, lance une flèche (même file de flèches que les unités, dégâts via la règle de GetDamage), cadence par BuildingAttackInterval ; garnison : question séparée.
3. Test rouge C++ : « TowerWood finie, un ennemi à portée : ses PV baissent ; hors portée : intacts ; tour en construction : ne tire pas ». Valeurs lues de la table, pas recopiées.
4. Mettre `Placeholders.txt` (« Tour de guet en bois [à venir] ») à jour au moment du modèle/tir.

## Question
`Retours\Q-towerwood-tir.md` (A recommandée : dégâts 5, cadence 1.625 s, portée 3000 ; défaut codé = colonnes de la seule ligne TowerWood).

## Champs
APPELS_DECLARES: 11 (arrêt à 25) ; HEURE_DEBUT/HEURE_FIN: 20:47 / ~20:52
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (rien à compiler)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (rien codé) ; COMMITS: 0, merge_main_fait: non
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun ; QUESTIONS: Retours\Q-towerwood-tir.md
NON VÉRIFIÉ: l'absence de tout tir de bâtiment dans les .as (grep des mots Tower/garrison seulement), que make_building_types.py lise les colonnes par nom (grep sur GarrisonSlots vide : hypothèse).
PROCESSUS_DETACHES: aucun (aucun lancement) ; ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: deux fichiers neufs (Retours\Q-towerwood-tir.md, ce rapport) : oui, docs voulus.
