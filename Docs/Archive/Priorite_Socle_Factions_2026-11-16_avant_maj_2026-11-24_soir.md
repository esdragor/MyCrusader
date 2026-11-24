# Priorité : le socle commun des factions (relevé du 2026-11-16, remis à jour le 2026-11-23 vers 17:2x)

**Pour qui** : la prochaine séance Auto Journey (orchestrateur et agents qu'il lance).
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) puisse avoir son identité jouable. Ce document ne remplace pas les dossiers `*InProgress.md` : il dit **quoi faire en premier** et renvoie à l'étape qui porte le détail. Les tâches sont les lignes S1 à S4 de la section « Socle commun des factions » d'`Auto_Journey_File.md`.

**État relevé le 2026-11-23 vers 17:2x** (DOCS-1123pe) d'après le tableau « Agents du 2026-11-23 » d'`Auto_Journey_Jour.md`, les rapports de `Docs\Rapports\` (jusqu'à 17:15), les fiches de `Retours\`, les décisions D896 à D899 du `GDD.md` et le journal git de `main` (5 avances de la séance : b5a285b, c3578b1, 86a2e15, a984c61, 3b6d29c). Pas de relecture du code ; aucun essai rejoué par moi. **Avant de lancer une ligne, revérifier dans `main`** (`Tools\AutoJourney\status.ps1 -InMain <sha>`). Anciennes versions : `Docs/Archive/Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-23_soir.md` (celle de 10:12), `..._avant_maj_2026-11-23.md` (2026-11-22) et `..._avant_maj_2026-11-22.md`.

**Trois états, à ne pas mélanger** :
- **DANS MAIN** (`main` = 3b6d29c, integ-62 PASS à 17:07 : build PASS, tests 2190/2190, parts net et build PASS ; cinq verts fusionnés ; 5 avances depuis 12:24 : b5a285b 15:37, c3578b1 16:00, 86a2e15 16:22, a984c61 16:46, 3b6d29c 17:07 ; `main` est local, rien n'est poussé depuis 12:24) : le lancer de *Lumière sacrée* par le joueur au point visé (e809222 : zone à 1200 cm de portée, l'IA aussi), le moral S2-c étape 8 immunité (a3de24e), la voie de spécialisation affichée dans le bouton SP3 (c1b5595, scénario `specialization_slot`), l'essai en vrai clic sous `-Hidden` du déploiement (519a81d), les scénarios `wall_line_long` et `ai_garrison` (bde5deb). Le contenu des avances b5a285b à a984c61 n'est pas recopié ici : voir le tableau des agents du Jour.
- **EN INTÉGRATION** (integ-63 lancée 17:16, liste figée ; au dernier relevé, 17:15, `INTEG-63.txt` dit seulement « branche creee, 1 rouge applique, tests des rouges en cours » : **résultat inconnu**) : rouge d455b04 (Arwen, tests `Test_WallLineBuildersGetOrder` et `Test_WallLineNoSelectionNoBuilders`) ; verts 565b88a (zone-effect : Honneur durci et volée 1 + occupants), 2f8a019 (socle-ai : remise à zéro de l'état statique du moral), 8ffb822 (integ-37 : les Paysans reçoivent l'ordre de construire la ligne), 731560e (scénario `faction_aim_free`, worktree Haldir).
- **SUR BRANCHE** : tout ce que la liste d'integ-63 contient est déjà verte sur sa branche (zone-effect build PASS 88 s et tests 2166/2166 à 17:14 ; integ-37 build PASS 20 s et tests 2164/2164 à 17:13 ; socle-ai BUILD PASS 54 s et tests 2151/2151 à 17:02). Les autres branches (Celeborn integ-41 : D893 et T8 ; Elrond integ-25 : lot de liaisons C++ du Moine ; Sauron integ-55) **n'ont pas été relues pour cette mise à jour** : voir les lignes d'agents de 15:07 à 16:40 du tableau du Jour.

**En une phrase** : dans `main`, le joueur peut lancer *Lumière sacrée* au point de sa souris (essai `faction_click_cast` : réglage demandé, D897, corrigé) et l'IA la lance (`ai_sacred_light` OK vu par l'utilisateur) ; ce qui manque encore à l'écran : la construction des palissades par les Paysans au relâché du tracé (BUG, correctif en integ-63, jamais vu), la volée de la tour garnie à 1 + occupants flèches (D899, en integ-63), la grille fine des murs (D898, pas commencée) et le feedback d'un tir hors portée (silencieux, Q-lumiere-sacree-visee). « Fait » veut dire ici : appelé en jeu ; la mention **VU À L'ÉCRAN** n'est donnée que si un essai l'a montré, sinon **LOGIQUE SEULEMENT** (tests verts) ou **PAS FAIT**.

## Ordre de priorité

| Ordre | Lot | Lignes | Ce que le joueur y gagne | État au 2026-11-23 17:2x |
|---|---|---|---|---|
| 1 | Fiabiliser les essais joués ce soir | BUG wall_line_long, D899, D897, faction_aim_free | Les murs en ligne se construisent au relâché ; la tour garnie tire 1 + occupants flèches ; la Lumière sacrée se pose sous la souris | main : D897 (vu par l'utilisateur : REGLAGE rendu) ; integ-63 : correctif des Paysans (8ffb822), volée (565b88a) ; essai `faction_aim_free` donné, sans réponse |
| 2 | Premier pouvoir jouable (reste) | S1-b à S1-e | L'Aube gagne de l'Honneur (soin et mort) et lance *Lumière sacrée* ; le Moine pose son Sceau | main : lancer joueur et IA ; integ-63 : Honneur durci (565b88a) ; Moine : non revérifié |
| 3 | Moral, sous-étapes 5 à 10 | S2-a à S2-d | Triomphe, Démoralisé et Hésitation s'appliquent après un duel | main : S2-c étape 8 immunité ; integ-63 : remise à zéro de l'état statique ; le reste non revérifié ; Q-moral-attenuation ouverte |
| 4 | Murs : grille fine puis portes | D898, F9, F10 | Un vrai mur en bois ou en pierre, tracé début / fin | rien commencé : PLAN d'abord ; Q-murs-grille-fine et Q-ligne-case-bloquee ouvertes |
| 5 | IA | S4-a, S4-b | L'adversaire utilise ses pouvoirs et ses garnisons | main : S4-a branché, `ai_sacred_light` et `ai_garrison` joués OK par l'utilisateur |

S2 et S3 peuvent avancer en parallèle de S1. S1-a, S1-e, S2-a, S2-b et S3-a touchent `HOLCombatProcessor` ou la simulation : un seul agent à la fois sur ce fichier, un build groupé (la volée D899 touche `HOLCombatProcessor.cpp:844-848`).

## État point par point

### 1. HUD de faction et livre de pouvoirs : À MOITIÉ (lot S1)

- **DANS MAIN** : jauge, recharges, panneau cliquable, RPC `ServerCastFactionPower`, soin de zone ; **lancer du joueur au point visé** (e809222, D897) : `faction_click_cast` rendu REGLAGE par l'utilisateur à 15:59 (« ca se met auto au milieu de l'armée, je devrai pouvoir choisir avec la position de ma souris ») ; **`ai_sacred_light` : OK** (même message, **VU À L'ÉCRAN** par l'utilisateur).
- **EN INTÉGRATION (integ-63)** : 731560e, scénario `faction_aim_free` (l'utilisateur clique la case, puis le sol) ; 565b88a : `CreditHonorPoints` gardé (`ParmsSizeMatches`) et Honneur remis à zéro en fin de monde (tests `Zone.HonorWorldEnd`, `Zone.HonorGuard`).
- **`faction_aim_free`** : **VU À L'ÉCRAN par l'agent** RECETTE-1123pc (PASS, 2 PNG : anneaux dorés au point visé, jauge 100 -> 85, PV du héros 180 -> 245, `Heal report` 0 -> 498) **sur un clic scripté à 5 m** ; GESTE réel non joué par un humain, SON non entendu ; l'utilisateur n'a pas répondu (essai donné vers 17:1x, fiche `Retours\E-faction_aim_free.md`). Défauts vus : refus à plus de 12 m **silencieux** (Q-lumiere-sacree-visee ouverte), grand cercle pâle d'identité inconnue, barres de vie illisibles.
- **Honneur du soin et de la mort (S1-e)** : sur la branche zone-effect (tests `Zone.HealHonor`, `Zone.KillHonor` à relancer : 2166/2166 PASS au build 17:14) ; **LOGIQUE SEULEMENT**, pas dans `main`.
- **Pas fait** : le crochet explicite `UHOLSimulationSubsystem::Deinitialize` -> `ResetHonor()` (une ligne, signalée par FARAMIR-1123pf) ; le type de retour des fonctions d'Honneur n'est pas vérifié.
- **Détail** : [AubeInProgress.md](../AubeInProgress.md) A4 et A6a ; D689, D859, D862, D890, D891, D897.

### 2. Capacités visées au sol : À MOITIÉ (S1-d et restes)

- **DANS MAIN** : colonnes `Target`, `RadiusCm`, `RangeCm`, lignes `LightSeal` et `KnightChallenge`, zone qui ralentit ; Sceau du Moine en script (intégré avant ce soir d'après la liste de 10:12, non revérifié).
- **Non revérifié ce soir** : le bouton du Moine (liaisons C++ d'Elrond, `LightSeal` dans la grille de commandes) : **PAS FAIT** à l'écran au dernier relevé sûr (10:12), à relire dans le tableau du Jour. `KnightChallenge` sans effet en jeu (A7).
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L6a ; D690, D875.

### 3. Moteur de moral : BIEN AVANCÉ (lot S2)

- **DANS MAIN** : sous-étapes 1 à 4 et 6a ; **S2-c étape 8, immunité** (a3de24e, integ-62) : **LOGIQUE SEULEMENT**.
- **EN INTÉGRATION (integ-63)** : 2f8a019 : remise à zéro de l'état statique du moral en fin de monde (test `Morale.ProcessorStaticStateEndsWithTheWorld`, tests 2151/2151 PASS sur la branche) : **LOGIQUE SEULEMENT**.
- **Manque** : dégâts infligés et cadence (S2-a), pont du duel (S2-b), atténuation (7, bloquée par **Q-moral-attenuation**), masque réseau C++ (9) et `HOLMoraleView::Read` réel, scénario `morale_look` (10) : non revérifiés ce soir ; rien de moral n'a été joué à l'écran.
- **Détail** : [MoraleInProgress.md](../MoraleInProgress.md), `Docs/Rapports/PLAN-MORAL-1120.md` ; D866, D880.

### 4. Choix de spécialisation : CHOIX FINI, VOIE AFFICHÉE

- **DANS MAIN** : serveur, cases du Forum, 3 voies des Templiers (D868), `hol.Cheat.Specialization` ; **SP3, voie choisie affichée dans le bouton** (c1b5595, integ-62, scénario `specialization_slot` ; ses deux tests rouges `Test_SpecializationSlotKeepsShowingTheChosenVoieOnceApplied` et `Test_SpecializationSlotSaysToChooseWhenNothingIsChosen` vus rouges puis verts). La fiche `Retours\E-specialization_slot.md` portait encore à 17:04 « branche integ-25, pas encore dans main » (VU À L'ÉCRAN sur integ-25) ; la mention est périmée depuis 17:07, la fiche n'a pas été mise à jour par moi.
- **Décision** : D896 (15:28) : **A pour les trois** unités du palier 1 des Templiers (copies provisoires : Sergent du Temple, Frère infirmier, et la troisième) ; D893 : trois lignes d'unités du palier 1 sans déblocage (Celeborn, integ-41 : état non revérifié).
- **Manque** : **aucun effet des voies en jeu** (relevé `Docs/Rapports/SPEC-VOIES-1123.md`) ; questions ouvertes **Q-commanderie-batiment** (dans quel bâtiment) et **Q-sp3-libelles** (texte du bouton).
- **Détail** : [SpecializationInProgress.md](../SpecializationInProgress.md).

### 5. Récompenses du duel : À MOITIÉ (S2-b et restes)

- **DANS MAIN** : ordres d'Honneur appliqués à la jauge ; XP du duel 150 (tests verts, **essai en jeu à faire** : LOGIQUE SEULEMENT).
- **Manque** : Triomphe, Démoralisé, Hésitation (S2-b) : état de ce soir non revérifié, rien joué à l'écran.
- **Détail** : [DuelInProgress.md](../DuelInProgress.md) D7.

### 6. Fortifications : EN COURS, DEUX BUGS CONFIRMÉS (lot S3)

- **DANS MAIN** : Tour de guet en bois (données), pose en ligne (D867), règle de garnison, T7 et T7b (flèche de tour affichée et répliquée), palissade (script), EndWallDrag, scénarios `wall_line_long` et `ai_garrison`.
- **BUG `wall_line_long`** (l'utilisateur, 17:00, **VU À L'ÉCRAN** : « quand je relâche le tracé, ils ne vont pas automatiquement construire ») : **cause confirmée** par VERDICT-1123pa (`HOLPlayerController.cpp:3495-3502` n'envoie aucun Paysan ; `HOLRpcPlayer.as:1540` `NoBuilders`) ; **correctif** ARWEN-1123pe : d455b04 (test rouge, NON VU à l'exécution) + 8ffb822 (build PASS 20 s, tests 2164/2164), **en integ-63**, **jamais rejoué à l'écran** : LOGIQUE SEULEMENT. Règle N fondations / M Paysans **non tranchée** (GDD:3348 : « plusieurs Paysans en parallèle ») ; défaut codé : chaque Paysan reçoit chaque cellule, la 1re cellule suit Shift, les suivantes en file.
- **D898 (grille fine)** : « une case se divise en 4 cases plus petites, le plus important dans le tracé est le début et la fin » : **PAS FAIT** (PLAN d'abord) ; 6 points à préciser ; **Q-murs-grille-fine** (palissade, coût par sous-case, sous-case bloquée) et **Q-ligne-case-bloquee** ouvertes.
- **D899 (volée de la tour garnie, 1 + occupants)** : `ai_garrison` **OK vu par l'utilisateur** mais il a noté que la tour ne tirait qu'une flèche ; VERDICT-1123pa : le code était conforme à D884 (compte 1 tant que non tranché) ; correctif FARAMIR-1123pg `GetBuildingArrowCount = 1 + Max(0, occupants)` (27f7476 test, 565b88a correctif, test `BuildingShot.ArrowsPerOccupant`), **en integ-63**, **LOGIQUE SEULEMENT** (flèches superposées à l'affichage, car même point de départ : non vérifié). Si un autre bâtiment armé apparaît (Bastion, forum), la règle sera à borner par type.
- **`tower_wood`** : fiche `Retours\E-tower_wood.md` [À ESSAYER], VU À L'ÉCRAN sur la branche zone-effect, pas dans `main` au listage de 17:04.
- **Manque** : portes (F9), murs (F10) ; T8 (coût par pas, banc 200 unités et 10 tours) non revérifié.
- **Détail** : [FortificationsInProgress.md](../FortificationsInProgress.md) F5, F8, F9, F10.

### 7. IA d'une autre faction : RÈGLES + BRANCHEMENT, ESSAIS JOUÉS OK (lot S4)

- **DANS MAIN** : `HOLAIPowerSpend`, `HOLAIGarrison`, règle S4-a branchée (`ThinkPowers`), lancer de l'IA à 1200 cm (e809222) ; scénario `ai_garrison` (bde5deb).
- **Essais de l'utilisateur** : `ai_sacred_light` **OK** (VU À L'ÉCRAN, 15:59) ; `ai_garrison` **OK** (VU À L'ÉCRAN, 17:00), avec le doute sur la volée, devenu D899. S4-b (`HOLAIGarrisonCall.as`, `Plan`) : branché au cerveau d'après l'essai joué ; le détail du branchement n'a pas été relu ici.
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L12, [AubeInProgress.md](../AubeInProgress.md) A12.

## Questions ouvertes (`Retours\`, listage de 17:04 et relevé de 17:2x : toujours 6, aucune tranchée)

- `Q-commanderie-batiment` : dans quel bâtiment se produisent les 3 unités de commanderie des Templiers (contenu non relu, titre seulement).
- `Q-ligne-case-bloquee` : une case bloquée sur la ligne de palissade refuse-t-elle toute la ligne ?
- `Q-lumiere-sacree-visee` : que doit faire le jeu quand on vise hors de portée (aujourd'hui : rien à l'écran, refus silencieux).
- `Q-moral-attenuation` : un effet de moral doit-il s'estomper avant sa fin (contenu non relu, titre seulement).
- `Q-murs-grille-fine` : palissade sur la grille fine, coût par sous-case, sous-case bloquée sur le tracé (recommandation A / A / A).
- `Q-sp3-libelles` : que lit le joueur sur le bouton de spécialisation (contenu non relu, titre seulement).
- Essais en attente de réponse : `E-faction_aim_free` (donné vers 17:1x), `E-talents_heal` et `E-tower_wood` [À ESSAYER] ; `E-deploy` PAS PRÊT ; `E-playtest_hero_army` LOGIQUE SEULEMENT (audit Sanity en échec) ; deux BUG de rotation en mêlée (`BUG-melee-spin-sanity`, `BUG-playtest_hero_army-spin`).
- Les questions du matin (`Q-sacred-light-values`, `Q-palissade`, `Q-spec-voies`, `Q-empreinte-jauge`, `Q-suivi-mutuel`) ne figurent plus dans `Retours\` au listage de 17:04 : leur réponse exacte n'a pas été relue ici.

## Déjà fait (ne pas refaire)

- Socle multi-faction L0 et L2 : rôles (`Factions.csv`), surcharges, colonne `Faction`, grilles filtrées, refus serveur ; technologies filtrées ; catégorie « Profane » (L3) ; faction lue par l'IA (G4).
- **Aube A1 (Hallebardier)** dans la Caserne, l'IA le connaît.
- **Aube A2 (Chevalier Vertueux, redirection passive 30 % / 8 m)** : essai `virtuous_knight` OK le 2026-11-21.
- **Essais joués par l'utilisateur le 2026-11-23** : `ai_sacred_light` OK, `faction_click_cast` REGLAGE (D897, corrigé dans `main`), `ai_garrison` OK (+ D899), `wall_line_long` BUG (+ D898), tous reportés dans le GDD et archivés.
- **Dans `main` depuis le 2026-11-23** : soin de zone *Lumière sacrée* (S1-a), flèche de tour (T7, T7b), règle et branchement S4-a, panneau cliquable (S1-c), XP du duel 150, palissade, lancer du joueur et de l'IA au point visé, immunité S2-c étape 8, voie affichée SP3.
- Relevé des effets des voies de spécialisation fait (lecture seule) : aucun effet en jeu.
- D860 (marque de signal et Alt + clic sur la mini-carte), D861 (engin démonté qui attaque ; le reste du chantier deploy est D877).

## Règles de séance à garder

Celles de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey.md` (test rouge d'abord sur la vraie chaîne, `Tools\build.ps1 -LiveCoding=0`, `quick_check.ps1 -Parts`, scénario pour tout essai demandé, valeurs proposées puis choisies par l'utilisateur (D345), commits en anglais avec la ligne `Agent: <NOM>` en dernière ligne, fusion locale à chaque passe verte (D888), **jamais de push**). Code neuf en Angelscript, C++ par exception avec sa raison (D839). Mettre à jour le dossier `*InProgress.md` concerné et `EtapeDefini.md` à chaque avancée.
