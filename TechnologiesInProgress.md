# TechnologiesInProgress.md : chapitre Technologies (GDD § 11)

Journal du chantier **Technologies**, ouvert le 2026-10-30 (Auto_Journey, Glorfindel, worktree `HeroesOfLiberty-Glorfindel`, branche `net-playtest` = `main` 436096c au départ ; le chantier réseau reste en pause, rien du réseau n'est touché).
Règles : GDD § 11 (11.1 paliers 0 à 2, 11.2 palier 3), décisions **D27** (arbre ~70-80 % commun, ~20-30 % de faction), **D180** (aucune unité ne se débloque par une recherche ; palier 3), **D181** (Fonderie supprimée, *Poudre raffinée* à la Forge), **D129** (grille 11.1 validée), **D316/D317/D393** (*Bénédiction des armes*, *Rites funèbres*, catégorie « Profane »), **D04/D34** (paliers aux niveaux 3, 6 et 9 du héros), **D483/D484** (le neuf en Angelscript sauf Mass/simulation/réseau/rendu ; tests en script quand le code est en script). Numéros de décision : voir § 22 du GDD, prochain libre D581 ; ce chantier n'en prend aucun (toutes les questions ci-dessous sont **posées, pas tranchées**).

## Ce que dit le GDD (résumé)

- Une technologie a **trois conditions** : niveau minimum du héros, bâtiment requis, coût. Exemple du GDD : héros niveau 3 + Grande Forge + 300 bois / 200 or.
- Cinq catégories : Économie, Militaire (statistiques seulement, jamais de remplacement d'unité, D38), Infrastructure, Héros, Faction.
- Les **unités** se débloquent avec le palier, jamais par recherche (D180) ; un palier ouvre des **bâtiments**.
- 11.1 : 7 familles communes (Forge ×4 lignes, Camps et ferme ×4 lignes, Centre, Académie ×3, Atelier de siège ×2) en I (palier 1) et II (palier 2) ; propres à l'Aube : *Liturgie*, *Hallebardes trempées* (palier 1), *Bénédiction des armes*, *Vœu de pauvreté* (palier 2). 11.2 (palier 3, hors prototype : niveaux 1 à 6 seulement) : Forge III, camps III, *Poudre raffinée*, *Fortifications*, *Balistique*, *Contrepoids*, *Levée de la garnison*.

## T0 : relevé de l'existant (2026-10-30, fait)

| Quoi | Où | État |
|---|---|---|
| Types de bâtiments | `Source/HeroesOfLiberty/Buildings/HOLBuildingTypes.h/.cpp`, `Tools/Data/BuildingTypes.csv` (importé par `Tools/Editor/make_building_types.py` en `/Game/Buildings/BuildingTypes`) | **8 types seulement** : `MainCentre`, `House`, `LumberCamp`, `MiningCamp`, `Mill`, `Barracks`, `Farm`, `ArcheryRange`. **Absents** : Forge, Académie, Atelier de siège, Chapelle, Sanctuaire, Écurie, Marché. Colonnes déjà utiles : `Age` (palier du panneau), `Category` (Economic/Military/Technological), `Produces`, `Cost`, `BuildTime`. |
| Grille de commandes | `Source/HeroesOfLiberty/Player/HOLCommandGrid.h/.cpp`, `Tests/HOLCommandGridTests.cpp`, `HeroesOfLibertyUI/Widgets/HOLGridCell.*` | Commandes : `Produce`, `Build`, `CancelConstruction`, `AttackMove`, `Stop`, `Hold`, `HeroAbility`. **Aucune commande « Rechercher »**. La grille des bâtisseurs réserve la rangée W X C V à la catégorie technologie (D30), vide. |
| Palier / niveau du héros | `HOLHeroXP::GetTier(Level) = Clamp(Level/3, 0, 3)` (`Simulation/HOLHeroXPRules.*`) ; `AHOLGameState::GetTier(Slot)` ; `FHOLGridContext::ReachedAge`, `HeroLevel` | **Existe** (paliers faits par la branche `tiers`, dans `main`). Les technologies peuvent lire le palier et le niveau du héros sans rien ajouter. |
| Statistiques modifiables | `Tools/Data/UnitTypes.csv` (Attack, MeleeArmor, RangedArmor, SiegeArmor, GatherRates, Speed...), `FHOLBuildingTypeRow` (MaxHealth...) | Valeurs lues dans les tables ; **aucun multiplicateur par joueur**. |
| Ressources, coût | `FHOLResources` (Food, Wood, Stone, Gold), `AHOLPlayerState`/stock par camp | Existe ; une recherche paiera comme une production (D282, D283). |
| File de production | `HOLProductionCommand.h`, `MaxQueue` (D283) | Existe pour les unités ; **pas de file de recherche**. |
| Script | `Script/Hero/*.as` (retour du héros), `Script/AI/*.as`, `Script/UI/*.as`, tests `*_Test.as` | Modèle à suivre pour les règles pures neuves (`namespace`, `FUnitTest`). |
| Technologies | grep `Technolog|Research|Tech` dans `Source/` et `Script/` | **rien** (hors `EHOLBuildingCategory::Technological`). |

## Plan (étapes de 15 minutes, une boucle = test rouge puis vert)

| Étape | Contenu | Langage | État |
|---|---|---|---|
| **T0** | Relevé de l'existant (ci-dessus) | : | **fait** |
| **T1** | Table `Tools/Data/Technologies.csv` des technologies 11.1 (paliers 1 et 2, communes + Aube) ; structure de ligne `FHOLTechnologyRow` ; import `Tools/Editor/make_technologies.py` vers `/Game/Technologies/Technologies` ; test `HeroesOfLiberty.Technologies.Table` qui lit le CSV par le même import que l'éditeur (`UDataTable::CreateTableFromCSVString`) et vérifie les trois conditions de chaque ligne, les chaînes I -> II, l'unicité, les bâtiments connus | C++ (une ligne de `UDataTable` ne s'écrit pas en script) | **fait** (20:48, import éditeur non lancé) |
| **T2** | Règle pure de disponibilité (palier ou niveau, bâtiment fini, coût payable, prérequis I -> II) en `Script/Technologies/HOLTechnologyRules.as` + `Script/Tests/HOLTechnologyRules_Test.as` (D483, D484) ; la lecture de la table par une liaison C++ reste à faire (T3) | Angelscript | **fait** (règle pure + 5 tests) |
| **T3** | État de recherche par camp (technologies faites, en cours), répliqué au propriétaire seul (comme la fiche privée du héros) ; sauvegarde du résultat dans la fin de partie | C++ (simulation, réseau) | à faire |
| **T4** | File de recherche d'un bâtiment (une à la fois ? voir T-Q6), coût pris au lancement, remboursé à l'annulation ; commande `Research` de la grille (cellule, grisée si non disponible, message de refus) | C++ + grille | à faire |
| **T5** | Effets sur les statistiques, **une famille par boucle** : d'abord l'économie (camps, ferme : bâtiments déjà là), puis le Centre ; la Forge et les autres quand leur bâtiment existera | C++ (simulation) | à faire |
| **T6** | Technologies propres à l'Aube (*Liturgie* lit l'Honneur ; *Bénédiction des armes* lit la catégorie « Profane » D393, donc après elle) | C++/Angelscript | à faire |
| **T7** | IA : recherche d'une technologie quand le stock le permet (après IA4) | Angelscript | à faire |
| **T8** | Interface (info-bulle, icône, libellés en trois langues), scénario `Tools/Scenarios/tech.txt`, GDD § 11 mis à jour, clôture | script + doc | à faire |

Dépendances : T5 pour la Forge, l'Académie, l'Atelier de siège demande ces bâtiments (chantiers à ouvrir, T-Q4) ; T6 demande la catégorie « Profane » (D393) pour *Bénédiction des armes* ; palier 3 (11.2) hors prototype (T-Q5).

## Questions posées (dans `Auto_Journey_Retours.md`, section 2, rubrique « Technologies »)

- **T-Q1** condition « niveau du héros » ou « palier » (le GDD dit les deux) ;
- **T-Q2** valeurs de départ (coût, durée, effet) : le GDD dit « indicatif, à régler en test » mais ne donne que Liturgie +10 % ;
- **T-Q3** bâtiment où se cherchent les technologies que le GDD ne place pas (propres à l'Aube, Brouette, Charrue avec le Moulin D346) ;
- **T-Q4** technologies dont le bâtiment n'existe pas encore (Forge, Académie, Atelier de siège, Chapelle, Sanctuaire) ;
- **T-Q5** lignes du palier 3 (11.2) dans la table maintenant ou plus tard ;
- **T-Q6** une seule recherche à la fois par bâtiment, ou une file comme les unités (D283).

**Réponses du 2026-10-30 : T-Q1 à T-Q6 = A (T-Q3 : Brouette à la Ferme), voir Suivi et GDD D583.**

Préparé derrière un réglage quand les deux options coûtent peu : T-Q1 (colonnes `MinTier` et `MinHeroLevel` dans la table : la règle prend le plus exigeant des deux, `MinHeroLevel` = 0 laisse le palier seul).

## Suivi

- 2026-10-30 21:4x (réponses de l'utilisateur, enregistrées par Auto_Journey, **rien de codé** ; GDD D583 et § 11) : **T-Q1 = A** (palier seul : `MinHeroLevel` = 0 partout, `MinTier` seul décide ; la colonne reste pour plus tard) ; **T-Q2 = A** (gabarit provisoire uniforme, déjà dans la table de T1, à régler à l'essai) ; **T-Q3 = A SAUF Brouette** : *Liturgie* et *Bénédiction des armes* au Sanctuaire, *Hallebardes trempées* à la Caserne, *Vœu de pauvreté* à la Chapelle, *Charrue* à la Ferme, et ***Brouette* à la Ferme (et non au Centre principal)** ; **T-Q4 = A** (table complète dès T1, les technologies dont le bâtiment n'existe pas restent injouables ; on code d'abord les 11 jouables : camps, ferme, Centre ; la Forge viendra comme chantier à part) ; **T-Q5 = A** (palier 3 de 11.2 plus tard : la table n'a que les paliers 1 et 2) ; **T-Q6 = A** (une file de 5 comme la production, `MaxQueue` D283, même code et même interface). La table `Tools/Data/Technologies.csv` est dans la branche `net-playtest` du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` : **non touchée ici** ; ligne `[ ]` en tête de la file de Glorfindel (placer Brouette à la Ferme, Charrue à la Ferme, puis T3 avec une file de 5).
- 2026-10-30 20:49 : **T1 fait (rouge puis vert)** et **T2 fait** (règle pure en script). Compilation `Tools\build.ps1` PASS (510 s, après une file d'attente). Rouge à 20:43 : `HeroesOfLiberty.Technologies.Table` échoue (« Technologies.csv is there » faux), les 5 tests script `HOLTechnologyRules_Test` verts, 188 tests de script verts. CSV posé (27 lignes, `Tools/Data/Technologies.csv`) : vert à 20:48 (`HeroesOfLiberty.Technologies.Table` Success, import par `CreateTableFromCSVString`). **Non fait** : l'import dans l'éditeur `make_technologies.py` (jamais lancé : l'asset `/Game/Technologies/Technologies` n'existe pas encore, et rien ne le lit avant T3) ; la liaison table -> règle script (T3). Suite : T3 (état de recherche par camp), après les réponses T-Q1 (règle) et T-Q6 (file) si possible.
- 2026-10-30 20:22 : écrits, pas encore compilés : `Source/HeroesOfLiberty/Technologies/HOLTechnologyTypes.h` (`EHOLTechCategory`, `FHOLTechnologyRow` : Category, Faction, Building, MinTier, MinHeroLevel, Cost, ResearchTime, Requires, Stats, Percent), `Tests/HOLTechnologyTableTests.cpp` (test `HeroesOfLiberty.Technologies.Table` : la grille 11.1 attendue est écrite dans le test, indépendante du CSV), `Tools/Editor/make_technologies.py`, et **T2 d'avance** (règle pure en script : `Script/Technologies/HOLTechnologyRules.as` + `Script/Tests/HOLTechnologyRules_Test.as`, 5 tests, ordre des raisons : déjà faite, palier, niveau, prérequis, bâtiment, coût). Le CSV (27 lignes, valeurs de gabarit T-Q2 = A) est gardé hors de `Tools/Data` pour que la première exécution soit **rouge** (fichier absent) ; compilation lancée à 20:30 par `Tools\build.ps1`.
- 2026-10-30 20:14 : T0 fait (relevé), plan écrit, questions posées ; T1 commencé (test rouge d'abord : la compilation n'est permise qu'après 20:30, `Tools\build.ps1` occupée par 4 autres agents).
