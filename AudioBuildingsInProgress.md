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

## Reponse de l'utilisateur, 2026-11-07 10:36 (enregistree par ARWEN-14)

Q3 = A : selection seule, un son par FAMILLE de batiments (bois : Maison, camps, Moulin, Ferme ; pierre : Centre principal, Caserne, Champ de tir, Ecurie, Atelier de siege ; sacre : Chapelle, Sanctuaire ; metal : Forge ; pieces : Marche), environ 5 sons ; construction finie, production et destruction restent muettes. `Retours\Q-audio-buildings.md` est marque « Reponse: A (utilisateur 10:36) ».

## B3/B4 (FAITES le 2026-11-07, ARWEN-14, sans C++ ; test rouge def0095, vert 8f3a27a)

- **Donnees** (`Script/Audio/HOLAudioBuildings.as`) : table `famille -> cue` (`Families()`, `CueNameOfFamily`) = Wood/Stone/Sacred/Metal/Coins -> `Building.<Famille>` ; table `batiment -> famille` (`FamilyOf`, les 14 types) ; `CueNameFor(batiment)`. Les cues de famille ne sont PAS des evenements de `HOLAudioTable.as` : `Select.Building` reste **muet** (le jeu entend exactement la meme chose qu'avant), tant qu'aucun son elu n'est branche.
- **Candidats** (`Script/Audio/HOLAudioCandidates.as`, `BuildBuildings`, lus par `ForCue` mais pas comptes par `CueCount`) : dix variantes A a J par famille, 50 au total, tous deja importes (Kenney rpg-audio et impact-sounds, VSCO 2 CE, tous CC0, `Docs\assets_audio.csv`) : bois = impactWood/impactPlank, chop, porte, creak, claves ; pierre = impactMining, impactPunch, impactSoft, timbale, maillet ; sacre = cloches impactBell/v_nbell, glockenspiel, harpe, triangle ; metal = impactMetal/impactPlate, metalPot1, metalLatch, gong, impactTin ; pieces = handleCoins, impactTin, tambourin, triangle, cuir (handleSmallLeather, dropLeather, beltHandle1).
- **Tests** (`Script/Audio/HOLAudioBuildingFamilies_Test.as`, 5 tests) : cinq familles, chaque batiment a sa famille (regroupement de l'utilisateur), noms de cue, dix candidats distincts par famille, EFFET : la selection ne joue rien sur le vrai plan (`HOLAudio::Plan`, 14 batiments x 10 graines). Rouge vu (« No matching signatures to HOLAudioBuildings::FamilyCount() »), vert `quick_check -Parts tests` 1084/1084. `check_audio_buildings.ps1` PASS (14 = 14), `check_audio_table.ps1` PASS.
- **Scenario** `Tools/Scenarios/audio_compare_buildings.txt/.json/.look` (genere par `Tools/Audio/make_audio_compare.py --only audio_compare_buildings --retours <Retours>`, groupe `Building.`, 50 etapes) et `Retours\E-audio_compare_buildings.md` (une ligne `<Famille> = _` par famille, `branches: heal-sleep`, GESTE: interne, SON: non entendu). `preflight.ps1` PASS (79 s, 50 « candidate played », 0 missing, PNG ouvert). Statut LOGIQUE SEULEMENT : personne n'a ecoute ; duree reelle environ 11 min (au-dessus de 3 min, a ecouter famille par famille).
- **B5 (C++, a faire par un autre agent, NE PAS CODER ICI) : le besoin.** (1) Quand le joueur selectionne UN bâtiment (clic simple ou groupe a un seul bâtiment), le controleur C++ doit transmettre au script le **type** du bâtiment (nom comme `BuildingTypes.csv`, ex. `Barracks`), comme il le fait deja pour le portrait du heros ; (2) le script appelle `HOLAudioBuildings::CueNameFor(type)` pour la famille, retrouve l'evenement elu (a creer apres l'ecoute : un evenement `HOLAudioEvent::SelectBuilding<Famille>` par famille dans `HOLAudioTable.as`, muet tant que l'utilisateur n'a pas choisi) puis `UHOLAudioPlayer.Play` ; (3) rien pour la construction finie, la production ni la destruction (muettes, reponse Q3 = A) ; (4) plafond de sons (B6) : un seul son de selection a la fois, pas de repetition si le meme bâtiment est reclique en moins de ~200 ms (valeur a regler, cvar). Prerequis : l'ecoute de `E-audio_compare_buildings` (une lettre par famille) puis l'election (cue + fichier dans `HOLAudioTable.as`, import du `.uasset` hors `Candidates`).
- **Non verifie** : aucun son entendu (50 candidats jamais ecoutes) ; le branchement C++ de la selection (releve d'`AudioInProgress.md`, non revu) ; que la fusion de main et ce scenario marchent avec le binaire de main (le pre-vol a joue le binaire du 09:00 de heal-sleep).

## Compteur d'appels d'ARWEN-14 (un trait par appel declare)

(32 traits environ au moment de cette ecriture ; le releve fait foi est le champ tool_uses de la session principale)