# Priorité : le socle commun des factions (relevé du 2026-11-16, remis à jour le 2026-11-26 vers 11:25)

**Pour qui** : la séance Auto Journey du 2026-11-26 (orchestrateur et agents). **Ancienne version (relevé du 2026-11-25 10:40)** : `Docs\Archive\Priorite_Socle_Factions_2026-11-16_avant_1126m.md`.
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) ait son identité jouable. Ce document dit **quoi faire en premier** ; le détail est dans les `*InProgress.md` et les lignes S1 à S4 de la section « Socle commun des factions » d'`Auto_Journey_File.md`.

**Relevé du 2026-11-26 11:25 (PREP-1126m), main = 5dc2318.** Chaque « fait » cite un sha vérifié par `git merge-base --is-ancestor <sha> HEAD` sur main, ou un grep daté de main. « Absent » = la commande a répondu non. Aucun essai rejoué par moi. Avant de lancer une ligne : revérifier (`Tools\AutoJourney\status.ps1 -InMain <sha>`).

## Ce qui est DANS main depuis le relevé du 2026-11-25 (à ne plus refaire)

- **Grille de commandes en Angelscript** : G-1, G-2 (3421414, c3765bd), **G-3** clic au sol (rouge 9ac8a67, `Script\Player\HOLGridGroundClick.as`), **G-4** pont C++ `UHOLGridActionBridge` (0ff98f8) + jonction script (0cfbbde), **G-5a** (a493984 : le switch C++ n'exécute plus que `Send`, les cas HeroAbility / LightSeal / AttackMove / Stop / Hold / NecroticRage / Deploy / UngarrisonAll / CancelConstruction / Specialize / CancelSpecialization passent par `HOLCommandGridAction::Decide`), **G-5b face** (ce38e14, `HOLGridFaceBridge.as`), **G-6** `HOLCommandGridFace::GetCellFace` (`Script\Player\HOLCommandGridFace.as`). Branches `grid-g3`, `grid-g5a`, `grid-face`, `integ-79` ne sont plus dans `git branch --no-merged`.
- **Murs** : D915 prix par sous-case branché (2b402fd ; cvar `hol.Wall.SubCellCostPercent` `HOLWallLineRules.as:21`, `LinePercent()` passé à `OrderPlaceBuilding` `HOLRpcPlayer.as:1621`) ; **G10 portes** : `GateReplacedCells` / `GateRetireIndices` (rouge 4ba5ec2, correctif a99f9ad) et branchement dans `ServerPlaceWallLine` (`HOLRpcPlayer.as:1613`) ET `ServerPlaceBuilding` (`:1533-1555`) ; G8 fantômes par sous-case (rouge ded45cd, correctif 1cf69e8). `wall_fine_ghost` / `wall_fine_block` dans main.
- **Pouvoirs Aube** : D908 Honneur contrôlé à l'ordre (5b7ddb6), D909 un ordre du héros annule le sort en attente (985d6db, `CancelPendingOnOrder`, **script, sans C++**), D911 avis de refus (0da0a6f + 985d6db), Sceau du Moine bouton + clic sol (e5d68be, a4d6a48), règle du défi du Chevalier `HOLKnightChallenge.as` + liaison C++ `CastKnightChallenge` (8d83c4b ; **aucun appelant UI**, voir « Reste »).
- **Moral** : S2-a sous-étapes 2 et 5 (a371040, 21facdc, 05f4465, 9c515ca), S2-c (4f688be, e61a729, a3de24e), `hol.Morale.Report / Clear`, masque réseau (fad4af7), `HOLMoraleView` avec `bTimeKnown`, scénario `morale_look` (essai OK 2026-11-25 14:3x).
- **IA** : D918 tours de guet Facile 1 / Normal 1 / Difficile 2 (d7b14d5, `HOLAITower.as:11-13`), `TowersToEmpty` (bf1f453).
- **Cheat et données** : `hol.Cheat.Resources` (86e5398), Chapelle qui produit le Frère infirmier (0746888), `Test_CGTables` et tests de grille.
- **Essais** : `tower_wood` recadré (2ab9d77, 9416e85), `duel_challenge_accept` attend l'arrivée par `waitlog` (01ee4bf).

## EN BRANCHE, pas dans main (`git branch --no-merged`, 2026-11-26)

- `market-quote-1125s` (1e73a88, G-5c : IsAttackMoveArmed / IsBannerArmed lisent le mode armé, devis Trade par le script) et `tests-bindings-1125s` (+10 dont 4dc6ef3, contient la précédente) : à passer par `integrate.ps1 -DryRun` puis une passe (Haldir, `integ-100`+) **avant** tout agent qui touche `HOLPlayerController.cpp`. Rouge `3436577` absent de main.
- `spec-voie-1125m` (Arwen, +12 : fc3b85f, 743e78f, 6cbbdbe, c32f6a0 est dans main) : **fc3b85f absent** (les Templiers ne posent plus la Chapelle) : bloquée par D920 (ligne `Chapel` `Dawn;Templars` de `BuildingTypes.csv`).
- `morale-look-fix-1125m` (Galadriel, +7) : c113cf1 test rouge, 4f3c147 correctif C++ `PushedBacks` (**absents**), ea9269e scénario instrumenté : décision L29-3 (cause vue au journal d'abord).
- `wall-footprint-1125m` (Elrond : 8dd4b9e test rouge, cb854ad correctif, **non compilé**, conflit `HOLBuilding.cpp` à résoudre).
- Anciennes, hors socle, non relues : `integ-31` à `integ-80`, `fog`, `envelope`, `selection`, `visual`, `try-as-rpc`, `mut-d882`, `port-tests`, `launcher-l18` / `-l21`, `test-world-5`, `tower-wood-shaders-1125m`, `tools-l288-1125m`.

## Ordre de priorité (états corrigés d'après main 5dc2318)

| Ordre | Lot | Ce qui reste (vérifié) | C++ ? |
|---|---|---|---|
| 1 | Fiabiliser les essais joués | BUG `deploy_far` « jamais lancé » (File l. 11, `Retours\E-deploy_far.md`) ; gate Essay qui valide la commande exacte (L29-5). Tout le reste de l'ancien lot 1 (D897, D908, D909, D911, Paysans, volée) est dans main, LOGIQUE SEULEMENT tant qu'un humain ne l'a pas joué | non |
| 2 | Premier pouvoir jouable | **Bouton du Défi du Chevalier (D923, touche T, libellé « Défi »)** : `HOLKnightChallenge::CastChallenge` n'a aucun appelant UI ; touche T non injectable (`hol.UI.Key`, File l. 28-29) ; effet réel de `KnightChallenge` (`a6ee4f4` test dans main, NON COMPILÉ) | **oui, C++ n°3** (`HOLPlayerController.cpp`) |
| 3 | Moral | R1 durée / immunité client, R4 version de protocole (réglé : rien à monter), S2-a étape 3 : `RageMs` encore lu (`HOLCombatProcessor.cpp:1160-1168`, `HOLSelectionMarkers.cpp:791`, `HOLInterpolatedInstancesProcessor.cpp:583`) ; spin du Paysan (cause à voir au journal, L29-3) | oui (Mass) |
| 4 | Murs : portes et remboursement | **D921 la porte rembourse 25 % des palissades retirées** (cvar `hol.Wall.GateRefundPercent`) : ni le cvar ni le crédit n'existent (`OrderRemoveOwnBuilding`, `HOLScriptRpcPlayerActor.cpp:496`, détruit sans rembourser : **liaison C++ nécessaire**, la règle pure est en script) ; empreinte de la porte orientée ; D917 mur en diagonale ; `wall_fine_diagonal` ; empreinte visible `wall-footprint-1125m` | règle script, crédit C++ |
| 5 | IA | `ai_tower` rejoué (D918 codé, non rejoué) ; l'IA lance ses pouvoirs et ses garnisons (S4-a, S4-b dans main) | non |
| 6 | Grille de commandes | **G-5b/c** : Produce / Research / Trade / Build encore en `switch` (`HOLPlayerController.cpp:2755-2872`) car `NativeDecideGridAction` ne porte pas `FHOLGridCostFacts` ; intégrer `market-quote-1125s` ; essai au vrai clic | oui, C++ n°3 |
| hors ordre | Chapelle et publication | D920 (`BuildingTypes.csv` Chapel + `.uasset` régénéré, file de build vide) puis `spec-voie-1125m` ; publier 0.4.4 `[B]` (notes x.txt / x.en.txt / x.ja.txt obligatoires) | non |
| outils | Chaîne d'agents | L29-6 `sync_worktrees.ps1` saute les worktrees « pris » ; L29-8 `commit_docs.ps1 -Only` diff limité ; L29-5 gate Essay | non |

S1-a, S1-e, S2-a, S2-b et S3-a touchent `HOLCombatProcessor` ou la simulation : un seul agent à la fois sur ce fichier, un build groupé.

## Effet des voies de spécialisation

Toujours **aucun effet en jeu** (relevé `Docs\Rapports\SPEC-VOIES-1123.md`) ; noms Fer / Foi / Pierre (D905, 21bea82) dans main ; cases du Forum et voie affichée dans le bouton (c1b5595) dans main ; clés `VoieFer` / `VoieFaith` / `VoieStone` absentes des trois `Game.po`. Détail : `SpecializationInProgress.md`.

## Questions ouvertes (`Retours\`, 2026-11-26)

Aucune `Reponse:` neuve (ETAPE0-1126m). `Q-clean-d-1126m`, `Q-file-reste-1126m`, `Q-retours-doutes-1126m` : sans réponse, défaut appliqué. Seule réponse non vide : `E-deploy_far.md:24` (BUG, ordre 1). Les anciennes questions du socle (`Q-commanderie-batiment`, `Q-ligne-case-bloquee`, `Q-sp3-libelles`, `Q-murs-grille-fine`, `Q-lumiere-sacree-visee`, `Q-moral-attenuation`) sont répondues dans le GDD (D896-D923) : non relues une à une ici.

## Déjà fait avant le 2026-11-25 (ne pas refaire)

Socle multi-faction L0 et L2, catégorie « Profane », faction lue par l'IA (G4) ; Aube A1 (Hallebardier) et A2 (Chevalier Vertueux, redirection 30 % / 8 m, essai `virtuous_knight` OK 2026-11-21) ; essais joués par l'utilisateur le 2026-11-23 (`ai_sacred_light` OK, `faction_click_cast` REGLAGE D897, `ai_garrison` OK + D899, `wall_line_long` BUG + D898) ; soin de zone *Lumière sacrée*, flèche de tour T7 / T7b, règle S4-a, panneau cliquable, XP du duel 150, palissade, lancer du joueur et de l'IA au point visé, immunité S2-c, voie affichée SP3 ; D860, D861.

## Règles de séance à garder

Celles de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey.md` : test rouge d'abord sur la vraie chaîne, `Tools\build.ps1 -LiveCoding=0`, `quick_check.ps1 -Parts`, scénario pour tout essai demandé, valeurs proposées puis choisies par l'utilisateur (D345), commits en anglais avec la ligne `Agent: <NOM>` en dernière ligne, fusion locale à chaque passe verte (D888), **jamais de push**. Code neuf en Angelscript, C++ par exception avec sa raison (D839). Mettre à jour le dossier `*InProgress.md` concerné et `EtapeDefini.md` à chaque avancée.
