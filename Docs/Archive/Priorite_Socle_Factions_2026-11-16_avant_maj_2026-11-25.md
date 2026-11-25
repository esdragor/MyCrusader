# Priorité : le socle commun des factions (relevé du 2026-11-16, remis à jour le 2026-11-24 soir vers 21:00)

**Pour qui** : la prochaine séance Auto Journey (orchestrateur et agents qu'il lance).
**Consigne de l'utilisateur** : concentrer la séance sur ce qui manque pour qu'**une faction quelconque** (Aube, Légions, puis les autres) puisse avoir son identité jouable. Ce document ne remplace pas les dossiers `*InProgress.md` : il dit **quoi faire en premier** et renvoie à l'étape qui porte le détail. Les tâches sont les lignes S1 à S4 de la section « Socle commun des factions » d'`Auto_Journey_File.md`.

**État relevé le 2026-11-24 soir vers 21:00** (GLORFINDEL-1124sg) d'après le `git log` de `main` (HEAD fddd44c = integ-72 fusionnée ; integ-71 fusionnée avant, 20:06), les rapports `Docs\Rapports\*-1124s*.md` (ARWEN, CELEBORN, ELROND, FARAMIR, GALADRIEL, GLORFINDEL, SAURON), `git branch --no-merged main` et le journal du worktree Glorfindel. Pas de relecture du code ; aucun essai rejoué par moi. **Avant de lancer une ligne, revérifier dans `main`** (`Tools\AutoJourney\status.ps1 -InMain <sha>`). Anciennes versions : `Docs/Archive/Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-24_soir.md` (celle du 2026-11-23 17:2x), `..._avant_maj_2026-11-23_soir.md`, `..._avant_maj_2026-11-23.md`, `..._avant_maj_2026-11-22.md`.

**Trois états, à ne pas mélanger** :
- **DANS MAIN (sans C++ sauf mention)** : D908, l'Honneur d'un lancer approché est contrôlé à l'ordre, refusé d'emblée s'il manque, payé à l'arrivée (5b7ddb6, test rouge 399989c) ; D911, avis « Honneur insuffisant » à l'arrivée refusée (0da0a6f, test rouge bfe3fcd, textes EN/JA dans `Game.po`) ; `hol.Morale.Report` / `hol.Morale.Clear` (ceb8c30, C++ de Galadriel, lit et vide les unités d'un camp) ; pont du duel S2-b branché et Triomphe / Démoralisé / Hésitation (HOLDuelLedger.as L191-212 et 311-320, HOLDuelRewards.as, ca50207 ; test d8a0b81 ; relevés ARWEN-1124sd et GLORFINDEL-1124sf) ; scénarios `sacred_light_heal` au clic réel (2ae0003, soin total 765), `tower_wood` au geste réel (71d4ae9), `tower_wood_garrison` (eb7e87a ; fiche `E-tower_wood_garrison`, 3 Lanciers dans la tour, preflight PASS) ; outils de fin de séance (`session_gate` L27-12, `play.ps1 -Hidden` qui rend 2 sur un timeout de scénario, SAURON-1124sf). Hors socle : lanceur 5 à 7 (accueil WPF, icône de zone de notification), traductions des nouveautés obligatoires. **Non revérifié ce soir** : integ-63 (Paysans des palissades 8ffb822, volée 565b88a, Honneur durci, moral remis à zéro 2f8a019) et integ-62 : `status.ps1 -InMain` avant de s'y appuyer.
- **EN BRANCHE, pas dans main** : **integ-73** (20:52, en cours) : test rouge « l'IA veut une Tour de guet par niveau » 5b5b026 (stub `HOLAITower`), masque réseau d'effets de moral d'une unité (A2 étape 9, 02b6cdd et test rouge fdf22c3, **C++ NON COMPILÉ**), test rouge D909 « un ordre du héros annule le sort en attente » et D911 2a0d691, test rouge de l'aperçu de visée qui marque où le héros s'arrête quand le point est hors de portée 592240a, test rouge du bouton de spécialisation 25d2dff ; `socle-power-1124sc/sf` : D909 fait (8bc37e7, ToggleDeploy, DropOff, Garrison annulent le sort en attente) ; `arwen-1124se` : `VoieNameOf` (D905, Fer / Foi / Pierre, 21bea82) ; `faramir-1124sg` : `HOLAITower.as` (aa19ecf, **l'IA ne bâtissait aucune tour avant ce soir**) ; `faramir-1124se` : `TowersToEmpty`, sortie de garnison de l'IA (bf1f453) ; `walls-g7-1124s` : `FillDiagonalGaps` de la grille fine (43db655, CELEBORN-1124si) ; `loc-ja-1124s` : `SetLanguageName` (b7345ad, C++ de liaison, lot C++) ; le masque réseau du moral existe aussi sur `socle-ai` (fad4af7).
- **RESTE, marqué** : **C++** : effet de `KnightChallenge`, masque réseau du moral (en integ-73, non compilé), `hol.Cheat.Resources` qui ne donne qu'au slot 0 (FARAMIR-1124si), `wall_fine_diagonal` (mesure navmesh de la fente), la liaison `SetLanguageName`. **Sans C++** : effet des voies de spécialisation, atténuation du moral (Q ouverte), `HOLMoraleView::Read` et `morale_look`, portes et murs F9 / F10, scénario `wall_fine_block` (FAIL preflight 7, timeout étape 43, CELEBORN-1124se), scénario `ai_tower` (timeout corrigé par `wait 55`, non rejoué), essais joués par un humain des scénarios au geste réel.

**En une phrase** : dans `main`, le joueur lance *Lumière sacrée* au point visé, l'Honneur manquant est refusé tout de suite avec un avis, et un duel gagné donne Triomphe au vainqueur et Démoralisé au vaincu (vu au journal, pas à l'écran) ; ce qui manque encore à l'écran : l'effet du moral sur les unités, le Sceau du Moine (FAIT EN BRANCHE, non fusionné : `monk-seal-1124s` e5d68be, `ground-cast-1124s` a4d6a48) et l'effet de *KnightChallenge* (C++), la volée de la tour garnie et les Paysans des palissades (integ-63, à revérifier), la grille fine des murs (branche `walls-g7-1124s`), des tours bâties par l'IA (branche `faramir-1124sg`). « Fait » veut dire ici : appelé en jeu ; la mention **VU À L'ÉCRAN** n'est donnée que si un essai l'a montré, sinon **LOGIQUE SEULEMENT**.

## Ordre de priorité

| Ordre | Lot | Lignes | Ce que le joueur y gagne | État au 2026-11-23 17:2x |
|---|---|---|---|---|
| 1 | Fiabiliser les essais joués | BUG wall_line_long, D899, D897, D908, D909, D911 | Les murs se construisent au relâché ; la tour garnie tire 1 + occupants ; un sort refusé le dit ; un ordre du héros annule son sort en attente | main : D897, D908, D911 ; branche : D909 (8bc37e7, test rouge en integ-73) ; Paysans et volée : integ-63, non revérifié |
| 2 | Premier pouvoir jouable (reste) | S1-b à S1-e | L'Aube lance *Lumière sacrée* et gagne de l'Honneur ; le Moine pose son Sceau | main : lancer joueur et IA, Honneur contrôlé à l'ordre, avis de refus, `sacred_light_heal` au clic réel ; Sceau du Moine FAIT EN BRANCHE (`monk-seal-1124s` e5d68be, `ground-cast-1124s` a4d6a48, à fusionner) ; reste **C++** : `KnightChallenge` |
| 3 | Moral, sous-étapes 5 à 10 | S2-a à S2-d | Triomphe, Démoralisé et Hésitation s'appliquent après un duel | main : pont du duel S2-b, Triomphe / Démoralisé / Hésitation, `hol.Morale.Report/Clear` ; integ-73 : masque réseau (**C++**, non compilé) ; reste sans C++ : atténuation (Q ouverte), `HOLMoraleView::Read`, `morale_look` |
| 4 | Murs : grille fine puis portes | D898, F9, F10 | Un vrai mur en bois ou en pierre, tracé début / fin | branche `walls-g7-1124s` : `FillDiagonalGaps` ; `wall_fine_block` FAIL (timeout étape 43) ; reste : `wall_fine_diagonal` (**C++**), portes et murs F9 / F10 (sans C++) |
| 5 | IA | S4-a, S4-b | L'adversaire utilise ses pouvoirs, ses garnisons et bâtit des tours | main : S4-a, `ai_sacred_light`, `ai_garrison` OK ; branches : `HOLAITower` (faramir-1124sg), `TowersToEmpty` (faramir-1124se), integ-73 en cours ; `ai_tower` non rejoué |

S2 et S3 peuvent avancer en parallèle de S1. S1-a, S1-e, S2-a, S2-b et S3-a touchent `HOLCombatProcessor` ou la simulation : un seul agent à la fois sur ce fichier, un build groupé (la volée D899 touche `HOLCombatProcessor.cpp:844-848`).

## État point par point

### 1. HUD de faction et livre de pouvoirs : À MOITIÉ (lot S1)

- **DANS MAIN** : jauge, recharges, panneau cliquable, RPC `ServerCastFactionPower`, soin de zone ; **lancer du joueur au point visé** (e809222, D897) : `faction_click_cast` rendu REGLAGE par l'utilisateur à 15:59 (« ca se met auto au milieu de l'armée, je devrai pouvoir choisir avec la position de ma souris ») ; **`ai_sacred_light` : OK** (même message, **VU À L'ÉCRAN** par l'utilisateur).
- **EN INTÉGRATION (integ-63)** : 731560e, scénario `faction_aim_free` (l'utilisateur clique la case, puis le sol) ; 565b88a : `CreditHonorPoints` gardé (`ParmsSizeMatches`) et Honneur remis à zéro en fin de monde (tests `Zone.HonorWorldEnd`, `Zone.HonorGuard`).
- **`faction_aim_free`** : **VU À L'ÉCRAN par l'agent** RECETTE-1123pc (PASS, 2 PNG : anneaux dorés au point visé, jauge 100 -> 85, PV du héros 180 -> 245, `Heal report` 0 -> 498) **sur un clic scripté à 5 m** ; GESTE réel non joué par un humain, SON non entendu ; l'utilisateur n'a pas répondu (essai donné vers 17:1x, fiche `Retours\E-faction_aim_free.md`). Défauts vus : refus à plus de 12 m **silencieux** (Q-lumiere-sacree-visee ouverte), grand cercle pâle d'identité inconnue, barres de vie illisibles.
- **Honneur du soin et de la mort (S1-e)** : sur la branche zone-effect (tests `Zone.HealHonor`, `Zone.KillHonor` à relancer : 2166/2166 PASS au build 17:14) ; **LOGIQUE SEULEMENT**, pas dans `main`.
- **Pas fait** : le crochet explicite `UHOLSimulationSubsystem::Deinitialize` -> `ResetHonor()` (une ligne, signalée par FARAMIR-1123pf) ; le type de retour des fonctions d'Honneur n'est pas vérifié.
- **Détail** : [AubeInProgress.md](../AubeInProgress.md) A4 et A6a ; D689, D859, D862, D890, D891, D897.
- **Mise à jour 2026-11-24 soir** (les lignes ci-dessus datent du 2026-11-23 17:2x) : **main** a D908 (Honneur contrôlé à l'ordre, payé à l'arrivée), D911 (avis de refus) ; **branche** D909 (8bc37e7) et aperçu de visée hors de portée (test rouge 592240a, integ-73) ; reste le crochet `Deinitialize` -> `ResetHonor()` (C++, une ligne).

### 2. Capacités visées au sol : À MOITIÉ (S1-d et restes)

- **DANS MAIN** : colonnes `Target`, `RadiusCm`, `RangeCm`, lignes `LightSeal` et `KnightChallenge`, zone qui ralentit ; Sceau du Moine : voir la mise à jour du 2026-11-24 soir (fait en branche, pas encore dans main).
- **Mise à jour 2026-11-24 soir** : le Sceau du Moine est **FAIT EN BRANCHE** (pas encore dans main) : `monk-seal-1124s` e5d68be (case de grille, clic sol, case R VUE à l'écran) et `ground-cast-1124s` a4d6a48 (ralentissement mesuré 0,66) ; plus de C++ pour le Sceau, reste à fusionner ces deux branches (le rouge voulu du scénario `monk_seal`, ELROND-1124sa, passe alors au vert). `KnightChallenge` sans effet en jeu : **C++** aussi (GLORFINDEL-1124sf, « point 2 : reste = C++ »).
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L6a ; D690, D875.

### 3. Moteur de moral : BIEN AVANCÉ (lot S2)

- **DANS MAIN** : sous-étapes 1 à 4 et 6a ; **S2-c étape 8, immunité** (a3de24e, integ-62) : **LOGIQUE SEULEMENT**.
- **EN INTÉGRATION (integ-63)** : 2f8a019 : remise à zéro de l'état statique du moral en fin de monde (test `Morale.ProcessorStaticStateEndsWithTheWorld`, tests 2151/2151 PASS sur la branche) : **LOGIQUE SEULEMENT**.
- **Manque** : dégâts infligés et cadence (S2-a), pont du duel (S2-b), atténuation (7, bloquée par **Q-moral-attenuation**), masque réseau C++ (9) et `HOLMoraleView::Read` réel, scénario `morale_look` (10) : non revérifiés ce soir ; rien de moral n'a été joué à l'écran.
- **Détail** : [MoraleInProgress.md](../MoraleInProgress.md), `Docs/Rapports/PLAN-MORAL-1120.md` ; D866, D880.
- **Mise à jour 2026-11-24 soir** : **main** : pont du duel S2-b, `hol.Morale.Report/Clear` ; **integ-73** : masque réseau des effets (étape 9, C++ non compilé) ; reste S2-a (dégâts et cadence, C++ `HOLCombatProcessor`), atténuation (7, Q-moral-attenuation), `HOLMoraleView::Read`, `morale_look` (10).

### 4. Choix de spécialisation : CHOIX FINI, VOIE AFFICHÉE

- **DANS MAIN** : serveur, cases du Forum, 3 voies des Templiers (D868), `hol.Cheat.Specialization` ; **SP3, voie choisie affichée dans le bouton** (c1b5595, integ-62, scénario `specialization_slot` ; ses deux tests rouges `Test_SpecializationSlotKeepsShowingTheChosenVoieOnceApplied` et `Test_SpecializationSlotSaysToChooseWhenNothingIsChosen` vus rouges puis verts). La fiche `Retours\E-specialization_slot.md` portait encore à 17:04 « branche integ-25, pas encore dans main » (VU À L'ÉCRAN sur integ-25) ; la mention est périmée depuis 17:07, la fiche n'a pas été mise à jour par moi.
- **Décision** : D896 (15:28) : **A pour les trois** unités du palier 1 des Templiers (copies provisoires : Sergent du Temple, Frère infirmier, et la troisième) ; D893 : trois lignes d'unités du palier 1 sans déblocage (Celeborn, integ-41 : état non revérifié).
- **Manque** : **aucun effet des voies en jeu** (relevé `Docs/Rapports/SPEC-VOIES-1123.md`) ; questions ouvertes **Q-commanderie-batiment** (dans quel bâtiment) et **Q-sp3-libelles** (texte du bouton).
- **Détail** : [SpecializationInProgress.md](../SpecializationInProgress.md).
- **Mise à jour 2026-11-24 soir** : **branche** `arwen-1124se` : `VoieNameOf` (D905, noms Fer / Foi / Pierre, NSLOCTEXT, EN et JA) et test rouge du bouton en integ-73 (25d2dff) ; toujours **aucun effet des voies en jeu** (sans C++).

### 5. Récompenses du duel : À MOITIÉ (S2-b et restes)

- **DANS MAIN** : ordres d'Honneur appliqués à la jauge ; XP du duel 150 (tests verts, **essai en jeu à faire** : LOGIQUE SEULEMENT).
- **Mise à jour 2026-11-24 soir, FAIT dans main, sans C++** : Triomphe, Démoralisé et Hésitation (HOLDuelLedger.as L191-212, HOLDuelRewards.as, ca50207) ; pont du duel S2-b : `AdvanceDuels` (HOLDuelLedger.as:311-320) appelle `ApplyMoraleOrders`, qui vide `PendingMorale` vers `HOLMorale::ApplyEffectAround` (test d8a0b81). **Vu au journal** à 20:50 dans `duel_challenge_accept` (worktree Glorfindel) : « Duel morale: Triumph slot 0 » et « Demoralised slot 1 », 150 XP : **LOGIQUE SEULEMENT**. **Reste** : essai joué par un humain ; effet visible du moral sur les unités (`morale_look`, sans C++).
- **Détail** : [DuelInProgress.md](../DuelInProgress.md) D7.

### 6. Fortifications : EN COURS, DEUX BUGS CONFIRMÉS (lot S3)

- **DANS MAIN** : Tour de guet en bois (données), pose en ligne (D867), règle de garnison, T7 et T7b (flèche de tour affichée et répliquée), palissade (script), EndWallDrag, scénarios `wall_line_long` et `ai_garrison`.
- **BUG `wall_line_long`** (l'utilisateur, 17:00, **VU À L'ÉCRAN** : « quand je relâche le tracé, ils ne vont pas automatiquement construire ») : **cause confirmée** par VERDICT-1123pa (`HOLPlayerController.cpp:3495-3502` n'envoie aucun Paysan ; `HOLRpcPlayer.as:1540` `NoBuilders`) ; **correctif** ARWEN-1123pe : d455b04 (test rouge, NON VU à l'exécution) + 8ffb822 (build PASS 20 s, tests 2164/2164), **en integ-63**, **jamais rejoué à l'écran** : LOGIQUE SEULEMENT. Règle N fondations / M Paysans **non tranchée** (GDD:3348 : « plusieurs Paysans en parallèle ») ; défaut codé : chaque Paysan reçoit chaque cellule, la 1re cellule suit Shift, les suivantes en file.
- **D898 (grille fine)** : « une case se divise en 4 cases plus petites, le plus important dans le tracé est le début et la fin » : **PAS FAIT** (PLAN d'abord) ; 6 points à préciser ; **Q-murs-grille-fine** (palissade, coût par sous-case, sous-case bloquée) et **Q-ligne-case-bloquee** ouvertes.
- **D899 (volée de la tour garnie, 1 + occupants)** : `ai_garrison` **OK vu par l'utilisateur** mais il a noté que la tour ne tirait qu'une flèche ; VERDICT-1123pa : le code était conforme à D884 (compte 1 tant que non tranché) ; correctif FARAMIR-1123pg `GetBuildingArrowCount = 1 + Max(0, occupants)` (27f7476 test, 565b88a correctif, test `BuildingShot.ArrowsPerOccupant`), **en integ-63**, **LOGIQUE SEULEMENT** (flèches superposées à l'affichage, car même point de départ : non vérifié). Si un autre bâtiment armé apparaît (Bastion, forum), la règle sera à borner par type.
- **`tower_wood`** : fiche `Retours\E-tower_wood.md` [À ESSAYER], VU À L'ÉCRAN sur la branche zone-effect, pas dans `main` au listage de 17:04.
- **Manque** : portes (F9), murs (F10) ; T8 (coût par pas, banc 200 unités et 10 tours) non revérifié.
- **Détail** : [FortificationsInProgress.md](../FortificationsInProgress.md) F5, F8, F9, F10.
- **Mise à jour 2026-11-24 soir** : **branche** `walls-g7-1124s` : `FillDiagonalGaps` (43db655) ; `wall_fine_block` FAIL preflight (7), timeout étape 43 (CELEBORN-1124se) ; budget 200 unités et 10 tours : moyenne 69 par pas, plan F9 dans `FortificationsInProgress.md` (FARAMIR-1124sb) ; reste portes F9 et murs F10 (sans C++), `wall_fine_diagonal` (C++).

### 7. IA d'une autre faction : RÈGLES + BRANCHEMENT, ESSAIS JOUÉS OK (lot S4)

- **DANS MAIN** : `HOLAIPowerSpend`, `HOLAIGarrison`, règle S4-a branchée (`ThinkPowers`), lancer de l'IA à 1200 cm (e809222) ; scénario `ai_garrison` (bde5deb).
- **Essais de l'utilisateur** : `ai_sacred_light` **OK** (VU À L'ÉCRAN, 15:59) ; `ai_garrison` **OK** (VU À L'ÉCRAN, 17:00), avec le doute sur la volée, devenu D899. S4-b (`HOLAIGarrisonCall.as`, `Plan`) : branché au cerveau d'après l'essai joué ; le détail du branchement n'a pas été relu ici.
- **Détail** : [LegionsInProgress.md](../LegionsInProgress.md) L12, [AubeInProgress.md](../AubeInProgress.md) A12.
- **Mise à jour 2026-11-24 soir** : **branches** `faramir-1124sg` (`HOLAITower.as` : l'IA ne bâtissait AUCUNE tour, la garnison S4-b ne lisait donc rien en partie ; TowerWood a 3 places, 482e26b), `faramir-1124se` (`TowersToEmpty`, sortie de garnison) ; `ai_tower` : timeout corrigé par `wait 55` (FARAMIR-1124si), non rejoué ; `hol.Cheat.Resources` ne donne qu'au slot 0 (C++, HOLPlayerController.cpp:3752).

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
