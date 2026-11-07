# Un son par type de bâtiment (AudioBuildingsInProgress)

Chantier ouvert le 2026-11-07 par ARWEN-12 (worktree HeroesOfLiberty-Arwen, branche `heal-sleep`, sans C++). Idée de l'utilisateur du 2026-11-06 (voir la fin de `AudioInProgress.md`) : à la sélection d'un bâtiment (le cue générique `Select.Building` est **muet** depuis son choix du 2026-11-06), jouer un son **propre au type** ; peut-être aussi la fin de construction par type. Aucun numéro de décision D ici : la session principale les attribue.

## Règle de départ : ce que l'utilisateur a voulu muet reste muet

Un son qu'il a voulu muet le 2026-11-06 le reste **tant qu'il ne le redemande pas** : `Select.Building` (générique), `Alert.BuildingDone`, `Alert.TechDone` (aussi muets : survol, bascule, tic, défilement, les 5 ordres). Donc l'étape 1 ne fait **rien entendre de nouveau** : chaque bâtiment pointe vers l'événement qu'il sonne aujourd'hui. Un bâtiment ne sonne qu'après une réponse de l'utilisateur (`Retours\Q-audio-buildings.md`).

## Ce qui existe

- `Script/Audio/HOLAudioTable.as` : 24 événements `HOLAudioEvent::*`, dont `SelectBuilding` (16, muet), `AlertBuildingDone` (18, muet), `AlertTrainingDone` (19, joue `bookPlace2`, à valider : l'utilisateur voulait du martial, essai `E-audio_compare_martial`), `AlertTechDone` (20, muet), `AlertTrade` (21, `handleCoins`). **Aucun événement « bâtiment détruit »** dans la table.
- Le branchement dans le jeu : `UHOLGameScreenScript` n'appelle `Sound(...)` que pour l'interface (portrait du héros, pause, duel, talents, Diplomatie, volcan). Sélection, fin de construction, fin d'entraînement, technologie et commerce naissent dans le C++ du contrôleur et des sous-systèmes : **ils ne sont pas branchés** (relevé de `AudioInProgress.md` A2b, non revérifié ici). Le jeu n'entend donc encore aucun de ces cinq moments.

## Les types de bâtiments (`Tools/Data/BuildingTypes.csv`, 14 lignes) et leurs moments sonores

Colonnes : sélection (S), construction finie (C), production finie (P), destruction (D). Événement qu'ils sonnent **aujourd'hui** (défaut codé) ; « muet » = cue muette voulue par l'utilisateur.

| Type | Nom | Produit | S | C | P | D |
|---|---|---|---|---|---|---|
| MainCentre | Centre principal | Peasant, Zombie | Select.Building (muet) | Alert.BuildingDone (muet) | Alert.TrainingDone (joue) | aucun événement |
| House | Maison | rien | muet | muet | aucun | aucun |
| LumberCamp | Camp de bûcherons | rien | muet | muet | aucun | aucun |
| MiningCamp | Camp de mineurs | rien | muet | muet | aucun | aucun |
| Mill | Moulin | rien | muet | muet | aucun | aucun |
| Farm | Ferme | rien | muet | muet | aucun | aucun |
| Barracks | Caserne | Spearman, ManAtArms | muet | muet | Alert.TrainingDone (joue) | aucun |
| ArcheryRange | Champ de tir | Archer, Crossbowman | muet | muet | Alert.TrainingDone (joue) | aucun |
| Stable | Écurie | LightCavalry | muet | muet | Alert.TrainingDone (joue) | aucun |
| SiegeWorkshop | Atelier de siège | Ram, Mangonel | muet | muet | Alert.TrainingDone (joue) | aucun |
| Chapel | Chapelle (Aube) | Monk | muet | muet | Alert.TrainingDone (joue) | aucun |
| Sanctuary | Sanctuaire de l'Aube | technologies | muet | muet | Alert.TechDone (muet) | aucun |
| Forge | Forge | technologies | muet | muet | Alert.TechDone (muet) | aucun |
| Market | Marché | commerce | muet | muet | Alert.Trade (joue `handleCoins`) | aucun |

Les types viennent de la colonne `Produces` et de la catégorie de `BuildingTypes.csv` ; la table `HOLAudioBuildings.as` en reprend les 14 noms.

## Plan en étapes de 15 minutes (la plus petite d'abord)

- **B1 (faite le 2026-11-07, sans C++)** : table `bâtiment -> événement` pour les 4 moments, en règles pures. Test rouge `Script/Audio/HOLAudioBuildings_Test.as` (5 tests : noms réels du tableau de types, sélection et construction muettes sur la vraie chaîne `HOLAudio::Plan`, production = cues d'aujourd'hui, destruction et inconnus sans cue, cue = celle de la table) : rouge vu (« Namespace 'HOLAudioBuildings' doesn't exist », d98f9e1), vert `quick_check -Parts tests` 1067/1067 (abe1ad4, `Script/Audio/HOLAudioBuildings.as`). Aucun changement de ce qu'on entend.
- **B2 (FAITE le 2026-11-07, ARWEN-13, sans C++ ; test rouge 0184a50, vert 7d1b5ef)** : contrôle des données. `Tools/Audio/check_audio_table.ps1` compare les noms de `HOLAudioBuildings.as` à `BuildingTypes.csv` (14 = 14, aucun oubli, aucun nom inventé ; un bâtiment qui a des `Produces` ne peut pas avoir « aucun » pour P). Test rouge d'abord : un faux CSV avec un 15e bâtiment doit sortir FAIL.
- **B3 (attend la réponse de `Q-audio-buildings.md`)** : un événement par moment propre (ex. `HOLAudioEvent::BuildingSelect<Famille>`), une cue par famille ou par type dans la table, **muette** au départ ; test rouge : le bâtiment choisi sonne l'élu, les autres restent muets.
- **B4 (dix échantillons)** : méthode `assets-ten-samples-method` : un scénario `audio_compare_buildings` (dix variantes par famille de bâtiment, cues de bois, pierre, métal, cloche, forge ; Kenney `impactWood`, `impactPlank`, `doorOpen`, `metalLatch`, `impactMining`, `chop`), l'utilisateur dit aimé / pas aimé ; fiche `Retours\E-audio_compare_buildings.md` avec `GESTE:` et `SON:`.
- **B5 (C++, autre agent)** : transmettre au script les moments avec le **type** du bâtiment (sélection, construction finie, production finie, destruction) ; appeler `HOLAudioBuildings::EventFor(type, kind)` puis `UHOLAudioPlayer.Play`. Hors de ce chantier tant qu'un créneau C++ n'est pas donné.
- **B6** : plafonds de sons (A3 d'`AudioInProgress.md`) pour la destruction en masse, et écoute en jeu.

## Questions

- **Q-audio-buildings** (`C:\Users\baudo\Documents\MyCrusader\Retours\Q-audio-buildings.md`, une seule question groupée, posée le 2026-11-07) : quels moments sonner, pour quels bâtiments. **Défaut codé : tout muet comme aujourd'hui**, une ligne par bâtiment dans `HOLAudioBuildings::Build`. Réponse attendue de l'utilisateur ; ne rien trancher avant.

## Journal

- 2026-11-07 09:50-10:00 (ARWEN-12, `heal-sleep`, sans C++) : chantier ouvert ; B1 faite (d98f9e1 test rouge, abe1ad4 vert, 1067/1067) ; question posée. Non vérifié : le branchement C++ des cinq moments (relevé d'`AudioInProgress.md`), l'écoute en jeu (aucun son entendu).
- 2026-11-07 10:0x (ARWEN-13, `heal-sleep`, sans C++) : **B2 faite.** `Tools/Audio/check_audio_buildings.ps1` (options `-Csv`, `-Table`) lit `BuildingTypes.csv` et `HOLAudioBuildings.as` et sort FAIL si un type manque, est en trop, mal orthographie ou en double, ou si un type qui a des `Produces` dit -1 pour sa production ; il ne lit aucun choix de son (aucun design). Test `Tools/Audio/check_audio_buildings_test.ps1` (8 faux CSV / fausses tables copies de la vraie chaine) : rouge d'abord (0184a50, script absent), vert 8/8 (7d1b5ef) ; passage reel : 14 types / 14 lignes PASS. Non verifie : rien dans le jeu (aucun C++, aucun son).
