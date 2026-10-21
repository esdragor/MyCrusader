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
| G.3 Commit, `EtapeDefini.md`, GDD (D340 ; D30, D201, D313 précisées) | en cours |

### Journal

#### 2026-10-21

- mycrusader-01. Worktree `MyCrusader-J5c` pris (`IN_USE.txt`), branche **`peasant-grid`** depuis `main` ea84b7b.
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
