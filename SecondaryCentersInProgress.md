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
- **Réponses de l'utilisateur (2026-11-12, D783)** : Q1 = **B, 150 bois + 400 pierre** (À CHANGER : `CostWood = 150`, `CostStone = 400`, durcir `Test_SecondaryCentreCostsMostlyStone`) ; Q2 = **A, population 10** (déjà codé).

## Suivi

- 2026-11-12 10:39-10:5x (HALDIR-1112a, `maps`, sans C++) : SC0 et SC1. Rouge 12aee4a (namespace absent), puis deux fautes de syntaxe du
  test corrigées (un `const` devant un type d'espace de noms n'est pas accepté par l'Angelscript ; espaces perdus) ; résultat du vert et
  commit : voir l'entrée suivante.
- 2026-11-12 10:5x (HALDIR-1112a) : **SC1 VERTE** : quick_check -Parts tests 1180/1180 PASS (211 s), -Parts values PASS (3 lignes au registre Tools/Data/ValueSheet.csv : cout bois, cout pierre, population ; Valeurs.md/.csv regeneres) ; commit 91b39ce. Arret a 45 appels (regle des agents) : ligne Haldir de Auto_Journey_File.md NON mise a jour (a faire par la session principale). **Prochaine etape pour un agent neuf : SC2** (ligne SecondaryCentre de BuildingTypes.csv + test d'accord script/table ; demande l'editeur pour regenerer la table), puis SC3 (C++ : type a part pour l'anti-rush, HOLConstructionSubsystem.cpp:100). Aucun test rouge en cours.
- 2026-11-12 10:55-11:05 (HALDIR-1112b, `maps`, sans C++) : fichier vérifié entier et commité (b140bd7, `git status` propre hors `Content/Maps/Crown.umap`) ; lignes de la file posées (`[x]` SC0-SC1, `[?]` SC2-SC3 sur `Q-secondary-centers.md`, `[ ]` SC3 C++). **Chapitre en pause** tant que Q1 et Q2 ne sont pas tranchées : ne pas coder SC2/SC3 avant. Aucun test rouge en cours.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D783 applique : CostWood 150, CostStone 400 (population 10 inchangee) ; rouge 8c12950 (Test_SecondaryCentreCostsMostlyStone durci aux valeurs 150/400, pierre >= 2x bois et >= 70 %, + Test_SecondaryCentreGivesTenPopulation via PopulationOf), vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.

## 2026-11-21 10:58 HALDIR-1121b (agent, maps, sans C++)
- Ligne Haldir SC3 prise (CELEBORN-1116a deja faite par 1121a bd900a5, stealth/regen/bSites = editeur ou interdit). SC3 = C++ + SC2 (table .uasset, editeur) : non faisable ici. Travail : classer les emplois de MainCentre (lecture seule). Appels vus: 8 (traits: ||||||||)


### SC3 classement des emplois de MainCentre (HALDIR-1121b 11:0x, lecture seule, git grep hors tests : ~100 lignes de production sur 287 ; les ~190 autres sont des tests)
Convention : le centre secondaire aura son propre type (nom propose : SecondaryCentre, ligne SC2 du CSV) ; un emploi de MainCentre = == sur le nom.
- FORUM PRINCIPAL SEUL (le comparer au nom suffit, le centre secondaire ne doit pas y entrer) :
  - Buildings/HOLConstructionSubsystem.cpp:102 : liste EnemyCentres de la zone anti-rush (D22) : seul le principal compte (point central de SC3).
  - Buildings/HOLBuildingTypes.cpp:80,92,103,114 : ouverture du premier forum, IsFirstForumOpen, IsForumComplete, bAlwaysOpen (le secondaire se debloque a l'age 1, CanOrder, pas ici).
  - Map/HOLStartSlot.cpp:23-32, HOLGameState.cpp:1274, HeroesOfLibertyGameModeBase.cpp:664-839, HOLPlayerController.cpp:479,967,3851 : place de depart, camera, spawn (le principal).
  - Match/HOLFactionRules.cpp:25-29,155,289,387, HOLStartRules.cpp:22, HOLFactionRulesReadLibrary.cpp:245 : role MainCentre de la faction (Factions.csv).
  - Player/HOLControllerReadLibrary.cpp:13-28, HOLControllerCommandLibrary.cpp:430-813, Script/Player/HOLControllerCommandsC9a.as:171-234, HOLControllerCommands.as:353 : cible/point de ralliement = le centre principal (a revoir : FindMainCentre du joueur peut rester le principal, mais la selection 'tous les centres' du SC5 passera par une autre fonction).
  - AI/HOLAISummary.cpp:225-269,553-557, HOLAIOrders.cpp:173, Net/HOLCheatRules.cpp:64, Script/AI/HOLAINomadThink.as:51-92, HOLAIOpening.as:431,655,1178 : IA, Nomade (premier forum), prix du premier forum. A relire quand SC6 (boom de l'IA) sera ouvert.
- TOUT CENTRE (le centre secondaire doit se comporter comme le forum) :
  - Buildings/HOLConstructionRules.cpp:466 (regle d'acceptation/depot : Accepts non vide ET type != MainCentre : A VERIFIER, lu en une ligne seulement) ;
  - Script/Player/HOLCommandGridGroups.as:126 (jamais regroupe avec un autre batiment) ; Script/Audio/HOLAudioBuildings.as:55,189,211 (son de fin de production, sons de selection, pierre) ; HOLControllerCommandsC2b.as:183,191 (EnsureOwnSelection d'un batiment producteur de Paysans) ; HOLGameNames.cpp:123 (nom affiche : un nom propre « Centre secondaire » a ajouter, provisoire) ; Simulation/HOLBuildProcessor.cpp:978 (A VERIFIER : non lu).
- NON VERIFIE : les lignes marquees A VERIFIER et les appels cites par plage n'ont pas ete lus ligne a ligne ; le classement est un relevé de grep, pas une decision de design.
- Reste (C++, lot de la seance) : (1) ligne SecondaryCentre de BuildingTypes.csv + regeneration de la table (editeur) ; (2) HOLConstructionSubsystem.cpp:102 inchange (il compare deja au nom MainCentre : le secondaire n'entre pas), donc le vrai travail SC3 = lecture du type, deblocage CanOrder, garnison/tir et les emplois 'tout centre' ci-dessus ; (3) test d'accord script/table (SC2). Aucun test rouge en cours, aucun code touche.
## 2026-11-21 11:02 HALDIR-1121c (agent, sans C++) : SC2 (fait, non lance)
- Test d'accord Script/Tests/HOLSecondaryCentreTable_Test.as (2 tests : ligne presente + cout/population/palier = regles ; forme du forum), commit e75f069, rouge attendu (type absent), NON LANCE.
- Ligne SecondaryCentre de Tools/Data/BuildingTypes.csv : 150 bois + 400 pierre, pop 10, Age 1, Produces Peasant, 4 ressources, emprise 16 m, vue 6400, GarrisonSlots 0 ; hors GDD (duree 150 s, PV 2400, case de grille 1) : Retours/Q-secondary-centre.md, defaut dans le CSV.
- Table regeneree par regen_tables.ps1 (code 0, 0 erreur AS, UnitTypes inchange, BuildingTypes.uasset 20562 -> 21863 octets, 14 noms d'avant toujours presents) ; reste : quick_check -Parts tests (rouge e75f069 sur l'ancienne table, vert avec la nouvelle), puis SC3 C++ (lot groupe). Appels vus: 27
- 2026-11-21 14:09 (RECORD-1121b, réponse de l'utilisateur, aucun code) : Q-secondary-centre = A (GDD D879) : 150 s, 2400 PV ; valeurs déjà codées (4599ab8, wall-line), rien à changer ; fiche archivée dans Docs\Archive\Retours_2026-11-21\.
- 2026-11-21 19:15 (HALDIR-1121sa, wall-line, sans C++) : D879 'tout centre' en script. Test rouge 0516682 (Script/Tests/HOLSecondaryCentreAll_Test.as, 5 tests), regle 7393200 : namespace HOLCentreKind (IsCentre / IsCentreName, HOLCommandGridGroups.as) appelee par IsAloneInGroup, HOLAudioBuildings.as (ligne de table SecondaryCentre, FamilyOf, SelectEventFor -> SelectBuildingForum). NON compile, non lance (brief : aucun build). Reste = lot C++ : HOLConstructionRules.cpp:466, HOLBuildProcessor.cpp:978, HOLGameNames.cpp:123. Prochaine etape : quick_check -Parts tests apres build.
- 2026-11-21 19:2x (HALDIR-1121sc, wall-line, sans C++) : merge integ-25 fc901f7 = 5a62867 (conflits BuildingTypes.csv resolu a la main, 2 lignes gardees ; uasset regenere par regen_tables.ps1 : 23106 octets) ; test Test_CGTables 14 -> 15 types, SecondaryCentre page II case 1 (Age 1, slot 1, grise), case 1 retiree de EmptyII = ed2566e ; ligne placeholders_map.csv + ligne Placeholders.txt. NON lance. Reste : quick_check -Parts tests, lot C++ SC3. Appels vus: 13

## 2026-11-21 19:2x HALDIR-1121sd (agent, sans C++)
- Ligne prise : la 1re [ ] sans C++ hors SC3 (HALDIR-1121a, bd900a5) est DEJA faite (bd900a5 dans main ; reste seulement quick_check de la session principale). Autres lignes Haldir = editeur / C++ / interdits. Travail : emplois script de MainCentre classes `tout centre` du releve f85b3c0 : HOLAudioBuildings.as et HOLCommandGridGroups.as deja faits (1121sa) ; ajoute ici HOLControllerCommandsC2b.as (hol.Prod.Rally, hol.Prod.Queue : EnsureOwnSelection) et C9a.as (hol.Prod.RallySite : FindOwnBuildingByType) via HOLCentreKind::OwnedCentreType / OwnedCentreTypeOf (principal d'abord, sinon secondaire, sinon principal).
- Test rouge 1a1b459 (Script\Tests\HOLCentreFallback_Test.as, 2 tests, stub = principal seul : Test_OwnedCentreTypeFollowsWhatIsOwned rouge attendu sur le cas secondaire seul), correctif b4628e4. NON LANCE (pas de quick_check) : commande : Tools\quick_check.ps1 -Parts tests (session principale). Reste hors script : les emplois IA / Nomade restent orum principal seul (A relire au SC6) ; Source\*.cpp inchange.
- HALDIR-1121se (19:35) : FindMainCentreOf et PointNearMainCentre (+ PointNearFor, RequestPlacementNear) passent tous par UHOLControllerReadLibrary::FindMainCentre (Player\HOLControllerReadLibrary.cpp) : cote joueur = principal d'abord, sinon centre secondaire (HOLBuildingTypeNames::FindCentreIndex, HOLBuildingTypes.h) ; cote ennemi (AttackBuilding) reste forum principal seul. Rouge 3bd682e (FHOLBuildingCentrePickTest, HeroesOfLiberty.Buildings.CentrePick), correctif a1360b1. NON COMPILE.
- 2026-11-22 16:1x (réponse de l'utilisateur, session principale, aucun code) : Q-secondary-centre-grid = A pour le prototype, B ensuite (GDD D883) : Paysan seul, déjà codé ; cloche W = ligne [B] de la section Haldir.
