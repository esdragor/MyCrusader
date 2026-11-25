# ENONCES-1125a-v2 (vague 2, séance autonome 2026-11-25 jusqu'à 19:04, dernier lancement de code 18:04)

Auteur : PREP-1125a, docs seulement, relevé 15:31-15:40 (Get-Date). main = 644c1bf (docs). Format : `ENONCES-1125a.md`. Chaque agent : « lis `Tools\AutoJourney\brief_commun.md` puis `agent_rules.md` et applique-les » ; arrêt 15 min ou 25 appels ; aucun agent ne build (la session principale compile, un build à la fois). **Elrond et Faramir exclus** (session étrangère : `wall-footprint-1125m`, `market-quote-1125s`, rien à y lancer ni fusionner).

## 0. Relevé git (L22-4, L27-6 : `git grep` sur main ET sur spec-voie-1125m, wall-long-fix-1125m, morale-look-fix-1125m, knight-challenge-1125a, wall-footprint-1125m, market-quote-1125s, tools-l288-1125m)

- **Vague 1 en cours ou finie, état au relevé** : Galadriel FINI (541768b rouge, d7b14d5 `EasyMax` 0 -> 1, `GALADRIEL-1125a.md`). Sauron FINI mais **NON COMPILÉ** (d40be6a test, dd60062 règle `ChooseTarget`, d0f6d27 fragment + drapeau, `SAURON-1125a.md`). Arwen EN COURS : d123d0a 15:31 (test rouge « la Chapelle produit le Frère infirmier », D902 : `Test_CGTables` cellule 1 + `Test_Construction_Tables`), correctif CSV + `regen_tables.ps1` pas encore commité ; `BuildingTypes.csv` et `.uasset` modifiés non commités dans son worktree. Celeborn EN COURS : 1c72b52 15:29 (test rouge `CostOfLine` D915) ; `SubCellCostPercent` n'existe que sur `wall-long-fix-1125m` (`HOLWallLineRules.as`), pas dans main.
- **Pas fait (Grep vide partout)** : `CastKnightChallenge` (aucune activation du défi : `ChallengeEndStep` n'est écrit par personne) ; appel de `HOLWallLine::GateReplacedCells` hors tests (la règle G10 a99f9ad existe dans `HOLWallLineRules.as:473-493`, aucun appelant de jeu) ; test C++ de `hol.Cheat.Resources` à deux slots (`HOLPlayerController.cpp:3736` `RequestCheatResources`, correctif 7e0613c dans main, aucun test) ; scénario `ai_tower_easy`. Piège de nom : `ChallengeCooldownMs` existe déjà dans `Script\Duel\HOLDuelRules.as` (défi de DUEL) : le champ du Chevalier s'appelle `KnightChallengeCooldownMs`.
- **Périmées dans `Auto_Journey_File.md` (ne pas prendre)** : l.18 D908, l.19 D911, l.20 D909, l.40 (aperçu de visée : DÉJÀ TRACÉ, `HOLGameScreenScript.as:2276-2307`, GALADRIEL-1125a), l.41 D900, l.151-153 (`HOLMoraleView::Read`, `morale_look`), l.14 `duel_challenge_accept` (ARWEN-1125mb PASS), l.22 JA Voie A/B (clés `HOLGameNames` `SpecializationOptionA/B/C` présentes dans `ja\Game.po`, 8a6fa25 / ba1cd7e dans main).
- **Plan C++ de la vague 2 (L27-11) : Sauron (2 tâches C++, UN seul build) + UNE autre place C++ (Galadriel, tâche G2).** D917 (mur diagonale, Celeborn) ne part PAS en vague 2 : il exige `wall-footprint-1125m` (cb854ad, NON COMPILÉ, chez Elrond) compilée et intégrée ; il reste pour une vague 3 à 17:00-17:15 si la condition est remplie.
- **Fichiers entre worktrees (L21-2)** : aucun fichier n'est pris par deux worktrees. `HOLCombatProcessor.cpp` : Sauron seul. `HOLPlayerController.cpp` est édité par Faramir (G-5c) : Galadriel n'y touche PAS (fichier neuf de test seulement). `BuildingTypes.csv` : Arwen seule (D902) ; Celeborn n'y touche pas (la ligne `PalisadeGate` est en LECTURE seule).

## 1. Énoncés par worktree (2 tâches de 15 min chacun)

### 1.1 Galadriel (`morale-look-fix-1125m`, tip d7b14d5, IN_USE libre, journal `AIInProgress.md`)

**G1 (sans C++) : commentaire `HOLAIBrain.as:1042` + essai `ai_tower_easy`** (Priorité ordre 5 : « `ai_tower` non rejoué » ; File l.39 suite D918)
- Lire (UN appel) : `Script\AI\HOLAIBrain.as:1040-1060` (commentaire « Normal one, Hard two » -> « Easy one, Normal one, Hard two », seul fichier de la ligne), `Script\AI\HOLAITower.as:8-33`, `Tools\Scenarios\ai_tower.txt` (niveau de l'IA du scénario ; journal attendu `AI slot 1 step S: build TowerWood ... (level 1, 0 already, an army of 5)`), `Tools\Scenarios\preflight.ps1` (usage).
- Premier test rouge : `Tools\Scenarios\ai_tower_easy.txt` NEUF (copie de `ai_tower` avec l'IA au niveau Facile ; ne PAS toucher `ai_tower.txt` : Faramir le recadre dans son worktree) : `waitlog` de la ligne « build TowerWood » du slot 1 ; rouge attendu sur le binaire AVANT d7b14d5 (EasyMax 0 = aucune tour), vert sur le tip ; sinon dire `LOGIQUE SEULEMENT`. Le commentaire de `HOLAIBrain.as:1042` n'a pas de test (une ligne, commit à part).
- Sans C++. Rejeu : `play.ps1 ai_tower_easy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` seulement si `Get-StaleBinaryProblem` est vide ; sinon preflight impossible, le dire.
- Fichiers pris : `Script\AI\HOLAIBrain.as` (commentaire seul), `Tools\Scenarios\ai_tower_easy.txt` et `.look`, `AIInProgress.md`, `Retours\E-ai_tower_easy.md` seulement si preflight PASS + PNG vus.

**G2 (C++, UNE place) : test C++ de `hol.Cheat.Resources` sur un monde à DEUX slots** (Priorité « RESTE C++ » ; repli déjà prévu par ENONCES-1125a §1.2 ; GALADRIEL-1125ma : 7e0613c « aucun test »)
- Raison C++ (D838, D839) : seul le harnais C++ fait naître un contrôleur de joueur par slot (G-5c : `HeroesOfLiberty.Market.GridQuote` / `Grid.ArmedModeCursor` sont des tests C++ pour la même raison) ; aucune règle neuve, le code est dans main.
- Lire (UN appel) : `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp:3736-3770` (`RequestCheatResources`, donne à tous les slots), `Source\HeroesOfLiberty\Tests\` (un test de contrôleur voisin, p. ex. `HOLMarketTests.cpp` / `HOLGridTests.cpp` de la branche market-quote pour le montage ; `git grep GridQuote market-quote-1125s -- Source`), `Script\Net\HOLRpcPlayer.as:559` et `Script\Player\HOLControllerCommands.as:31`.
- Premier test rouge : `HeroesOfLiberty.Cheat.ResourcesGivesEverySlot` : monde à 2 slots, 2 contrôleurs ; `hol.Cheat.Resources 100` exécuté depuis le slot 0 -> le stock du slot 1 vaut +100 de chaque ressource (valeur LUE par le sous-système, pas recopiée) ; **le code étant déjà correct (7e0613c), le test est vert d'emblée** : fournir le contrôle rouge en retirant temporairement la boucle des slots dans un worktree jetable ou une copie du test (`TEST_ROUGE_SEPARE: non` + raison au rapport, sans contourner).
- NE PAS toucher `HOLPlayerController.cpp` (Faramir, G-5c) : fichier neuf `Source\HeroesOfLiberty\Tests\HOLCheatResourcesTests.cpp` seul. NON COMPILÉ au rapport ; commande : `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Galadriel`, puis `quick_check.ps1 -Parts tests` ; le build rejoint le lot de 16:00 de Sauron (un seul build).
- Fichiers pris : `Source\HeroesOfLiberty\Tests\HOLCheatResourcesTests.cpp` (neuf), `AIInProgress.md` ou `MoraleInProgress.md` (une ligne). Ligne `Placeholders.txt` : aucune.

### 1.2 Arwen (`spec-voie-1125m`, tip d123d0a, IN_USE libre, journal `SpecializationInProgress.md`)

**Premier geste : finir D902 (vague 1)** : si `git log main..HEAD --oneline` ne montre pas de commit de correctif après d123d0a (CSV `Produces` + `regen_tables.ps1`), la vague 1 n'a pas fini : A1 devient « fin de D902 » (voir ENONCES-1125a §1.1) et A2 reste inchangée. Le reste suppose D902 vert.

**A1 (sans C++) : `E-talents_heal`, du LOGIQUE SEULEMENT au GESTE RÉEL** (File l.119 ; Priorité « Sans C++ : essais joués au geste réel »)
- Lire (UN appel) : `Retours\E-talents_heal.md` (statut LOGIQUE SEULEMENT, `GESTE: interne`, trait « doré » vert sur `talents_heal_boosted`), `Tools\Scenarios\talents_heal.txt` et `.look`, un modèle de geste réel (`neutrals_qground`, brief_commun § 8), `Tools\Scenarios\preflight.ps1`.
- Premier test rouge : le scénario réécrit au geste réel (vraie touche d'ouverture de l'arbre de talents puis vrai clic sur la case, plus de commande `hol.*`) ; preflight rouge attendu tant que le texte du `.txt` appelle encore la commande interne ; PNG ouverts un par un, le trait du talent (doré ou vert) NOMMÉ dans le `.look` (L15-8). Ne pas trancher la couleur du trait : `Retours\Q-talents-trait-couleur.md` si elle n'est pas dans le GDD.
- Sans C++. Rejeu : `play.ps1 talents_heal -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` (binaire à jour sinon logique seule).
- Fichiers pris : `Tools\Scenarios\talents_heal.txt` et `.look`, `Retours\E-talents_heal.md`, `SpecializationInProgress.md` (ligne). Aucun fichier de D902.

**A2 (sans C++) : `E-deploy` PAS PRÊT, refaire le scénario `deploy` en vrai clic** (File l.120 ; BUG D877 moitié A en File l.98)
- Lire : `Retours\E-deploy.md`, `Tools\Scenarios\deploy.txt` et `.look`, `Docs\Rapports\RECETTE-1121se.md` (engin démonté + cible lointaine : marche, ne se monte pas, aucune ligne `Deploy:`).
- Premier test rouge : cas `deploy_far` (cible lointaine) du scénario : `waitlog Deploy:` rouge (aucune ligne) sur main, c'est le BUG D877 A ; l'agent livre le scénario rouge seul + `.look`, SANS correctif C++ (le diagnostic, s'il demande du C++, est SIGNALÉ pour le lot de la séance : `TYPES_A_EXPOSER` / fichier + ligne).
- Sans C++. Fichiers pris : `Tools\Scenarios\deploy.txt` et `.look`, `Retours\E-deploy.md`, `SpecializationInProgress.md` (ligne). Si A1 déborde, A2 saute (arrêt à l'heure).

### 1.3 Celeborn (`wall-long-fix-1125m`, tip 1c72b52, IN_USE libre, journal `FortificationsInProgress.md`)

**Premier geste : D915 (vague 1)** : `git log --oneline main..HEAD` doit montrer le correctif après 1c72b52 ET l'appel dans `ServerPlaceWallLine` (`HOLRpcPlayer.as:1540`) ; sinon la vague 1 n'a pas fini : C1 devient la fin de D915 (déduction branchée + 2e test sur le RPC, ENONCES-1125a §1.2), C2 saute.

**C1 (sans C++) : G10, brancher la porte dans la pose** (Priorité ordre 4 « portes G10 / F9, sans C++ » ; File l.94 : `GateReplacedCells` n'a AUCUN appelant de jeu, relevé 15:36)
- Lire (UN appel) : `Script\Buildings\HOLWallLineRules.as:473-520` (`GateCells`, `GateReplacedCells`), `Script\Tests\HOLWallLineRules_Test.as:738-770` (cas de la règle, rouge 4ba5ec2, vert a99f9ad), `Script\Buildings\HOLConstructionBoard.as` (ordre des portes de pose ; Grep `PalisadeGate` et `Replace` dans `Script\` pour l'endroit de la pose), `Tools\Data\BuildingTypes.csv:19` (ligne `PalisadeGate`, 400 cm, EN LECTURE).
- Premier test rouge sur la vraie chaîne : poser une `PalisadeGate` sur une case dont les 4 sous-cases alignées portent des palissades du MÊME camp : après la pose, ces 4 palissades sont remplacées (retirées de la file de construction / du plan), les palissades d'un autre camp ou un mur de pierre NE le sont PAS ; bouchon = l'appelant ne remplace rien (effet assertit sur l'état de la pose, pas sur `GateReplacedCells` seule : TESTS_TAUTOLOGIQUES 0). Si le retrait d'un acteur déjà posé exige du C++ (liaison de suppression), le SIGNALER (fichier + ligne) sans le faire : pas de C++ dans cette vague pour Celeborn.
- Fichiers pris : `HOLWallLineRules.as`, `HOLWallLineRules_Test.as`, `HOLConstructionBoard.as` seulement si l'appelant y vit (sinon le fichier trouvé, dit au rapport), `FortificationsInProgress.md`. Pas `BuildingTypes.csv` (Arwen).

**C2 (sans C++) : `wall_fine_block`, preflight FAIL à corriger** (Priorité ordre 4 « `wall_fine_block` : dernier état FAIL preflight, non rejoué » ; CELEBORN-1124se : timeout à l'étape 43)
- Lire : `Tools\Scenarios\wall_fine_block.txt` (modifié 2026-11-24 21:37) et `.look`, `Docs\Rapports\CELEBORN-1124se.md` (étape 43), `Tools\Scenarios\preflight.ps1`.
- Premier test rouge : `preflight.ps1 wall_fine_block -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` : rouge VU (étape 43 timeout) AVANT toute retouche ; correctif = `waitlog` à la place du `wait` fixe de l'étape ; PASS puis PNG ouverts un par un (la sous-case bloquée NOMMÉE dans le `.look`). Si le binaire est périmé : le dire, aucune retouche à l'aveugle.
- Sans C++. Fichiers pris : `Tools\Scenarios\wall_fine_block.txt` et `.look`, `Retours\E-wall_fine_block.md` si PASS, `FortificationsInProgress.md`.

### 1.4 Sauron (`knight-challenge-1125a`, tip d0f6d27, NON COMPILÉ, IN_USE libre, journal `AubeInProgress.md`) : **PLACE C++ 1, deux tâches, UN build**

**Premier geste** : lire `Docs\Rapports\SAURON-1125a.md` (RESTE), puis demander à la session principale l'état du build de 16:00 : si d40be6a..d0f6d27 ne compilent pas, la correction de compilation passe avant tout.

**S1 (Angelscript pour la règle, C++ de liaison minimal) : activation de `KnightChallenge` par la capacité** (« Reste » de SAURON-1125a ; Priorité ordre 2 ; D919 = provocation seule, 8 s, 8 m, 45 s, aucun ralentissement)
- Règle en script : `Script\Units\HOLKnightChallenge.as` NEUF (namespace `HOLKnightChallenge`) : `EndStepOf(NowStep, DurationMs, StepMs)` (= NowStep + DurationMs / StepMs) et `CanCast(bAlive, CooldownMs)` ; les valeurs viennent de `HOLDawnValues::KnightChallengeDurationMs() / RadiusCm() / CooldownMs()` (`Script\Factions\HOLDawnValues.as:50-64`, ligne `KnightChallenge` de `Tools\Data\UnitAbilities.csv:4`), jamais de constante recopiée (D839 : une seule source).
- Raison C++ (D839, liaison + écriture du fragment Mass) : `UHOLUnitSubsystem::CastKnightChallenge(UnitId, RequiredOwner)` sur le modèle de `CastRage` (`Source\HeroesOfLiberty\Simulation\HOLUnitSubsystem.cpp:1091-1118`) : gardes type `VirtuousKnight`, vivant, propriétaire, `KnightChallengeCooldownMs` (champ NEUF du fragment, `HOLSimulationFragments.h` près de `ChallengeEndStep:496`, décompté près de `HOLCombatProcessor.cpp:1168`, ajouté à l'empreinte `HOLSimulationSubsystem.cpp:190` SEULEMENT si non nul) ; la règle appelle l'effet par une fonction de la bibliothèque `HOLUnitAbilityReadLibrary` / `HOLUnitScriptLibrary`, le C++ n'écrit que `ChallengeEndStep` et `ChallengeRadiusCm`.
- Lire (UN appel) : `Docs\Rapports\SAURON-1125a.md`, `HOLUnitSubsystem.cpp:1091-1118`, `HOLUnitAbilityReadLibrary.h/.cpp`, `Source\HeroesOfLiberty\Tests\HOLKnightChallengeTests.cpp`, `HOLRageMoraleTests.cpp:22-140` (montage d'un lancer par `CastRage` dans un monde Mass).
- Premier test rouge : (a) script `Test_KnightChallengeEndStep` (`Script\Tests\HOLKnightChallenge_Test.as`, bouchon `EndStepOf` rend -1 : 8000 ms à 50 ms par pas doit rendre NowStep + 160, valeur CALCULÉE par la règle) ; (b) Mass `HeroesOfLiberty.Combat.KnightChallengeCast` : après `CastKnightChallenge`, un ennemi à 600 cm attaque le Chevalier pendant 8 s même si un allié est plus proche, puis reprend sa cible, et un 2e lancer avant 45 s est refusé. Bouchon + test dans UN commit, correctif en 2e commit, NON COMPILÉ au rapport avec la commande `Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check.ps1 -Parts tests`.
- Fichiers pris : `Script\Units\HOLKnightChallenge.as`, `Script\Tests\HOLKnightChallenge_Test.as` (neufs), `HOLUnitSubsystem.cpp/.h`, `HOLSimulationFragments.h`, `HOLUnitAbilityReadLibrary.*`, `HOLKnightChallengeTests.cpp`, `AubeInProgress.md`.

**S2 (C++, même lot) : le défi atteint tous les chercheurs** (RESTE de SAURON-1125a : exclusion des héros, Chevalier hors rayon de recherche, chemins parallèles)
- Raison C++ : choix de cible par entité et par pas (`HOLCombatProcessor.cpp`, hot path Mass).
- Lire : `HOLCombatProcessor.cpp:2165-2200` (`AddCandidate`, TODO héros à `:2181`), `:119-122` et `:1517` (`ParallelSearch` / `AheadSearches` : poids calculé d'avance qui ne lit pas `bChallengesMe`), l'endroit où `Target` / `LastAttacker` sont ajoutés aux extras (même fichier, Grep `extras`), `HOLCombatRules.cpp` `ChooseTarget`.
- Premier test rouge (C++, `HOLKnightChallengeTests.cpp`) : (a) un HÉROS ennemi à 700 cm n'est PAS provoqué (règle D919 à confirmer : si le GDD ne dit rien, défaut codé = les héros ne sont pas provoqués, une ligne, `Retours\Q-defi-chevalier-heros.md`) ; (b) un Chevalier au-delà du rayon de recherche du chercheur mais à moins de 800 cm est candidat (ajouté aux extras) ; (c) avec `hol.Combat.ParallelSearch 1` et `2` le même choix qu'en `0`. Bouchons dans le même commit que les tests, correctif au 2e.
- Fichiers pris : `HOLCombatProcessor.cpp` (zone `AddCandidate` + extras + prévisions, seule), `HOLCombatRules.cpp`, `HOLKnightChallengeTests.cpp`, `AubeInProgress.md`. Reste hors vague : redirection des dégâts alliés, Honneur A5d, scénario `dawn_knight_active` (sans C++ après build).

## 2. Plan C++ et calendrier

| Place | Worktree | Tâche | Raison C++ | Build |
|---|---|---|---|---|
| 1 | Sauron | S1 + S2 (activation du défi, héros, extras, chemins parallèles) | Mass, par entité et par pas ; liaison | un build, lot de ~16:00 avec les builds déjà dus (a6cb051, cb854ad, 6919000) |
| 2 | Galadriel | G2 (test C++ `hol.Cheat.Resources` à 2 slots) | harnais de contrôleur (D838) | même build |
| (vague 3) | Celeborn | D917 tranche 1 (condition : `wall-footprint-1125m` compilée et intégrée) | par entité (navmesh, collision) | 17:00-17:15 au plus tard, fini avant 18:04 |

Builds lents (Elrond, 14:15 : 687 s pour 21 actions) : prévoir 8 à 12 min. Intégration : Haldir P1 vers 16:15 (arwen, celeborn, galadriel si verts), P2 vers 17:30, P3 vers 18:15.
