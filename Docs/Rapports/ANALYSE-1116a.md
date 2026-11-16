# ANALYSE-1116a : builds C++ et plan de migration vers Angelscript

2026-11-16, 16:29-16:5x. Lecture seule (aucun build, aucun jeu lancé, aucun commit ; seul ce fichier est écrit).
Mission d'origine : préparer la discussion « revoir notre manière de coder entre C++ et Angelscript » (L16-1).
Mission changée à 16:3x par la session principale : l'utilisateur a décidé de passer **le maximum du C++ en
Angelscript, tests compris**. Ce document garde les mesures (partie 1 et 2) et remplace les trois options par un
**plan de migration à valider à la prochaine séance** (parties 3 à 7). Rien n'est tranché ici.

## En bref

- **La file** : 337 builds en 8 jours (2026-11-09 au 16), 22,2 h de compilation, **38,4 h d'attente d'agents**.
  L'attente médiane est de 6 s : le coût vient de la queue de distribution, pas du cas courant. 67 builds dont la file
  comptait un build de l'autre projet totalisent 31,5 h des 38,4 h d'attente ; 31 builds de plus de 600 s font 11,1 h
  des 22,2 h de compilation.
- **Ce qui déclenche les builds** : 150 commits C++ (18 % des 828 commits hors fusions), soit 2,2 builds par commit C++.
  48 % des lignes C++ changées sont des **tests** ; 32 commits C++ sur 150 ne contiennent que des tests ; 52 ne touchent
  que l'enveloppe (interface, contrôleur, partie, outils de dev), sans simulation ni réseau.
- **Projection** (sur le mélange de la semaine passée, pas une promesse) : tests et enveloppe en script = 84 commits C++
  sur 150 (56 %) sans build ; avec le C++ restant regroupé en un lot par séance, de ~54 builds par jour à ~10-15.
- **Le code** : 149 500 lignes C++ (535 fichiers) contre 35 800 lignes de script (179 fichiers). Estimation par
  marqueurs : ~55-60 000 lignes doivent rester en C++ (Mass, réplication, rendu instancié, règles appelées par entité
  et par pas), ~40-46 000 lignes hors tests sont migrables, et les 44 800 lignes de tests se partagent en trois familles
  de difficulté très différente (partie 4).
- **Deux inconnues à lever par un pilote avant tout le reste** : les RPC et propriétés répliquées écrites en script
  (le moteur les sait, le projet ne l'a jamais fait, Iris non vérifié), et les tests de la simulation Mass depuis le
  script (il faut un harnais C++ exposé, qui n'existe pas).

## 1. Mesures

### 1.1 La file de build (`Docs\BuildQueue.csv`, 589 lignes depuis le 2026-11-03)

Sur tout le fichier : 530 PASS, 41 FAIL, 13 ORPHAN, 5 SUPERSEDED. Par jour depuis le 2026-11-09 :

| Jour | Builds | PASS / FAIL / autres | Attente médiane (s) | Attente totale (h) | Durée médiane (s) | Compilation (h) | Créneau occupé | Commits C++ | Commits `.as` |
|---|---|---|---|---|---|---|---|---|---|
| 11-09 | 44 | 40 / 2 / 2 | 13,5 | 2,2 | 85 | 2,5 | 29 % | 5 | 8 |
| 11-10 | 35 | 30 / 3 / 2 | 2 | 3,8 | 73 | 1,9 | 13 % | 7 | 3 |
| 11-11 | 67 | 56 / 9 / 2 | 11 | 2,2 | 106 | 2,6 | 33 % | 28 | 8 |
| 11-12 | 9 | 6 / 3 / 0 | 9 780 | 18,6 | 898 | 3,4 | 100 % | 10 | 14 |
| 11-13 | 59 | 56 / 3 / 0 | 3 | 6,5 | 148 | 6,2 | 51 % | 46 | 33 |
| 11-14 | 43 | 38 / 4 / 1 | 3 | 1,5 | 124 | 2,0 | 27 % | 25 | 16 |
| 11-15 | 76 | 73 / 3 / 0 | 3 | 3,0 | 121 | 2,5 | 44 % | 28 | 19 |
| 11-16 (jusqu'à 16:24) | 4 | 3 / 0 / 1 | 514,5 | 0,6 | 674 | 1,1 | 80 % | 1 | 4 |
| **Total** | **337** | 302 / 27 / 8 | **6** | **38,4** | **118** | **22,2** | | **150** | **105** |

Lecture des colonnes : durée médiane des builds de plus de 10 s (les builds « rien à compiler » sont exclus) ;
« créneau occupé » = compilation divisée par l'intervalle entre la première demande et la dernière fin du jour ;
commits hors fusions, par date d'auteur.

Ce que ces chiffres disent :

- **Un jour normal coûte peu** : 6 jours sur 8 ont une attente médiane de 2 à 13,5 s ; 165 builds sur 337 n'attendent
  pas du tout (5 s ou moins).
- **Deux jours saturés coûtent la moitié du total** : le 11-12 et le 11-16 font 19,2 h d'attente pour 13 builds.
  Le 11-16 : Haldir 3 088 s pour 22 actions (médiane 141 s), Galadriel 674 s pour 4 actions (médiane 45 s).
- **La charge de la machine est le premier facteur** : les 67 builds dont la file comptait « another project »
  totalisent 31,5 h d'attente, les 270 autres 6,9 h. (La colonne `waited_behind` cite aussi les autres worktrees :
  ces 31,5 h ne sont donc pas toutes dues à l'autre projet.)
- **La taille du build compte** : depuis le 11-12 (160 lignes avec le compte d'actions), médiane 48 s pour 1 à 10
  actions, 138 s pour 11 à 44 actions. 82 commits C++ sur 150 touchent un header.
- **Répartition des durées** : 99 builds de 60 s ou moins, 131 de 61 à 180 s, 68 de 181 à 600 s, 31 au-delà.
- **Dix builds marqués BUILD LENT** depuis le 11-13, dont quatre sans personne devant eux dans la file.

### 1.2 Les commits (`git log --since=2026-11-09 main`)

1 132 commits, dont 304 fusions. Sur les 828 autres :

| Touche | Commits | Part |
|---|---|---|
| `Source\` (C++) | 150 | 18 % |
| `Script\` (`.as`) | 105 | 13 % |
| `Source\` et `Script\` ensemble | 18 | 2 % |
| Données (`Content\`, `Config\`, `Data\`) | 20 | 2 % |
| `Tools\` | 265 | 32 % |
| Documents seuls | 380 | 46 % |

Les catégories se recouvrent, sauf la dernière. Lignes changées : ~14 700 en C++ contre ~6 500 en script
(3 946 de code, 2 532 de tests).

**Où va le C++** (lignes ajoutées et retirées dans `Source\`) : Tests 48 %, Simulation 12 %, Match 10 %, module
d'interface 9 %, Player 6 %, Dev 3 %, le reste 12 %.

**Nature des 150 commits C++** (d'après les dossiers touchés hors tests) :

| Nature | Commits | Dont avec tests C++ |
|---|---|---|
| Enveloppe seule (interface, Player, Match, Dev, Buildings, racine) | 52 | 21 |
| Cœur seul (Simulation, Net, Commands, Presentation, Map, AI...) | 44 | 23 |
| Tests seuls | 32 | 32 |
| Cœur et enveloppe | 20 | 9 |

Fichiers les plus touchés : `HOLGameScreen.cpp` 18 commits, `HOLPlayerController.cpp` 13, `HOLCombatProcessor.cpp` 10,
`HOLStartingHeroTests.cpp` 9, `HOLDeployProcessor.cpp` 8, `HOLSpecializationSubsystem.h` et `.cpp` 7 chacun.

### 1.3 Douze commits C++ récents : ce qui imposait le C++

Classement fait sur le sujet du commit, la liste des fichiers et le compte des lignes ajoutées portant un marqueur
(réflexion, RPC, Mass, commande console). Les diffs n'ont pas été lus en entier.

| Commit | Sujet | Devait être en C++ | Aurait pu être en script |
|---|---|---|---|
| 8b8e309 | Transport du chat, 440 lignes ajoutées | 2 déclarations de RPC (`ServerPostChat`, `ClientReceiveChat`) | Le miroir `HOLChatRules.cpp` (202 lignes pour 77 en script), la garde de longueur et de cadence, `hol.Chat.Say`, la ligne du fil, les 129 lignes de tests |
| 1bce302 | Pont du tableau de spécialisation, 379 lignes | Vue répliquée au propriétaire, 2 RPC | Le sous-système pont lui-même (394 lignes) si le sous-système vit en script |
| b85afbb, 6627655, e4c9c92, c3b95d3, d2c3ab6, d305fdd | Série Nomade : une fonction de pont par commit | La déclaration du pont ; une part de placement côté serveur dans deux d'entre eux | L'essentiel : six builds avec header, chacun pour étendre `UHOLSpecializationSubsystem` |
| 341ad4b | Garnison lot 2, 309 lignes | Ordres appliqués au début du pas, état caché des vues, fragments Mass | La règle jumelle `HOLGarrisonRules.h` (80 lignes) |
| 43156bd | Garnison répliquée aux clients | Tout (sérialiseur réseau, sous-systèmes de réplication) | Rien |
| 45be389 | Jumeau garnison atteignable du script, 66 lignes | Rien d'autre que le coût du jumeau | Tout, avec une seule source |
| 2f92882 | Signal par Alt + clic, 187 lignes | Rien de visible (aucune RPC ajoutée) | La saisie, si le contrôleur du joueur était en script |
| a28395b | Cases de spécialisation de la grille, 184 lignes | Rien (0 ligne de réflexion, 0 RPC) | Tout : seul motif, `HOLCommandGrid` est en C++ |
| a11ed2a | Ligne « population pleine », 121 lignes | 2 liaisons sur `UHOLScriptScreen` (une fois) | Le test C++ de 82 lignes |
| 59093f0, b112b0e | Commandes `hol.Signal.Post`, `hol.Spec.*`, 90 lignes | Rien | Des `FConsoleCommand` en script (7 existent déjà), si les RPC étaient appelables du script |
| 53c106f, 3f4a972, c5e9b76, 91299fc, da10225 | Verbes de scénario dans `HOLTestSequence.cpp` | Rien (0 ligne de réflexion) | Tout : outil de dev, sans réplication ni performance |
| 2bde715, a714e74 | Population comptée sans bâtiment : 7 lignes, plus un test de 19 lignes | Le correctif, tant que `HOLProductionSubsystem` est en C++ | Le test rouge, donc un des deux builds |
| b62c102 | Colonne `GarrisonSlots` de la table, 45 lignes | La structure de ligne de table (`USTRUCT`) | Rien tant que les lignes de table sont en C++ |

Trois motifs reviennent : le **jumeau** (la règle écrite deux fois), le **pont fonction par fonction** (un build avec
header par fonction ajoutée), et le **test rouge d'abord en C++** (un build pour voir le rouge, un pour le vert).

## 2. Les jumeaux `.as` / `.cpp` aujourd'hui

**Quatorze règles existent des deux côtés** : Chat, Diplomacy, Market, Garrison, Camp, ExplorationXP, Relay, Site,
UnitAbility, UnitTier, Technology, Terrain, Deploy, Siege. Total : 2 068 lignes en script, 2 834 en C++, plus
6 825 lignes de tests C++ et 2 936 de tests en script sur les mêmes règles.

**Pourquoi** (commentaires des headers) :

- La simulation ne peut pas appeler une fonction de namespace du script : `HOLGarrisonRules.h`, `HOLRelayRules.h`,
  `HOLUnitAbilityRules.h` le disent en toutes lettres. Seule une classe de script (`UCLASS`) est appelable du C++.
- D483 garde en C++ « la simulation et ses règles pures appelées par le pas du serveur ».
- La procédure pousse à écrire la règle en script d'abord : au-delà du plafond de 3 agents C++, un agent commence par
  « un travail sans compilation (tests Angelscript) », et le câblage serveur arrive ensuite en C++ avec sa propre copie.
  Exemple du jour : D806, partie script faite à 15:21, « jumeau C++ en file (plafond C++ atteint) ».

**Coût** :

- La règle s'écrit, se teste et se corrige deux fois, et le côté C++ redemande un build.
- L'accord entre les deux copies n'est pas toujours vérifié. Un test nommé `*Agree*` existe pour Terrain, Garrison,
  Relay, Site, Talent, Technology et UnitTier ; je n'en ai pas trouvé pour Chat, Diplomacy, Market, Camp,
  ExplorationXP, UnitAbility et Deploy (recherche par nom de test seulement : à confirmer).
- Vérifier l'accord demande une bibliothèque C++ de plus (13 `*Library.h`, dont `HOLSiteRulesLibrary` et
  `HOLGarrisonRuleLibrary`, écrites « seulement pour que les règles du script soient vérifiées contre elles »).

**Un autre modèle existe déjà et marche** : le pont. Six sous-systèmes C++ abstraits ont une sous-classe en script
(`UHOLSignalSubsystem`, `UHOLSpecializationSubsystem`, `UHOLDuelSubsystem`, `UHOLHeroReturnSubsystem`,
`UHOLAIBrainSubsystem`, `UHOLScriptScreen`), appelée par 40 `BlueprintNativeEvent`. Là, le serveur exécute bien la
règle du script, sans copie. Son défaut est le build par fonction ajoutée (série Nomade ci-dessus).

## 3. Inventaire de `Source\` : ce qui reste, ce qui passe

Mesure : `git ls-files`, lignes brutes, `.cpp` et `.h`. Les totaux diffèrent un peu de ceux de l'énoncé (44 800
lignes de tests ici contre 40 500), sans doute par la façon de compter. La colonne « reste » est une **estimation par
marqueurs** (processeurs Mass, réplication, Slate, `ParallelFor`), pas une lecture fichier par fichier.

| Dossier | Lignes | Reste en C++ | Passe en script |
|---|---|---|---|
| Tests (2 modules, 123 fichiers, 598 tests) | 44 800 | Voir partie 4 | Voir partie 4 |
| Simulation (98 fichiers) | 28 700 | ~21 000 : 12 processeurs et 11 fragments Mass, `ParallelFor`, `HOLUnitSubsystem`, règles appelées par entité (Combat 1 261, Formation 1 581, Gather 950, Avoid) | ~7 000 : sous-systèmes à basse fréquence et leurs règles (Volcano 1 866, Site, Camp, Duel, Relay, ExplorationXP, UnitTier, HeroXP), sous réserve de mesure |
| Player (16 fichiers) | 11 000 | ~3 000 : les 57 déclarations de RPC et leur enveloppe, `HOLUnitPicking`, `HOLSelectionMarkers` (à regarder) | ~8 000 : saisie, ordres du joueur, 100 commandes console, `HOLCommandGrid` (1 030) |
| Match (55 fichiers) | 10 250 | ~4 000 : propriétés répliquées de `HOLGameState` (52) et `HOLPlayerState` (26), `HOLMatchConfig` | ~6 000 : Forum (règles 907, messages 853), Faction, Diplomacy, Chat, Market, Victory, Start, Disconnect, Stats |
| Interface HUD, Screens, Widgets | 14 400 | ~1 500 : `HOLScriptScreen` (la base), `HOLMinimapView` (Slate, 917) | ~13 000 : arbre UMG construit en code, recette déjà écrite (J8a.5) |
| Interface Dev, Net, Style, Menu, Input | 2 800 | ~1 300 : `HOLManagerClient` (Slate, 855), styles | ~1 500 : panneaux de dev (triche, scénario) |
| Commands (9 fichiers) | 6 450 | ~6 000 : `HOLUnitOrderProcessor` (4 969, Mass), file de commandes du pas | ~430 : `HOLPlayerOrders` |
| Buildings (20 fichiers) | 5 650 | ~2 300 : réplication de `HOLBuilding` (26), lignes de table | ~3 300 : règles de construction (1 230), sous-systèmes de construction et de production, sous réserve de mesure |
| Net (27 fichiers) | 5 350 | ~4 400 : sérialiseurs Iris, réplication et réception des unités, brouillard | ~900 : `HOLManagerClientRules` (règles pures) |
| Map (30 fichiers) | 4 750 | ~2 900 : vision et brouillard (2 169), rendu du brouillard, rapport de navigation | ~1 800 : règles de carte et de taille, disposition des sites |
| Dev (14 fichiers) | 3 800 | ~2 300 : `HOLSanity`, essais de foule et de croisement (Mass) | ~1 500 : `HOLTestSequence` (915), crochets de test |
| Presentation | 2 600 | Tout : rendu instancié | Rien |
| AI | 2 400 | Tout pour l'instant : c'est déjà la couche de liaison des 13 300 lignes d'IA en script | À réduire plus tard |
| StressTest, Resources, racine, Siege, Technologies, Settings | 6 700 | ~4 500 : essai de charge, sources (Mass), `GameMode` en partie | ~2 200 : jumeaux Siege et Technologies, part du `GameMode` |
| **Total hors tests** | **104 700** | **~55-60 000** | **~40-46 000** |

**Ce qui doit rester en C++, et pourquoi** :

- **Mass** : processeurs, fragments, requêtes. Trois fichiers utilisent `ParallelFor` (14 appels) : le script ne
  s'appelle que du fil du jeu.
- **Réplication Iris** : sérialiseurs, tableaux rapides, sous-systèmes de réplication des unités.
- **Slate natif** : minimap, client du gestionnaire.
- **Structures de ligne de table** et types du cœur lus par la simulation.
- **Tout ce qui s'exécute par entité et par pas** : règle de performance, à garder tant qu'aucune mesure ne dit le
  contraire.
- **La couche de liaison** : ce que le script appelle (`UFUNCTION`), ce qui appelle le script (événements).

**Ce que le script sait faire dans ce moteur** (vérifié dans les sources Hazelight ou dans le projet) :

- Classes `UObject`, `AActor`, widgets (`UHOLGameScreenScript`), sous-systèmes (`UScriptWorldSubsystem` : une classe
  du projet en dérive déjà ; `ScriptGameInstanceSubsystem.h` et `ScriptLocalPlayerSubsystem.h` existent).
- Commandes et variables console (`FConsoleCommand` 7 usages, `FConsoleVariable` 18), 137 structures, textes
  `NSLOCTEXT` ramassés par GatherText.
- **RPC et réplication** : le préprocesseur du moteur lit `Server`, `Client`, `NetMulticast`, `Replicated`,
  `ReplicatedUsing`, `ReplicationCondition` (`AngelscriptPreprocessor.cpp`, lignes 1213-1216 et 2232-2235).
  **Le projet n'en a aucun usage** et rien ne prouve que cela passe avec Iris : c'est une inconnue, pas un acquis.

**Ce que le script ne sait pas faire ici** (notes du projet) :

- Voir un type C++ sans réflexion : `FHOLResources` « n'est pas un type blueprint », d'où un pont par ressource
  (6627655) ; les namespaces C++ (`HOLCombat::`, `HOLGarrison::`) sont invisibles sans bibliothèque.
- Être appelé du C++ autrement que par une classe : pas de fonction de namespace.
- Lire un CSV (`HOLMoraleTable_Test.as`), relire certaines propriétés (`bCanEverAffectNavigation`), voir certains
  délégués (`OnHeroSheetsChanged`, `JourneyInProgress.md`).
- Le serveur dédié compile tous les scripts au démarrage avec `-as-exit-on-error` : un script cassé arrête le serveur.

## 4. Les tests en Angelscript

**Le cadre existe et tourne déjà** (AS1, D484) :

- `void Test_<Nom>(FUnitTest& T)` dans un fichier `*_Test.as` ; assertions `AssertEquals`, `AssertTrue`,
  `AssertFalse`... ; un monde neuf par test.
- **`quick_check -Parts tests` les lance** : `Automation RunTests HeroesOfLiberty+Angelscript.UnitTests`
  (`Tools\Tests\quick_check.ps1`, ligne 354), avec les contrôles « scripts compiled » et « script tests ran (N) ».
- Aujourd'hui : **725 tests en script** (88 fichiers) et **598 tests C++** (123 fichiers).
- Limites connues : tests d'éditeur seulement (comme les tests C++) ; aucun `IntegrationTest_` dans le projet ;
  deux fichiers de tests en script seulement touchent un monde ; le commandlet `-run=AngelscriptTest` peut donner un
  faux vert (D484).

**Les 123 fichiers de tests C++ ne se valent pas** (classement par ce qu'ils incluent) :

| Famille | Fichiers | Lignes | Tests | Passage en script |
|---|---|---|---|---|
| Règles pures (aucun monde, aucun Mass) | 51 | 14 960 | 220 | Direct si la règle passe en script ; par bibliothèque si elle reste en C++ |
| Monde sans Mass | 27 | 8 040 | 130 | Possible si les classes touchées sont réfléchies ; à vérifier fichier par fichier |
| Interface | 15 | 5 250 | 68 | Suivent la migration de l'écran testé |
| Monde et Mass | 25 | 12 670 | 132 | Seulement avec un harnais C++ exposé (créer une partie, poser une unité, ordonner, avancer de N pas, lire la vue) |
| Réseau | 5 | 3 910 | 48 | Les plus difficiles : sérialiseurs et réplication ; candidats à rester en C++ |

**Conséquence à dire franchement** : « tous les tests en script » ne dépend pas seulement de la traduction. Pour les
16 600 lignes des deux dernières familles, il faut d'abord écrire le harnais en C++ (un lot de liaisons), et les tests
qui lisent l'intérieur des fragments ne passeront pas par lui. D484 dit aussi « les règles restées en C++ gardent leurs
tests C++ » : la décision de l'utilisateur la remplace, il faudra un nouveau numéro de décision à la validation.

**Gain attendu** : un test rouge en script ne demande aucun build. Les 32 commits « tests seuls » de la semaine
(21 % des commits C++) n'auraient pas compilé.

## 5. Ordre des lots

Unité : le lot d'agent de 15 à 30 min. **Hypothèse de calcul : 300 à 600 lignes C++ transposées par lot**, tests
relancés compris. Cette cadence n'a jamais été mesurée : les pilotes servent à la mesurer, et les chiffres ci-dessous
sont à recalculer après eux.

### Étape 0 : deux pilotes (une demi-séance)

**Pilote 1, côté serveur : le chat.** Le plus petit jumeau (77 lignes en script, 202 en C++, 129 de tests C++), récent,
couvert par la phase chat de `netsig3`, et un besoin réel attend (jumeau D806 en file aujourd'hui).

1. Tests C++ du chat réécrits en script, garde de longueur et de cadence comprise ; rouge vu, puis vert. Aucun build.
2. Bascule : `ServerPostChat` appelle la règle du script par un pont ; `HOLChatRules.cpp`, `.h` et leurs tests C++
   supprimés. Un build.
3. `quick_check -Parts tests,net` ; mesure : un changement de règle du chat ne demande plus de build.
4. Essai séparé, sur une branche jetable : une RPC `Server` et une `Client` écrites en script, avec Iris. Réponse
   attendue : oui ou non, avec la preuve dans le journal du serveur.

**Pilote 2, côté interface : `HOLGameMenu`** (421 lignes, règles à part, 7 tests de 142 lignes) par la recette
`UHOLScriptScreen` de J8a.5. `HOLEndBanner` (215 lignes, 2 tests) est l'alternative plus petite déjà citée en AS-Q3.

Sortie de l'étape 0 : lignes par lot mesurées, réponse sur les RPC en script, liste des types à exposer.

### Étape 1 : le socle de liaisons (un seul lot C++, une fenêtre de build)

- Types du cœur rendus visibles du script (`FHOLResources`, et ceux que les pilotes auront trouvés).
- RPC existantes appelables du script, pour que les commandes console sortent du contrôleur.
- Points d'appel génériques vers le script (début de pas, fin de pas, événements d'unité), pour qu'un sous-système en
  script n'exige plus une fonction de pont par besoin.
- Harnais de test de la simulation, si la question Q1 de la partie 7 reçoit un oui.
- À discuter, pas à décider ici : une paire de RPC génériques (demande du client, réponse du serveur) que le script
  étend sans C++. Elle supprime les builds des nouvelles demandes, mais déplace la validation du serveur dans le script.

### Étapes suivantes

| Étape | Contenu | Volume C++ | Lots estimés |
|---|---|---|---|
| 2. Jumeaux | 14 règles : une seule source par règle, bibliothèques d'accord supprimées | 2 800 lignes, plus 6 800 de tests | 15 à 25 |
| 3. Règles pures et leurs tests | Les 51 fichiers de tests purs et les règles qu'ils testent (Forum, Faction, Start, Victory, MapSize, GameMenu...) | ~15 000 de tests (dont une part déjà comptée à l'étape 2), ~8-10 000 de règles | 40 à 70 |
| 4. Interface | Écrans, widgets, HUD panneau par panneau, puis leurs 15 fichiers de tests | ~13 000, plus 5 250 de tests | 35 à 60 |
| 5. Enveloppe de jeu | Contrôleur hors RPC, grille de commandes, ordres du joueur, logique de `GameState`, `HOLTestSequence`, sous-systèmes de construction et de production ; tests « monde sans Mass » | ~15 000, plus 8 000 de tests | 40 à 75 |
| 6. Tests de la simulation | Les 25 fichiers Mass par le harnais ; les 5 fichiers réseau restent sans doute en C++ | 12 700 à 16 600 | 30 à 50, si Q1 = oui |
| **Total** | | **~85-95 000 lignes** | **160 à 280 lots** |

Soit **3 à 5 séances à 8 agents si la migration est le seul travail** (hypothèse : 50 à 60 lots utiles par séance),
davantage si des fonctions avancent en parallèle.

Règles d'ordre :

- **Petits d'abord** dans chaque étape : les 25 plus petits fichiers de tests purs font 32 à 190 lignes.
- **Un fichier très touché se migre avant le chantier qui va le toucher** (règle de D483), par tranches :
  `HOLGameScreen` (18 commits en 8 jours) et `HOLPlayerController` (13) ne se migrent pas d'un bloc.
- **Jamais deux agents sur le même fichier en migration.**
- Une règle appelée par un processeur Mass ne migre pas : c'est la copie en script qui disparaît, et le script lit la
  version C++ par sa bibliothèque.

## 6. Risques et garde-fous

| Risque | Garde-fou |
|---|---|
| **Double source pendant la transition** | Une règle n'a deux copies que le temps d'un lot. Tant que les deux existent, un test d'accord est obligatoire (7 jumeaux n'en ont pas de visible aujourd'hui). La suppression du C++ fait partie du lot, pas d'un « plus tard ». |
| **Tests perdus en route** | Registre par fichier migré : N tests C++ retirés, N tests en script ajoutés, noms en regard. Le total de `quick_check` ne baisse jamais. Le compte « script tests ran (N) » devient un plancher. |
| **Test traduit qui ne teste plus rien** | Rouge vu pour chaque fichier migré (une valeur attendue faussée exprès, une fois), comme la règle du test rouge d'abord. Le test C++ n'est supprimé qu'après ce rouge puis le vert. |
| **Régression serveur ou réseau** | Tout lot qui change du code appelé par le serveur : `quick_check -Parts tests,net`, plus `aiserver` et `aiduel` (déterminisme) avant fusion. Le contrôle « server: scripts compiled » reste bloquant. |
| **Performance du script dans le pas du serveur** | Rien de ce qui tourne par entité et par pas ne migre. Mesure avant et après sur l'essai de charge existant pour chaque sous-système migré. Recours prévu par D484 : le script traduit en C++, écarté jusqu'à mesure. |
| **Appel du script hors du fil du jeu** | Les trois fichiers à `ParallelFor` sont exclus de la migration et de tout point d'appel vers le script. |
| **RPC en script non prouvées** | Pilote 1, point 4. En cas d'échec, les RPC restent déclarées en C++ et l'enveloppe du contrôleur garde ~3 000 lignes. |
| **Binaire en retard dans un worktree** | Un script qui appelle une liaison neuve ne compile pas là où la dll est ancienne, et tous les tests y tombent. Les liaisons entrent dans `main` par le lot C++ de la séance, avant les scripts qui s'en servent ; `LaunchCheck` existe déjà. |
| **Un script cassé bloque tout le monde** | Un `.as` qui ne compile pas arrête éditeur, tests et serveur. `quick_check -Parts tests` vert avant toute fusion (règle actuelle), sans exception pour les lots « script seul ». |
| **La migration crée elle-même des builds** | Supprimer un `.cpp` ou basculer un appelant est un changement C++. Les bascules de la séance se regroupent dans la fenêtre de build (partie 7). |
| **Conflits avec les chantiers en cours** | Fichier en migration verrouillé dans `Auto_Journey_File.md` ; migration par tranches ; pas de chantier de fonction sur un fichier le jour où il migre. |
| **Paquet et lanceur** | Cache précompilé par cible refait à chaque paquet (AS2) ; `packaged_smoke` après la première grosse étape. À revérifier : cible Server qui lie l'interface (noté dans `AngelscriptInProgress.md`). |

## 7. Changements de procédure Auto_Journey

1. **Le script devient le cas normal, le C++ l'exception justifiée.** L'énoncé d'une tâche C++ dit pourquoi
   (Mass, réplication, Slate, mesure). L'étiquette `sans C++` disparaît au profit d'une étiquette `C++ : <motif>`.
2. **Un lot C++ par séance, dans une fenêtre réservée.** Un agent, un worktree, en début de séance : liaisons neuves
   et bascules préparées la veille. Puis un build de rafraîchissement par worktree. Remplace le plafond de 3 agents C++.
3. **Fenêtre liée à la charge de la machine.** `session_gate` refuse la fenêtre si un build de l'autre projet tourne
   ou si la mémoire virtuelle libre est sous le seuil ; `-MaxWaitMin` par défaut dans les énoncés, pour qu'un agent
   en attente reparte sur du script.
4. **Test rouge d'abord, en script.** Le rouge se voit sans build ; l'exception « un build pour le premier test rouge »
   ne vaut plus que pour le cœur C++.
5. **Modèle d'énoncé de migration** : tests en script et rouge vu ; règle en script ; accord vérifié ; bascule et
   suppression mises dans la file du lot C++ ; registre des tests rempli.
6. **Champs de rapport nouveaux** : builds demandés (0 attendu pour un lot script), tests C++ retirés et tests en script
   ajoutés, lignes C++ supprimées.
7. **Indicateurs suivis dans `build_queue_report.ps1`** : builds par jour (~54 aujourd'hui hors jours saturés),
   commits C++ par jour (~19), heures d'attente. Cible proposée après l'étape 3 : moins de 15 builds par jour.
8. **Décisions à écrire à la validation** : remplacement de la règle de D483 (« simulation et ses règles pures en
   C++ ») et de D484 (« les règles restées en C++ gardent leurs tests C++ »).

**Deux questions pour l'utilisateur à la validation** :

- **Q1. Les tests de la simulation Mass** (25 fichiers, 12 700 lignes). A : écrire le harnais C++ et les migrer.
  B : les laisser en C++, la simulation changeant moins souvent. C : décider après le pilote. Recommandation : C, puis
  A si le harnais tient en un lot.
- **Q2. Le rythme.** A : migration seule pendant 3 à 5 séances. B : moitié migration, moitié fonctions. C : migration
  d'un fichier seulement quand un chantier va le toucher (règle actuelle de D483). Recommandation : A pour les étapes
  0 à 3, qui suppriment le plus de builds, puis B.

## Ce qui n'a pas été vérifié

- Les diffs des commits n'ont pas été lus en entier ; la colonne « reste en C++ » de l'inventaire vient de marqueurs.
- Aucun essai de RPC ou de réplication en script, aucun essai de test de simulation en script : rien n'a été lancé.
- La cadence de 300 à 600 lignes par lot est une hypothèse.
- La part exacte de l'autre projet dans les attentes n'est pas isolable dans le CSV.
- `EtapeDefini.md` et `AngelscriptInProgress.md` n'ont pas été mis à jour (lecture seule imposée).
