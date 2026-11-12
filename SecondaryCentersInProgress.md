# SecondaryCentersInProgress.md : le centre secondaire (GDD § 5.4, D384 ; avec D22, D347, D351, D388, D397, D403, § 11.1)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112a, worktree Haldir, branche `maps`). Méthode des dossiers par feature : règles tirées du
GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions (`C:\Users\baudo\Documents\MyCrusader\Retours\Q-secondary-centers.md`).
Règle D483 : règles pures en Angelscript, le bâtiment (simulation, table, réseau) en C++ plus tard.

## D384 est-il couvert par les relais ? NON (relevé du 2026-11-12)

- `RelaysInProgress.md` couvre seulement l'**XP de territoire** des centres : `HOLRelay::TerritoryXP(Memory, Relays, Centres)` et
  `CentreXPPerMinute = 10` (D587) dans `Script/Relays/HOLRelayRules.as`, testés dans `Script/Tests/HOLRelayRules_Test.as`.
- Le bâtiment n'existe pas : aucune ligne `SecondaryCentre` dans `Tools/Data/BuildingTypes.csv` ; `UHOLRelaySubsystem` donne toujours
  0 centre (`Source/HeroesOfLiberty/Simulation/HOLRelaySubsystem.cpp:251`, « no secondary centre in the game yet »).
- Déjà prêts ailleurs : `AHOLBuilding::IsPrimaryForum()` (seul le forum de départ a des vies, un centre secondaire est détruit comme tout
  bâtiment, `HOLBuilding.h`) ; la zone anti-duel ignore les centres secondaires (`HOLDuelRules.as:339`, D403) ; le bloc de vies ne
  s'affiche pas pour eux (`FHOLForumBlock`, `HOLForumRules.h`).

## Règles tirées du GDD (rien à trancher)

1. Débloqué au **palier 1** (Essor, grille § 11.1, D129). **Sans plafond** (le boom est un pari que le rush punit).
2. **Coûteux, surtout en pierre** (§ 5.4, tableau des ressources : la pierre construit les centres secondaires). Valeur : question Q1.
3. Debout et fini : **produit des Paysans**, **dépôt de toutes les ressources**, **garnison et tir**, **base de population**, **lieu de
   réapparition du héros** (§ 9.8, D397 : forum par défaut, clic sur un centre secondaire contrôlé pour un autre lieu).
4. **Pas de vies**, ne protège jamais de la défaite (D347), **détruit comme tout bâtiment** (pas de ruine, D351 est au seul forum principal).
5. **Pas de zone anti-construction** autour de lui (D22 : seulement le centre principal ennemi) ; **pas de zone anti-duel** (D403).
6. **Compte pour l'XP de territoire** (D388, 10 XP par minute, D587) ; le forum principal n'y compte pas (« points stratégiques et centres
   secondaires contrôlés »).
7. Rappel D351 (pour la règle commune) : le forum principal à terre perd production, dépôt, tir et garnison, garde la réapparition, la
   population et ses zones.

Lectures faites sans décision (à confirmer si l'utilisateur le veut, pas des questions) : une **fondation** ne fait rien (ni population, ni
réapparition, ni territoire) ; « contrôlé » = **à soi** (le centre d'un allié n'est pas un lieu de réapparition).

## Écarts du code à corriger quand le bâtiment arrivera (C++)

- **Zone anti-rush par nom de type** : `HOLConstructionSubsystem.cpp:100` ne relève que le type `MainCentre`. Si le centre secondaire
  réutilisait `MainCentre`, il porterait à tort la zone anti-construction (contre D22, D384). Il faut un **type à part** (`SecondaryCentre`)
  ou un filtre `IsPrimaryForum()`.
- **104 emplois du nom `MainCentre`** dans `Source/` et `Script/` (46 fichiers : IA, grille de commande, victoire, sons, tests) : chacun est
  à classer « forum principal seul » ou « tout centre » à l'étape SC3.
- `UHOLRelaySubsystem` passe 0 centre à `TerritoryXP` : à brancher sur `HOLSecondaryCentre::CountTerritory` (SC4).

## Plan (étapes de 15 min)

- **SC0 relevé** : FAIT (ce fichier, 2026-11-12).
- **SC1 règles pures en script** : FAIT (2026-11-12) : `Script/SecondaryCentres/HOLSecondaryCentreRules.as` (`CanOrder`, `Cost`, `RoleOf`,
  `IsRespawnPlace`, `CountTerritory`, `PopulationOf`) et `Script/Tests/HOLSecondaryCentreRules_Test.as` (7 tests, rouge d'abord 12aee4a).
- **SC2 données** (agent avec éditeur, régénération de la table) : ligne `SecondaryCentre` de `Tools/Data/BuildingTypes.csv` (forme du
  forum, `Produces` Paysan, `Accepts` tout, `PopulationProvided` = `PopulationBase`, `Cost` = `HOLSecondaryCentre::Cost`, `Age` 1, case de
  la grille du Paysan : AoE4 place le centre-ville dans l'onglet économie), mesh du forum = provisoire (ligne à ajouter à `Placeholders.txt`
  à ce moment-là) ; test d'accord script / table (coût, population, palier lus des deux côtés).
- **SC3 C++** : type à part dans la construction (anti-rush, D22), garnison et tir comme le forum, classement des 104 emplois de `MainCentre`.
- **SC4 territoire** : `UHOLRelaySubsystem` compte les centres secondaires debout de chaque slot (`build:relay` étendu : un centre posé,
  une minute, XP d'exploration reçu = taux).
- **SC5 réapparition** : clic sur un centre secondaire à soi pendant le bouton « Ramener le héros » (D397) ; test sur le geste réel.
- **SC6 IA** : boom de l'IA non décidé (aucune question posée : après le chantier IA4 ; note seulement).
- **SC7 vérification** : scénario `secondary_centres` (poser, produire un Paysan, déposer, réapparaître), essai listé seulement VU À L'ÉCRAN.

## Premier test rouge (SC1, fait)

`Test_SecondaryCentreStandingIsAForumMore` et 6 autres : rouge « Namespace 'HOLSecondaryCentre' doesn't exist » (quick_check `tests`,
2026-11-12 10:44, commit du test 12aee4a), vert ensuite (voir « Suivi »). Le test de territoire passe par la vraie règle
`HOLRelay::TerritoryXP` (une minute de pas de simulation réels : XP reçu = 3 x `CentreXPPerMinute`).

## Questions (`Retours\Q-secondary-centers.md`, défauts codés)

- Q1 : coût. Défaut A = 300 bois + 300 pierre (`CostWood`, `CostStone` de `HOLSecondaryCentreRules.as`).
- Q2 : base de population. Défaut A = 10, comme le forum principal (`PopulationBase`).

## Suivi

- 2026-11-12 10:39-10:5x (HALDIR-1112a, `maps`, sans C++) : SC0 et SC1. Rouge 12aee4a (namespace absent), puis deux fautes de syntaxe du
  test corrigées (un `const` devant un type d'espace de noms n'est pas accepté par l'Angelscript ; espaces perdus) ; résultat du vert et
  commit : voir l'entrée suivante.
- 2026-11-12 10:5x (HALDIR-1112a) : **SC1 VERTE** : quick_check -Parts tests 1180/1180 PASS (211 s), -Parts values PASS (3 lignes au registre Tools/Data/ValueSheet.csv : cout bois, cout pierre, population ; Valeurs.md/.csv regeneres) ; commit 91b39ce. Arret a 45 appels (regle des agents) : ligne Haldir de Auto_Journey_File.md NON mise a jour (a faire par la session principale). **Prochaine etape pour un agent neuf : SC2** (ligne SecondaryCentre de BuildingTypes.csv + test d'accord script/table ; demande l'editeur pour regenerer la table), puis SC3 (C++ : type a part pour l'anti-rush, HOLConstructionSubsystem.cpp:100). Aucun test rouge en cours.
