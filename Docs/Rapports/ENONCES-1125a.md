# ENONCES-1125a (étape 0, séance autonome 2026-11-25 15:04 -> 19:04, dernier lancement de code 18:04)

Auteur : ETAPE0-1125a, lecture seule, relevé 15:11-15:19 (Get-Date). main = bcedc73 (docs ; dernier code = 78375b3 `Merge branch 'main' into integ-82`). Priorité de la séance : `Docs\Priorite_Socle_Factions_2026-11-16.md` (lu en entier). Chaque agent : « lis `Tools\AutoJourney\brief_commun.md` puis `agent_rules.md` et applique-les » ; arrêt 15 min ou 25 appels. Aucun agent ne build : la session principale compile (un build à la fois).

## 0. Relevé git (L22-4, L27-6 : `git merge-base --is-ancestor` et `git grep` sur main ET sur les branches non fusionnées)

- DANS main : 7e0613c (`hol.Cheat.Resources` tous les slots), b7345ad (`SetLanguageName`), ba1cd7e (panneau de spécialisation localisé), a99f9ad (correctif portes G10 `GateCells`), 4ba5ec2 (rouge G10), 0ff98f8 (pont C++ `UHOLGridActionBridge`), a6cb051 (HUD : la case du Sceau redessinée avec le temps, **C++ NON COMPILÉ**). Donc `integ-79` (tip 79af31d, +1 : un merge) et les branches grid-g3 / walls-g7 / morale sont refondues dans main.
- ABSENT de main : cb854ad (empreinte visible des pièces de mur, branche `wall-footprint-1125m`, **C++ NON COMPILÉ**, test rouge 8dd4b9e) ; branches non fusionnées utiles : `spec-voie-1125m` (+8, WIP d43fc39), `wall-footprint-1125m` (+3), `market-quote-1125s` (+3, **C++ NON COMPILÉ**, 3436577 rouge, 6919000), `tools-l288-1125m` (+4, L28-8), `tower-wood-shaders-1125m` (c8f4c02 = revert d'un essai ; le correctif utile `waitshaders` est DÉJÀ dans main : bcf3b84, eb1d4f0) ; le reste de `git branch --no-merged main` (integ-31 à integ-68, integ-79/79b/80, launcher-*, fog, selection, visual, envelope...) est hors socle, non relu.
- Pas fait dans main (Grep vide) : `SubCellCostPercent` (D915) ; `FHOLAITowerSettings.EasyMax` vaut encore 0 (`Script\AI\HOLAITower.as:11`, D918) ; aucun effet de `KnightChallenge` dans `Source\` (Grep vide ; seule la table `Tools\Data\UnitAbilities.csv:4` et `HOLDawnValues.as:50-64`).
- PÉRIMÉES dans `Auto_Journey_File.md` (code dans main, à retirer au prochain nettoyage, pas touchées ici) : l.18 D908, l.19 D911, l.20 D909 (Priorité : 5b7ddb6, 0da0a6f, 8bc37e7) ; l.40 et l.41 (aperçu de visée D900 : `HOLFactionAim::PreviewOf` existe déjà, tests `HOLFactionAim_Test.as:57-74`) ; l.151-153 (`HOLMoraleView::Read` et `morale_look`, essai rendu OK à 14:3x) ; l.93 (mur : correctif sur branche, non compilé, ligne à garder).
- SESSIONS ÉTRANGÈRES POSSIBLES (à dire à l'utilisateur) : **Faramir** `market-quote-1125s` : `IN_USE.txt` « pris : CLAUDE-1125s lot C++ marche quote + armements morts (session principale, 14:55) », commits 14:59, 15:09 et merge de main 15:10 (après le début de séance 15:04) : session vivante hors plan, NON COMPILÉE, aucun agent dans ce worktree. **Elrond** `wall-footprint-1125m` : `IN_USE` « libre : ELROND-1125mf a fini (14:01) », commits `Agent: ELROND-1125mf` 14:00 / 14:01, merge de main 14:36 par la sync ; `status.ps1` dit « AUTRE SESSION ? » parce que le worktree n'est plus dans `## Agents du` (section du matin archivée) : plutôt la séance de 14:00 de l'utilisateur que du travail d'aujourd'hui, **non tranché** ; ne rien y lancer ni y fusionner.
- Charge : `status.ps1` signale BUILD LENT (Elrond, 14:15 : 687 s pour 21 actions, médiane 152 s). Prévoir 8 à 12 min par build.

## 1. Énoncés par worktree

### 1.1 Arwen (`spec-voie-1125m`, +8, IN_USE libre) : D893 étape 3 / D902 : colonne `Produces` des 3 unités de commanderie
- Tâche (file l.79 et l.121) : vérifier puis finir le WIP d43fc39 : bâtiment de l'unité copiée (Q-commanderie-batiment = A, D902) pour Sergent du Temple, Frère infirmier et la 3e. Priorité : ordre 2/4 (effet des voies : sans cela une voie choisie ne produit rien).
- Lire (UN appel) : `Tools\Data\BuildingTypes.csv` (colonne `Produces`), `Tools\AutoJourney\regen_tables.ps1`, `Script\Tests\HOLCommandGrid_Test.as` (`Test_CGTables`, cellule 5 Barracks / 3 ArcheryRange), `Script\Tests\HOLProductionQueue_Test.as`, `Script\Tests\HOLConstruction_Test.as:1218` (Sanctuaire : 0 production), `SpecializationInProgress.md` (ARWEN-1125mf : le CSV seul fait échouer `Test_CGTables`, `GetProducesOf` lit la table COMPILÉE).
- Premier test rouge : après `regen_tables.ps1`, `quick_check.ps1 -Parts tests` (celui du worktree) ; le rouge attendu est `Test_CGTables` + le test de `SeatMayProduce` (2b51397 : une unité de la voie choisie est produite par le bâtiment de l'unité copiée). Rouge d'abord seul en commit, correctif ensuite.
- Sans C++ (CSV + tests script). Si la question du Sanctuaire (un Templier a-t-il le Sanctuaire ?) bloque : `Retours\Q-sanctuaire-templiers.md`, défaut codé = pas de production, continuer.
- Fichiers pris : `Tools\Data\BuildingTypes.csv` (+ son `.uasset` régénéré), `Script\Tests\HOLCommandGrid_Test.as`, `HOLProductionQueue_Test.as`, `HOLConstruction_Test.as`, `SpecializationInProgress.md`.

### 1.2 Celeborn (`wall-long-fix-1125m` = main, IN_USE libre) : D915, prix des murs sur la grille fine
- Tâche (file l.11) : `ServerPlaceWallLine` déduit le prix d'une case / 4 par sous-case de 1 m (palissade : 20 m = 25 Bois), cvar `hol.Wall.SubCellCostPercent` (défaut 25) ; les trous (D901) ne sont pas payés. Ordre 4 de la Priorité (« déduction branchée »).
- Lire : `Script\Net\HOLRpcPlayer.as:1540` (`ServerPlaceWallLine`), `Script\Buildings\HOLWallLineRules.as:17` (modèle de cvar : `CVar_WallDiagonalFill`), `:79` (`StockAfterLine(Stock, Plan, CostPerCell)`), `:359` (règle du RPC à tout SubDivision), `Script\Tests\HOLWallLineRules_Test.as:522-535` (tests existants de `StockAfterLine`), `FortificationsInProgress.md` (CELEBORN-1125ma, DOCS-1125me).
- Premier test rouge : `Test_WallLineCostsAQuarterPerSubCell` : plan de 20 sous-cases, palissade à 5 Bois, stock 100 -> le stock après la ligne vaut 75 (PAS 0), calculé par la règle (arrondi sur le total, pas par sous-case : 1,25 par sous-case) ; bouchon = fonction `CostOfLine` qui rend l'ancien prix plein. Un 2e test sur la vraie chaîne (le RPC, pas la règle seule) si `HOLRpcPlayer.as` le permet ; sinon le dire (`TESTS_TAUTOLOGIQUES`).
- Sans C++. Question de valeur de l'arrondi : défaut codé = arrondi au supérieur sur le total, une ligne.
- Fichiers pris : `HOLWallLineRules.as`, `HOLWallLineRules_Test.as`, `HOLRpcPlayer.as` (ligne 1540 seule), `FortificationsInProgress.md`. À la fin : ligne `Placeholders.txt` seulement si un provisoire naît.
- **Place C++ 2 réservée à CE worktree, lancement à 17:00-17:15 au plus tard** : D917 (file l.12, mur en diagonale lisse, pièces à 45 degrés), tranche 1 = orientation par pièce + empreinte non alignée (`HOLConstruction::GetPieceFootprint`, cb854ad). Raison C++ : code appelé par entité (obstacle navmesh tourné, collision). **Condition** : wall-footprint-1125m compilée et intégrée (sinon repli : test de `hol.Cheat.Resources` à deux slots, C++ de test, Priorité « RESTE C++ »).

### 1.3 Galadriel (`morale-look-fix-1125m` = main, IN_USE libre) : D918, tours de guet de l'IA (Facile 1, Normal 1, Difficile 2)
- Tâche (file l.39). Ordre 5 de la Priorité (IA). Faramir étant occupé, l'IA va à Galadriel.
- Lire : `Script\AI\HOLAITower.as:8-12` (`FHOLAITowerSettings` : EasyMax 0 -> 1, NormalMax 1, HardMax 0/2 -> 2), `:29-33` (`MaxFor`), `Script\AI\HOLAITower_Test.as:5` (`Test_AITowerQuantityByLevel`), `:19` (`Test_AITowerFirstTowerByLevel`), `AIInProgress.md` (fin : DOCS-1125me).
- Premier test rouge : dans `Test_AITowerQuantityByLevel`, l'IA Facile veut 1 tour (aujourd'hui 0) et `Test_AITowerFirstTowerByLevel` : l'IA Facile pose une tour après `MinSoldiers` soldats ; rouge sur la valeur lue par `MaxFor`, pas recopiée.
- Sans C++. Ensuite (si temps) : l.40-41 à vérifier : le dessin de l'aperçu de visée pendant l'armement (`HOLFactionAim::PreviewOf` existe ; `DrawZoneCircles` de `HOLFactionPowerServer.as:416` ne dessine qu'après le lancer) ; dire si l'aperçu est déjà tracé à l'écran, sinon test + tracé script.
- Fichiers pris : `HOLAITower.as`, `HOLAITower_Test.as`, `AIInProgress.md`.

### 1.4 Sauron (`integ-82` = main, IN_USE libre) : effet de `KnightChallenge` (Aube A7, **PLACE C++ 1**)
- Tâche (file l.63 ; Priorité ordre 2 « reste C++ »). Raison C++ (D839) : règle de simulation par entité et par pas (les ennemis à 800 cm du Chevalier sont défiés 8 s : leur choix de cible dans le combat Mass ; ralentissement). Un seul build, dans le lot C++ de 16:00.
- Lire : `Tools\Data\UnitAbilities.csv:4` (ligne `KnightChallenge`, `Self`, 800 cm, 8000 ms, 45000 ms), `Script\Factions\HOLDawnValues.as:50-64`, `Source\HeroesOfLiberty\Combat\HOLCombatProcessor.cpp` (choix de cible ; Grep `Taunt` vide : rien n'existe), le cadre d'effets du moral (`HOLMorale::Apply` / `MakeEffectFromRow`, modèle d'un effet temporaire sur l'unité), `DawnInProgress`/`AubeInProgress.md` A7.
- Premier test rouge : test Mass C++ (D838, `HeroesOfLiberty.Combat.KnightChallengeTargetsTheKnight`) : un ennemi à 600 cm attaque le Chevalier pendant 8 s même si un autre allié est plus proche, puis reprend sa cible normale ; bouchon qui compile + test dans UN commit, rouge vu à l'exécution par la session principale, correctif en 2e commit (NON COMPILÉ au rapport).
- Conception non tranchée (provocation forcée ou pondérée, ralentissement oui / non) : `Retours\Q-defi-chevalier.md`, défaut codé derrière un cvar, une ligne.
- Fichiers pris : `HOLCombatProcessor.cpp` (zone de choix de cible seule), nouveau `Source\...\Tests\HOLKnightChallengeTests.cpp`, `AubeInProgress.md`. Ligne `Placeholders.txt` si effet visuel provisoire.

### 1.5 Glorfindel (`recette-1125mb` = main, IN_USE libre) : RECETTEUR (L1-14, Q5)
- Tâche : recette indépendante de main (78375b3, passe integ-82 : waitshaders, morale-look-fix, wall-long-fix, monk-seal-cooldown). Rejouer `play.ps1 <nom> -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` : `tower_wood` (le FAIL RECETTE-1125ma était l'ancien cadrage), `wall_line_long`, `morale_look`, `monk_seal` (cooldown : a6cb051 est NON COMPILÉ : le binaire doit être refait d'abord), `duel_challenge_accept` (ARWEN-1125mb : PASS). Chaque PNG ouvert et vu, verdict VU À L'ÉCRAN ou LOGIQUE SEULEMENT.
- Lire : `Docs\Rapports\RECETTE-1125mb.md`, `RECETTE-1125mc.md`, `RECETTE-1125md.md`, `Retours\` (fiches E restantes), `Tools\Scenarios\preflight.ps1`.
- Premier geste : `Get-StaleBinaryProblem` du worktree ; si périmé, le dire (« recette impossible, build demandé à la principale ») et se limiter aux tests script (`quick_check.ps1` du worktree, `-Parts tests`).
- Pas de code ; fichiers pris : `Docs\Rapports\RECETTE-1125n*.md` seulement.

### 1.6 Haldir (`monk-seal-cooldown-1125m` = main, IN_USE libre) : INTÉGRATEUR
- Passes `Tools\AutoJourney\integrate.ps1` (nouvelle branche integ-83 et suivantes), calendrier : **P1 vers 16:15** (arwen `spec-voie-1125m` + celeborn D915 + galadriel D918 si verts ; `wall-footprint-1125m` seulement après décision Elrond / session étrangère et compilation), **P2 vers 17:30**, **P3 vers 18:15** (après L18-c). `market-quote-1125s` : seulement quand la session CLAUDE-1125s a rendu la main (`IN_USE` libre) ; `tools-l288-1125m` : vérifier d'abord `git cherry main tools-l288-1125m` (L28-8 906ff27 / f1f65d0 est déjà dans main sous d'autres shas) ; `tower-wood-shaders-1125m` : NE PAS intégrer (revert, correctif déjà dans main).
- Tâche du lancement de 15:30 (sans C++) : lecture seule des branches à intégrer (`git cherry`, conflits probables sur `Placeholders.txt` et `BuildingTypes.csv`), liste d'intégration écrite au journal ; aucun merge avant que les agents aient rendu.
- Fichiers pris : `Docs\Rapports\HALDIR-1125n*.md`, ses `*InProgress.md`.

### 1.7 Elrond (`wall-footprint-1125m`, +3) : NE PAS LANCER
- Voir §0 : `AUTRE SESSION ?` non tranché, 3 commits NON COMPILÉS, `Agent: ELROND-1125mf`. À faire par la principale, hors agent : build du lot (cb854ad + a6cb051 + market-quote), rejouer `wall_fine_ghost`, puis intégration (Haldir P1).

### 1.8 Faramir (`market-quote-1125s`, +3) : NE PAS LANCER
- Session étrangère vive (`IN_USE` CLAUDE-1125s 14:55, commits 14:59 / 15:09 / 15:10, merge de main à 15:10) ; lot C++ NON COMPILÉ (prix du marché par le script, armements morts retirés, G-5c) ; ne pas y fusionner main (règle ForeignSession).

## 2. Plan C++ de la séance (L27-11) : 2 places au plus

| Place | Worktree | Tâche | Raison C++ | Lancement | Build |
|---|---|---|---|---|---|
| 1 | Sauron | effet de `KnightChallenge` (A7) | Mass, par entité et par pas | 15:30 | lot de ~16:00 |
| 2 | Celeborn | D917 mur diagonale, tranche 1 (ou repli : test `hol.Cheat.Resources` à 2 slots) | par entité (navmesh, collision) | 17:00-17:15, après D915 | avant 18:04 (L18-c) |

Builds déjà dus hors des 2 places (compilés par la session principale) : a6cb051 (HUD Sceau), cb854ad (empreinte des murs), 6919000 (marché) : à grouper dans UN build dès 15:30, avant le lot de Sauron.

## 3. Intégrateur / recetteur

- INTÉGRATEUR : **Haldir** (continuité : HALDIR a fait integ-77 à integ-81 ce matin).
- RECETTEUR : **Glorfindel** (worktree `recette-1125mb`, différent de l'intégrateur, Q5).
