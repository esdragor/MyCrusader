# Interface — avancement

Chantier de l'interface en jeu (HUD), hors parcours du joueur. Chaque étape est notée ici dès qu'elle avance.

## Grille du Paysan façon AoE4

Demande de l'utilisateur (capture d'AoE4, journal de J7b.6 du 2026-10-20 dans `JourneyInProgress.md`) : chez les
Paysans d'AoE4, les **ordres** sont **à gauche** et la **construction à droite**, en **onglets par âge** (I à IV)
choisis par **A Z E R**. Déjà fait : l'attaque-déplacement est passée de A à Q (0f20427, 2026-10-20).

### Choix de l'utilisateur (2026-10-21, les recommandations sauf la 3e)

1. **Onglets** : **4 onglets toujours visibles**, I à IV = paliers 0 à 3 (Fondation, Essor, Puissance, Légende ;
   GDD § 11.1). L'onglet I contient Maison, Camp de bûcherons, Camp de mineurs, Caserne ; II à IV **grisés** (« rien à
   bâtir pour l'instant ») jusqu'aux paliers. Nouvelle colonne **palier** dans `BuildingTypes.csv`. Écartés : onglet I
   seul ; disposition seulement (A Z E R directs).
2. **Touches** : **deux touches, façon AoE4** : A Z E R ouvre l'onglet, puis Q S D F / W X C V choisit le bâtiment
   (Maison = A puis Q) ; Échap referme l'onglet ; hors onglet ouvert, les touches sont les ordres. À la souris, un clic
   sur une case pose directement. Écarté : onglet mémorisé (bâtiments directs, ordres déplacés).
3. **Ordres à gauche pour toutes les unités**, aux places d'AoE4 : **Q** attaque-déplacement (soldats), **S** arrêt,
   **V** maintien (au lieu de D) ; **le Paysan seul n'a toujours pas l'attaque-déplacement**. À droite : la
   construction (Paysans) ou la production (bâtiments, inchangée). Écartés : Q aussi au Paysan ; Paysans seulement.

### Conception

- **Paliers côte à côte** (G.1c, 1er essai de l'utilisateur ; Paysans choisis, aucun soldat ; sinon cachés comme
  avant, Tab vers les Paysans) : à droite du panneau d'ordres, 4 panneaux I à IV, chacun un en-tête (« I » et sa touche
  A / Z / E / R) sur ses bâtiments aux places de leurs touches Q S D F / W X C V (`BuildGridSlot` 4-11, colonne `Age`) ;
  chaque panneau a la largeur de ses colonnes utilisées, seulement ses rangées utilisées (palier vide : l'en-tête seul,
  une case de large). Palier non atteint grisé (en-tête et bâtiments) ; `HOLCommandGrid::GetReachedAge()` = 0 pour
  l'instant. Infobulles : « Palier I — Fondation », grisé « Palier non atteint ». Palier I : Maison **Q**, Camp de
  bûcherons **S**, Camp de mineurs **D**, Caserne **F**.
- **Touches** (`HOLCommandGrid::PressKey`, `ClickAge`, `GetChosenAgeAfter`) : sans palier choisi, A Z E R choisissent le
  palier (non atteint : rien), les autres touches vont aux ordres. Palier choisi (en-tête « [ I ] ») : ses touches lancent
  le fantôme de son bâtiment (case vide : rien, jamais d'ordre) ; le palier reste choisi tant que rien n'est posé ni
  annulé : une autre touche change le fantôme (A, Q puis D), A Z E R changent de palier. Pose ou annulation (Échap, clic
  droit) le referment, Maj + pose le garde ; Échap sans fantôme le referme ; changement de sélection aussi. Souris : clic
  sur un bâtiment d'un palier atteint = fantôme (palier choisi), clic sur un en-tête = palier choisi.
- **Panneau d'ordres** à gauche de la grille, mêmes places de touches : Q (soldats dans la sélection), S, V.
- Bâtiment choisi : la grille de droite inchangée (production, « Annuler » en V d'un chantier), pas d'ordres à gauche.
- Rapport `hol.HUD.Report` (Paysans) : `grid ages I=open [Q=House enabled S=LumberCamp enabled ...] II=locked [] III=locked []
  IV=locked [] (chosen none), orders S=Stop V=Hold, queue hidden` ; `hol.HUD.Click AgeHeader0`-`3`, `AgeCell<palier><case>`
  (`AgeCell004` Maison, `AgeCell007` Caserne), `OrderCell00`-`11` ; journal `Grid: age 0 chosen` / `closed` / `not reached`.

### Étapes

| Étape | État |
|---|---|
| G.1 Règles, données, HUD, touches, textes fr / en / ja, tests et `quick_check` (sous-agents) | **faite** (2026-10-21, rien de commité) : tests 88/88, `quick_check -Parts build` PASS (410 s) |
| G.2 Scénario d'essai, essai de l'utilisateur | **faite** : 4e essai **validé** (« très bien », 2026-10-21) |
| G.3 Commit, `EtapeDefini.md`, GDD (D340 ; D30, D201, D313 précisées) | **faite** (2026-10-21, `main` 3ca380b) : **chantier clos** |

### Journal

#### 2026-10-21

- mycrusader-01. Worktree `HeroesOfLiberty-J5c` pris (`IN_USE.txt`), branche **`peasant-grid`** depuis `main` ea84b7b.
  Choix de l'utilisateur ci-dessus. G.1 confiée à un sous-agent (un livrable, 15 min / 300k tokens).
- G.1, 1re passe (sous-agent, 15 min, rien de commité) : `BuildingTypes.csv` colonne **`Age`** (tous 0) et
  **`BuildGridSlot` = index de cellule de la grille 3 × 4** (Maison 4 = Q, Bûcherons 5 = S, Mineurs 6 = D, Caserne 7 = F),
  table refaite (`make_building_types.py`) ; règles pures `GetCommand` (onglets A Z E R = `AgeTab`, page 4-11 du palier
  affiché), `GetOrder` (Q attaque si soldats, S arrêt, V maintien), `PressKey` (deux touches) ; contrôleur
  `PressGridKey`, `ActivateOrderCell`, onglet ouvert refermé par Échap (en premier), un bâtiment choisi ou un changement
  de sélection ; HUD : panneau d'ordres à gauche de la grille (cases inutilisées cachées), onglets « · I · » affiché,
  « [ I ] » ouvert, grisés sinon ; rapport `orders …` et `(tabs shown I, open none)` ; `hol.HUD.Click OrderCell05` /
  `AgeTab0`, nouvelle commande `hol.Select.OrderCell N` (`hol.Select.Order` existait déjà : clic droit) ; tests
  `CommandGrid.UnitOrders` refait, `CommandGrid.AgeTabs` nouveau ; `quick_check` adapté (touches A puis S / Q / F, cellule
  05, regex du rapport, des cases, des infobulles). Compile sans avertissement. Note de `attack_move_q.txt` mise à jour.
  `quick_check -Parts tests,build` : FAIL en 437 s (tests 87/88 : `Construction.Tables` attendait les anciennes cases ;
  `build hud` : regex F1/F3 sans `orders none`) ; les deux corrigés ensuite, **pas encore recompilés ni revérifiés**.
  Reste : recompiler, relancer `quick_check -Parts tests,build`, textes en / ja (`fill_po.py`), capture du HUD à regarder.
- G.1b (sous-agent, 17:03-17:17, arrêté à la limite, rien de commité). Compilé sans avertissement avec les deux
  corrections de la 1re passe (`HOLConstructionTests.cpp`, `quick_check.ps1`). `fill_po.py` : 7 clés ajoutées (`Age0`-`3`
  Foundation / Rise / Power / Legend, 基盤 / 興隆 / 権勢 / 伝説 ; `AgeTabName` « Tier {0} — {1} » / « 段階{0}：{1} » ;
  `AgeTabEmpty`, `AgeTabEffect`) ; **GatherText pas encore relancé** (ni le décompte de clés inconnues). Aide de
  `hol.HUD.Click` (`HOLUIHUD.cpp`, en-tête et texte de la commande) : `AgeTab0`-`3`, `OrderCell00`-`11`, page 04-07 ;
  recompilation lente (machine chargée : build ProjectShiva et `-Full` d'une autre session), finie après coup : réussie,
  sans avertissement. `quick_check` : section `mark build hud ages` après le fantôme (clic `AgeTab1` grisé → « hidden,
  greyed or unknown » et `(tabs shown I, open none)` ; `hol.Select.Grid 0` puis `4` → `Placement: House (cost` ; Échap ;
  `hol.HUD.Click OrderCell05` → `Order: N units stop`) et son contrôle. Captures de la 1re passe regardées
  (`build_hud_greyed`, `build_hud_ja`, `build_hud_block`) : panneau d'ordres (S, V) à gauche de la grille, onglets
  « · I · » / II-IV grisés lisibles, rien ne déborde ; chantier choisi : pas d'ordres. `quick_check` **pas lancé**.
  Reste : GatherText, `fill_po.py`, GatherText, puis `quick_check -Parts tests,build` une fois.
- G.1 finie par la session principale (machine très chargée : `-Full` d'Elrond, ProjectShiva ; la recompilation de
  G.1b a pris ~10 min) : GatherText, `fill_po.py` (en / ja : 178 textes, 0 clé inconnue), GatherText ;
  `quick_check -Parts tests,build` : tout PASS sauf le contrôle neuf « build hud ages », dont l'attente était fausse
  (un onglet grisé prend le clic, comme les cases grisées, et ne fait rien : `HUD click AgeTab1: done`, aucun « age tab
  1 open ») ; attente corrigée, `-Parts build` **PASS** (410 s). Capture `build_hud_ghost` : ordres S / V à gauche,
  onglets I (actif) à IV (grisés), page Q S D F. Leçon de processus : `quick_check` une seule fois, en dernier, lancé
  par la session principale pendant que les sous-agents ne font que du code.
- G.2 : scénario `Tools/Scenarios/peasant_grid.txt` (6 Paysans, 4 Hommes d'armes, 5000 de tout), essai lancé.
- **1er essai de l'utilisateur (2026-10-21), 2 retours** (avec une capture d'AoE4 : 4 panneaux côte à côte, « I A »,
  « II Z », « III E », « IV R », chacun de la largeur de son contenu, III et IV estompés) :
  1. **Pas d'onglets** : les 4 paliers sont **tous affichés côte à côte**, chacun avec son en-tête (chiffre romain et
     sa touche) et ses bâtiments ; un palier **pas encore débloqué** est affiché **grisé** (ses bâtiments visibles mais
     grisés). Revient sur le choix 1 (onglets) : A Z E R **choisissent un palier**, ils n'affichent plus une page.
  2. **Changement de bâtiment pendant la pose** : A puis Q → fantôme de la Maison ; puis D → fantôme du Camp de mineurs
     à la place : le palier choisi reste actif **tant que rien n'est posé ni annulé** (les touches du palier changent
     le bâtiment au lieu d'aller aux ordres). Retenu : une pose ou une annulation (Échap, clic droit) le referme ; Maj
     garde le fantôme et le palier pour les poses en série.
  Touches gardées (dites par l'utilisateur) : dans un palier, Q S D F / W X C V (Maison Q, bûcherons S, mineurs D,
  Caserne F). G.1c confiée à un sous-agent.
- G.1c (sous-agent, 17:46-18:06, rien de commité) : **paliers côte à côte**. Règles pures : `EHOLGridCommand::AgeTab`,
  `HasAgeBuildings`, `ShownAge` supprimés ; `GetReachedAge` (0), `IsAgeReached`, `GetAgeBuilding`, `PressKey` (cibles
  `Age` / `Build`, `ChosenAge`, `TypeName`), `ClickAge`, `GetChosenAgeAfter` (`EHOLAgeEvent`) ; `GetCommand` ne donne plus
  rien aux bâtisseurs. Contrôleur : `ChosenAge` (`SetChosenAge`, `ApplyAgeKey`, `ClickAge`), refermé par la pose (gardé
  avec Maj), le clic droit, Échap, la sélection. HUD : 4 panneaux `AgePanel0`-`3` (en-tête `AgeHeaderN`, cases
  `AgeCell<palier><case>` repliées hors colonnes / rangées utilisées), rapport `ages ... (chosen ...)`, cellules et
  infobulles `I A: ...; I Q: ...`. Test `CommandGrid.AgeTabs` → `CommandGrid.Ages`. `fill_po.py` : `AgeTabName`,
  `AgeTabEmpty`, `AgeTabEffect` retirées, `AgeHeaderName`, `AgeLocked` ajoutées (**GatherText pas relancé**).
  `quick_check` : clics `AgeCell005`, section `build hud ages` réécrite (en-tête II : rien ; A, Q : Maison ; D : Camp de
  mineurs ; Échap : fin et palier refermé ; S des ordres : arrêt), regex du rapport, des cellules, des infobulles.
  Scénario `peasant_grid.txt` : 3 notes. Compilé sans avertissement (1re compilation : `Slot` masquait `UWidget::Slot`, corrigé ; `CommandGrid.Tables` lisait encore `GetCommand` pour les bâtisseurs, corrigé) ; `quick_check -Parts tests` : **88/88 PASS** (54 s). **Partie build de `quick_check` pas lancée**, captures pas regardées.
- G.1c vérifiée par la session principale : GatherText, `fill_po.py` (177 textes, 0 clé inconnue), GatherText ;
  `quick_check -Parts build` : 2 échecs, (1) **défaut réel** : l'en-tête du palier choisi (« [ I ] ») et `(chosen …)`
  du rapport ne suivaient pas le palier refermé (le HUD ne le relisait qu'au changement de sélection) → relu à chaque
  image quand il change (`NativeTick`) ; (2) regex du contrôle de la Caserne (`; I F: Caserne;`). Capture : le panneau
  d'ordres, à 3 × 4 cases, couvrait le bloc de la sélection → **resserré comme les paliers** (colonnes et rangées
  utilisées seulement, places des touches gardées). `quick_check -Parts build` **PASS** (275 s) ; contact léger avec
  le bloc de la sélection à 1600 × 900, à juger à l'essai. Reste noté : un clic sur l'en-tête II écrit aussi « Grid:
  age -1 not reached » (journal seul, sans effet ; origine non trouvée).
- 2e essai lancé (scénario `peasant_grid`).
- **2e essai de l'utilisateur (2026-10-21), 3 retours** (captures d'AoE4 : Caserne, Paysans, Homme d'armes) :
  1. « Pourquoi A est cliquable ? » : l'en-tête d'un palier n'est **pas une case** ; c'est un **médaillon** (chiffre
     romain dans un rond, sa touche à côté) posé sur le bord haut du panneau ; le palier choisi est mis en évidence
     (médaillon ou bord), sans « [ I ] ». A Z E R au clavier restent le choix du palier.
  2. « [ I ] mal placé » : le texte débordait sous l'en-tête ; réglé par le médaillon.
  3. **Positions fixes** : les panneaux (ordres, construction, production…) ne sont jamais au même endroit selon la
     sélection ; dans AoE4, **un seul panneau de commandes** (taille fixe, cases vides visibles en sombre) toujours au
     même endroit : ordres des unités, production et technologies d'un bâtiment ; **à sa droite**, le panneau
     secondaire (paliers du Paysan), aligné à gauche.
  **Choix de l'utilisateur (disposition, précise D201)** : **rangée façon AoE4** en bas, de gauche à droite : bloc de
  la sélection (largeur fixe), panneau de commandes (fixe), panneau secondaire (paliers) ; **mini-carte en bas à
  droite** (et non plus à gauche) ; **les ressources et les inactifs restent en haut**, où ils sont. Écartés :
  sélection centrée gardée ; paliers au-dessus des commandes.
- G.1d (sous-agent, 18:51-19:05, rien de commité) : **barre du bas façon AoE4**, `BottomBar` alignée en bas à gauche,
  de gauche à droite à places fixes : `SelectionPlace` (largeur fixe 340, hauteur de la barre 252 ; un `USpacer` garde
  la place quand `SelectionPanel` est replié), `CommandBox` (largeur fixe 332 : message bref, file de production
  **au-dessus** du panneau de commandes, comme avant ; cases de la file 58 → 56 pour tenir dans la largeur), puis
  `AgePanel0`-`3` (panneau secondaire, Paysans seuls). **Un seul panneau de commandes** `GridPanel` (12 cases
  `GridCell00`-`11` toujours là, les vides sombres = désactivées) : ordres des unités à soi (Q, S, V) ou grille du
  bâtiment / groupe ; caché sans rien à soi en `Hidden` (sa place gardée). `OrderPanel` et `OrderCellNN` supprimés
  (`hol.HUD.Click GridCell05` = arrêt). Groupes de contrôle remontés au-dessus du bloc de sélection ; place
  `MinimapPlace` (240, vide) en bas à droite (il n'y avait pas de mini-carte). **Médaillon** `AgeMedallionN` (chiffre
  dans un rond, touche dans un carré `AgeKeyN`) posé sur le bord haut du panneau (translation de rendu), sans clic,
  infobulle « Palier N — Nom » (« Palier non atteint » en plus) ; palier choisi : médaillon doré et bord du panneau
  doré ; non atteint : panneau estompé (opacité 0,55). `AgeHeaderN` et le clic d'en-tête supprimés (le « Grid: age -1
  not reached » venait de `ClickAge` → `ApplyAgeKey(…, INDEX_NONE)` : sans objet). Rapport inchangé (`grid …, orders
  …`) ; `HUD cells` / `HUD tooltips` : les ordres en tête (`S: Arrêt; V: Tenir; I Q: Maison; …`), plus d'en-têtes
  dans les cellules, les infobulles des médaillons gardées (`I A: Palier I — Fondation`). `quick_check` : touche Z
  (`hol.Select.Grid 1` → `Grid: age 1 not reached`, et jamais `age -1`) au lieu du clic d'en-tête, `GridCell05`, regex
  des cellules, infobulles, Caserne (`; I F: Caserne$`). Aide de `Dev/HOLUIHUD.cpp`, notes de `peasant_grid.txt`. Aucun
  texte nouveau (`fill_po.py` inchangé). Compilé sans avertissement ; `quick_check -Parts tests` **88/88 PASS**
  (28 s). **Pas lancés** : partie build, GatherText, captures. Restent à juger à l'essai : chevauchement possible de
  la barre des groupes (plus de 5 groupes) avec le message bref, barre ~1230 px de large (touche la place de la
  mini-carte sous 1500 px de large).
- Retour de l'utilisateur (capture d'AoE4, sélection de 83 unités) : « normalement, on n'aura jamais de problème de
  place pour les blocs d'unités » : le bloc de la sélection (largeur fixe) range ses unités en **pastilles** qui
  **passent à la ligne** (« Tout (83) », puis une pastille par type avec son nombre, la pastille active de Tab mise en
  évidence), au lieu d'une ligne de texte par type qui s'allonge vers le bas. À faire en G.1e.
- G.1e (sous-agent, 19:09-19:20, rien de commité) : **pastilles du bloc de sélection** : `SelectionChips` (`UWrapBox`,
  largeur du bloc, à la place de `SelectionName` dès qu'il y a plusieurs unités ou un groupe de bâtiments), 12
  pastilles `SelectionChipN` (`UHOLQueueItem`, hauteur 30, au plus une demi-largeur, découpées) dans leur cadre
  `SelectionChipFrameN` (doré pour le sous-groupe actif). Unités à soi de plusieurs types : les sous-groupes de Tab
  (`GetUnitSubgroups` : « Tous (N) », soldats, Paysans à la fin) ; un seul type : une pastille « Paysan (5) » ; à
  autrui : « Tous » puis un par type, rien d'allumé. Bâtiments (F1/F2) : les sous-groupes de `GetSubgroups` (Centre
  principal seul sans nombre). Nom de plus de 12 caractères coupé (« Homme d'arme. (3) »). **Clic** sur une pastille :
  son sous-groupe actif (Tab répété depuis l'actif, journal `HUD: chip N clicked`). Lignes « Vous » et activité
  gardées ; une unité seule inchangée. Rapport `selection …` inchangé (pas de rapport des pastilles). **Médaillons** :
  rond 28 → 40, chiffre en style titre HUD (18), touche 18 → 24 (style 13), montée 20 → 26 (panneau vide ~80 px de large).
  Texte nouveau `SelectionChip` « {0} ({1}) » dans `fill_po.py` (GatherText pas relancé). Compilé sans avertissement ;
  `quick_check -Parts tests` **88/88 PASS** (93 s). **Pas lancés** : partie build, GatherText, captures (rendu des
  pastilles et des médaillons non regardé).
- G.1e vérifiée (session principale) : GatherText, `fill_po.py` (178 textes, 0 clé inconnue), GatherText ;
  `quick_check -Parts build` **PASS** (284 s). Captures : pastilles « Paysan (5) », « Tous (8) · Centre prin. ·
  Maison (7) » (l'active dorée), médaillons lisibles, panneau de commandes à la même place pour un groupe de bâtiments.
  3e essai lancé (scénario `peasant_grid`).
- **3e essai de l'utilisateur (2026-10-21)** : « tout est super », et une demande : dans un palier, **la 1re rangée de
  bâtiments sur A Z E R** (Maison = A puis A ; Caserne = A puis Q), **une rangée par catégorie** si possible : rangée 1
  économie, rangée 2 militaire, rangée 3 la suite (technologie : les catégories de F2 / F1 / F3). Donc palier choisi :
  A Z E R / Q S D F / W X C V choisissent les bâtiments (A Z E R ne changent plus de palier ; Échap d'abord). Onglet I :
  Maison A, Camp de bûcherons Z, Camp de mineurs E, Caserne Q. G.1f confiée à un sous-agent.
- G.1f (sous-agent, 19:37-19:45, rien de commité) : **une rangée par catégorie dans les paliers**. `BuildingTypes.csv`
  `BuildGridSlot` : Maison 0 (A), Camp de bûcherons 1 (Z), Camp de mineurs 2 (E), Caserne 4 (Q) ; Centre principal -1 ;
  table refaite (`make_building_types.py` par `UnrealEditor-Cmd -run=pythonscript`). Règles pures : `FirstBuildCell`
  supprimé, `GridColumns`, `NumGridRows`, `GetCategoryRow` (Economic 0, Military 1, Technological 2) ajoutés ;
  `GetAgeBuilding` sur les cases 0-11 ; `PressKey` : sans palier choisi, A Z E R choisissent le palier (non atteint :
  rien), Q S D F / W X C V les ordres ; **palier choisi : toutes les touches 0-11 sont ses bâtiments** (case vide :
  rien, palier gardé ; A Z E R ne changent plus de palier) ; `ClickAge` sur un en-tête choisit le palier sans passer
  par `PressKey`. Contrôleur : « Grid: age N not reached » seulement sans palier choisi. Tests : `CommandGrid.*` mis à
  jour (A puis A Maison, E Mineurs, Z Bûcherons palier gardé, R / Q / V vides) ; `CommandGrid.Tables` contrôle que la
  rangée de chaque bâtiment est celle de sa catégorie ; `Construction` (cases 0, 1, 2, 4). HUD : cibles
  `AgeCell<palier><case>` sur 0-11 (Maison `AgeCell000`, Caserne `AgeCell004`), rapport / cellules / infobulles
  (`I A: Maison; I Z: …; I Q: Caserne`). **Précision de l'utilisateur** (« pas de problème si une ligne est vide ») :
  chaque panneau de palier garde **toujours ses 3 rangées**, même vides (cases sombres désactivées, comme le panneau
  de commandes) : une catégorie toujours à la même hauteur ; seules les colonnes restent resserrées (palier vide : une
  colonne). Aides `hol.HUD.Click` et `hol.Select.Grid`. `quick_check` : A, S → A, Z ; A, Q → A, A ; A, F → A, Q ;
  `AgeCell005` → `AgeCell001` ; « build hud ages » : `Grid 0, 0` Maison, `Grid 2` Mineurs, `Grid 1` Bûcherons palier
  gardé (5 rapports) ; regex du rapport, des cellules, des infobulles, de la Caserne. Notes de `peasant_grid.txt` et
  `attack_move_q.txt`. Compilé sans avertissement ; `quick_check -Parts tests` **88/88 PASS** (25 s). **Pas lancés** :
  partie build, GatherText (aucun texte nouveau), captures (panneaux à 3 rangées non regardés).
- G.1f vérifiée (session principale) : `quick_check -Parts build` **PASS** (273 s) ; capture : palier I Maison A,
  Bûcherons Z, Mineurs E, Caserne Q, 3 rangées gardées partout (II à IV compris). 4e essai lancé (`peasant_grid`).
- **4e essai validé** (« très bien »). G.3 : **D340** au GDD (grille du Paysan et barre du bas façon AoE4 ; précise
  D30, D201 — mini-carte en bas à droite —, remplace la grille directe de D313) ; `EtapeDefini.md` : ligne de la grille
  retirée, reste « barre du bas trop large sous 1500 px », review du GDD reprise à D341.
- G.3 faite : commit f87134d sur `peasant-grid`, `main` fusionné dedans (79131ef, sans conflit), `quick_check -Parts
  tests,production,combat,build` **PASS** (444 s, tests 90/90), fusion dans `main` **3ca380b**. Worktree
  `HeroesOfLiberty-J5c` libéré. Reste noté dans `EtapeDefini.md` : barre du bas trop large sous 1500 px. **Chantier clos.**
- 2026-10-21 (mycrusader-82, choix de l'utilisateur) : la barre du bas trop large sous 1500 px **sera corrigée avec la
  mini-carte**, quand on attaquera celle-ci (pas avant).
- 2026-10-29 : la barre du bas trop large corrigée par l'étape T d'`EtapeDefiniInProgress.md` (barre dans un `UScaleBox` `ScaleToFitX` sous 1512 px Slate, 1500 × 1080 échelle 0,94, 1280 × 1024 échelle 0,82 ; sous-cas `build:barwidth`), essai validé, `main` 29e5522. Reste la mini-carte elle-même.

- 2026-10-30 (séance de questions, l'utilisateur) : **mini-carte du prototype = option A, façon AoE4 complète** : unités et bâtiments aux couleurs des camps, sources explorées, cadre de la caméra, clic gauche qui déplace la caméra, clic droit qui donne un ordre de déplacement, alertes qui clignotent ; elle suit le brouillard (texture de cases de F7, plan dédié, F-Q3 = A). À faire après F7 (`FogInProgress.md`).

- 2026-10-30 (Galadriel, branche `galadriel-tab`, test d'une heure de la journée autonome) : **ordres et onglet de Tab**. Constat : après Tab sur « Hommes d'armes » (3 Paysans + 2 Hommes d'armes + 3 Archers), le clic droit (`OrderAt` : déplacer, attaquer, récolter, déposer, bâtir sur un chantier), la ligne d'ordre (`OrderLineAt`, glisser) et la pose d'un bâtiment (`RequestPlaceBuilding`) lisaient `GetOwnSelectedUnits()` (toute la sélection) ; seuls stop, garde et attaque-déplacement lisaient `GetCommandUnits()` (l'onglet actif). Pas de question de design : le voulu est dit par l'utilisateur. **Test rouge d'abord** : cas « move tab » de la partie `move` de `quick_check` (8 unités boîtées, `hol.Select.Subgroup` deux fois, clic droit 10 m au nord ; contrôles : le journal `Order: N units to` et les positions avant / après) : rouge « 8 ordonnées, 8 bougent » avant correction. **Correction** : `OrderAt`, `OrderLineAt`, `GetOrderLine`, `RequestPlaceBuilding` et l'ordre d'attaque de la console lisent `GetCommandUnits()` (« Tous » = toute la sélection, comportement inchangé). Vert : `-Parts move` PASS, `-Parts move,formation` PASS, tests C++ 482/482. Commit dans Galadriel sur `galadriel-tab` (non fusionné). **Anomalie hors sujet** : `quick_check -Parts build` (en entier) échoue à l'étape 1390 (`waitlog plants his banner`, cas « hero aura ») sur `main` e6fbaae aussi, sans mon correctif (refait sans lui) ; `-Parts build:hero` seul passe.

- 2026-10-30 (Galadriel, suite) : **anomalie `quick_check -Parts build` rouge, cause trouvée, corrigée dans le script, pas dans le jeu**. Journal du jeu : dans le cas « hero aura » le héros (combat actif) dérive d'environ 1 m en se battant ; la bannière demandée à 17 m (`+1700`) était donc plantée (journal `plants his banner` écrit avant la dernière commande), celle à 16 m (`+1600`) refusée (attente), et `waitlog plants his banner` (« écrit depuis la dernière commande ») manquait la ligne : délai à l'étape 1390 ; seul `build:hero` passait parce que le héros n'y dérivait pas. 16 m est aussi le bord exact de la portée : trop fragile. Correction : refus à 30 m, plantée à 15 m (`+3000` / `+1500`), contrôle `level 4: the banner refused 30 m away, planted 15 m away` à jour. Second défaut révélé derrière : le cas « hero grid » finissait par `hol.Camera.TestCursor off` ; le curseur réel prenait alors la main (fenêtre sans premier plan / bord d'écran) et la caméra défilait pendant `dropoff camp` (le fantôme du camp balayait x 20400 à 29600, `waitlog Placement ghost: LumberCamp at (-18400, -6000) valid` en délai à l'étape 1485, puis dropoff, rallysteps, corner, barwidth rouges) : remplacé par `hol.Camera.TestCursor 0.5 0.5` (le curseur reste au milieu comme dans les autres cas de `build`). Vert : `quick_check -Parts build` **PASS** (138 s, tout `build` d'un coup, camera 2p comprise). Aucune décision de design à trancher.
- 2026-10-30 (Galadriel, branche `galadriel-tab`) : **scénario d'essai `tab`** (`Tools/Scenarios/tab.txt` / `.json`, d4fadd0), oublié à l'étape précédente : 3 Paysans + 3 Hommes d'armes + 3 Archers sélectionnés au cadre, arbre, filons d'or et de pierre, 3 Lanciers et une maison ennemis, fondation de maison à bâtir ; 3 notes. Lancé hors écran (`-RenderOffScreen`) : « scenario tab ready, 3 notes », « Selection: 9 units (own): Archer x3, ManAtArms x3, Peasant x3 », aucune erreur. Lancement : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 tab -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`. Essai de l'utilisateur à faire.
- 2026-10-30 (Elrond, `tiers`, 1093e52) : **médaillons d'âge coupés en haut** (essai du scénario `tiers`) : la boîte d'échelle de la barre du bas (étape T) rognait les médaillons I à IV qui dépassent du bord haut de la colonne ; espaceur `MedallionRoom` (26) en tête de la colonne, ligne `HUD layout` enrichie (`medallion top / clip / cut`), contrôle dans `build:barwidth` (7 tailles) et `build:tiers`. Détail dans `VictoryInProgress.md`. Une boîte d'échelle rogne son contenu (`ScaleToFitX/Y/Fill`) et ignore le remplissage de son emplacement : tout ce qui dépasse de la colonne a besoin d'un espaceur.
- 2026-10-30 (réponse de l'utilisateur à l'essai du scénario `tab`, enregistrée par Auto_Journey) : **« Tout est bon »** : le correctif des ordres après Tab (clic droit, ligne d'ordre, pose de bâtiment qui ne lisent que l'onglet actif ; « Tous » = toute la sélection) est validé. Le HUD « crop » vu pendant l'essai est le défaut des médaillons d'âge, déjà corrigé par Elrond (1093e52, branche `tiers`). Reste : fusion de `galadriel-tab` dans `main`, mini-carte, test de contrôleur du double clic caché.

- 2026-10-30 (Galadriel, Auto_Journey test 2, branche galadriel-tab) : **barre du bas sous 1500 px : rien a corriger, deja faite** (etape T du 2026-10-29, UScaleBox ScaleToFitX, essai valide, main 29e5522 ; la ligne de la file etait perimee). Verifiee: quick_check -Parts build:barwidth PASS ; garde-fou elargi aux fenetres **800x600 et 1024x768** (echelle 0,89, ecart 6 >= 0, medaillons non coupes ; avec 1280x720...1920x1080 : 8 tailles + retour a 1600x900), sans correction a faire. **Test de controleur du double clic meme type cache** (regression F7.3a) : partie ogclick de quick_check (~25 s, aussi dans -Full), en solo sur Plain : 4 Hommes d'armes a moi, 3 Lanciers ennemis a 45 m (dans la vue du centre), 4 a 80 m (hors de toute vue), tous a l'ecran ; brouillard Normal (hol.Fog.Mode 2) : hol.Select.ClickUnit 0 double slot=1 prend **3** (rapport 3 units (other): Spearman x3, HUD fog : 3 montres, >= 4 caches) ; temoin sans brouillard (mode 0) : **7**. **Rouge d'abord sur la vraie chaine** : le filtre IsUnitShown de GatherPickCandidates desactive temporairement (alse &&), recompile : KO 	ook 7 ; filtre remis, recompile : PASS (source revenue identique, seul quick_check.ps1 commite). Commit 76a76e8 sur galadriel-tab. Mini-carte : toujours bloquee par la texture de cases de F8/F9 (Faramir), non prise.
- 2026-10-31 (Galadriel, Auto_Journey, branche `galadriel-tab`) : **mini-carte du prototype faite** (décision de l'utilisateur du 2026-10-30 : façon AoE4 complète, option A). Règles pures en script `Script/UI/HOLMinimapRules.as` (conversion sol <-> image, l'écran en haut = +X du monde et à droite = +Y comme la caméra, carte rectangulaire gardée par axe ; `ClickAction` : gauche = caméra, droite = déplacement seulement avec des unités à ordonner ; alertes : une chose à soi dont les PV baissent entre deux lectures, fusionnée avec une alerte à moins de 25 m, 6 s, clignote par demi-périodes de 0,5 s) : **7 tests rouge d'abord** (`Script/Tests/HOLMinimapRules_Test.as`, stub : 597/604, 7 KO ; réel : 604/604), commit e4bdc3a. Widget : `UHOLMinimapView` (`Source/HeroesOfLibertyUI/HUD/HOLMinimapView.*`, Slate, rendu seulement) lit le jeu pour le script (`GatherEntities` : unités vues par `IsUnitShown`, bâtiments des autres seulement sur sol exploré ; `GetMapArea`, `GetCameraGround`, `HasOrderableSelection`), agit (`FocusCamera`, `OrderMove` : journal « Minimap order: N units to (x, y) », `RequestMoveUnits` sur les unités de l'onglet actif) et peint (terrain : le brouillard du camp, noir / gris / clair, et les sources explorées en points jaune or / gris pierre / vert bois / rose baies ; points des camps aux couleurs des joueurs, bâtiments 6 px, unités 3 px ; cadre blanc de la caméra ; alertes = carré rouge clignotant) ; le script `UHOLGameScreenScript` (en bas à droite, la place `MinimapPlace` gardée) fait le lien (rafraîchi 4 fois par seconde, le cadre de la caméra à chaque image : le dernier quadrilatère mesuré glisse du déplacement du foyer). Appui gauche maintenu : la caméra suit. Brouillard : le terrain lit `UHOLFogRenderSubsystem::GetCellVision` (main contient `fog` : fusionné), un seul endroit (`ReadVision`). Scénario `Tools/Scenarios/minimap.txt` (Plain, brouillard Normal, 4 Hommes d'armes, 3 + 5 Lanciers ennemis). **Vérifié hors écran sur la vraie chaîne** (captures, journal `HUD minimap: ...`) : le noir / gris, les sources, le cadre, un clic gauche qui déplace la caméra, un clic droit qui envoie les unités, une alerte rouge qui s'allume au combat puis disparaît ; un plantage trouvé et corrigé (`Points.Add(Points[0])` sur un tableau en croissance, `Assertion failed Addr < GetData()` dans `OnPaint`). Nouveau cas `quick_check -Parts build:minimap` : un Paysan choisi, clic droit au milieu de l'image -> ordre au centre de la zone, clic gauche à (30, 30) px -> caméra au sol calculé ; **rouge d'abord** (clic droit coupé dans `ClickAction` : l'étape `waitlog Minimap order` expire) puis vert. `-Parts tests,smoke,build` PASS (604/604, 175 s) avant la fusion de main. Commit 2bc106d. **Choix provisoires** (rien d'enregistré au GDD) : couleurs et tailles ci-dessus, alerte = dégâts reçus (pas encore « attaque vue » au sens de D386), carte non carrée étirée sur un carré. **NON VÉRIFIÉ** : un vrai clic de souris (Slate : seul le chemin à partir du point de l'image est testé, par `hol.HUD.Click MinimapClick<L|R><px>x<py>`), le rendu aux tailles de fenêtre autres que 1280 x 720, le coût avec 1 400 unités, un client réseau, le mode Carte explorée, un spectateur, les alertes de bâtiments.
- 2026-11-01 (Auto_Journey, agent de docs A) : **retours d'essais `minimap` et `market`**. `minimap` : « très bien pour commencer, une meilleure résolution plus tard, moins pixellisé » : validée telle quelle (couleurs, cadre, clic gauche, clic droit, alertes) ; à faire plus tard, pas maintenant : texture de la mini-carte de meilleure résolution (moins de pixels visibles). `market` (noté ici car c'est de l'interface) : les cases « 100 or = X » deviendront « 100 [icône or] -> X » quand les icônes de ressources existeront (voir `MarketInProgress.md`). Aucune ligne `[ ]` : les deux attendent les icônes et le passage à une meilleure texture.
- 2026-11-01 18:05 (Galadriel, seance interactive Auto_Journey, branche galadriel-tab) : **alertes de la mini-carte (D386) : durée et forme en réglages, bâtiments vérifiés.** Constat : l'alerte (une chose à soi dont les PV baissent, fusion à moins de 25 m, clignote) existait déjà (e4bdc3a), durée 6 s et demi-période 0,5 s codées en dur, bâtiments non vérifiés. **Test rouge d'abord** : Test_MinimapAlertSettings (HOLMinimapRules_Test.as, Age/IsLit prennent la durée et la demi-période, SaneLifetime 0,5-60 s et SaneBlink 0,1-10 s, jamais de division par zéro) : stub puis vert, tests 780/780 ; cas quick_check -Parts build:minalert (Maison à moi frappée par hol.Cheat.DamageNear : alerte allumée ; durée 0,5 s : disparue 1,5 s plus tard) : rouge par mutation (durée remise à 6 s en dur : KO « lit 1, alerts 1 »), puis vert. **Réglages de console** (C++ HOLMinimapView.cpp, TAutoConsoleVariable) : hol.Minimap.AlertSeconds (6), hol.Minimap.AlertBlink (0,5), hol.Minimap.AlertShape (0 carré + cœur, 1 plein, 2 deux carrés, 3 croix), hol.Minimap.AlertSize (16 px) ; la durée et le clignotement passent par GetAlertSeconds / GetAlertBlink au script. **Rien n'est décidé** : question MM-Q1 posée (Auto_Journey_Retours.md), recommandation A. Scénarios minimap_alerts (+ .look, 3 captures vues) et minimap_alerts_compare (4 options A-D, 4 captures vues), pré-vol PASS. **NON VÉRIFIÉ** : alertes d'unités alliées (pas encore d'alliance en code, D369), unité ou bâtiment qui disparaît d'un coup sans passer par une baisse de PV lue (mort d'un coup entre deux lectures : pas d'alerte, à voir), réseau, spectateur, clignotement vu en mouvement (captures figées allumées), taille à d'autres résolutions.
- 2026-11-01 18:12 (Galadriel, reponse de l'utilisateur a MM-Q1) : **la forme de l'alerte de la mini-carte est un CERCLE** (« Cercle » : aucune des options A-D de la comparaison) ; taille, clignotement et duree gardes (16 px, 0,5 s, 6 s, derriere les reglages de console). hol.Minimap.AlertShape : 0 = cercle (anneau + coeur rond, DEFAUT), 1 = carre + coeur (l'ancien), 2 = carre plein, 3 = deux carres, 4 = croix. Scenario minimap_alerts_compare supprime. **Un numero de decision GDD est a ecrire par la session principale** (precise D386 / D202). A refaire apres la compilation : pre-vol minimap_alerts, PNG ouverts (cercle lisible, rouge, contraste avec les points de camp).
- 2026-11-01 18:25 (Galadriel, precision de l'utilisateur sur MM-Q1 : « cercle qui se retrecit ») : **l'alerte est un cercle rouge qui se resserre** (ping / radar) : hol.Minimap.AlertShape 0 (defaut) = anneau de AlertSize (24 px) qui se resserre vers 8 px en hol.Minimap.AlertShrinkSeconds (1 s), en boucle pendant AlertSeconds (6 s) ; AlertBlink par defaut 10 s (pas de clignotement avec le cercle ; 0,5 le rend aux formes carrees 1-4). Rayon calcule a la peinture (horloge murale), tests 780/780 et uild:minalert PASS. Preflight minimap_alerts PASS, PNG ouverts : le cercle rouge avec son coeur est lisible et contraste avec les points de camp ; **le retrecissement n'est PAS prouve par les 3 captures espacees de 0,3 s (rayon quasi identique, game time en pas fixe hors ecran) : LOGIQUE SEULEMENT pour l'animation**, a voir en vrai. Numero de decision GDD a ecrire par la session principale.
- 2026-11-01 19:45 (Galadriel, `galadriel-tab` = main 16c342b + non commité) : **mini-carte moins pixellisée** (retour d'essai « meilleure résolution »). Constat sur l'ancienne capture : la zone vue avait des bords en escalier (cases du brouillard de 4 m = 1 à 2 px sur une carte de 400 m), terrain 200 x 200 étiré, points carrés. **Test rouge d'abord** (`HeroesOfLiberty.Vision.Picture`, stubs puis vert, 793/793) : `HOLFogPicture::ReadSmooth` (bilinéaire entre les centres de cases, 0 à 1) et `HOLFogPicture::Blur` (noyau 1-2-1 x 1-2-1, un champ uniforme reste lui-même). **Widget** (`HOLMinimapView.cpp`) : terrain **480 x 480** (au lieu de 200), cases floutées une fois puis lues en bilinéaire à chaque texel (dégradé noir -> gris -> clair, bord doux de quelques px), sources = disques ronds de 3 texels à bord adouci, marques en pavé arrondi anti-crénelé (`FSlateRoundedBoxBrush` : unités <= 4 px = disques, bâtiments = carrés à coins ronds de 1,5). `quick_check` : `terrain 480 x 480` ; `-Parts tests,build:minimap,build:minalert` PASS. Scénario `minimap` : capture `minimap_view` (+ `.look`) ouverte et vue : bord de la zone vue doux, plus d'escalier. **NON VÉRIFIÉ** : rendu à 1920 x 1080 (mini-carte de 240 px) et en DPI 2, forme des disques d'unités à 3 px (trop petits pour juger), coût de `RefreshTerrain` avec 230 400 texels (2 fois par seconde, non mesuré), la première image de la partie montre la carte entièrement claire avant la première image du brouillard (existait déjà). Rien committé (règle : après validation de l'utilisateur).
- 2026-11-02 12:0x (Galadriel, séance interactive, branche `galadriel-tab`) : **alerte de la mini-carte prouvée en mouvement + outil de capture en séquence + réglages de l'utilisateur.** (1) **Outil** : verbe `captureseq <Nom> <N> <pas> [commande]` du lanceur de scénarios (`HOLTestSequence.cpp`, écrit en `seqclock` + `waitclock t` + commande + `capture Nom_i`) : N images espacées de `pas` secondes sur l'**horloge du jeu** (pas fixe : celle de la simulation), le journal dit `clock T s` ; le préflight (`preflight.ps1`) lit `captureseq` (noms `Nom_0`...). **Test rouge d'abord** : `HeroesOfLiberty.Dev.CaptureSeq.Expand` et `HeroesOfLiberty.UI.Minimap.AlertRing` (stubs : 793/795, 2 KO) puis vert 795/795 ; cas `build:minalert` de `quick_check` (4 images à 1 s, période 10 s : le diamètre baisse de 1 à 2,2 px par image, l'âge monte d'1 s, 4 PNG écrits), **rouge par mutation** (l'âge lu à l'horloge murale : KO « diamètres 20.8 20.8 17.6 17.6 ») puis vert. **Cause du « rétrécissement non prouvé »** : le rayon était lu à l'horloge MURALE (`FPlatformTime`) ; il suit maintenant l'âge de l'alerte sur l'horloge du JEU (`UHOLMinimapView::AlertDiameter(Âge, Période, Taille)`, `GetAlertAge` : âge lu par le script + temps de jeu écoulé ; `SetLayers` reçoit les âges). Commits b0c96ad, 37c869e. Essai `minimap_alerts` : 4 images à 0, 1, 2, 3 s (période 5 s pour la preuve) : diamètres 22,1 / 19,0 / 15,7 / 12,5 px, PNG ouverts et vus, plus aucun cercle 7 s après le coup. (2) **Réponses de l'utilisateur** : **`E-minimap-alerts` OK** (« E3 OK », 2026-11-02 ~11:15) ; **Q-minimap-alerts-vitesse : « Autre : 24 px, 0,75 s »** = taille 24 px inchangée, passe de **0,75 s en boucle**, vie 6 s inchangée : `hol.Minimap.AlertShrinkSeconds` vaut 0,75 par défaut (test `AlertRing` rouge à 1 s puis vert ; défaut de taille et de vie contrôlés aussi). Numéro de décision GDD à écrire par la session principale (précise D624). Scénario de comparaison `minimap_alerts_compare` supprimé (question close). **À noter** : sous ~13 px de diamètre l'anneau (trait de 3 px) + son coeur (rond de 3 px de trait) se referment en disque plein ; à 0,75 s la passe le disque plein dure environ la moitié de la boucle ; lisible comme un point rouge, plus comme un anneau : à juger à l'oeil (non demandé). (3) **Traductions en/ja du tribut** (D372, annonce au seul destinataire) : clés `HOLGameScreen,TributeFood/Wood/Stone/Gold` et `MessageTribute` ajoutées à `Tools/Localization/fill_po.py` (en : « {0} gave you {1} {2} (tax {3}) », ja provisoire), GatherText / fill_po / GatherText ; `Localization.Translations` rouge (794/795) avant, vert après. Le bloc du héros à 600 px sur le panneau Triches : constat du journal (panneau Triches en haut à gauche, bloc du héros en bas à gauche, ils se recouvrent quand la fenêtre est basse), **choix de présentation non tranché : question `Q-heros-triches-600` posée**.
- 2026-11-02 12:20 (Galadriel, séance interactive, branche `galadriel-tab`, ab2384a) : **Triches / bloc du héros à 600 px (Q-heros-triches-600 = A, réponse de l'utilisateur).** **Remesure d'abord** : la sonde `HUD overlap` n'était pas dans ma branche (elle venait de ai-ia1) : cherry-pick de 78cac00 et 67d2e69 (e515fe7, 87ad386). Partie de 1280 x 600 (scénario, `r.SetRes 1280x600w`), règle coupée (`hol.HUD.CheatsLowFold 0`), Triches OUVERT et bloc du héros affiché : **`HUD overlap: none`** : l'échelle du HUD suit la hauteur (dpi 0,555 à 600 px, la mise en page vaut celle de 1080 px), le chevauchement du constat du journal n'existe pas à cette taille ; le test rouge de la demande (overlap non none) n'a donc pas de réalité ; rouge obtenu à la place sur la règle : panneau resté ouvert à 600 px (`HUD cheats: cheats open`). **Défaut de la sonde trouvé** : avec un héros affiché elle lisait `HeroPanel x HeroPortraitFill` (cadres imbriqués) dans tout scénario avec héros (pré-vol FAIL) : seuls les cadres extérieurs sont comparés (`HOLScriptScreen.cpp`). **A implémentée** : règle pure `HOLHUDLayout::IsLowWindow` (< 700 px, 0 = inconnu), `CheatsStartOpen`, `ShouldFoldCheats` (front : le bloc du héros s'ouvre ou la fenêtre devient basse, panneau ouvert ; un clic qui rouvre reste ouvert) ; `UHOLCheatPanel::SetOpen(bool, bKeep)` (un repli de la règle ne change pas le choix gardé), `SetHeroBlockShown` appelé par le script (`ShowHero`), `hol.HUD.CheatsLowFold` (1 par défaut, 0 = avant), `hol.HUD.Cheats <0|1>`. Test `HeroesOfLiberty.UI.Layout.CheatsLowWindow`, `quick_check -Parts tests` 797/797. Scénario `heros_triches_600` : pré-vol PASS (`HUD overlap: none` x 2, Sanity, `waitlog HUD cheats: cheats folded`), PNG ouvert et vu (Triches + replié, bloc du héros entier) ; essai `Retours/E-heros-triches-600.md`. **NON VÉRIFIÉ** : démarrage replié avec le choix gardé ouvert au lancement réel (règle testée en pur) ; fenêtre 600 px vraie (hors écran seulement) ; les tests `-Parts build` non relancés. Ordre des commits : le test pur est dans le même commit que le code (pas de commit rouge séparé).
- 2026-11-02 14:30 (session principale) : **`E-heros-triches-600` : « Oui » (OK) de l'utilisateur** : Triches replié et bloc du héros à 600 px acceptés, règle `hol.HUD.CheatsLowFold 1` gardée par défaut. Essai rangé, item supprimé de Retours. Dans `main` (6249c29).
- 2026-11-02 12:27 (Galadriel, `galadriel-tab`) : **rendu en / ja de l'annonce du tribut (D372) sur le fil de messages, vu à l'écran.** Console de test `hol.Tribute.Notice <GiverSlot> <res> <Arrives> <Tax>` (`HOLPlayerController.cpp` : appelle `NoteTributeReceived` pour le joueur local, rien n'est donné) ; scénario `tribute_i18n` (fr, en, ja puis retour fr, 3 captures, pré-vol PASS) ; PNG ouverts et vus : fr « IA Normal 2 vous a offert 70 or (taxe 30) », en « AI Normal 2 gave you 70 gold (tax 30) », ja « AI ふつう 2から金70の贈り物（税 30） » lisible, sans carré vide, ligne dans le cadre. Constat : le fil garde les lignes déjà reçues et les rend dans la langue courante (2 lignes en anglais, 3 en japonais après les 3 annonces : voulu). Le japonais reste provisoire (texte écrit par fill_po.py). **NON VÉRIFIÉ** : une annonce reçue d'un vrai joueur au réseau (commande locale seulement), `quick_check` non relancé après la commande.
- 2026-11-06 11:2x (Galadriel, galadriel-tab, agent C++ n2) : **3 correctifs du HUD dans HOLGameScreen.cpp, tests d'abord (scenarios : 79973c5, f9de502, rouges sur le binaire inchange : forge 'timeout waitlog +10 %' et legions_roster 'timeout waitlog F 45 | W 15') puis verts.** (1) BUG forge-tier1-grid : la case Research memorise le palier (ShownEngineTier) et le tick redessine la grille quand il change ; forge.txt sans re-selection : le tooltip du W passe a 'Degats de melee +10 %'. (2) BUG legions_roster : FindUnitTypeFor(type, joueur) = FindTypeRowFor, utilise par la case Produce (cout, nom) et le nom de la selection ; tooltip Lancier Legions F 45 | W 15. (3) REGLAGE forge : ligne de statistiques sous les points de vie (RefreshUnitStats, hol.HUD.UnitStats 0/1/2, ligne 'HUD stats' de hol.HUD.Report) : 'attack 12, armor 4/4, range melee, speed 7' puis 'attack 13.2 (+1.2)' apres Lames affutees I ; PNG forge_stats_before/after vus. Question Q-unit-stats (A ligne par defaut, B colonne) + scenario forge_stats_compare. NON VERIFIE : Caserne (Hallebardes trempees = Bientot, inobservable), tooltip survol reste affiche si la souris est dessus (Slate), armure sans bonus (Maille/Barde non codees), quick_check non relance. Preflight forge / legions_roster / forge_stats_compare PASS.

- 2026-11-06 11:53 (Galadriel, galadriel-tab, agent C++ n2) : **compiler et tester apres le merge de main + reponse a Q-unit-stats.** Build 1 (HEAD 1400cae) PASS en 3 s (deja a jour). Preflight forge / legions_roster / forge_stats_compare PASS, PNG ouverts. **Defaut vu** : forge_stats_compare_A et _B ne montraient PAS le panneau de l'unite (Selection: none : Select.Type 23 ms apres SpawnHere, waitunits 3 trivial). Rouge d'abord : 96db5dc (waitlog sur la ligne de stats : timeout) puis dc60fac (waitunits 7 + frames 20) : les deux PNG montrent la ligne (A) et la colonne (B). **Reponse de l'utilisateur a Q-unit-stats (ligne A + hover)** : rouge 2211b13 (forge.txt : melee armor 4, ranged armor 4, hero armor 2, speed 7 + ligne HUD stats tip, timeout sans le code) puis code : RefreshUnitStats (HOLGameScreen.cpp) : ligne Attaque / Armure melee / Armure distance / Armure heros / Vitesse ; infobulle sur SelectionStats (devenu Visible) : Points de vie, Attaque de base, Technologies +10 %, Portee, Armure siege ; DescribeStatsTip + ligne 'HUD stats tip' de hol.HUD.Report (lue depuis le widget). Build PASS (128 s), preflight forge PASS (stats before 'attack 12, melee armor 4, ranged armor 4, hero armor 2, speed 7', after 'attack 13.2 (+1.2)'...), compare PASS, PNG vus : la ligne se replie sur 3 lignes dans la colonne etroite. quick_check -Parts tests,build:hero : sequence : PASS / camera 2p : FAIL /   [KO] right edge 1 s: +Y (got 0 m) / build hero : PASS / build hero return : PASS / build hero texts : PASS / build hero xp : PASS / build hero aura : PASS / build hero hold : PASS / build hero dawn : PASS / build hero grid : PASS / tests 1039/1039 : PASS / quick_check : FAIL in 127 s (game launch done at 46 s). **NON VERIFIE** : le survol reel (les infobulles Slate ne sortent pas en capture hors ecran : la preuve est le texte lu sur le widget), la ligne etroite a 1280 px (3 lignes), les autres langues (LOCTEXT sans traduction en/ja : fill_po a faire), toast 'Palier superieur requis' encore visible sur forge_stats_after alors que le journal dit expire (artefact de capture, pas de la logique). camera 2p (build:hero) : KO 'right edge 1 s: +Y (got 0 m)' sur 2 essais, camera non touchee, a recontroler sur main.
- 2026-11-06 12:3x : E-forge_stats : l'utilisateur confirme que l'infobulle au survol (stats de l'unite selectionnee) fonctionne ; forge OK ; legions_roster : noms par faction a cabler (voir LegionsInProgress.md).

- 2026-11-09 16:2x (Auto_Journey, Galadriel-C++, REGLAGE unit_stats_attack) : **infobulle PAR stat du bloc d'unite** (HOLGameScreen.cpp/.h) : le bloc n'est plus un seul UCommonTextBlock mais un UWrapBox SelectionStats de 6 UCommonTextBlock SelectionStatChips, chacun avec son SetToolTipText (Attaque : base + technologies + portee ; Attaque batiments ; Armure melee ; Armure distance ; Armure heros + armure siege ; Vitesse). Textes d'infobulle ecrits par l'agent (a relire par l'utilisateur, pas de design tranche). Mode hol.HUD.UnitStats 2 (une ligne par stat) n'est plus distinct du mode 1 dans la mise en page (WrapBox). Rapport HUD stats tip = une infobulle par stat visible. Test rouge = scenario unit_stats_attack (0c05a38, waitlog des infobulles), a rejouer apres build. Survol reel non rejoue : LOGIQUE SEULEMENT.
