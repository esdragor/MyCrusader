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
## 2026-11-07 12:1x-12:4x (ARWEN-15, heal-sleep, sans C++) : sons PAS USANTS, 3e tour de Alert.TrainingDone et les 5 familles de batiments

- Consigne (memoire "Sons : pas usants", reponses du 2026-11-07) : graves, sourds, courts, doux ; pas de trompette, cor, clochette, aigus. Les 10 variantes martiales et les 50 candidats du matin sont remplaces (Alert.TrainingDone et Building.* de HOLAudioCandidates.as ; la ligne Alert.TrainingDone sert aussi au scenario audio_compare_alerts, regenere).
- Test rouge d'abord : b477691 (Test_AudioCandidatesSoftCuesAreSoft : 6 cues, 60 stems en s_, dossier soft, aucun mot cor/trompette/cloche/glock/triangle/tambourin ; check_audio_soft.py qui MESURE les WAV : duree <= 0,8 s, centre spectral <= 800 Hz, <= 1 % d'energie au-dessus de 3 kHz ; autotest : une trompette et un glockenspiel sont refuses) ; rouge vu 	ests 1089/1091 (2 echecs) et check_audio_soft FAIL (aucun s_ ). Vert 9bce044 : 	ests 1091/1091, check_audio_soft PASS sur 60 sons.
- Outils : Tools/Audio/make_soft_candidates.py (60 recettes : source CC0 Kenney ou VSCO, silence coupe, hauteur baissee, 3 passe-bas, coupe, fondu, pic -4 dB ; ffmpeg + numpy) ; stems s_* -> dossier soft (HOLAudioCandidates::Folder, import_audio_candidates.py, check_audio_candidates.ps1). 11 recettes ont d'abord echoue a la mesure (pieces, claves, gong) : filtres plus bas ou source remplacee (claves -> claquoir, ceintures -> cuir/etain/bois), seuils jamais touches. Correctifs d'outils au passage : check_audio_candidates.ps1 accepte les 5 cues de famille Building.* (il etait FAIL depuis B4), check_audio_compare.ps1 lit les lignes de feuille Libelle [Cue] = _ (FAIL depuis B4).
- Scenarios : udio_compare_training (10 sons, remplace audio_compare_martial, supprime) et udio_compare_buildings (50 sons) ; feuilles Retours\E-audio_compare_training.md et E-audio_compare_buildings.md (LOGIQUE SEULEMENT, SON: non entendu, GESTE: interne, branches: heal-sleep, consigne et mesures en tete). 60 .uasset importes dans Content/Audio/Candidates/soft (non suivis, jamais commites : D736). preflight PASS pour les deux (37 s / 74 s, 10/10 et 50/50 candidate played, 0 missing, PNG _start ouverts).
- Non verifie : AUCUNE ecoute ; la consigne pas usant n'est verifiee que par mesure du spectre ; l'avance reelle par Entree. Question : aucune (rien tranche, le jeu entend toujours bookPlace2). Suivant : l'utilisateur ecoute audio_compare_training puis audio_compare_buildings (une lettre par cue ou dix autres) ; ensuite election dans HOLAudioTable.as (Alert.TrainingDone) et B5 (C++).

## 2026-11-07 12:4x-12:5x (ARWEN-16, heal-sleep, sans C++) : essais audio decoupes a 3 min (regle 18), commit 5311940

- FAIT : audio_compare_buildings (50 sons, 11 min) -> 5 scenarios audio_compare_buildings_<wood|stone|sacred|metal|coins> (10 sons, ~2 min, Entree = suivant, waitkey, le panneau dit le cue et la lettre) ; preflight PASS x5 (31-40 s, 10/10 candidate played, 0 erreur), PNG _start ouverts avec Read (panneau lisible ; capture metal : avertissement passager 'Shaders en preparation'). Fiches Retours\E-audio_compare_buildings_<famille>.md (GESTE: interne, SON: non entendu, branches: heal-sleep, ligne 'Cue = _', liste des lettres avec fichiers) ; E-audio_compare_buildings.md = INDEX vers les 5 fiches. L'ancien 50 sons est GARDE comme 'tout' (premiere ligne 'non compte', pas de fiche). Idem audio_compare, _orders, _alerts (100, 70, 70 sons) marques 'tout, non compte'.
- audio_compare_alerts decoupe en 6 scenarios audio_compare_alerts_<attack|buildingdone|techdone|trade|volcano|diplomacy> (10 sons chacun, preflight PASS x6, 10/10 candidate played) mais NON LISTES, sans fiche : leurs candidats sont les anciens medievaux (cloches, harpe, glockenspiel) que la consigne 'pas usants' rejette ; a refaire doux (make_soft_candidates.py) puis regenerer avec make_audio_compare.py --retours.
- Outils : make_audio_compare.py (groupes par famille et par alerte, 4e element False = pas de fiche, 'cue' au singulier ('1 cues' corrige), duree = n*12 s arrondie, lettres des fichiers dans la fiche) ; check_audio_compare.ps1 (premiere ligne 'non compte' = pas de fiche ; sinon max 15 sons, regle 18) : PASS, 16 scenarios ; check_audio_candidates.ps1 PASS.
- Non verifie : aucune ecoute ; Entree reelle ; duree reelle de 2 min (estimee 12 s par son) ; la fiche audio_compare_training.md dit encore 'environ 3 min' (non regeneree pour ne pas ecraser son pre-vol).
- Suite : l'utilisateur ecoute les 5 fiches ; election dans HOLAudioTable.as ; B5 (C++) ; candidats doux pour les 6 alertes.

## 2026-11-09 16:2x-16:3x (Arwen, d751-root, sans C++) : ligne 21 deja faite, mesure de l'ecretage, scenario regenere

- CONSTAT : la ligne 21 d'Auto_Journey_File.md etait PERIMEE. Bois F et Sacre H sont elus a x2,5 (7a1dc25), Marche = handleCoins (1a4008f), dix nouveaux candidats Pierre (`s_stone2_*`) et Metal (`s_metal2_*`) existent (scenarios `audio_compare_buildings_stone|metal`, panneau `audio_panel` 96 sons, 8a38b56), B5 C++ fait par Galadriel-11 (selection cablee, Pierre/Metal muets). Rien a refaire ; verifie sur disque : check_audio_table PASS (27 cues, 13 muets), check_audio_buildings PASS (14 = 14), check_audio_candidates PASS (320 stems), check_audio_soft PASS (120 sons, python SANS -I : numpy est dans le site-packages utilisateur).
- CORRIGE : `check_audio_compare.ps1` etait FAIL (audio_compare_orders.txt perime : 70 etapes au lieu de 80 du generateur) ; scenario regenere par `make_audio_compare.py --only audio_compare_orders` (f349f4c), check_audio_compare PASS (16 scenarios). Non rejoue (preflight non lance : scenario ancien « tout, non compte »).
- NOUVEAU : `Tools/Audio/check_audio_headroom.py` (ffmpeg volumedetect sur les vrais fichiers + gain lu dans HOLAudioTable.as ; `--selftest` : une tonalite a -4 dB ne clippe pas a 100 % et clippe a 250 %, parser de table sur une table reduite). MESURE : les trois sons elus ECRETENT a x2,5 (Bois +4,0 dBFS, Sacre +4,0, Marche +8,0 car handleCoins culmine a 0 dB). Avec un limiteur a -1 dBFS le gain reel n'est que +2 a +5 dB. Rien change dans le jeu ; question `Retours\Q-audio-select-gain.md` [A REPONDRE] (defaut A = x2,5 inchange). Outil ecrit en meme temps que son autotest (pas de commit rouge separe).
- Technologies : `TechnologiesInProgress.md` (39,6 K) et `TalentsInProgress.md` (40,6 K) EXISTENT deja : rien ouvert.
- NON VERIFIE : aucun son entendu ; l'ecretage est mesure sur les fichiers et le gain, pas dans le mixeur du jeu (Unreal peut avoir un limiteur de sortie) ; preflight de audio_panel non relance (non touche).
