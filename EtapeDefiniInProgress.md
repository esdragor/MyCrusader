# Lignes d'EtapeDefini en cours

Journal des lignes de `EtapeDefini.md` prises hors des grands chantiers. Une section par ligne ; quand la ligne est
fusionnée dans `main`, elle disparaît d'`EtapeDefini.md` et sa section reste ici comme trace.

Choix de l'utilisateur le 2026-10-23 (« peux-tu faire quelque chose de EtapeDefini ? ») : **A** (interface) et **B**
(chantier qui pousse), en parallèle et en sous-agents. Le déplacement (MV.4) est pris par une autre session dans
`HeroesOfLiberty-Elrond` : ne pas y toucher.

## A — Double clic sur une unité, curseur de Q

- Worktree `HeroesOfLiberty-Haldir`, branche `select-type-q-cursor` (depuis `main` d239a1c).
- A.1 : double clic sur une unité = toutes les unités du joueur de ce type visibles à l'écran (façon AoE4).
- A.2 : Q (attaque) : pendant le choix du point, le curseur change d'icône et passe au rouge (retour de l'essai de
  J7c.3, `JourneyInProgress.md`).

## B — Unité posée dans les murs d'un chantier qui démarre

- Worktree `HeroesOfLiberty-J5c`, branche `site-push` (depuis `main` d239a1c).
- Retour de l'essai 1 de `UnitStatsInProgress.md` (2026-10-22) : une unité repoussée par un chantier qui démarre est
  posée dans les murs du bâtiment ; il faut la pousser au-delà de la marge.

## Suivi

- 2026-10-23 : deux sous-agents lancés en parallèle, avec un minuteur de 15 min : **A.1** (double clic) dans Haldir,
  **B** (chantier qui pousse) dans J5c. Pour chacun : test rouge d'abord, `quick_check` court ou `-Parts` ciblé, pas de
  `-Full`, pas de commit, pas de scénario (un sous-agent à part ensuite). A.2 (curseur de Q) après A.1.
- **A.1 faite** (sous-agent, Haldir, non commitée) : règles pures `IsDoubleClick` (0,5 s, 6 px = `BoxThreshold`) et
  `PickSameTypeOnScreen` (`HOLUnitPicking`), `SelectSameTypeAt` dans le contrôleur ; **Ctrl + clic** fait pareil (AoE4) ;
  Maj ajoute ; unité ennemie, bâtiment, source, vide : sélection simple comme avant ; « visible » = milieu du corps dans
  la fenêtre (comme le cadre), plafond `MaxOrderedUnits`. `hol.Select.Click` / `ClickUnit` acceptent `double`. Test
  `HeroesOfLiberty.Movement.PickSameType` rouge (102/103) puis vert (103/103) ; `quick_check` court PASS (63 s). À voir
  à l'essai : les unités cachées par la barre du bas comptent comme visibles ; 0,5 s fixe. A.2 lancée (sous-agent).
- **B faite** (sous-agent, J5c, non commitée) : cause dans `UHOLFootprintKeepOutProcessor`, une unité au centre dans le
  corps était posée à rayon + 1 cm du corps (61 cm, Archer 53) alors que le mur visible est à 75 cm (BodyInset 105 −
  VisualInset 30) : 14 cm dans le mur. Désormais **max(marge navmesh 25, mur visible 75) + rayon + 1** (136 cm, Archer
  128) ; le cas « frôle de l'extérieur » reste à rayon + 1. Règles pures `GetKeepOutInsideGap`, `KeepOutOfBody`
  (`HOLConstructionRules`). Test `HeroesOfLiberty.Construction.SiteKeepOut` rouge (24 erreurs) puis vert (103/103).
  `-Parts tests,build` **FAIL** : « build cut » (23 put back) et « build gap » (two : 15 put back, l'un après l'autre),
  marcheurs au centre dehors, cas non changé. Vérification sur main d239a1c sans le correctif lancée (sous-agent, J5c).
  Restes notés : passage de 210 cm entre deux maisons (sortie à 15 cm du mur voisin), unité coincée bâtiment/ressource,
  unité au repos qui reviendrait vers sa place dans l'emprise.
- **A.2 faite** (sous-agent, Haldir, non commitée) : curseur logiciel Slate dessiné en C++ (`HOLAttackCursor` : cercle,
  croix, point, rouge sur contour sombre, 36 unités × DPI), enregistré pour `EMouseCursor::Crosshairs` (personne
  d'autre ne s'en sert) ; règle pure `HOLCommandGrid::GetMouseCursor(bAttackMoveArmed, bPlacing)` lue à chaque
  `PlayerTick`. Le mode Q s'arrête s'il ne reste aucun soldat sélectionné (pas à chaque changement : une mort purge la
  sélection). Module `Slate` ajouté. Test `HeroesOfLiberty.CommandGrid.MouseCursor` rouge (103/104) puis vert
  (104/104) ; `quick_check` court PASS (81 s). Pas de capture (pas de souris hors écran) : l'essai jugera taille et tenue.
- Scénario d'essai **`select_type_q`** écrit (Haldir) : 6 Hommes d'armes, 6 Lanciers, 4 Archers, 3 Hommes d'armes hors
  de l'écran, 6 Lanciers ennemis à ~28 m. Répétition hors écran confiée à un sous-agent.
- **B, vérification sur main d239a1c sans le correctif** (sous-agent, J5c, ~13 min) : « build cut » (23 put back) et
  « build gap » (15 put back, « one after the other ») **échouent déjà sur main**, mêmes chiffres, avec ou sans le
  correctif (constant, pas un aléa) ; remises de 3 à 35 cm de marcheurs qui longent un mur : pas le cas de B. Correctif
  remis et recompilé. **Nouveau bug à noter dans EtapeDefini au commit** : `quick_check -Parts build` rouge sur main
  (build cut, build gap), origine à chercher (MV.4 détour sans contact 02d8ea4 ? D345 ?).
- Scénario d'essai **`site_push`** écrit (J5c) : 4 Paysans sélectionnés, 2 Hommes d'armes, 2 Lanciers, 2 Archers sur
  l'emplacement d'une maison posée ; l'utilisateur lance le chantier. Répétition hors écran confiée à un sous-agent.
- `select_type_q` répété hors écran (sous-agent) : prêt en ~15 s ; double clic Homme d'armes **6**, Lancier **6**,
  Archer **4** ; aucun combat seul en 20 s. Correction : les 3 Hommes d'armes « hors de l'écran » étaient au bord
  (40 m) → 60 m (`FocusNear 4000 -6000`). Remarques : `hol.Select.ClickUnit` a raté une fois sur 4 lancements (« no own
  unit N on the screen ») ; il clique même hors de l'écran. **Essai de l'utilisateur lancé** (Haldir).
- `site_push` répété hors écran (sous-agent) : prêt ~1,4 s après la carte. Correction : les Paysans sélectionnés avant
  la pose y étaient envoyés tout seuls ; désormais pose sans sélection, soldats nés sur la fondation, puis Paysans pris
  au cadre (`hol.Select.BoxGround`). Au premier coup : **les 6 soldats sortent**, centre à rayon + 86 cm du corps
  (maison : demi-corps 285, mur visible +85 = BodyInset 115 − VisualInset 30), corps 1 cm hors du mur visible ; Paysans
  non bloqués, maison finie. **Reste vu** : ensuite les bâtisseurs bousculent un Lancier, que la garde « frôle de
  l'extérieur » (rayon + 1) laisse ~59 cm dans le mur visible ; les bâtisseurs eux-mêmes se tiennent dans le mur visible.
- **Essai de A (`select_type_q`) validé** (« tout est bon »). Commit **9c8e5f4** (Haldir, `select-type-q-cursor`),
  `main` avancé en avance rapide (rien ne tournait dans la copie principale). EtapeDefini : deux lignes d'interface
  retirées ; ajoutés : le retour ci-dessous et « `quick_check -Parts build` rouge sur main ». **A close.**
- Retour de l'utilisateur pendant l'essai, **pour le chantier déplacement** (pas pour A) : des unités d'un groupe à qui
  l'on donne l'ordre de bouger d'un rien (se mettre en formation là où elles sont) peinent : elles ne poussent presque
  pas les autres. Noté dans EtapeDefini (retours d'essai) et dans `MovementInProgress.md`.
- Essai de B (`site_push`) lancé (J5c).
- **Essai de B (`site_push`) validé** (« tout est bon ») ; l'utilisateur a bien vu le Lancier bousculé ensuite par les
  bâtisseurs. Commit 3737743 (J5c, `site-push`), `main` fusionné dedans, compilation PASS (49 s), `quick_check -Parts
  tests` **105/105 PASS** (41 s), `main` avancé en avance rapide. EtapeDefini : ligne du chantier retirée, remplacée par
  « unité bousculée contre un bâtiment (par les bâtisseurs) : reste à moitié dans le mur visible ». **B close.**

## C — Unité bousculée contre un bâtiment, restée dans le mur visible

- Rappel : D324 (« une épaule », 2026-10-17) laisse le milieu d'une unité venir jusqu'à la ligne du mur visible (passage
  un par un entre deux maisons collées) ; le Lancier vu à l'essai de B avait son milieu pile sur cette ligne (86 cm du
  corps, mur visible à 85). La garde « frôle de l'extérieur » permet aujourd'hui jusqu'à rayon + 1 (61 cm).
- **Choix de l'utilisateur (B, la recommandation)** : une unité **arrêtée** (ni en marche, ni à bâtir, ni à décharger,
  ni à attaquer un bâtiment) est poussée **entièrement hors du mur visible** ; en marche et au travail, l'épaule de
  D324 reste. Options écartées : laisser tel quel (D324) ; faire respecter la ligne de D324 partout.
- Worktree `HeroesOfLiberty-J5c`, branche `idle-wall` (depuis `main` f5f4be3). C.1 (code + test rouge) confiée à un
  sous-agent.
- **C.1 faite** (sous-agent, J5c, non commitée) : règles pures `StandsClearOfVisibleWall(Activity, Phase, bFighting)`
  (arrêtée = `Idle`, `Phase == None`, ni cible ni attaque ; tout le reste garde l'épaule) et `KeepOutOfVisibleWall`
  (après `KeepOutOfBody` : une unité arrêtée est éloignée jusqu'à mur visible + rayon + 1, **10 cm par pas** au plus,
  `KeepOutWallStep`) ; la **place tenue** (`Order.Slot` si `PlaceHold.bHeld`) sort avec elle : pas d'aller-retour.
  Test `HeroesOfLiberty.Construction.IdleWall` rouge (105/106) puis vert (106/106). Douteux : chaque pas doux compte
  dans « put back » (~9 pas) ; entre deux maisons collées, les deux poussées s'annulent (immobile, compté poussé).
  `-Parts build,gather` lancé par la session principale.
- `-Parts build,gather` sur C.1 (session principale, 322 s) : tout PASS sauf les deux déjà rouges sur main ; build gap
  inchangé (15 put back) ; **build cut 23 → 33 put back**, les 10 de plus = une seule unité arrêtée (#41) sortie en
  douceur, comptée à chaque pas ; sa place affichée restée à l'ancien endroit. **C.2** (sous-agent) : compter la sortie
  douce à part, vérifier la place de #41 (tenue ou non, pas d'aller-retour).
- **C.2 faite** (sous-agent, J5c, non commitée) : sortie douce comptée à part (`NumEasedOut`, ligne `eased out of the
  visible wall … held|not held`, rapport `hol.Nav.Footprints`, libellés de quick_check ; aucun seuil touché). #41 :
  place **non tenue**, rien ne le ramène (WatchPlace ne marche vers le Slot que si `bHeld`), sort de ~41 cm en 5 pas puis
  plus rien. Test IdleWall étendu (4 cas), rouge puis vert (106/106). `-Parts build` (299 s) : build cut **23 put back +
  10 eased out** (échec = 2 nouveaux chemins d'un marcheur, comme main), build gap 15 put back et « one after the
  other » (comme main ce matin). Reste : le rapport du stress test n'affiche pas le nouveau compteur.
- Scénario d'essai **`idle_wall`** écrit (J5c) : deux maisons collées bâties d'un coup, une troisième à bâtir avec 6
  soldats dessus, Paysans au cadre. Répétition hors écran confiée à un sous-agent.
- `idle_wall` répété hors écran (sous-agent) : prêt en 9,8 s après corrections (pas de `hol.Select.None` → Échap +
  `Selection report: selected none` ; caméra avant chaque `OrderNear` ; bâtisseurs renvoyés devant le centre avant le
  cadre ; 3e maison en -1400 800 pour ne pas se coller aux deux autres). Soldats arrêtés finis **hors du mur visible**
  (431 cm du centre de la maison, Archers 423 = 285 + 85 + rayon + 1) ; Paysan à travers le passage des maisons
  collées en ~2 s, sans sortie douce en marche. **Défaut trouvé** : une unité arrêtée dans ce passage ne sort jamais
  (sortir d'un mur la remet dans l'autre), « eased out » écrit à chaque pas (17 775 lignes en 4 min).
- **Choix de la session principale** (utilisateur absent, à confirmer à l'essai) : dans ce cas l'unité reste où elle
  est, avec l'épaule de D324, sans compte ni journal. **C.3** confiée à un sous-agent.
- **C.3 faite** (sous-agent, J5c, non commitée) : `HOLConstruction::EaseOutOfVisibleWalls(Bodies, Radius, VisualGap,
  MaxStep, Position)` regarde tous les murs visibles voisins d'un coup : dans deux murs, ou centre dans un corps → reste ;
  dans un seul → pas de 10 cm seulement si le pas et la sortie complète sont hors de tous les autres murs ; sinon reste
  (épaule de D324), rien compté ni écrit. Le processeur ne fait plus que le corps dans ses passes, puis la sortie douce
  pour une unité (et une place tenue) arrêtée, bâtiments de sa case. Test IdleWall étendu (maisons collées, 3 unités
  dans le passage immobiles sur 100 pas ; une près d'un seul mur sort) rouge puis vert (106/106). `-Parts build`
  (287 s) comme main (build cut 23 + 10 eased out, build gap 15). Reste noté : un mur d'une case voisine, touché
  seulement par le point de sortie, pourrait être manqué (rare). Vérification du passage en jeu lancée (sous-agent).
- Vérification du passage (sous-agent, hors écran, `c3run.log`) : 4 Paysans arrêtés entre les maisons collées pendant
  60 s → **0 « eased out », 0 « put back »** (avant C.3 : 17 775 lignes en 4 min), positions identiques à 0, 30 et
  60 s, aucun corps dans une emprise (plus proche : 39 cm du corps). **C prête pour l'essai** (`idle_wall`, J5c), à
  lancer au retour de l'utilisateur.
- **Essai de C (`idle_wall`) validé** (« tout est bon », choix du passage entre deux maisons collées compris). Commits
  e333332 (code, test, scénario) et 7b18e14 (D324 précisée au GDD ; ligne d'EtapeDefini retirée), `main` (e2b478a, doc
  seule) fusionné dans `idle-wall`, `main` avancé en avance rapide. Dans la copie principale, `quick_check.ps1` avait
  perdu son BOM (non commité, 11:58, contenu identique) : remis avant l'avance (PowerShell 5.1 en a besoin). **C close.**

## D — Sélection d'un groupe d'ennemis

- Choix de l'utilisateur le 2026-10-23 (deuxième « que peux-tu faire de EtapeDefini ? ») : **A** seule (cette ligne),
  parmi A (sélection ennemie), B (`quick_check -Parts build` rouge sur main), C (trancher J7c.4).
- Ligne : sélectionner un groupe d'ennemis ouvre le panneau d'info (pastille avec leur nombre) et montre leurs barres de
  vie, même pleines ; sélection mêlée : on ne garde que le camp prioritaire, Moi > Allié > Ennemi > Neutre.
- Worktree `HeroesOfLiberty-Haldir`, branche `enemy-selection` (depuis `main` 239a9a9). D.1 (code + test rouge) confiée à
  un sous-agent.
- 2026-10-23 : sous-agent D.1 lancé (Haldir) : sélection ennemie au cadre, panneau sans commandes, barres même pleines,
  règle pure du camp prioritaire (Moi > Allié > Ennemi > Neutre) ; test rouge d'abord, `quick_check` court, pas de
  commit, pas de scénario (un sous-agent à part ensuite).

## E — `quick_check -Parts build` rouge sur main (build cut, build gap)

- Choix de l'utilisateur le 2026-10-23 (troisième « que peux-tu faire de EtapeDefini ? », D étant prise par une autre
  session dans Haldir) : **B**, la recommandation.
- Worktree `HeroesOfLiberty-J5c`, branche `build-red` (depuis `main` 239a9a9).
- Dernier PASS connu : `-Parts build` (285 s) sur `unit-stats` pendant D345.2a (`UnitStatsInProgress.md`) ; ensuite la
  fusion 1473564 (main 464c9f0 + 02d8ea4, MV.4) n'a relancé que `tests,combat,bodies`. Échecs constants depuis
  (build cut 23 put back, build gap 15 put back et « one after the other »). Suspects : 4663530 (D345.2b, attaque des
  bâtiments), 213f470 (MV.4 mur d'ennemis), 6ab675c (MV.4 détour sans contact).
- E.1 (bissection) confiée à un sous-agent, boucle de 15 min, pas de code.
- La session du déplacement avance `main` (239a9a9 → 8021c71, MV.4 renforts et « porter secours ») : accord donné (rien ne
  tournait ici). Avant le commit de D, fusionner `main` dans `enemy-selection`.
- La session de la victoire avance aussi `main` (fe4214e → 4541266, Victoire V1) : accord donné. Elle touche
  `HeroesOfLibertyUI` et la localisation (fr / en / ja) : conflits possibles avec D à la fusion de `main`.
- E.1, 6ec5aae (1473564^1 : unit-stats avec 4663530 et 213f470, sans 6ab675c ; quick_check du commit) : build cut PASS,
  build gap FAIL (two : 15 put back, 0 nouveau chemin, « one after the other » KO ; remises de 1 à 12 cm, #36 le long
  du mur). Le reste de `build` PASS (327 s). Donc build gap rouge avant 6ab675c ; build cut rouge vient de 6ab675c.
- **D.1 faite** (sous-agent, Haldir, non commitée) : avant, le cadre ne prenait que ses unités (un cadre sur des ennemis
  vidait la sélection) ; le panneau savait déjà montrer un groupe « other » (pastilles par type, sans commandes, sans
  Tab) et les barres étaient déjà montrées pour toute unité sélectionnée. Ajouts : `EHOLSelectionSide` (Own, Ally,
  Enemy, Neutral), `GetSide` (Own ou Enemy seulement : ni alliés ni neutres en partie aujourd'hui), `GetTopSide`,
  `KeepTopSide`, `CombineBox` (cadre, avec Maj) dans `HOLUnitPicking` ; `SelectInBox` prend tous les camps (log
  `Select: box, N units (own|enemy|…)`) ; règle pure `ShowsHealthBar` dans `HOLSelectionMarkers`. Test
  `HeroesOfLiberty.Movement.PickTopSide` rouge (18/22) puis vert (22/22) ; `quick_check` court PASS (30 s, 107/107).
  Choix faits seul : ennemis de plusieurs joueurs gardés ensemble (panneau au nom du premier) ; Maj + cadre sur des
  ennemis déjà pris les retire ; Maj + cadre vide garde la sélection. **Reste** : clic, Maj + clic, double clic sur un
  ennemi non touchés (ennemi pris seul ; Maj + clic sur un ennemi remplace encore sa propre sélection). Question posée à
  l'utilisateur.
- **Choix de l'utilisateur (B, la recommandation)** : Maj + clic sur un ennemi suit la priorité des camps (ses unités
  restent) ; double clic / Ctrl + clic sur un ennemi prend tous les ennemis de ce type à l'écran (façon AoE4). Écartés :
  priorité seule ; ne rien changer au clic. Par défaut (session principale) : seulement ceux du même propriétaire que
  l'unité cliquée. **D.2** confiée au même sous-agent (Haldir).
- E.2, 4663530 (D345.2b, sans MV.4) : build cut PASS, build gap FAIL à l'identique (two : 15 put back, #36, « one after
  the other » KO), 286 s. Reste à départager 35a8be9 (MV.2, règle de priorité au croisement, venu par 2e086ed) et
  4663530 : prochain passage = 35a8be9 (ou 2e086ed). Copie J5c remise sur `build-red`.
- **D.2 faite** (même sous-agent, Haldir, non commitée) : `CombineClick` (= `CombineBox` sur une unité), `CombineBox`
  avec `bRemoveWhenAll` ; `PickSameTypeOnScreen` sur un ennemi : même type, **même propriétaire**, visibles à l'écran ;
  `GatherSelectionSides` dans le contrôleur (clic, double clic, cadre). Maj + clic sur un ennemi avec les siennes
  sélectionnées : les siennes restent ; Maj + clic sur une des siennes avec un ennemi sélectionné : la sienne seule.
  `hol.Select.ClickUnit` : options `enemy` et `slot=<S>`. Test `PickTopSide` rouge (9/32) puis vert (32/32) ;
  `quick_check` court PASS (31 s, 107/107). Choix faits seul : Maj + double clic sur des ennemis déjà tous pris les
  garde (comme pour les siens) ; unité inconnue → clic simple.
- `main` (2d15cff : MV.4 et Victoire V1) fusionné dans `enemy-selection`, sans conflit, travail de D gardé. Sous-agent
  lancé : compilation et `quick_check` court après fusion, scénario d'essai **`enemy_select`** (4 Hommes d'armes à soi,
  5 Lanciers + 3 Archers ennemis, 3 Lanciers d'un 3e joueur si possible), répétition hors écran des gestes.
- E.3, 2e086ed (fusion unit-stats cd091c8 dans main 32aba2a : D345.1/D345.2a + MV.2, sans 4663530 ni MV.4) : build cut
  PASS, build gap FAIL à l'identique (two : 15 put back, 0 nouveau chemin, #36 le long du mur, « one after the other »
  KO), 291 s. Le PASS de référence (285 s) portait sur D345.2a avant 95afb99, sans MV.2 (35a8be9 absent de cd091c8) :
  **build gap rouge = MV.2 (35a8be9)**, 4663530 innocent. Hypothèse : `ChooseDetourSide` prend toujours la droite quand
  le corps est presque droit devant (le second Paysan derrière le premier dans le passage) et le détour est **retenu**
  (`FHOLDetourFragment`, relâché d'un cran seulement après 4 pas libres) ; `AvoidBodies` ignore les bâtiments : le
  suiveur tient son virage à droite, longe le mur de la maison (remis par le keep-out) et marche à côté du premier au
  lieu de derrière.
- E.4, 1473564 (fusion qui amène 6ab675c dans unit-stats) : build cut FAIL (411 pas, 23 put back, 2 nouveaux chemins,
  #29 le long du mur), build gap FAIL comme avant, 288 s. Avec E.1 (6ec5aae : build cut PASS) : **build cut rouge =
  6ab675c (MV.4 sans contact), confirmé**. Hypothèse : les corps immobiles (les bâtisseurs au chantier) sont vus à
  1 s au lieu de 0,5 s et le virage s'ouvre jusqu'à 90° ; toujours sans regarder les bâtiments, le marcheur part de
  côté plus tôt et plus large, droit dans le mur visible. Copie J5c remise sur `build-red` et recompilée.
- **Choix de l'utilisateur (A, la recommandation)** : le correctif va à la session du chantier déplacement (MV.4,
  Elrond), qui travaille sur `AvoidBodies` ; diagnostic complet envoyé (piste : virages de `AvoidBodies` qui tiennent
  compte des emprises, ou virage mémorisé lâché s'il mène dans une emprise). Écartés : corriger dans J5c (conflit) ;
  noter seulement. Ligne d'EtapeDefini précisée (cause). J5c libérée (`build-red` = 239a9a9, aucun travail). **E close
  de ce côté.** Amélioration notée : `quick_check` ne sait pas lancer « build cut » ou « build gap » seules (~5 min par
  passage, ~1 min utile).
- Après fusion : compilation PASS (75 s), `quick_check` court PASS (28 s, 122/122). Scénario **`enemy_select`** écrit
  (Haldir, non commité) : Plain, 3 joueurs ; à soi 4 Hommes d'armes (4000, 0) ; slot 1 : 5 Lanciers (6800, -1300) et
  3 Archers (7300, -1300) ; slot 2 : 3 Lanciers (6800, 1500) ; plus de 22 m bord à bord entre camps (Archer agressif à
  21,3 m) ; caméra `FocusNear 6000 0`. Répété hors écran (prêt ~14 s) : cadre sur les ennemis 8 `(enemy)`, panneau
  « Tous (8) / Archer (3) / Lancier (5) », grille cachée, 8 barres pleines ; cadre mêlé 4 `(own)` ; Maj + clic ennemi
  garde les 4 ; Maj + clic entre ennemis 1 → 2 → 1 ; double clic Lancier slot 1 → 5 (pas ceux du slot 2) ; 20 s sans
  combat. Aucun bug. Remarque : les Archers ennemis peuvent tomber sous le panneau du scénario (le fermer). **Essai de
  l'utilisateur lancé** (Haldir).
- **Essai de D (`enemy_select`) : refusé.** Un cadre sur les deux groupes d'en face prend les 11 (log `box, 11 units
  (enemy)`) : l'utilisateur tenait le groupe de droite pour un allié. Cause : `GetSide` ne rend que Own ou Enemy (D.1 :
  « pas d'alliés en partie ») ; depuis Victoire V1 (fusionnée), les équipes existent (`FHOLMatchConfig::Slots[].Team`,
  `HOLVictoryRules::GetCamp`) ; le scénario mettait un 2e ennemi, pas un allié. Les cadres moi + ennemis ont bien gardé
  les miens (`box, 4 units (own)`).
- **Leçon (question de l'utilisateur : « pourquoi tu ne fais pas red test et boucle ? »)** : le test rouge vérifiait la
  règle avec des camps donnés à la main, pas la chaîne réelle configuration → camp → sélection ; la répétition ne
  couvrait pas Allié > Ennemi ; l'hypothèse « pas d'alliés » n'a pas été revue après la fusion. Règle précisée en
  mémoire (`red-test-first`) : liste des cas de la demande, chacun testé sur la vraie chaîne et répété avant l'essai.
- **Choix de l'utilisateur (A, la recommandation)** : le camp d'une unité vient des équipes de la partie (un joueur de
  mon équipe = Allié) ; le joueur de droite du scénario passe dans mon équipe. Écartés : deux ennemis séparés ; les deux.
  **D.3** confiée à un sous-agent.
- Cas de la ligne à couvrir, chacun par un test sur la vraie chaîne (config → camp → sélection) **et** par la
  répétition : (1) cadre ennemis seuls ; (2) cadre alliés seuls ; (3) moi + ennemis → moi ; (4) moi + alliés → moi ;
  (5) alliés + ennemis → alliés ; (6) alliés pris, Maj + clic ennemi → alliés restent ; (7) les miens pris, Maj + clic
  ennemi puis allié → les miens restent ; (8) double clic allié / ennemi → même type, même joueur ; (9) Neutre : aucune
  unité neutre en jeu, règle seule testée. D.3 lancée : test d'automatisation depuis un vrai `FHOLMatchConfig` + partie
  `quick_check` « select sides » (vraie partie à 3 joueurs), toutes deux rouges d'abord ; scénario `enemy_select` avec le
  joueur de droite allié ; vérifier si le combat respecte les équipes.

## F — `quick_check` : lancer une sous-partie seule

- Choix de l'utilisateur le 2026-10-23 (après E) : pouvoir lancer « build cut » ou « build gap » seules, au lieu de
  toute la partie `build` (~5 min par passage pour ~1 min utile pendant la bissection de E).
- Worktree `HeroesOfLiberty-Arwen` (libre : victoire V1 fusionnée, session finie), branche `quickcheck-cases` (depuis
  `main` ce7c9f3). Outil seulement (`Tools/Tests/quick_check.ps1`).
- F.1 confiée à un sous-agent, boucle de 15 min : syntaxe `-Parts build:cut,build:gap`, test rouge d'abord.
- F.1 rouge (Arwen compilée en 5 s) : `-Parts build:gap` → `parts: build:gap`, rien de joué, « sequence : PASS » en
  14 s (faux vert : aucun contrôle de build).
- F.1 codée (non commitée) : table `$SubCaseTable` (build : `cut` = cut + cut move ; `gap` = gap + gap one + gap two +
  wall, car le rapport « build gap » lit aussi « build wall »), filtre dans `Step` sur les « mark build … » (cas non
  demandés sautés avec leurs touches ; au premier cas gardé : bois 1000, LogDeposits pour cut, centre sélectionné puis
  Échap), seulement la partie à 2 de camera, seuls les rapports des cas demandés. `build:xyz` → « unknown case 'xyz'
  of build; cases: cut, gap », code 1 ; `move:abc` → « sub-cases not supported for move », code 1. BOM gardé.
- F.1 vert : `-Parts build:gap` **54 s**, FAIL comme sur main (two : 15 put back, « one after the other » KO, 2
  échantillons) ; un premier passage avait pris 207 s, dont 147 s d'attente d'UnrealBuildTool (ValidatePlatforms, verrou
  partagé avec une autre copie) au lancement : hors script. `-Parts build:cut` **93 s** (le cas attend ~67 s à lui
  seul), FAIL comme sur main (413 pas, 23 put back, 2 nouveaux chemins). Les analyses des cas sautés lisaient des
  sections vides : `[regex]::Match([string]$X…)` levait sur `$null`, remplacé par `"$X"` (7 lignes, même sens).
- `-Parts build:cut,build:gap` **120 s** : les deux mêmes FAIL (cut 23 put back / 2 chemins ; gap two 15 put back).
- Pas refait : `-Parts build` complet (~290 s), faute de temps dans la boucle (13 min) ; à faire avant le commit.
- `-Parts build` complet (session principale) : **séquence arrêtée** à l'étape 76 (`waitlog Placement: ended`, après
  l'Échap de la pose), tous les cas suivants KO. **Même arrêt avec le quick_check.ps1 de main ce7c9f3** : rouge nouveau
  sur main, pas de F ; la session MV.4 l'avait vu aussi. Cause : victoire V1 (D356), Échap sans rien de sélectionné ouvre
  le menu en jeu ; le premier Échap de la partie build (« nothing selected ») l'ouvrait, l'Échap de la pose ne faisait
  plus que le fermer. Correctif (Arwen) : centre sélectionné puis Échap (désélection, pas de menu), comme la mise en
  place des cas seuls. `-Parts build` **298 s** : tout PASS sauf build cut et build gap (mêmes chiffres : 23 / 15 put
  back). Session MV.4 prévenue. Attend la validation de l'utilisateur pour le commit.
- **Validé par l'utilisateur** (« go »). Commit 0ee0f19 (Arwen, `quickcheck-cases`) ; `main` (sélection ennemie, GDD
  D369-D376) fusionné dedans, compilation PASS (34 s), `-Parts build` 322 s : tout PASS sauf build cut et build gap
  (mêmes chiffres) ; `main` avancé à efaa230. Arwen libérée. Rien à retirer d'EtapeDefini (ni l'option ni le blocage
  d'Échap n'y avaient de ligne). **F close.**
- **D.3 faite** (sous-agent, Haldir, non commitée, ~19 min) : `GetSide(Config, OwnerSlot, LocalSlot)` par
  `HOLVictoryRules::GetCamp` (même camp = Allié) ; `AHOLGameState::LaunchConfig` répliquée (`GetLaunchConfig`), sans
  quoi un client ignore ses alliés ; `hol.Select.ClickUnit … ally`. Tests sur la vraie chaîne :
  `HeroesOfLiberty.Movement.PickSideByTeams` (vrais `FHOLMatchConfig`, avec et sans équipes) rouge (16/21) puis vert
  (21/21) ; partie `quick_check` **`sides`** (vraie partie à 3 joueurs) FAIL sur l'ancien code (cas 2 : `5 units
  (enemy)` ; cas 5 : 13 mêlés ; cas 6 : ennemi ajouté aux alliés) puis PASS ; `-Parts tests,smoke,sides` PASS (123/123).
  Répétition hors écran du scénario, les 8 cas : (1) 8 `(enemy)`, grille cachée, 8 barres ; (2) 5 `(ally)` Archer x2,
  Spearman x3 ; (3) 4 `(own)` ; (4) 4 `(own)` ; (5) 5 `(ally)` ; (6) les 5 alliés restent ; (7) 4 `(own)` puis 4 ;
  (8) 3 Lanciers du slot 2 / 5 du slot 1 ; calme. (9) Neutre : règle seule, aucune unité neutre.
  **Le combat ne respecte pas les équipes** (`HOLCombatProcessor`, évitement : `OwnerPlayer !=`) : des alliés se battent
  entre eux ; non touché, scénario espacé (> 20 m). Scénario : moi + slot 2 équipe 1, slot 1 équipe 2 ; l'allié a
  3 Lanciers et 2 Archers ; mes Hommes d'armes en 4800, caméra `FocusNear 6600 0`, Zoom -3.
- **Incident** : le sous-agent a réécrit par erreur, avec un chemin relatif, deux fichiers de la **copie principale**
  (`HOLUnitPicking.h` : 2 lignes ; `quick_check.ps1` : BOM perdu — sans doute aussi l'origine du BOM perdu vu en C).
  Remis par `git checkout --` (session principale) ; rien d'autre de touché.
- Partie `net` non lancée par le sous-agent alors que la config est maintenant répliquée : `-Parts tests,sides,net`
  lancé par la session principale (Haldir).
- `-Parts tests,sides,net` (session principale, Haldir) : **PASS en 133 s** (tests 123/123, sides PASS, net 2 clients
  PASS). Liste des 9 cas pointée par la session principale : 1 à 8 testés sur la vraie chaîne et répétés, 9 (Neutre)
  règle seule, dit. **Deuxième essai de l'utilisateur lancé** (`enemy_select`, Haldir).
- **Deuxième essai de D (`enemy_select`) validé** (« tout est bon »). Commits 9f9de38 (code, tests, partie `sides`,
  scénario) et 926090f (EtapeDefini : ligne retirée ; ajoutée en revue de code « le combat ignore les équipes », le camp
  Neutre de la sélection à brancher sur les postures de D369), `main` (7e869e4, doc seule) fusionné dans
  `enemy-selection`, `main` avancé en avance rapide (rien ne tournait dans la copie principale). Haldir libre. **D close.**

## G — Le combat ignore les équipes (des alliés se battent entre eux)

- Ligne de « Revue de code » d'`EtapeDefini.md`, vue à la clôture de D (D.3 : la sélection suit les équipes, le combat
  non). Choix de l'utilisateur le 2026-10-23 (« go » sur la recommandation : ce bug d'abord, avant J7c.4).
- Worktree `HeroesOfLiberty-Elrond`, branche `allied-combat` (depuis `main` e26ede1).
- Règle : deux joueurs sont hostiles s'ils ne sont pas du même camp (`HOLVictoryRules::GetCamp`, comme `GetSide` de la
  sélection) ; une seule fonction, où se brancheront les postures de D369 plus tard. Le combat est sous surveillance
  (J7c.4, S3/S4 au-dessus du seuil) : le correctif ne doit rien coûter (camp par slot lu dans un petit tableau).
- Cas à couvrir, chacun par un test sur la vraie chaîne (vrai `FHOLMatchConfig` → camp → combat) **et** par la
  répétition avant l'essai : (1) mes unités et celles d'un allié côte à côte : personne ne se bat, en Auto, Tenir et
  attaque-déplacement ; (2) un ennemi arrive : mes unités et celles de l'allié le combattent ; (3) clic droit sur une
  unité ou un bâtiment allié : pas d'attaque (comme sur les miens) ; (4) les bâtiments alliés ne sont pas pris pour
  cible d'office ; (5) les tirs ne blessent pas les alliés si ceux de mon camp sont épargnés ; (6) l'évitement traite
  l'allié comme les miens, pas comme un ennemi ; (7) sans équipes (chacun pour soi), tout le monde se bat comme avant.
- Attention : Haldir (ridge-rock, non commité) touche aussi `HOLSimulationProcessors.cpp` (évitement) : conflit
  possible à la fusion.
- **Boucle 1** (sous-agent, ~9 min, arrêtée à la limite) : règle unique `FHOLCampTable` (`HOLVictoryRules.h/.cpp`, un
  camp par propriétaire sur 256 octets, `FromWorld` lit `GetLaunchConfig`, sans game state chacun seul) et
  `HOLVictoryRules::AreHostile`. Combat (`HOLCombatProcessor`) : `Camp` dans `FFighter` et `FStepBuilding`, rempli une
  fois par pas ; toutes les comparaisons d'unités et de bâtiments par camp ; grille dense groupée par camp ; lignes de
  front gardées par joueur (sa propre armée). Ordres (`HOLUnitOrderProcessor`) : attaque sur unité ou bâtiment allié
  refusée. Tirs (cas 5) : une flèche ne touche que sa cible, pas de zone : rien à corriger. Partie `sides` : 4 contrôles
  (côte à côte en Auto, Tenir, attaque-déplacement ; ennemi qui arrive) **rouges** (10, 18, 32 coups entre alliés ;
  allié qui ne frappe pas l'ennemi) puis **verts** (`-Parts sides` PASS 31 s).
- Reste : `-Parts tests,combat` (cas 7) ; test d'automatisation de `FHOLCampTable` sur vrais `FHOLMatchConfig` ;
  cas 3 testé ; clic droit sur un allié (`HOLPlayerController` ~2073) envoie encore une attaque (refusée par le
  serveur : l'unité ne fait rien) → doit se déplacer comme sur les miens ; évitement (`HOLSimulationProcessors.cpp`,
  table locale, centres et lignes gardés par joueur) ; `HOLBuilding.cpp:344` ; scénario d'essai.
- Processus : boucle 1 trop large pour 15 min ; découpe en boucles plus étroites (2 : cas 7, règle, ordre et clic
  droit ; 3 : évitement et bâtiments).
- **Boucle 2** (sous-agent, ~15 min) : (7) `-Parts tests,combat` PASS 253 s avant tout changement (132/132) : pas de
  régression chacun pour soi. Test `HeroesOfLiberty.Movement.CampsByTeams` (vrais `FHOLMatchConfig` : 1/2/1, sans
  équipes, slot fermé, deux contre deux, sans config, propriétaire hors slots) vert 133/133 (règle écrite en boucle 1,
  pas de rouge montré). (3) `hol.Select.Attack slot=S` ; étape `sides fight order` : rouge sur un build avec l'ancienne
  comparaison (0 refus) puis vert. Clic droit (`OrderAt`) : unité ou bâtiment allié → déplacement ; `FindTargetAt` ne
  vise plus les alliés (`bHostileOnly`) ; pas de curseur d'attaque au survol (rien à changer) ; `hol.Select.ClickUnit
  … order` ; étape `sides fight rightclick` rouge (1 attaque, 0 déplacement) puis vert. `-Parts tests,sides,combat`
  PASS 247 s. Pas testé : ordre d'attaque sur un bâtiment allié (refusé par camp côté serveur).
- **Boucle 3** (sous-agent, ~12 min) : évitement (`HOLSimulationProcessors.cpp/.h`) : `FHOLCampTable CampTable` construite
  une fois par exécution dans les deux processeurs ; sens « ennemi » par camp (~487 `FirstContact`, ~502 `IsClear`,
  ~582, ~620 D293, ~671 front au contact, ~1398 `Touches`, ~1598 pas de poussée entre ennemis) ; sens « mon armée »
  gardés par joueur (~750 main choisie par les amis, centres, lignes de front : leur « milieu ennemi » inclut les
  alliés, approximation laissée). `HOLBuilding.cpp:344` : affichage client, pas une recherche d'ennemis : non touché.
  Revue du reste de `Source/` : seule `HOLUnitOrderProcessor.cpp:2130/2135` (rupture du bloc en mêlée, MV.4.4) a le sens
  « ennemi » ; `HOLCombatProcessor.cpp:176` à vérifier. Cas `sides avoid` (4 Hommes d'armes traversent un mur de 25
  Lanciers à l'arrêt, combat coupé ; témoin : mur à moi, puis mur allié) : **encore rouge** après le correctif
  (mur à moi : tous arrivés à 4 s, 8 déplacés ; mur allié : 1472 cm restants, 5 déplacés). Cause probable : la
  rupture du bloc compte l'allié immobile comme ennemi.
- Fusion à prévoir : mycrusader-7d (rochers, Haldir) avance `main` à f6703a9 (touche aussi `HOLUnitOrderProcessor` et
  `HOLSimulationProcessors`) ; mycrusader-e5 (M6) veut aussi avancer `main`. Fusionner `main` dans `allied-combat` avant
  l'essai, puis relancer les parties touchées.
- `main` avancée deux fois pendant la boucle 4 : f6703a9 (rochers) puis 7cacb4c (M6 R1 et R2). G couvre **R6** de
  `NetPlaytestInProgress.md` (R6.1 règle, R6.2 cibles, R6.3 clic droit ; R6.4 = la ligne retirée d'EtapeDefini au
  commit) : mycrusader-e5 ne la refait pas et la notera faite à la fusion. Après fusion de `main` : relancer aussi
  `-Parts net` (la config est répliquée, le combat tourne sur le serveur).
- **Boucle 4** (sous-agent, ~16 min) : rupture du bloc (`HOLUnitOrderProcessor.cpp` ~1866 table une fois par
  `AdvanceBlocks`, ~2132-2138 : ennemi par camp ; ami qui ne combat pas, tout le camp ; mon propre ordre par joueur) ;
  `HOLSimulationProcessors.cpp:750` aussi par camp. **Le cas `avoid` de la boucle 3 mesurait mal** (« slot » de
  `hol.Units.Orders` = place du pas en cours : 0 unité trouvée, d'où le faux « tous arrivés ») : réécrit avec
  `hol.Units.Where 0` (distance réelle de mes 4 au but, mur compté sans les miens). Somme à 4 s / déplacés : ancien
  code mur à moi 3957 / 4, mur allié **817 / 0** (contourné comme un ennemi, jamais poussé) ; nouveau code mur à moi
  3952 / 4, mur allié 3118-3235 / 10 (bruit ±60) : toujours rouge, écart stable ~770 cm, l'allié traversé **plus
  vite**. Piste : `OwnerCentres` (centre de l'armée par joueur, inclut mon mur, pas celui de l'allié) : règle par
  joueur légitime. `-Parts tests,sides,move,bodies,combat` 317 s : tout vert sauf `avoid` (133/133).
- Décision (session principale) : vérifier la piste avant de toucher au critère (boucle 5).
- **Boucle 5** (sous-agent, ~9 min, diagnostic) : l'écart venait **du test** : `hol.Units.Spawn … 60` tire les positions
  au hasard ; le mur à moi (tiré le premier) et le mur allié n'avaient pas la même forme (trous ~400 cm). `OwnerCentres`
  hors de cause (`SlideAlongFriends` seulement en marche au combat) ; aucune règle « s'écarter pour les miens » par
  joueur. Même graine pour les deux murs (`hol.Units.SpawnSeed 1234`) : PASS deux fois (4 s : 3774 / 8 contre 3775 / 8) ;
  même graine et ancien évitement : rouge (3774 / 8 contre 3409 / 0). Graine ajoutée au cas (session principale), seuils
  inchangés.
- Session principale : `allied-combat` avancée sur `main` 7cacb4c (aucun commit sur la branche : remise de côté, avance
  rapide, réapplication ; aucun conflit) ; aucune nouvelle comparaison « ennemi » par joueur venue des rochers.
  Lancés : build puis `-Parts tests,sides,move,bodies,combat,rocks,net`.
- Scénario d'essai **`ally_combat`** écrit (Elrond, `Tools/Scenarios/ally_combat.txt/.json`, non commité) : 3 joueurs
  comme `enemy_select` ; mes 4 Hommes d'armes + 3 Archers collés à 4 Lanciers + 2 Archers alliés ; 6 Lanciers ennemis
  au loin (> 25 m).

### Reprise exacte (G)

État : tout le code est dans `HeroesOfLiberty-Elrond`, branche `allied-combat` = `main` 7cacb4c + changements **non
commités** (11 fichiers + scénario `ally_combat`) ; `IN_USE.txt` posé. Sauvegarde des sources avant l'avance sur main :
scratchpad de la session du 2026-10-23 (`elrond_src_backup`, `elrond_qc_backup.ps1`), inutile si `git status` d'Elrond
montre les 11 fichiers. Étapes restantes, dans l'ordre (cocher ici au fur et à mesure) :
1. [x] Build Elrond (PASS 64 s) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
2. [x] (`net` vert après fusion de M6 R3) `quick_check -Parts tests,sides,move,bodies,combat,rocks,net` (script d'Elrond) : tout vert attendu (tests 133+).
   Un rouge : regarder s'il est connu (« build site » en enchaînement) ou vient de la fusion avec les rochers.
3. [x] Répétition hors écran de `ally_combat` (`play.ps1 ally_combat -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`,
   sans fenêtre) : prêt, 19 unités, aucun coup entre camps alliés sur 20 s (`hol.Combat.Report`), puis les 3 notes.
4. [x] Essai de l'utilisateur (validé 2026-10-24 : « works ») : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 ally_combat -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
5. [x] Après validation : commit en anglais sur `allied-combat` (sans Co-Authored-By), retirer la ligne « Le combat ignore
   les équipes » d'`EtapeDefini.md` (même commit, R6.4), fusionner dans `main` (avance rapide si possible), prévenir
   mycrusader-e5 (M6 : R6 faite, avec le hash), libérer Elrond (`IN_USE.txt` supprimé), clore G ici.
- Étape 2, 1er passage (645 s) : **tout PASS** (tests 134/134, sides avec `avoid`, move, rocks, bodies, combat et ses
  sous-parties, orders, camera) **sauf `net 2 clients`** : aucun client ne prend de place. Journal client
  (`Elrond\Saved_Tests\Logs\quick_net_client1.log`) : Iris « Protocol mismatch » sur `HOLPlayerController` (ensure
  `NewProtocolId == ProtocolId`), serveur et clients pourtant lancés depuis Elrond. Notre diff de
  `HOLPlayerController.h` ne touche rien de répliqué. En cours : même `-Parts net` sans nos changements (remis de côté
  par `git stash`, sauvegarde `scratchpad\elrond_src_backup2`) pour savoir si `main` 7cacb4c casse déjà `net`.
  **Si la session s'arrête ici** : dans Elrond, `git stash list` ; si « allied-combat wip2 » y est, `git stash pop`
  avant tout (nos 11 fichiers), puis rebuild.
- Résultat : **`main` 7cacb4c seul casse déjà `net`** (Elrond sans nos changements, build 125 s, `-Parts net` FAIL
  104 s, même « Protocol mismatch » sur `HOLPlayerController`). Pas G. Nos changements remis (`git stash pop`, plus de
  stash), rebuild lancé. mycrusader-e5 (M6) prévenue (rochers f6703a9 ou M6 7cacb4c à départager ; Haldir à f6703a9).
  Étape 2 considérée **faite pour G** (tout vert hors `net`, rouge venu de `main`) ; à refaire `net` quand `main` sera
  réparée, avant la fusion de G.
- **Correction** (mycrusader-e5) : pas un bug de `main` : **collision de port**. La partie `net` lance son serveur sur
  7777 en dur ; au même moment Arwen (7777) et Glorfindel (7778) faisaient tourner le leur : nos clients allaient chez
  le serveur d'une autre copie (autre code → « Protocol mismatch »). Le correctif (port libre `$NetPort`) est sur
  `net-playtest`, entrera dans `main` avec R3. **À faire** : relancer `-Parts net` dans Elrond quand
  `Get-NetUDPEndpoint -LocalPort 7777` ne montre plus rien (PowerShell), puis cocher l'étape 2.
- mycrusader-7d avance `main` à ecc0190 (`hol.Units.Remove [Type]` facultatif, rocks hors de `-Full`) : à reprendre
  dans `allied-combat` avant le commit (avance rapide de la branche comme pour 7cacb4c : `git stash`, `git merge
  --ff-only main`, `git stash pop`), puis `-Parts sides` (utilise `hol.Units.Remove` sans Type).
- Étape 3 faite : copie temporaire du scénario (sans notes, + `wait 20`, rapports, `exit`), `-RenderOffScreen`
  (`-nullrhi` bloque à `waitui`) : prêt en 21,8 s, 19 unités (0=7, 1=6, 2=6), `Combat report` à 0 s et à 20 s :
  fighting 0, hits 0, kills 0. Fichiers temporaires supprimés. Le clic droit et la traversée sont couverts par
  `sides` (fight order, fight rightclick, avoid). `main` est passée à ecc0190 puis a74a5d1 (V3, touche aussi
  `HOLPlayerController`) : à reprendre dans la branche après l'essai, avant le commit (puis `-Parts tests,sides,victory`).
- mycrusader-e5 avance `main` avec M6 R3 (e8a0e36 + 1) : touche `HOLPlayerController`, `HOLGameState`,
  `HOLMatchConfig` v2, `quick_check.ps1` (partie `net` : port libre, la collision 7777 réglée). Fusion de `main` dans
  `allied-combat` après l'essai : conflits probables `HOLPlayerController.cpp/.h`, `quick_check.ps1` ; vérifier que
  `Slots[].Team` / `GetCamp` sont inchangés (question posée à e5). Ensuite `-Parts tests,sides,combat,move,net,victory`.
- Réponse de e5 : `main` 174a0f4 ; `HOLVictoryRules` non touchés, `Slots[].Team` inchangé (ajout `PlayerName` en fin
  de `FHOLMatchSlot`) ; `HOLPlayerController` : garde `IsCheatRefused()` en tête des `ServerCheat*` + surcharge
  `ServerExecRPC` (conflit improbable avec nos lignes ~470 `hol.Select.Attack slot=`, ~1771-1794, ~2087-2095) ;
  `quick_check` partie `net` réécrite (port libre), pas `sides`.
- Étape 5 en cours : `allied-combat` avancée sur `main` 174a0f4 (stash, `--ff-only`, pop : aucun conflit ; sauvegarde
  `scratchpad\elrond_src_backup3`). Nouveau venu de `main` : `HOLSimulationProcessors.cpp` ~588 `NearPressed` (cache
  rapide de `FirstContact`, `Owners[Other] != Owner`) passé par camp comme `FirstContact`. Lancés : build puis
  `-Parts tests,sides,move,bodies,combat,rocks,net,victory`.
- Après fusion de `main` 174a0f4 : build 57 s, `-Parts tests,sides,move,bodies,combat,rocks,net,victory` **PASS en
  622 s** (tests 146/146, `net 2 clients` PASS, `victory forum` PASS). Ligne « Le combat ignore les équipes » retirée
  d'`EtapeDefini.md` (R6.4). Commit sur `allied-combat`, `main` avancée, mycrusader-e5 prévenue (R6 faite), Elrond
  libérée. **G close.**

## H — `quick_check -Parts move,bodies,build` : « build site » rouge (plafond +20)

Demandé le 2026-10-25 (« go » sur la recommandation : H tout de suite, le lanceur en parallèle). Le plafond de
population vu par « build site » passe de 30 à 40 au lieu de 10 à 20 : un reste des parties d'avant (sans doute deux
maisons du slot 0 posées par `bodies`). `-Parts build` seul passe.

- 2026-10-25 : Elrond (libre, `allied-combat` fusionnée) passé sur une nouvelle branche `build-site-leftover` = `main`
  c9fc0eb, `IN_USE.txt` posé. Sous-agent lancé : rouge (`-Parts move,bodies,build`), cause des +20, correctif dans le
  script de test (nettoyer l'état plutôt qu'affaiblir les contrôles), vert (`move,bodies,build` puis `build` seul).
  En parallèle, dans la session principale : questions L-Q1 à L-Q6 du lanceur (`LauncherInProgress.md`).
- Sous-agent (20 min, dont ~12 min de tests) : **cause** = les deux maisons de la partie `bodies` (`hol.Build.PlaceNear
  House 6000 -3400` et `6000 -2600`, finies par `hol.Build.Instant 1`, à (-8800, -3200) et (-8800, -2400)), gardées par
  « mark bodies end » (« the two houses stay ») : +10 de plafond chacune. `move` et `gather` hors de cause.
  **Correctif** (`quick_check.ps1` ~875-880, +5 −2) : « mark bodies end » les détruit par `hol.Cheat.DamageNear 99999
  … own melee` et attend `Building destroyed: House of slot 0 at (…)` ; contrôles de « build site » inchangés (absolus).
  Build PASS 39 s ; **rouge** `-Parts move,bodies,build` (359 s) : build site FAIL (30 → 40, HUD pareil ; « 30 avec A et
  C » lu 50) ; **vert** `-Parts move,bodies,build` PASS 359 s (toutes sections), `-Parts build` seul PASS 290 s.
  Non commité ; attend l'accord de l'utilisateur. Au commit : la copie principale a `EtapeDefini.md` modifié par une
  autre session (J7c.5) : ne stager que notre ligne retirée.
- Accord de l'utilisateur (« go commit oui ») : correctif appliqué dans la copie principale et commité sur `main`
  (avec la ligne retirée d'`EtapeDefini.md`, seule notre partie du fichier, et le lanceur : questions tranchées, plan
  L0-L10) ; Elrond remis sur `main`, `IN_USE.txt` retiré. **H close.**

## I — Roi : log du refus et activité « Attaque » qui reste

Ligne d'`EtapeDefini.md` (« Aléas et petits restes ») : « Roi : le refus d'un ordre sur un bâtiment se logue « not its
own » (texte commun, trompeur) ; activité « Attaque » qui reste après le départ de son agresseur » (vus en V5.5a et
V5.5b, `VictoryInProgress.md`). Menée le 2026-10-25 par un sous-agent dans Arwen, branche `etape-king-leftovers` =
`main` a080862, `IN_USE.txt` posé ; l'utilisateur absent : décisions prises et notées ici.

- **(a) Cause** : `HOLUnitOrderProcessor.cpp` (boucle des unités d'un ordre) comptait dans un seul `NumRefused` les
  trois refus (pas à lui, D288 ; ordre sur un bâtiment sans attaque de bâtiment, le Roi V5.1 ; cible de son propre camp,
  ordre du serveur sans emplacement) et les loguait tous « Move order of slot %d: %d units ignored, not its own ».
  **Correctif** : trois compteurs, trois textes. Le texte « not its own » est gardé pour le seul refus de propriété
  (la partie `net` de `quick_check` le lit, ~l. 4340, inchangée) ; nouveaux : « Attack order of slot %d on a building:
  %d units ignored, no building attack » et « Attack order of slot %d on a building|#id: %d units ignored, a target of
  their own camp ». Aucun autre lecteur de l'ancien texte (grep `Source`, `Tools/Tests`, `Tools/Scenarios`).
- **(b) Cause** : pas dans la riposte. Combat allumé, la cible retirée ou morte est lâchée au pas suivant
  (`HOLCombatProcessor.cpp`, « Its target, if it still stands » : `bAttacking = false`) et une cible hors de portée
  aussi. Mais `quick_check` (pas « victory regicide riposte ») retire l'Homme d'armes **et** fait `hol.Combat.Enabled 0`
  dans la même image : combat éteint, le processeur ne faisait plus que finir les mourants et laissait tout
  `FHOLCombatFragment` figé, donc `bAttacking` vrai → la vue (`GetUnitView`), le HUD, le geste et les clients montrent
  « Attacking » tant que le combat reste éteint, sous le bandeau compris (la simulation continue après la fin). Même
  défaut pour toute unité qui frappait (Hommes d'armes testés), pas propre au Roi. **Correctif** (+3 lignes) : combat
  éteint, chaque unité perd `bAttacking` (« personne ne frappe ») ; sa cible est gardée, et rallumé, le pas normal la
  lâche si elle est partie ou reprend le combat (un nouvel élan `StartAttack` : seul changement, sur une commande de
  test). Rien d'autre du combat ne bouge.
- **Tests rouges d'abord**, vraie chaîne (file de commandes + `UHOLUnitOrderProcessor`, `UHOLCombatProcessor` lancés
  dans un monde de test comme `Forum.Builders`) : `HeroesOfLiberty.Victory.Regicide.RefusalLog` (Roi + unité ennemie
  envoyés par l'emplacement 0 sur une Maison ennemie, puis une unité de l'emplacement 1 par un ordre sans emplacement
  sur la Maison de son camp) et `…Regicide.RiposteEnds` (riposte du Roi contre un Homme d'armes, retiré combat allumé ;
  un second retiré dans l'image où le combat s'éteint ; deux Hommes d'armes qui se battent, combat éteint puis rallumé).
  Le monde de test reste au pas 0 et une unité ne cherche qu'un pas sur 4 selon son identifiant : `PlaceUnit(…, true)`
  refait naître l'unité jusqu'à un identifiant multiple de 4. **Rouge** (build PASS 30 s, Regicide 6/8) : RefusalLog
  (les 3 textes attendus absents, lus « 2 units ignored, not its own » et « slot -1: 1 units ignored, not its own »),
  RiposteEnds (3 KO : Roi « Attacking » combat éteint, les deux Hommes d'armes aussi ; les témoins combat allumé verts).
  **Vert** après correctif (build PASS 109 s, en file derrière Elrond) : Regicide 8/8, deux fois ; Victory + Combat +
  Net 68/68.
- **`quick_check`** (`-Parts victory`) : 3 contrôles ajoutés à « victory regicide » — après le retrait de l'agresseur,
  le Roi n'est plus « Attacking » (rapport du HUD) ; nouveau pas « mark victory regicide refusal » (Maison ennemie par
  `hol.Cheat.SpawnBuilding House 1 -1800 -1600`, `hol.Select.Army`, `hol.Select.OrderNear -1800 -1600`, `wait 1`) : ordre
  « Order: 1 units attack House of slot 1 » et seul refus loggé « no building attack » ; sous le bandeau, pas
  « Attacking ». **Rouge** rejoué hors jeu sur le `quick_game.log` d'avant (V5.5a) : 3 KO sur 3 (« EHOLUnitActivity::Attacking »
  deux fois, pas de refus). 1er passage : le refus manquait (`frames 5` trop court, l'ordre part au pas suivant) →
  `wait 1`. **Vert** : `-Parts victory` PASS en 83 s ; `-Parts victory,net` PASS en 139 s (texte « not its own » du
  réseau toujours lu).
- **Scénario** : `Tools/Scenarios/regicide.txt` inchangé (le refus ne se voit qu'au log, le combat n'y est jamais
  éteint). L'utilisateur peut rejouer `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 regicide -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
  et regarder, au temps 3, que le bloc du Roi ne garde pas « Attaque » après la mort du Lancier ; au temps 4, que le
  Roi seul ne bouge pas sur la Maison (le log dit « no building attack »).
- Fichiers : `Source/HeroesOfLiberty/Commands/HOLUnitOrderProcessor.cpp`, `Source/HeroesOfLiberty/Simulation/HOLCombatProcessor.cpp`,
  `Source/HeroesOfLiberty/Tests/HOLRegicideTests.cpp`, `Tools/Tests/quick_check.ps1`. Non commité.

### Reprise exacte (I)

État : tout dans `HeroesOfLiberty-Arwen`, branche `etape-king-leftovers` = `main` a080862 + changements **non
commités** (4 fichiers ci-dessus + ce fichier) ; `IN_USE.txt` posé.
1. [x] Tests rouges, correctifs, tests verts (Regicide 8/8, Victory + Combat + Net 68/68).
2. [x] `quick_check -Parts victory` PASS (83 s), `-Parts victory,net` PASS (139 s).
3. [ ] Essai de l'utilisateur (facultatif, scénario `regicide` ci-dessus) puis son accord.
4. [ ] Après accord : commit en anglais sur `etape-king-leftovers` (sans Co-Authored-By), retirer la ligne « Roi : le
   refus… » d'`EtapeDefini.md` dans le même commit, fusionner dans `main`, libérer Arwen (`IN_USE.txt`), clore I ici.

**Clos le 2026-10-25** : essai `regicide` validé par l'utilisateur (temps 3 et 4 conformes), commité et fusionné dans `main`. Vu pendant l'essai, noté dans `EtapeDefini.md` (« Retours d'essai ») : des unités qui attaquent un bâtiment s'empilent au même point.

## J — Unités qui s'empilent en attaquant un bâtiment

Ligne d'`EtapeDefini.md` (« Retours d'essai ») : « Unités qui s'empilent au même point en attaquant un bâtiment (groupe
d'Hommes d'armes fondu en une masse sur le bord d'une Maison ennemie, essai du 2026-10-25) ». Choisie le 2026-10-26
par l'utilisateur (« le moins de retour de ma part », il part) ; menée en sous-agents dans `HeroesOfLiberty-Elrond`,
branche `etape-building-crowd` = `main` 24c7ce5, `IN_USE.txt` posé. Décisions prises en son absence : notées ici.

- **Piste de départ** (session principale) : `HOLCombat::GetBuildingApproachPoint` (`Simulation/HOLCombatRules.cpp`)
  vise le point du mur **le plus proche** de l'unité (+ rayon + écart). Un groupe serré vise donc presque le même point,
  et un groupe qui arrive en biais vise tous **le coin** (même projection). Appelé par `HOLCombatProcessor.cpp`
  (~l. 1566) à chaque marche d'approche.

### Plan

1. [x] Boucle 1 : diagnostic confirmé et test rouge sur la vraie chaîne (ordre d'attaque d'un groupe sur une Maison,
   plusieurs angles dont le biais, graines tirées au hasard), montré en échec.
2. [x] Boucle 2 : correctif (places réparties le long du mur), test vert, non-régression des tests de combat.
3. [x] Boucle 3 : `quick_check` des parties touchées.
4. [x] Boucle 4 : scénario d'essai `building_crowd` et sa répétition hors écran.
5. [ ] Essai de l'utilisateur, puis commit, fusion dans `main`, ligne retirée d'`EtapeDefini.md`, Elrond libéré.

### Suivi

**2026-10-26 — Boucle 1 (sous-agent) : cause confirmée, test rouge.**

- **Cause.** `HOLCombat::GetBuildingApproachPoint` (`Simulation/HOLCombatRules.cpp`, l. 309-323) : le point visé est le
  point du mur le plus proche de l'unité, plus rayon + 40 cm. Chaque unité du groupe le calcule seule, depuis sa
  place (`HOLCombatProcessor.cpp`, l. 1564-1570). Un groupe serré vise donc 2 à 5 m de mur, jamais plus. Rien ne
  donne une place à chacun.
- **Ce qui ne bouge plus.** Une unité qui frappe est « du front » : elle est un mur pour ses amis
  (`HOLSimulationProcessors.cpp`, l. 1441 et 1848, FrontHoldsFriends). Les suivants se rangent derrière, pas à côté.
- **Portée.** La portée de bâtiment de l'Homme d'armes est de 400 cm (table des unités) : le 3e ou 4e rang frappe
  encore, à travers les autres. D'où la masse : pas d'empilement (les unités restent à 120 cm = 2 rayons), mais un
  paquet de 3 ou 4 rangs sur un bout de mur.
- **Le test.** `hol.Test.BuildingCrowd <Count> [Seed] [Type] [quit]` (`Source/HeroesOfLiberty/Dev/HOLBuildingCrowdTest.cpp`),
  lancé par `Tools/Tests/building_crowd.ps1` (hors écran, comme `crossings.ps1`). Plain, joueur + IA. À chaque
  répétition : une Maison de l'IA posée par le chemin de `hol.Cheat.SpawnBuilding`, un groupe de 6 à 12 né à un angle
  et une distance tirés (12 à 22 m), puis l'ordre du clic droit (`RequestAttackBuilding`). La Maison garde ses PV
  jusqu'à la mesure. Mesure 2 s après que tous frappent (ou à 25 s). Critères : (1) tous frappent, à portée du mur ;
  (2) aucune paire à moins de 1,5 rayon ; (3) le bout de mur couvert (périmètre moins le plus grand trou) au moins
  80 % de 2 rayons par unité (un seul rang).
- **Rouge (graine 1, 8 répétitions) : 8 sur 8 en échec**, toujours par le critère 3. Mur couvert / besoin (cm) :
  256/576, 419/768, 190/672, 197/768, 272/480, 538/672, 345/960, 760/864. Paire la plus proche : 120 cm partout.
  Unité la plus loin du mur : 214 à 379 cm (des rangs derrière). Tous frappent en 4,5 à 9 s.
- **Commande.** `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Tests\building_crowd.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -Count 20 -Seed 1`
  (~10 s de jeu par répétition ; 20 répétitions ≈ 4 min).
- **Note de compilation.** Après le passage au moteur Hazelight 5.8.4 (D483), les `.gen.cpp` d'Elrond étaient
  périmés (erreurs C2440) : dossier `Intermediate\Build\Win64\UnrealEditor\Inc\HeroesOfLiberty\UHT` effacé, puis
  compilation verte. Les autres copies auront sans doute le même souci.
- **Reste.** Lancer 20 répétitions et d'autres graines (boucle 2, avec le correctif). Peut-être resserrer le
  critère 1 (frapper depuis le 1er rang : distance au mur ≤ ~1 rayon) si l'on veut aussi chasser les rangs de derrière.

**2026-10-26 — Boucle 2 (sous-agent) : correctif écrit, compilé ; test vert pas encore mesuré (arrêt à 15 min).**

- **Mécanisme.** Une place par unité, donnée une fois à l'ordre d'attaque de bâtiment. `HOLCombat::GetBuildingPlaces`
  (`Simulation/HOLCombatRules.h/.cpp`, fonction pure, O(n log n)) : un anneau autour de la boîte (rayon + 40 cm, côtés
  et quarts de cercle aux coins), places à 2 rayons ou un peu plus ; la 1re en face du milieu du groupe, puis de part et
  d'autre en alternant ; unités rangées par leur projection sur l'anneau (identifiant en cas d'égalité), pas de
  croisement. Au-delà de la place de l'anneau, les unités les plus loin du mur vont sur un 2e anneau (2 rayons plus loin).
- **Où.** `UHOLUnitOrderProcessor::ApplyCombatOrder` (`Commands/HOLUnitOrderProcessor.cpp`) calcule les places pour le
  groupe ; `FHOLCombatFragment` garde `PlaceBuilding` + `BuildingPlace` (effacés par `Disengage`) ; la marche d'approche
  et ses reprises (`HOLCombatProcessor.cpp`, ~l. 1566) visent la place si elle est pour ce bâtiment, sinon le point le
  plus proche comme avant. L'écart de 40 cm est passé dans `HOLCombat::BuildingApproachGap` (même valeur). Portée de
  frappe (400 cm) et valeurs d'équilibrage : inchangées.
- **Test pur ajouté** (pas encore compilé ni lancé) : `HeroesOfLiberty.Combat.BuildingPlaces` (`Tests/HOLCombatTests.cpp`) :
  groupe de 1, groupe en biais vers un coin, tous au même point (égalités par identifiant), ordre de la liste inversé,
  groupe de 25 (18 sur le 1er anneau, 7 sur le 2e).
- **Fait.** Compilation verte avec le correctif (avant l'ajout du test pur). `building_crowd.ps1 -Count 8 -Seed 1` lancé,
  **rouge** : 1/8 : 7 Hommes d'armes, mur couvert 319/576 cm, le plus loin à 273 cm du mur ; 2/8 : 9 Hommes d'armes, 8/9 frappent au bout de 25 s, mur couvert 745/768 cm, le plus loin à 257 cm ; puis le jeu s'arrête sans ligne de fin (journal : `Saved_Tests\Logs\building_crowd.log`). Cause probable : une marche bloquée par ceux qui frappent finit court (`bWalkEndedShort`), la portée passe à 400 cm et l'unité frappe depuis le 2e rang, loin de sa place ; à suivre : « à portée » seulement près de sa place quand l'unité en a une.
- **Reste.** Lire le résultat graine 1, puis graine 2 ; recompiler avec le test pur et lancer `HeroesOfLiberty.Combat` ;
  `HOLCombatTests.cpp` l. 673 (point d'approche seul) ne change pas : la fonction reste pour les attaques sans place.
- **Restes notés.** Sans place (attaque-déplacement, recherche automatique, file Shift prise une à une) : comportement
  d'avant. Place dans un obstacle : la marche finit court (`bWalkEndedShort`), l'unité frappe si elle est à 400 cm ; sinon
  la reprise vise la même place et peut tourner en rond (avant, le point le plus proche changeait avec sa position) :
  risque, pas traité.
- **Bilan de méthode** (session principale) : la boucle 2 a voulu coder, compiler, tester puis recompiler ; à 13 min, le
  test tournait encore. Désormais une boucle = **un seul changement, une seule compilation, un seul passage** du test
  (8 répétitions, ~2 min) ; la 2e graine et les tests d'automatisation, une boucle à part.

**2026-10-26 — Boucle 3 (décision de la session principale, utilisateur absent).** Une unité qui a une place ne frappe
qu'arrivée près de sa place (≈ 1 rayon). Repli contre le blocage : si sa marche finit court plusieurs fois de suite (ou
après quelques secondes), elle reprend la règle d'avant (portée 400 cm), pour ne jamais tourner en rond. La portée
elle-même ne change pas.
- **Boucle 3 arrêtée au minuteur** (15 min) : le code était écrit (repli `BuildingPlaceGiveUpMs` = 4 s, place atteinte à
  `BuildingPlaceReachRadii` = 1 rayon, `PlaceBlockedMs` dans `FHOLCombatFragment`), mais la compilation n'était pas
  finie. Cause : machine chargée (compilation du moteur depuis `C:/UE/Src/HZ` depuis 11 h 22, et deux compilations
  d'un autre projet dans la même file d'UnrealBuildTool). **Bilan de méthode** : avec la machine chargée, la
  compilation sort de la boucle du sous-agent ; la session principale la lance en tâche de fond, puis un sous-agent ne
  fait que le test et son diagnostic.
- **Boucle 3, résultat** (compilation par la session principale : 2179 s, presque tout en file d'attente ; test lancé
  par la session principale, 189 s) : `building_crowd.ps1 -Count 8 -Seed 1` → **5/8 OK** (avant : 0/8). Un seul rang
  partout : le plus loin du mur à 87-143 cm (avant 214-379). Échecs : **1/8** mur couvert 528/576 (7 unités, le plus
  loin à 143 cm) ; **4/8** 1 unité sur 9 ne frappe pas en 25 s (bloquée ?) ; **8/8** mur couvert 846/864 (10 unités,
  paire la plus proche 107 cm < 2 rayons : des unités pas sur leur place). Test pur `BuildingPlaces` : compilé, pas
  encore lancé.

**2026-10-26 — Boucle 4 (sous-agent) : 7/8.**

- **Diagnostic (lecture du code).** (a) L'unité qui ne frappe jamais (4/8 de la boucle 3) : le repli ne comptait que
  si la marche finissait court (`bWalkEndedShort`). Une unité poussée, ou qui marche sans finir, ne comptait jamais :
  elle gardait sa place sans fin. Et au repli, sa vieille marche vers la place continuait (pas de nouvelle marche).
  (b) Les manques de mur (1/8, 8/8) : les places étaient espacées de 2 rayons sur l'anneau, quarts de cercle compris.
  Un quart de cercle (157 cm) ne couvre aucun mur : deux unités autour d'un coin couvraient bien moins de 2 rayons de
  mur, et leur paire passait sous 120 cm (corde). D'où 107-108 cm et 846/864.
- **Changement.** (a) `HOLCombatProcessor.cpp` + `FHOLCombatFragment::PlaceBestDistance` : le repli compte le temps
  sans s'approcher de sa place d'au moins 20 cm (`HOLCombat::BuildingPlaceProgress`) depuis son meilleur, quoi que
  fasse la marche. À 4 s (inchangé), l'unité oublie sa place et repart tout de suite (`bApproaching` remis à faux)
  par la règle d'avant. (b) `HOLCombatRules.cpp` (`FBuildingRing`) : plus d'arc aux coins ; les places sont espacées
  le long du mur lui-même, chacune au large de son côté. Test pur `BuildingPlaces` : 13 + 12 au lieu de 18 + 7 (pas
  encore lancé). (c) Diagnostic dans le test : pour une répétition en échec, une ligne par unité (position, place,
  distance, place gardée ou oubliée, temps bloqué, meilleure distance, phase de marche, frappe).
- **Résultat** (`building_crowd.ps1 -Count 8 -Seed 1`, compilation 286 s, test 143 s) : **7/8 OK** (avant 5/8).
  1/8 OK (6,5 s, mur 718/576, le plus loin 135 cm) ; 2/8 OK (5,8 s, 946/768) ; **3/8 FAIL** : une paire à 84 cm (< 90) ;
  4/8 OK (9,4 s, 9/9 frappent, 912/768) ; 5/8 OK (4,5 s, 542/480) ; 6/8 OK (10,7 s, 842/672, le plus loin 174 cm) ;
  7/8 OK (7,2 s, 1140/960) ; 8/8 OK (10,2 s, 994/864, paire la plus proche 120 cm).
- **Cause de 3/8 (lignes par unité).** Deux unités sur 8 ont oublié leur place (bloquées 4 s) : l'unité 23 (sa place
  sur le côté gauche, elle finit sur le haut, à 633 cm de sa place) et l'unité 17 (136 cm de sa place). Par la règle
  d'avant, elles visent le point du mur le plus proche et frappent collées au mur (1 cm), entre les autres : l'unité 17
  à 84 cm de l'unité 18 (sur sa place, poussée à 62 cm du mur).
- **Restes.** Le repli colle une unité contre une autre déjà en place. Piste : au repli, ne pas viser le mur nu mais
  frapper depuis où elle est (portée 400 cm, donc 2e rang, comme à la boucle 3), ou prendre une place libre. Puis une
  2e graine, et lancer `HeroesOfLiberty.Combat` (test pur `BuildingPlaces` modifié, pas encore lancé). Case 2 non cochée.

**2026-10-26 — Boucle 5 (sous-agent) : 8/8, Combat 10/10.**

- **Changement.** `HOLCombatProcessor.cpp` : au repli (place oubliée après 4 s sans progrès), si l'unité est déjà à
  portée comme après une marche finie court (`IsBuildingInReach` avec `bWalkEndedShort` vrai, 400 cm), elle s'arrête
  et frappe là où elle est. Sinon seulement, règle d'avant (marche vers le point du mur le plus proche). Portée,
  équilibrage et critères du test : inchangés. Déterministe, rien de nouveau sur le réseau.
- **Résultat** (`building_crowd.ps1 -Count 8 -Seed 1`, compilation 30 s, test 98 s) : **8/8 OK** (avant 7/8).
  1/8 (7 unités, 3,8 s, paire 114 cm, le plus loin 89 cm, mur 726/576) ; 2/8 (9, 4,4 s, 118 cm, 98 cm, 949/768) ;
  3/8 (8, 8,8 s, 114 cm, 119 cm, 762/672) ; 4/8 (9, 9,4 s, 114 cm, 138 cm, 926/768) ; 5/8 (6, 8,3 s, 120 cm, 136 cm,
  544/480) ; 6/8 (8, 8,8 s, 120 cm, 130 cm, 870/672) ; 7/8 (11, 7,2 s, 120 cm, 140 cm, 1140/960) ; 8/8 (10, 10,2 s,
  120 cm, 106 cm, 994/864). Tous frappent partout, aucune paire empilée.
- **Tests d'automatisation** `HeroesOfLiberty.Combat` seuls (ligne de `quick_check`, filtre changé, 34 s) : **10/10**,
  dont `BuildingPlaces` vert.
- **Restes.** Une 2e graine (et plus de répétitions). Puis boucle 3 du plan (`quick_check` des parties touchées).
  Case 2 cochée.
- **Vérifications après la boucle 5** (session principale) : graine 2, 20 Hommes d'armes → **19/20** (échec 8/20 :
  une paire à 87 cm, seuil 90 : deux unités voisines arrêtées à ~55 cm de leurs places, l'une vers l'autre ; la marche
  s'arrête à 50 cm du but, `OrderArrivalTolerance`, et la frappe est permise à 1 rayon = 60 cm de la place) ; graine 3,
  12 Paysans → **12/12**.
- **Boucle 6** (session principale, utilisateur absent) : resserrer la marge de 60 cm ne marche pas (la marche s'arrête
  à 50 cm : presque tous attendraient le repli de 4 s). À la place, quand le groupe est plus petit que le tour du
  bâtiment, les places s'écartent jusqu'à 2,5 rayons (`BuildingPlaceSpreadRadii`, `GetBuildingPlaces`) au lieu de 2 ;
  un groupe qui remplit le tour garde 2 rayons. Puis : compilation, `HeroesOfLiberty.Combat`, graines 1, 2 (Hommes
  d'armes) et 3 (Paysans).
- **Boucle 6, résultat** : compilation 125 s ; `HeroesOfLiberty.Combat` **10/10** (dont `BuildingPlaces`) ; graine 1
  **8/8** (paire la plus proche ≥ 120 cm partout) ; graine 3 Paysans **12/12** ; graine 2 **19/20** : l'ancien échec 8/20
  passe (140 cm), un nouveau 16/20 échoue de justesse (paire à 89,6 cm, seuil 90), rejoué à l'identique (déterministe).
  Détail : les unités 136 et 137 frappent toutes deux sur leur place, 136 poussée contre le mur (1 cm) à 59 cm de sa
  place (marge 60) ; deux unités qui frappent ne s'écartent plus l'une de l'autre (« le front tient »), d'où un
  chevauchement de 30 cm. À suivre : fréquence sur d'autres graines avant de décider.
- **Vérifications** (session principale) : `quick_check -Parts tests,combat` **PASS** en 232 s (tests 177/177, combat
  anims, bodies, arrows, arrows flee, reinforce, aid, orders) ; graine 4 **19/20** (14/20 : paire à 82 cm, l'unité 127 a
  oublié sa place et frappe collée au mur à côté de la 128) ; graine 5 **20/20**. **Bilan Hommes d'armes : 66/68 sur
  4 graines, Paysans 12/12** (avant : 0/8). Les deux échecs : chevauchement léger (82 et 89,6 cm ; 120 = au contact)
  de deux voisines qui frappent, l'une poussée. Le seuil n'est pas relâché ; reste noté. Le corriger toucherait au
  déplacement (deux unités qui frappent ne s'écartent pas, « le front tient ») : décision laissée à l'utilisateur après
  son essai. Plan : cases 2 et 3 faites.

**2026-10-26 — Boucle scénario (sous-agent) : `building_crowd` prêt, répétition hors écran verte.**

- **Fichiers.** `Tools/Scenarios/building_crowd.txt` et `building_crowd.json` (Plain, joueur + IA, comme
  `building_attack`). Positions dans le repère du centre du joueur. Au départ : 10 Paysans (groupe 3), 12 Hommes
  d'armes A (groupe 1) et 12 Hommes d'armes B (groupe 2 = tous moins le groupe 1, par `hol.Select.Group Remove`),
  trois Maisons ennemies à ~24 m des groupes (hors des 16 m d'agressivité : rien ne part tout seul).
- **Temps 1** : A face au côté ouest de la Maison 1 ; Entrée = clic droit (`hol.Select.OrderNear`, le chemin du clic
  droit). **Temps 2** : B en biais (45°) face au coin de la Maison 2, Entrée = clic droit. **Temps 3** : les 10 Paysans
  sur la Maison 3, devant le centre. **Temps 4** : une Maison neuve à ~25-30 m des deux groupes, le groupe 1
  sélectionné : l'utilisateur fait lui-même le clic droit (touches 1 et 2 pour les groupes).
- **Maison debout.** Aucun moyen sans code : pas de commande de PV ni d'invulnérabilité (`hol.Cheat.*` : seulement
  dégâts, pose, vitesse de marche `UnitSpeed`), pas de vitesse du jeu ; `hol.Combat.Enabled 0` couperait toute
  l'attaque. D'où une Maison neuve par temps ; la note du temps 1 dit qu'elle tombe ~5 s après les premiers coups.
- **Commande pour l'utilisateur :**
  `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 building_crowd -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
- **Répétition hors écran** (`-RenderOffScreen -ResX=1600 -ResY=900 -nosound -LiveCoding=0 -saveddirsuffix=Tests`,
  `-ExecCmds="hol.Scenario.AutoContinue 1, hol.Scenario building_crowd"` ; journal
  `Saved_Tests\Logs\building_crowd_scenario.log`). 1er passage : temps 2 en rouge (« hol.Select.OrderNear: the point is
  not on the screen » : la caméra n'était pas sur la Maison) ; corrigé par un `hol.Camera.FocusNear` sur la Maison
  juste avant chaque clic. 2e passage : **vert**, `Scenario building_crowd: ready` en 44 s de jeu. Sélections 10 / 12 /
  24, groupes 3, 1, 2 (12 après retrait) ; 3 Maisons posées ; `Attack order of slot 0 for 12 units on building House
  of slot 1` (temps 1 et 2), `... for 10 units ...` (temps 3) ; Maisons détruites 8,7 s, 10,5 s et 17,1 s après
  l'ordre ; Maison du temps 4 posée, aucune attaque d'elle-même ensuite. La répartition elle-même n'est pas mesurée
  par le scénario (c'est le rôle de `building_crowd.ps1`) : à l'œil de l'utilisateur.

**2026-10-26 — Boucle 7 (sous-agent) : test exigeant écrit ; pas compilé (Visual Studio introuvable).**

- **Diagnostic du point 2 (lecture du code).** Le combat vise la **boîte du corps** : `Step.Box = Building->GetBodyBounds()`
  (`HOLCombatProcessor.cpp`, l. 806), les tuiles moins `BodyInset` = 115 cm de chaque côté (`DefaultGame.ini`). Maison :
  tuiles 800 cm, **corps 570 cm**. Le mur visible (socle, forme) : tuiles moins `VisualInset` = 30 cm, **740 cm**. Écart :
  **85 cm** de chaque côté (`VisualGap`). Le test mesurait aussi depuis le corps (`GetBodyBounds`) : « 1 cm from the wall »
  = 1 cm du corps = **84 cm dans le mur visible**.
- **Les places.** L'anneau des places est à rayon + 40 cm du corps (`BuildingApproachGap`) : milieu à 100 cm du corps,
  bord du corps à 40 cm du corps, donc **45 cm dans le mur visible**. Toutes les places de la boucle 2 sont dans la Maison.
- **D324 « une épaule ».** Oui, elle laisse entrer : une unité qui attaque est « au travail » (`StandsClearOfVisibleWall`
  faux dès qu'elle a une cible), la garde ne la tient qu'à rayon + 1 = 61 cm du corps (milieu 24 cm dans le mur visible,
  bord du corps **84 cm dedans**). Seule une unité arrêtée sort du mur visible (C.1). C'est ce que montrent les captures :
  cercles centrés sur le bord du socle.
- **Nouveaux critères** (`Dev/HOLBuildingCrowdTest.cpp`, les trois anciens gardés tels quels) :
  (4) distance **signée** du bord de chaque corps au mur visible (corps + 85 cm) ≥ **−10 cm** (`VisibleWallSlack`) :
  la garde laisse 1 cm, une foule pousse de quelques cm ; 10 cm = un sixième de rayon, invisible à la caméra ; le
  défaut vu est de −45 à −84 cm. Pire valeur loguée à chaque répétition.
  (5) **tour régulier** : le 1er rang (autant d'unités que le tour en tient au contact, les plus près du mur d'abord)
  projeté sur le tour = mur visible élargi d'un rayon, quarts de cercle aux coins (Maison, Hommes d'armes :
  4 × 740 + 2π × 60 = **3337 cm**, 27 places au contact). Part égale = tour / unités. Plus grand trou ≤ **1,5 ×** la
  part (pas de côté vide, pas de coin vide) **et** plus petit écart ≥ **0,5 ×** la part (pas de 4 unités serrées). Les
  deux bornes, parce qu'un trou seul ne voit pas un paquet quand le groupe est petit. Mesure le long du tour arrondi, pas
  du mur nu : un coin compte pour son quart de cercle (94 cm), pas pour un point.
  Lignes par unité des échecs : distance signée au mur visible et position sur le tour en plus.
- **Résultat : pas de rouge mesuré.** `Tools\build.ps1` échoue en 5 s : « No valid Visual C++ toolchain was found »
  (UnrealBuildTool). Cause hors du projet : plus aucune installation de Visual Studio n'est vue (`vswhere` vide, dossier
  `C:\ProgramData\Microsoft\VisualStudio\Packages` absent), juste après un téléchargement de mise à jour de Visual
  Studio en arrière-plan (journaux `%TEMP%\dd_BackgroundDownload_20261026150635*`, 15 h 06-15 h 08). À 13 h 59, la même
  compilation trouvait `CommunityA` 14.44.35222. Les fichiers de MSVC sont toujours sur le disque. Rien touché à la
  machine. **À faire avant la boucle 8** : réparer ou réenregistrer Visual Studio (Visual Studio Installer, « Réparer »),
  puis compiler et lancer `building_crowd.ps1 -Count 8 -Seed 1`.
- **Rouge attendu** (calcul, pas mesuré) : (4) rouge sur **8/8** (toute unité sur sa place est à −45 cm, celles poussées
  contre le corps à −84) ; (5) rouge là où un côté reste vide (cas de l'essai), à mesurer.
- **Recommandation pour la boucle 8.** Places depuis la **boîte visible** (corps + 85 cm), plus depuis le corps : anneau
  à **rayon + 10 cm** du mur visible (milieu à 155 cm du corps ; portée 400 cm et zone interdite du navmesh à 85 cm : bon).
  Places **régulières sur tout le tour** : pas = longueur du tour / unités (au moins 2 rayons), coins compris (quarts de
  cercle), la 1re en face du milieu du groupe ; chaque unité prend la plus proche dans l'ordre du tour (sans croisement).
  Au-delà de 27 (Hommes d'armes), 2e anneau 2 rayons plus loin. Place atteinte : garder 1 rayon, mais **une unité qui
  frappe un bâtiment depuis sa place doit rester hors du mur visible** (étendre la sortie douce de C.1 aux unités qui
  frappent un bâtiment, ou ne pas coller au corps quand elle a une place).

**2026-10-26 — Boucle 8 (sous-agent) : correctif écrit, non compilé (Visual Studio en réparation).**

- **Places depuis le mur visible.** Nouveau `AHOLBuilding::GetVisibleBodyBounds()` (`Buildings/HOLBuilding.h/.cpp`) :
  la boîte du corps élargie de `BodyInset - VisualInset` (85 cm sur une Maison), la même source que la garde du mur
  visible (`KeepOutOfVisibleWall`) et que le test de charge. `FStepBuilding::VisibleBox` (`HOLCombatProcessor.h`), calculée
  une fois par pas à côté de `Box`. `ApplyCombatOrder` (`HOLUnitOrderProcessor.cpp`) donne les places autour de la boîte
  visible, écart `HOLCombat::BuildingPlaceVisibleGap` = **10 cm** (`HOLCombatRules.h`). Milieu de l'unité à 155 cm du
  corps, bord à 95 cm.
- **Portée.** Homme d'armes et Paysan : portée de bâtiment 400 cm (`UnitTypes.csv` ; toutes les unités de mêlée aussi).
  La règle « à portée près de sa place » (`Edge <= BuildingAttackRange`, au corps) reste vraie : 95 cm au 1er anneau,
  215 cm au 2e, 335 cm au 3e (+ 60 cm de marge de place). Aucune unité n'a une portée plus courte : rien de spécial.
- **Tour complet régulier** (`HOLCombat::GetBuildingPlaces`, `HOLCombatRules.cpp`). L'anneau a de nouveau ses quarts de
  cercle aux coins (enlevés à la boucle 4) : une place peut tomber dans un coin. Écart = longueur du tour / unités, pour
  tout groupe (plus de plafond à 2,5 rayons : `BuildingPlaceSpreadRadii` supprimé). La 1re place en face du milieu du
  groupe. Au plus ce que le tour tient à 2 rayons ; en plus, **une place de moins tant que deux places sont à moins de
  2 rayons en ligne droite** (autour d'un coin, la corde est plus courte que le chemin sur l'anneau : à 2 rayons sur
  l'anneau, 107 cm de corde). Le reste va au 2e anneau, 2 rayons plus loin, même règle. Maison, Hommes d'armes : tour de
  3400 cm, 28 places à 2 rayons, **24** après la règle des cordes (calcul en Python, pas en jeu).
- **Affectation.** Sur chaque anneau : unités triées par leur projection sur l'anneau (identifiant en cas d'égalité),
  places dans le même ordre ; on essaie les n décalages circulaires et on garde la plus petite somme des distances
  (en ligne droite) unité → place, le plus petit décalage en cas d'égalité. O(n²) par anneau, à l'ordre seulement.
  La projection sur un coin suit maintenant l'angle autour du coin (avant : le coin entier projeté sur un point).
- **Repli** (place oubliée après 4 s). Nouveau `FHOLCombatFragment::PlaceGivenUpBuilding` (effacé par `Disengage` et à un
  nouvel ordre). Une unité qui a oublié sa place pour ce bâtiment marche vers le point du **mur visible** le plus proche,
  plus rayon + 10 cm (`GetBuildingApproachPoint` sur `Step.VisibleBox`). « Frappe là où elle est » (boucle 5) : inchangé.
  Les unités sans place (attaque-déplacement, recherche seule, Shift) : inchangées (corps + rayon + 40 cm).
- **Frapper sans entrer dans le mur visible.** Choix : étendre la sortie douce de C.1 (`HOLSimulationProcessors.cpp`,
  garde des bâtiments) à une unité qui **frappe** (`bAttacking`) un bâtiment (`TargetBuilding`) dont elle a, ou avait,
  la place (`PlaceBuilding` ou `PlaceGivenUpBuilding` = ce bâtiment). Même pas de 10 cm, même refus entre deux murs
  (C.3), même place de formation poussée avec elle. Raisons : c'est le chemin déjà testé (C.1-C.3) ; il ne touche que ces
  unités ; une unité qui frappe a une portée de 400 cm, la sortir à 146 cm du corps ne lui fait pas perdre sa cible. Les
  marcheurs, les ouvriers et les autres combattants gardent l'épaule de D324. J'ai inclus les unités qui ont oublié leur
  place : le critère (4) vaut pour toutes, et « frappe là où elle est » peut la laisser collée au corps.
- **Test pur** `HeroesOfLiberty.Combat.BuildingPlaces` (`Tests/HOLCombatTests.cpp`), réécrit sur une boîte visible de
  400 cm, rayon 60, écart 10 : seul (point le plus proche, hors du mur visible) ; **groupe de 4** (les 4 places à un quart
  de tour l'une de l'autre = les milieux des 4 côtés, dans l'ordre des unités) ; groupe en biais de 8 (tous sur l'anneau,
  chaque côté pris, une place dans un coin, écarts égaux le long du tour, 2 rayons) ; ordre de la liste inversé ; 5 unités
  au même point ; 25 unités (13 à 16 sur le 1er anneau, le reste sur le 2e, écarts égaux sur chacun, 2 rayons partout).
  Vérifié par une copie Python de l'algorithme : seul (470, 200) ; 4 → les 4 milieux ; 8 → dont 4 coins ; 25 → 14 + 11,
  paire la plus proche 120 cm. Valeurs d'équilibrage et portée de 400 cm : inchangées. Test de charge : pas touché.
- **À vérifier à la compilation.** (a) `FMath::Atan2(double, double)` et `UE_DOUBLE_PI` (présents dans le moteur
  5.8 AS 3) ; (b) `TWeakObjectPtr == TWeakObjectPtr` dans la garde (`Combat->PlaceBuilding == Combat->TargetBuilding`) ;
  (c) `Combat.PlaceGivenUpBuilding == Step.Building` (pointeur faible contre brut, comme `PlaceBuilding` déjà) ;
  (d) `GetVisibleBodyBounds` utilise `UHOLBuildingSettings` (déjà inclus par `HOLBuilding.cpp`).
- **À surveiller au test.** (a) La place à 155 cm du corps : la zone interdite du navmesh (corps + `BodyClearance`,
  érodée du rayon) doit rester en deçà, sinon la marche finit court partout (repli à 4 s). (b) Un groupe de 4 envoie
  une unité de l'autre côté de la Maison (voulu) : marche plus longue, frappe plus tard ; le critère « tous frappent »
  a 25 s. (c) Des unités poussées par la sortie douce pendant qu'elles frappent, contre leurs voisines (« le front
  tient ») : regarder la paire la plus proche. (d) Maison collée à une autre : une place peut tomber dans l'autre
  bâtiment (risque déjà noté). (e) Lancer `HeroesOfLiberty.Combat`, puis `building_crowd.ps1 -Count 8 -Seed 1` (critères
  4 et 5 attendus verts), puis graines 2 et 3, puis `quick_check -Parts tests,combat` (la garde des bâtiments a changé).

- **Rouge de la boucle 7, mesuré** (session principale, après la réparation de Visual Studio ; état d'avant la boucle 8
  sauvé dans `refs/building-crowd/loop7`, boucle 8 dans `refs/building-crowd/loop8`) : compilation 112 s ;
  `building_crowd.ps1 -Count 8 -Seed 1` **rouge 8/8** sur les deux nouveaux critères : 4 à 7 corps dans le mur visible
  (pire −34 à −73 cm) et trous du tour de 10,8 à 23,7 m (part égale 3 à 5,6 m). Les 3 anciens critères passaient.

**2026-10-26 — Boucle 9 (sous-agent) : coins du socle ; 8/8 et Paysans 12/12.**

- **Cause.** La garde voyait bien ces unités (`bStrikesByPlace` vrai). Mais elle mesure le mur visible depuis le
  **corps**, à 85 + rayon + 1 cm, donc avec un **coin arrondi**. Le socle dessiné (et le test) a un **coin carré**
  (corps élargi de 85 cm). Dans un coin, une unité à 146 cm du coin du corps est « dehors » pour la garde, mais jusqu'à
  ~35 cm dans le socle. Preuve par les lignes du test : sur un côté, les deux distances diffèrent de 85 cm (unité 2 :
  86 cm du corps, 1 cm du mur visible) ; pour les unités 6, 11 et 51, l'écart est de 113 à 120 cm : elles sont dans un
  coin (unité 6 : 92 cm du corps, −21 cm du socle).
- **Changement.** `HOLConstruction::EaseOutOfVisibleWalls` (`Buildings/HOLConstructionRules.h/.cpp`) : nouveau paramètre
  `bSquareCorners` (faux par défaut). Vrai : distance mesurée depuis la **boîte visible carrée** (corps + 85 cm), sortie
  à rayon + 1 cm ; centre dans le socle mais hors du corps : sortie par le côté du socle le plus proche. Sur un côté :
  le même pas qu'avant. La garde (`HOLSimulationProcessors.cpp`, `PushOut`) le passe **seulement** pour `bStrikesByPlace`
  (et sa place de formation). Les unités arrêtées (C.1), les ouvriers, le ramassage : inchangés. Portée, équilibrage,
  tolérance d'arrivée, critères du test : inchangés.
- **Compilation** 157 s, verte.
- **Résultats.** `building_crowd.ps1 -Count 8 -Seed 1` (90 s) : **8/8 OK** (avant 5/8). Pire distance au mur visible :
  1, 1, 1, 1, 1, 5, 1, 1 cm (avant −21, −14, …, −16). Trous du tour (min-max / part égale, cm) : 411-537/477,
  315-448/371, 395-474/417, 317-425/371, 519-664/556, 344-495/417, 258-352/303, 271-415/334. Paire la plus proche :
  366, 312, 405, 316, 416, 347, 253, 271 cm. Tous frappent en 4,4 à 7,5 s.
  `-Count 12 -Seed 3 -Type Peasant` (123 s) : **12/12 OK**, pire 1 à 7 cm du mur visible, paire la plus proche ≥ 221 cm.
- **Restes.** (a) Les unités arrêtées (C.1) gardent le coin arrondi : jusqu'à ~35 cm dans le coin du socle. Même
  correctif possible (passer `bSquareCorners` pour `bStill`), mais il touche C.1-C.3 et leurs tests : pas fait. (b) Pas
  lancé : `HeroesOfLiberty.Combat`, `HeroesOfLiberty.Construction` (la fonction a changé, chemin par défaut identique),
  graine 2, `quick_check -Parts tests,combat`.

- **Vérifications après la boucle 9** (session principale) : `quick_check -Parts tests,combat` **PASS** (221 s, tests
  177/177) ; graine 2 **19/20**, graine 4 **19/20** : chaque échec = critère (5), une unité a oublié sa place (à 6-7 m
  le long du mur, derrière des unités qui frappent déjà) et frappe d'où elle est : un trou, un paquet. Boucle 10 lancée.

- **Boucle 10** (sous-agent arrêté au minuteur de 15 min sans écrire ici ; reconstitué par la session principale depuis
  le code et le log). **Changement** : une unité bloquée 4 s sur le chemin de sa place, par des alliées déjà sur les
  leurs le long du tour, prend la place de la plus proche d'entre elles ; chacune de ces alliées avance d'une place vers
  la sienne (`HOLCombat::GetBuildingPlaceChain`, fonction pure, `HOLCombatRules.cpp` ; `UHOLCombatProcessor::
  ShiftBuildingPlaces`, `HOLCombatProcessor.cpp`, O(unités) une fois, au blocage seulement ; `bPlaceShifted` dans
  `FHOLCombatFragment`, remis à zéro à l'ordre et par `Disengage`). Une seule fois par unité : bloquée encore, elle
  oublie sa place comme avant. Compilé à 16 h 15. **Résultat** : graine 2 **20/20** (avant 19/20), paire la plus proche
  moyenne 318 cm. Pas de test pur pour `GetBuildingPlaceChain` (à ajouter). **Bilan de méthode** : le sous-agent a
  lancé son test en tâche de fond puis attendu ; consigne à répéter : test en avant-plan, journal écrit avant le test.

- **Vérifications après la boucle 10** (session principale) : `quick_check -Parts tests,combat` **PASS** (219 s, tests
  177/177) ; `building_crowd.ps1` : graine 2 **20/20**, graine 4 **20/20**, graine 1 **8/8**, Paysans graine 3
  **12/12** : **60/60**, les 5 critères (frappent, pas d'empilement, mur couvert, hors du mur visible, tour régulier).
  Prêt pour le 2e essai de l'utilisateur (même scénario `building_crowd`).

**2026-10-26 — Question de l'utilisateur avant le 2e essai** : « pourquoi les troupes sont aussi distantes l'une de
l'autre ? » (tour de 33 m réparti : ~4 m d'écart pour 8 Hommes d'armes). **Décision de l'utilisateur : au contact** :
places à 2 rayons, en un arc continu qui part en face du groupe et passe les coins sans trou ; un grand groupe fait
tout le tour, puis 2e rang. Remplace « tout le tour régulier ». Boucle 11a : critère (5) adapté, rouge ; 11b : correctif.

**2026-10-26 — Boucle 11a (sous-agent) : critère (5) « arc continu au contact ».**

- **Ce qui change.** Seul le critère (5) de `Dev/HOLBuildingCrowdTest.cpp` ; les critères (1) à (4) et le code du jeu :
  inchangés. Mêmes outils qu'avant : 1er rang (autant d'unités que le tour en tient à 2 rayons, les plus près du mur
  d'abord), positions le long du tour arrondi (mur visible élargi d'un rayon, quarts de cercle aux coins), triées.
- **Nouveau (5).** On prend les écarts entre voisines le long du tour, celui qui ferme le tour compris. Le plus grand =
  le **vide** hors de l'arc. Tous les autres écarts : entre **0,75 × 2 rayons** et **1,5 × 2 rayons** (Homme d'armes :
  90 à 180 cm). Et l'**arc** (tour moins le vide) ≤ **1,3 × (unités − 1) × 2 rayons + 2 rayons** (8 Hommes d'armes :
  1212 cm ; au contact parfait : 840 cm). Si le 1er rang remplit le tour : pas de vide, tous les écarts dans les bornes
  (le plus grand peut garder en plus le reste du tour, moins de 2 rayons). Ce dernier cas ne sort pas dans ce test
  (6 à 12 unités, le tour en tient 27).
- **Pourquoi ces bornes.** Haut 180 cm : une unité peut s'arrêter jusqu'à 50 cm de sa place (`OrderArrivalTolerance`
  par défaut ; 5 cm dans `DefaultGame.ini`) et la foule pousse de ~10 cm : 120 + 60. Une place vide fait 240 cm : rouge.
  Bas 90 cm : c'est la borne de l'empilement du critère (2) (1,5 rayon) ; des corps peuvent se presser un peu, pas
  se monter dessus. Arc 1,3 : écart moyen jusqu'à 156 cm, soit le pire de chaque unité réparti ; + 2 rayons : chaque bout
  de l'arc peut être à un rayon de sa place. Un groupe étalé sur le tour (code actuel) dépasse de très loin.
- **Log par répétition** : écarts mini/maxi dans l'arc, longueur de l'arc et son maximum, vide.
- **Compilation** 37 s, verte.
- **Résultat** : `building_crowd.ps1 -Count 8 -Seed 1` (89 s) **rouge 8/8**, seulement sur le nouveau (5) ; les
  critères (1) à (4) passent partout (tous frappent en 4,2 à 7,5 s, 0 paire empilée, pire 1 à 5 cm du mur visible).
  Par répétition (unités : écarts dans l'arc, arc / maximum, vide, en cm) :
  1. 7 : 436-489, arc 2767 / 1056, vide 570 ;
  2. 9 : 327-391, arc 2901 / 1368, vide 436 ;
  3. 8 : 395-428, arc 2863 / 1212, vide 474 ;
  4. 9 : 317-407, arc 2911 / 1368, vide 425 ;
  5. 6 : 519-543, arc 2673 / 900, vide 664 ;
  6. 8 : 344-458, arc 2842 / 1212, vide 495 ;
  7. 11 : 257-333, arc 3000 / 1680, vide 337 ;
  8. 10 : 281-387, arc 2950 / 1524, vide 387.
  Le critère voit bien le défaut : les écarts sont 2 à 4,5 fois le maximum de 180 cm, l'arc 1,8 à 3 fois son maximum.
  Aucun écart sous 90 cm. Prêt pour la boucle 11b (correctif : places à 2 rayons en arc depuis la face du groupe).

**2026-10-26 — Boucle 11b (sous-agent) : places au contact, en arc.**

- **Changement.** `HOLCombat::GetBuildingPlaces` (`Simulation/HOLCombatRules.cpp/.h`). Place du 1er anneau (« room ») :
  comme avant (autant de places régulières que le tour en tient, jamais deux à moins de 2 rayons en ligne droite).
  Groupe plus petit : **arc continu** de places à 2 rayons le long du tour, centré sur la projection du milieu du groupe
  (une place là pour un nombre impair, deux de part et d'autre pour un pair), puis une de chaque côté à tour de rôle.
  Au coin, la place avance par pas de 1 cm jusqu'à être à 2 rayons de la précédente en ligne droite : pas de trou.
  Unités triées par projection (identifiant en cas d'égalité), places triées le long de l'arc : la n-ième prend la
  n-ième, sans croisement, sans décalage. Groupe aussi grand que la place ou plus : 1er anneau plein, régulier
  (tour / places), décalage circulaire gardé ; le reste en arc sur le 2e anneau. Repli : si les deux bouts de l'arc se
  rejoignent, répartition régulière. Une fois à l'ordre, O(n²) par anneau. `GetBuildingPlaceChain` : inchangé.
- **Test pur** `HeroesOfLiberty.Combat.BuildingPlaces` : groupe de 4 → x = 20, 140, 260, 380 devant le bas, dans l'ordre ;
  biais vers le coin haut-droit (8) → 4 à droite + 4 en haut, au contact par le coin ; 5 au même point → arc au contact ;
  25 → 1er anneau plein régulier (14, copie Python) + 11 en arc au contact sur le 2e ; nouveau cas
  `GetBuildingPlaceChain` (3 places alliées entre l'unité et la sienne, une sur le 2e anneau, une derrière : la chaîne
  donne les 3, la plus proche d'abord). Portée, équilibrage, tolérance d'arrivée, test de charge, garde : inchangés.
- **Compilation** 78 s, verte.
- **Résultats.** `HeroesOfLiberty.Combat` (113 s) : **10/10**, dont `BuildingPlaces` vert (cas du chaînage compris).
  `building_crowd.ps1 -Count 8 -Seed 1` (100 s) : **rouge 8/8**. Critères (1), (2), (4) verts partout : tous frappent en
  3,8 à 12,8 s, 0 paire empilée, paire la plus proche 107 à 120 cm, pire 1 à 23 cm du mur visible. Arc : 565 à
  1150 cm, toujours sous son maximum (avant : 2673 à 3000). Échecs par répétition (unités : écarts dans l'arc ; mur
  couvert / besoin, cm) :
  1. 7 : 80-164 ; 428/576 ;
  2. 9 : 90-199 ; 660/768 ;
  3. 8 : 73-145 ; 583/672 ;
  4. 9 : 61-176 ; 678/768 ;
  5. 6 : 78-134 ; 361/480 ;
  6. 8 : 57-145 ; 518/672 ;
  7. 11 : 77-135 ; 885/960 ;
  8. 10 : 103-136 ; 779/864.
- **Lecture.** (a) **Critère (3) rouge 8/8** : il mesure le mur couvert sur le **corps** (2280 cm de tour). Un arc au
  contact qui passe un coin met 2 à 3 unités en face du coin : sur le corps, elles tombent sur un seul point (le coin
  du tour compte ~85 + 85 cm de mur visible et 110 cm de quart de cercle). L'ancien critère (3) contredit donc la
  décision « arc au contact qui passe les coins » : à adapter (boucle de test, comme 11a), pas le code. (b) **Critère (5)
  rouge 7/8 par l'écart du bas** (57 à 80 cm, seuil 90) et 1/8 par un trou (199 cm, maximum 180). La paire la plus
  proche reste ≥ 107 cm en ligne droite : ce sont des unités **décalées en profondeur** (une à 1 cm du mur visible,
  sa voisine à 60-106 cm), donc proches le long du tour. Cause : la frappe est permise à 1 rayon de sa place et la
  marche finit jusqu'à ~50 cm avant ; quelques unités (46, 47, 52) ont oublié leur place, bloquées par leurs voisines
  déjà au contact. Le rang n'est pas droit.
- **Restes.** Décider (3) (le mesurer sur le tour arrondi, ou le retirer : (5) le remplace). Pour (5) : rang plus droit
  (place atteinte plus près que 1 rayon, ou unité qui frappe ramenée sur sa place), ou bornes de (5) mesurées en ligne
  droite. Pas lancé : graine 2 (`-Count 20 -Seed 2`), faute de temps.

- **Décisions de la session principale après 11b** (utilisateur présent mais sans question nouvelle ; à lui signaler) :
  (a) le critère (3) « mur couvert » reste (premier détecteur du tassement) mais se mesure **le long du tour visible**,
  comme (5) : sur le corps, 2-3 unités en face d'un coin comptent pour un point, ce qui contredit « passer les coins » ;
  (b) `OrderArrivalTolerance` vaut 5 cm dans `Config/DefaultGame.ini` (D290), pas 50 : la place est donc « atteinte » à
  ~15 cm (`BuildingPlaceReachRadii` 1 → 0,25), pour un rang droit (fini le zigzag en profondeur) ; (c) places de l'arc
  à 2,2 rayons (10 % d'air) au lieu de 2, pour qu'une unité se glisse entre deux voisines qui frappent déjà. Boucle 12.

**2026-10-26 — Boucle 12 (sous-agent) : (3) sur le tour, place atteinte à 15 cm, arc à 2,2 rayons.**

- **Test de charge, critère (3)** (`Dev/HOLBuildingCrowdTest.cpp`) : même exigence (80 % de 2 rayons par unité,
  plafonné au tour), mais mesurée **le long du tour visible** (`RoundCoordinate` / `RoundLength`, ceux de (5)), toutes
  les unités du groupe. Le log « spread X of Y » donne maintenant le tour (Y) au lieu du périmètre du corps. Code mort
  retiré (`PerimeterCoordinate`, `Perimeter`). Rien d'autre ne change ; aucun seuil relâché.
- **Place atteinte plus près** : `BuildingPlaceReachRadii` 1,0 → **0,25** (15 cm pour un rayon de 60). La marche vise
  la place (`IssueCombatWalk` vers `BuildingPlace`) et s'arrête à `OrderArrivalTolerance` = 5 cm (`DefaultGame.ini` ;
  `AdvanceUnits`) : une marche finie normalement est dans les 15 cm. Seule exception possible : une marche coupée
  court (bloquée, `FinalLegStuckSteps`) ; alors le repli à 4 s (chaîne de la boucle 10, puis oubli) reste en place.
- **Arc à 2,2 rayons** : nouvelle constante `HOLCombat::BuildingPlaceArcRadii = 2.2` (`HOLCombatRules.h`) ;
  `GetBuildingPlaces` l'utilise pour l'arc (pas, demi-pas du nombre pair, pas au coin, repli si les bouts se
  rejoignent). Place d'un anneau, anneau plein et écart entre anneaux : 2 rayons, comme avant.
- **Test pur** `BuildingPlaces` ajusté (calcul à la main, pas = 132 cm) : groupe de 4 → x = 2, 134, 266, 398 ; biais
  vers le coin haut-droit : **6 unités** symétriques (milieu 980, 980) au lieu de 8 → 3 à droite, 3 en haut (places à
  79, 211, 343 cm du milieu du coin le long du tour, corde au coin 133 cm ; à 8, la 4e passerait le coin suivant) ;
  « au contact » = voisines à 132 cm (−0,5/+2) ; écart mini des arcs ≥ 131,5. Anneau plein : inchangé.
- **Compilation** 500 s (file d'attente de `build.ps1`), verte.
- **Tests : pas lancés.** La compilation a pris 8 min ; à 13 min de la boucle, la limite de 15 min ne laissait pas la
  place. À lancer tels quels (pas de recompilation) : `HeroesOfLiberty.Combat`, puis `building_crowd.ps1 -Count 8
  -Seed 1`, puis `-Count 20 -Seed 2`.

- **Boucle 12, tests** (lancés par la session principale : la compilation du sous-agent a attendu 500 s dans la file) :
  `HeroesOfLiberty.Combat` **10/10** ; graine 1 **8/8**, graine 2 **20/20** : écarts entre voisines 120 à 153 cm (contact
  120), arc continu, pire distance au mur visible 1 à 10 cm (dehors), tous frappent en 3,1 à 7,7 s.

- **Vérifications après la boucle 12** (session principale) : `quick_check -Parts tests,combat` **PASS** (221 s, tests
  177/177) ; graine 4 **20/20**, Paysans graine 3 **12/12** (paire la plus proche moyenne 126-127 cm). Total **60/60**
  au contact. Prêt pour le 2e essai de l'utilisateur.

### Reprise exacte (J)

État : tout dans `HeroesOfLiberty-Elrond`, branche `etape-building-crowd` = `main` 24c7ce5 + changements **non
commités** (13 fichiers C++ modifiés, nouveaux : `Dev/HOLBuildingCrowdTest.cpp`, `Tools/Tests/building_crowd.ps1`,
`Tools/Scenarios/building_crowd.txt/.json`, et ce fichier) ; `IN_USE.txt` posé ; compilé (boucle 10). Photos :
`refs/building-crowd/loop7` (rouge), `refs/building-crowd/loop8`.
1. [x] 1er essai de l'utilisateur : coins vides, corps dans la Maison → boucles 7 à 10 ; puis « trop distants » → au contact, boucles 11a à 12.
2. [x] Rouge (8/8 en échec, deux fois : 7 et 11a) puis vert : 60/60 au contact sur 4 graines, Combat vert,
   `quick_check -Parts tests,combat` PASS.
3. [x] 2e essai de l'utilisateur, validé le 2026-10-26 (« très bien ») :
   `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 building_crowd -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
4. [ ] Questions encore ouvertes : (a) portée de frappe sur bâtiment 400 cm (équilibrage, non touchée) ; (b) attaque-
   déplacement, recherche automatique, file Shift : ancienne règle (point le plus proche) ; (c) unités arrêtées
   (C.1) : coins arrondis de la garde, jusqu'à ~35 cm dans le coin du socle.
5. [ ] Restes avant commit : test pur pour `GetBuildingPlaceChain` ; après accord : avancer sur `main` à jour (section K
   d'`EtapeDefiniInProgress.md` déjà dans `main` : fusionner les deux), relancer `-Parts tests,combat,build` et
   `building_crowd.ps1 -Count 8`, commit en anglais (sans Co-Authored-By), retirer la ligne « Unités qui s'empilent… »
   d'`EtapeDefini.md` dans le même commit, fusionner dans `main`, effacer les refs `refs/building-crowd/*`, libérer
   Elrond (`IN_USE.txt`), clore J ici.

**Clos le 2026-10-26** : 2e essai validé par l'utilisateur, commité, fusionné dans `main`. Les trois questions ouvertes
(portée de 400 cm, attaques sans ordre direct, coins arrondis des unités arrêtées) sont devenues des lignes
d'`EtapeDefini.md` (« Aléas et petits restes »).

## K — L'abandon en réseau (`ServerSurrender`) contrôlé par `quick_check`

Ligne d'`EtapeDefini.md` (« Aléas et petits restes ») : « `quick_check` : l'abandon en réseau (`ServerSurrender`) n'est
plus joué par la partie `net` (remplacé par la mort du Roi en V5.5) ; reste vérifié en solo ». Prise le 2026-10-26
(l'utilisateur bientôt absent ; moteur Hazelight en compilation : rien en C++). Arwen, branche `net-surrender-check` =
`main` 24c7ce5, `IN_USE.txt` posé.

- **Constat** : la partie `lobby` de `quick_check` (`lobby_match.ps1`, R5.4) joue déjà l'abandon en réseau : Bob fait
  `hol.Victory.Surrender`, donc `RequestSurrender` → RPC `ServerSurrender` vers le serveur dédié (cuit) ; elle ne
  contrôlait que les bandeaux (Alice Victory, Bob Defeat), pas la raison de la défaite côté serveur. Pas besoin d'une
  seconde partie à 2 clients dans `net`.
- **Correctif** (scripts seuls) : `lobby_match.ps1`, après « Bob holds slot 1 », trois contrôles du journal du serveur :
  aucun « Victory: surrender of slot … refused », « Victory: slot 1 defeated (Surrender) », « Victory: match over,
  winners 0 ». `quick_check.ps1` : les deux commentaires de la fin de la partie `net` qui parlaient encore de l'abandon
  de client 1 renvoient à la partie `lobby`.
- **Rouge** rejoué hors jeu sur le journal d'une vraie partie `lobby` (Glorfindel, `M20261026-094645-1\server.log`) :
  sans la ligne d'abandon → FAIL « no 'Victory: slot 1 defeated (Surrender)' » ; avec un refus ajouté → FAIL « refused » ;
  sans la fin de partie → FAIL « no 'match over, winners 0' » ; journal tel quel → PASS.
- Vert en vrai : compilation d'Arwen (`main` a L13b en plus de son ancien état), `build_manager.ps1`, puis
  `quick_check -Parts lobby` (lancé en fond).
- 1er passage : compilation **FAIL** (216 s, erreurs C2440 dans les `.gen.cpp`) : `C:/UE/UE_5.8_AS_3` est déjà le moteur
  Hazelight 5.8.4 depuis le 2026-10-26 10 h 37 (D483) et l'`Intermediate` d'Arwen venait de l'ancien. 2e passage : Arwen
  recompilée de zéro (`Intermediate` et `Binaries` effacés), puis la partie `lobby` avec le serveur cuit d'Arwen du
  2026-10-25 (ancien moteur ; la cible Server du nouveau se compile dans `C:/UE/Src/HZ`). Si les clients 5.8.4 refusent
  ce serveur : attendre la cible Server, `net_match.ps1 -Stage`, relancer.
- 2e passage : compilation de zéro PASS (365 s), gestionnaire PASS ; le vieux serveur cuit accepte les clients 5.8.4.
  `-Parts lobby` **FAIL** (468 s) « Alice : no lobby end banner », mais pas à cause de l'abandon : le journal du serveur
  a bien « Victory: slot 1 defeated (Surrender) » puis « match over, winners 0 » (nos contrôles passeraient). **Cause** :
  Alice attend son bandeau par un `waitlog` (60 s au plus, fixé dans `HOLTestSequence.cpp`) dès sa capture en partie,
  alors que Bob n'abandonne qu'après l'arrivée de Carol (`waitfile … 180`) ; machine chargée (compilation du moteur et
  d'un autre projet), Carol est arrivée tard : Alice a expiré à 11:34:19 (UTC), Bob a abandonné à 11:34:34. Défaut du
  test, antérieur à K. **Correctif** (script seul) : Bob fait `signal bob_gives_up.signal` juste avant
  `hol.Victory.Surrender`, Alice fait `waitfile … 240` avant son `waitlog End banner: lobby match` (un `waitfile` ne
  déplace pas le point de départ de la recherche des `waitlog` : le bandeau est trouvé même arrivé pendant l'attente).
  3e passage : **FAIL avant toute partie** (« game launch ended by itself », « no lobby created by Alice ») : au
  démarrage, le jeu lance UnrealBuildTool (`-Mode=QueryTargets` / `ValidatePlatforms`), bloqué derrière les compilations
  de ProjectShiva (même moteur, 12 h 24 et 12 h 29) et d'Elrond ; processeur à 100 %. Environnement, pas le test.
  4e passage : attente qu'aucun UnrealBuildTool ne tourne (45 min au plus) : arrêtée sans test à 13 h 32, le filtre
  comptait aussi ProjectShiva sur `UE_5.8_AS_2` (autre moteur, ne bloque pas). 5e passage (13 h 33) : filtre limité à
  `UE_5.8_AS_3`, puis `-Parts lobby` (`scratchpadrwen_lobby_run5.txt` de la session 16edf49a).
- 5e passage : **Alice et Bob verts** (le correctif du signal tient) ; FAIL plus loin, « Carol : « Quitter vers la liste »
  not pressed » : Carol attend `dave_done.signal` (150 s) et Dave, lancé à 13 h 36, est resté bloqué au démarrage sur
  UnrealBuildTool (`-Mode=ValidatePlatforms`) pendant une compilation d'Elrond (13 h 35 min 57 s). Environnement encore.
  6e passage : jusqu'à 3 essais, chacun après 30 s sans UnrealBuildTool d'`UE_5.8_AS_3` ; nouvel essai seulement si un
  jeu n'a jamais commencé sa séquence (`scratchpadrwen_lobby_run6.txt`).
- **Vert** : 6e passage, 1er essai (13 h 41) : `quick_check -Parts lobby` **PASS en 250 s** (Alice, Bob, Carol, Dave,
  contrôles du serveur dont l'abandon de Bob).
- Au passage : `MovementInProgress.md` disait encore « seul reste l'aléa « build site » » ; il était corrigé depuis H
  (`fb462ff`) : noté clos (même commit).

### Reprise exacte (K)

1. [x] Contrôles ajoutés, rouge hors jeu.
2. [x] `quick_check -Parts lobby` PASS dans Arwen (2026-10-26 13 h 45, 250 s).
3. [x] Accord de l'utilisateur, puis commit en anglais sur `net-surrender-check` (sans Co-Authored-By) avec la ligne
   « `quick_check` : l'abandon en réseau… » retirée d'`EtapeDefini.md`, fusion dans `main`, Arwen libérée.

**Clos le 2026-10-26** : accord de l'utilisateur (« go »), commité sur `net-surrender-check` et fusionné dans `main` ; Arwen libérée.

## L — Clic droit sur un dépôt : aller y décharger

Demande de l'utilisateur le 2026-10-27 : « lorsque l'on clique sur le bâtiment de prod de la ressource que l'on porte,
on va drop ce qu'on a dans la main (bois et camp de bûcheron par exemple) ». Règle : **D485** au GDD (après décharge :
inactif ; Maj : en file). Worktree `HeroesOfLiberty-Haldir`, branche `dropoff-click` depuis `main` 60230cd
(`IN_USE.txt` posé ; la branche `perf-target` d'avant reste intacte).

### Constat (code de `main` 60230cd)

- `AHOLPlayerController::OrderAt` : un clic droit sur son bâtiment fini envoie les unités au point du sol (aucun cas
  « dépôt ») ; chantier à soi → bâtir, bâtiment ennemi → attaque.
- La récolte (`UHOLGatherProcessor`) a déjà la phase `ToDeposit` : `WalkToDeposit` va au **dépôt le plus proche**
  (`FindDeposit`), `Unload` décharge ; `SourceIndex == INDEX_NONE` en route → `Stop` (inactif) après la décharge.
- Maj + source va dans la file des ouvriers (`IssueGatherOrder` bQueue → `FHOLBuildOrder.SourceIndex` →
  `HOLConstruction::TWorkItem::MakeGather`) ; à son tour, `UHOLBuildProcessor` la confie à la récolte (phase `Gathering`)
  et passe à la suite quand la récolte s'arrête.
- Le client ne connaît pas la charge de chaque unité (D303) : le tri porteur / non porteur se fait chez le serveur.

### Plan (boucles de 15 min, sous-agents)

- **L.1** Tests rouges (`quick_check`, vraie chaîne : `hol.Select.OrderGround` sur le bâtiment) : sans Maj, des Paysans
  qui portent du bois, clic droit sur le camp de bûcheron → déchargent **à ce camp** (même si le forum est plus près
  pour l'un d'eux), puis inactifs ; un Paysan qui ne porte rien / porte de la nourriture → marche au point ; avec Maj
  (Paysan qui bâtit, Maj + clic sur le camp) → décharge après son chantier.
- **L.2** Code sans Maj : ordre `DropOff` (controller → RPC → `HOLPlayerOrders::DropOff` → file des ordres), dépôt visé
  dans `FHOLGatherFragment`, `FindDeposit` le préfère ; non-porteurs et soldats → ordre de déplacement au point.
- **L.3** Code Maj : élément de file « dépôt » (`TWorkItem`), confié à la récolte à son tour ; ligne de file bleue.
- **L.4** Scénario d'essai `Tools/Scenarios/dropoff_click.txt` (`play.ps1`), répétition hors écran, puis essai de
  l'utilisateur.

### Suivi

- 2026-10-27 : questions tranchées (D485), plan écrit ; Haldir compilé sur la nouvelle branche (PASS, 78 s). L.1 (tests rouges) confiée à un sous-agent.
- 2026-10-27 (L.1, sous-agent) : cas `build:dropoff` ajouté à `Tools/Tests/quick_check.ps1` (worktree Haldir, dernier cas de
  la partie build ; marques `build dropoff` et `build dropoff order`) : camp de bûcheron D (-19200, -3200) bâti d'un coup,
  3 Paysans récoltent 5 s l'arbre près de CampTree (forum plus près qu'eux de D), un 4ᵉ vide ; les 4 choisis
  (`ClickUnit`), clic droit au centre de D (`OrderGround`) ; `hol.Units.Where 0` avant, +10 s, +13 s ; contrôles : mise en
  place (3 portent du bois partiel, forum plus près, le 4ᵉ rien), chacun des 3 décharge son bois **à D** et nulle part
  ailleurs, le vide rien, puis les 4 inactifs à moins de 12 m de D sans charge. (c) Maj pas encore fait.
- 2026-10-27 (L.1, résultat) : `quick_check.ps1 -Parts build:dropoff` **ROUGE pour la bonne raison** en 43 s (séquence
  PASS, jeu 24,7 s ; mise en place PASS : 3 Paysans portent 2-3 bois, forum plus près, le 4ᵉ rien) :
  `[KO] right click on D: each of the three unloads its wood at D ... (0 unloads: )` et `[KO] then the four idle in front
  of D ... (#2 Idle Wood 3, D 552 cm, #3 Idle Wood 3, ... #5 Idle, D 491 cm)` : ils marchent au point et restent avec leur
  bois. Sélection par `BoxGround` (le `ClickUnit -N add` basculait la même unité). Reste : (c) Maj, et la tenue du cas
  dans `-Full` (boîtes près du bois où récoltent les Paysans du cas `build camp`).
- 2026-10-27 (bilan L.1) : sous-agent arrêté au minuteur de 15 min, en train d'attaquer (c) Maj après le rouge de (a) et
  (b) ; son diff (`quick_check.ps1`, +54 lignes, cas `build:dropoff`) est cohérent et gardé. **Amélioration** : une
  boucle = un seul objectif (un cas, ou un changement de code), le sous-agent rend la main dès cet objectif atteint.
  (c) Maj passe en L.3 (son cas rouge d'abord). Suite : L.2, code sans Maj, jusqu'au vert de `build:dropoff`.
- 2026-10-27 (L.2, code, Haldir `dropoff-click`) : clic droit des Paysans sur un bâtiment à soi, fini, qui reçoit des
  ressources → `Order: N units drop off at …`, `RequestDropOffUnits` → RPC `ServerDropOffUnits` →
  `HOLPlayerOrders::DropOff` : tri chez le serveur (porteurs d'une ressource acceptée → `IssueDropOffOrder`, les autres
  `IssueMoveOrder` au point) ; `FHOLGatherOrder::Deposit` (ordre de dépôt, SourceIndex INDEX_NONE) ;
  `UHOLGatherProcessor::ApplyGatherOrders` : chantier oublié, `Phase = ToDeposit`, `SourceIndex = INDEX_NONE`,
  `FHOLGatherFragment::DepositTarget` ; `FindDeposit` préfère ce bâtiment s'il est encore dépôt du joueur et accepte la
  charge, sinon le plus proche ; après la décharge, inactif (chemin `Stop` existant).
- 2026-10-27 (L.2, résultat) : compilation PASS (Haldir, 312 s) ; `quick_check.ps1 -Parts build:dropoff` **VERT** du
  premier coup en 88 s (séquence PASS, jeu 24,7 s ; `build dropoff : PASS`, ses trois contrôles : mise en place, les 3
  déchargent leur bois à D et nulle part ailleurs, puis les 4 inactifs devant D sans charge). Rien commité. Reste : L.3
  (Maj, son cas rouge d'abord), la tenue du cas dans `-Full`, l'essai de l'utilisateur.
- 2026-10-27 (L.3, cas rouge, Haldir `dropoff-click`) : nouveau sous-cas `build:dropoffshift` (`quick_check.ps1`) ; la
  construction de D sort du cas `dropoff` dans sa marque `build dropoff camp`, partagée par les deux cas (chacun marche
  seul, et `dropoffshift` après `dropoff`). Mise en place : un Paysan récolte l'arbre de CampTree 5 s (bois partiel), un
  second vide ; les deux sélectionnés, maison à (-17600, -4400) sans FastBuild, puis Maj + clic droit sur D. Contrôles :
  (1) le porteur ne décharge pas avant « finished House » et bâtit ; (2) ensuite il décharge son bois à D ; (3) puis
  inactif devant D sans charge ; (4) le vide finit la maison, ne décharge rien, inactif (position notée). Attendu ROUGE
  sur (1) (décharge tout de suite).
- 2026-10-27 (L.3, résultat) : **pas de verdict**. Le lancement de `quick_check -Parts build:dropoffshift` (12:14) est
  resté bloqué plus de 7 min avant le jeu : la vérification des SDK (Turnkey `-WaitForUATMutex`) de l'éditeur attend le
  verrou UAT tenu par le `BuildCookRun` d'Elrond (lancé à 12:06, cuisson Windows+WindowsServer). Lancement arrêté au
  minuteur. Le cas est écrit (`quick_check.ps1` analysé sans erreur) ; à relancer tel quel quand la cuisson d'Elrond est
  finie. La place de la maison S (-17600, -4400) n'a pas encore été vue valide.
- 2026-10-27 (L.3, code, sous-agent, Haldir `dropoff-click`, **ni compilé ni testé**) : Maj + clic droit sur un dépôt =
  élément de file. `HOLConstruction::TWorkItem` : `bDeposit`, `MakeDeposit(Site)`, `IsDeposit()`, `IsBuild()` ;
  `operator==` le compare ; `AdvanceWorkQueue` garde un dépôt en attente comme une source (`!IsBuild() || IsSiteOpen`).
  `FHOLBuildOrder::bDeposit` ; `UHOLCommandSubsystem::IssueDropOffOrder(…, bQueue)` : avec Maj, un `FHOLBuildOrder`
  bDeposit/bQueue/bChain dans `PendingBuildOrders`. `HOLPlayerOrders::DropOff` avec Maj (dépôt valide) : tous les
  ouvriers reçoivent le dépôt en file (charge jugée à leur tour), les soldats marchent au point en file
  (`SendNonWorkersTo` prend un `bQueue`, défaut false : Gather/Build inchangés). `UHOLBuildProcessor::ApplyBuildOrders` :
  ordre bDeposit accepté si bâtiment du joueur, fini, `TakesDeliveries`, `Accepts` non vide ; sinon « not one of its
  deposits ». `Advance` : tête dépôt → `StartUnloading` (Build.Phase = Gathering ; porte une ressource acceptée →
  `FHOLGatherFragment::StartDropOff`, partagé avec `ApplyDropOffOrder`, activité GatherTrip ; sinon élément fini au pas
  suivant) ; après le dépôt, `Before.IsBuild()` : ni `GatherForCamp` ni `FindNextSite`, inactif si rien après. Chaînes :
  `HOLOrderChain::EStepKind::Deposit` / `FStep::MakeDeposit(id du bâtiment)`, placé comme Build (`LocateChainStep`),
  bleu (`HOLSelectionMarkers`), nom « Deposit ». Test pur : 7 contrôles dépôt ajoutés à `Construction.WorkQueue`.
  Fichiers : `Buildings/HOLConstructionRules.h`, `Commands/HOLConstructionCommand.h`, `Commands/HOLCommandSubsystem.{h,cpp}`,
  `Commands/HOLPlayerOrders.{h,cpp}`, `Commands/HOLUnitOrderProcessor.cpp`, `Simulation/HOLBuildProcessor.{h,cpp}`,
  `Simulation/HOLGatherProcessor.cpp`, `Simulation/HOLSimulationFragments.h`, `Simulation/HOLOrderChainRules.{h,cpp}`,
  `Player/HOLSelectionMarkers.cpp`, `Tests/HOLConstructionTests.cpp`. Incertain : un Paysan qui récolte **hors file** (ou
  en décharge sans Maj) et reçoit Maj + dépôt n'a pas de file : sa récolte s'arrête et il va décharger tout de suite
  (comme Maj + source aujourd'hui), au lieu d'attendre ; un élément « rien à décharger » coûte un pas avant la suite ;
  dépôt perdu en route : le plus proche (chemin L.2).
- 2026-10-27 (L.3, session principale) : rouge de `build:dropoffshift` vu sur l'ancien binaire (après la fin du paquetage
  d'Elrond, qui bloquait tout lancement par le verrou UAT) : (1) le porteur décharge tout de suite (« #2 unloaded 3 Wood at
  LumberCamp » avant la maison), et le Paysan vide part marcher vers D au lieu de bâtir (pas de première frappe sur la
  maison : la séquence expire). Code L.3b compilé (PASS, 517 s) : `tests` 189/189 PASS, `build:dropoff` PASS ;
  `build:dropoffshift` : (1)-(3) PASS, (4) KO parce que trop strict sur le moment (le Paysan vide marchait encore vers la
  maison 1 s après la première frappe du porteur ; il bâtit à la fin, saute la décharge, reste inactif : comportement
  voulu) ; contrôle (4) corrigé (bâtit à la première frappe **ou** à la fin). Puis `-Parts build:dropoff,build:dropoffshift`
  **PASS en 40 s**. En cours : `-Parts build,gather` en entier, puis le scénario d'essai (L.4).
- 2026-10-27 (L.4) : scénario `Tools/Scenarios/dropoff_click.txt` (Haldir) : camp de bûcheron fini à (-19200, -3200),
  5 Paysans qui portent 2-3 bois (clic droit sur le camp ou le forum), puis 2 autres pour Maj (maison puis Maj + camp).
  Répétition hors écran (`hol.Scenario.AutoContinue 1`, `-RenderOffScreen`) : 41 étapes, « ready » en 14 s, aucune erreur.
- 2026-10-27 (tenue dans la partie entière) : `-Parts build,gather` : `gather` et tous les anciens cas de `build` PASS,
  mais `dropoff` et `dropoffshift` FAIL : (1) D à (-19200, -3200) tombait sur une maison d'un cas d'avant (maisons
  à y = -3200 : le fantôme était déplacé) → D à (-18400, -6000), son bâtisseur à (-17200, -6000), attente 12 s au lieu
  de 10 (~66 m) ; (2) le cadre près du bois prenait aussi les récolteurs du cas `camp` → sélection par groupes
  (3 pour `dropoff`, 4 pour `dropoffshift` : les porteurs juste nés par cadre ou clic, puis le dernier né par
  `ClickUnit -1`, `Group Add`, `Group Select`). Puis `-Parts build` **PASS en 101 s** (tous les cas, caméra 2p et 8p),
  et `-Parts build:dropoff,build:dropoffshift` seuls **PASS en 50 s**.
- **Reprise exacte (L)** : tout est codé et vert dans Haldir (`dropoff-click`, non commité). Reste : essai de
  l'utilisateur (`play.ps1 dropoff_click -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`), puis, après son
  accord : fusion de `main` dans la branche, recompilation, `-Parts tests,build,gather` sur la base à jour, commit en
  anglais (avec la ligne « Récolte » retirée d'`EtapeDefini.md`, D485 au GDD), fusion dans `main` (accord des sessions de
  la copie principale), `IN_USE.txt` d'Haldir remis à son état d'avant (perf-target).
- 2026-10-27 (essai de l'utilisateur) : « tout est bon » sauf **Maj + clic déplacement, puis Maj + dépôt : le
  déplacement est supprimé**. Cause : deux files qui s'ignorent. Le déplacement en Maj va dans la file des déplacements
  (`UHOLUnitOrderProcessor::QueuedMoves`) ; le dépôt en Maj va dans la file de travail (`UHOLBuildProcessor`), démarre
  tout de suite (pas de file de travail avant), et `StartQueuedMoves` oublie les déplacements d'une unité « mise au
  travail ». **Correctif prévu (L.5)** : un ouvrier **sans file de travail** reçoit le dépôt en Maj dans la file des
  déplacements (nouveau genre d'ordre `DropOff`, comme l'attaque d'un bâtiment) ; quand il démarre, ses porteurs
  reçoivent l'ordre de dépôt, les autres marchent au point ; une unité **en route pour un dépôt ordonné** compte comme
  occupée (un déplacement en Maj donné après attend la décharge, et sa file n'est pas oubliée). Un ouvrier qui bâtit
  garde la file de travail (L.3, validé). L.5a : cas rouge `build:dropoffmove` (Maj déplacement P puis Maj dépôt D : passe
  près de P avant de décharger à D ; Maj dépôt puis Maj déplacement B : décharge à D puis arrive à B). L.5b : code.
- 2026-10-27 (L.5a, sous-agent, Haldir) : cas rouge `build:dropoffmove` en cours d'écriture dans `quick_check.ps1` (groupe 5,
  marques `dropoff camp`, `dropoffmove`, `dropoffmove order`) : un Paysan qui porte du bois partiel ; A : Maj + sol P
  (-13400, -3300) puis Maj + D : passe à 3 m de P avant de décharger à D, puis inactif devant D ; B : il récolte de
  nouveau, puis Maj + D puis Maj + sol B (-18400, -7500) : décharge à D, puis arrive à 3 m de B, inactif. Lancement seul
  (`-Parts build:dropoffmove`, binaire L.4) attendu rouge.
- 2026-10-27 (L.5b, sous-agent, Haldir) : code écrit, **ni compilé ni essayé**. Tout dépôt du joueur (Maj ou non) passe
  maintenant par la file des déplacements : nouveau genre `EHOLOrderKind::DropOff` (`TargetBuilding` = le dépôt, `Target`
  = le point cliqué), émis par `UHOLCommandSubsystem::IssueDropOffMoveOrder` ; `HOLPlayerOrders::DropOff` n'y trie plus
  rien (soldats en Maj : `SendNonWorkersTo` en file, inchangé). `UHOLUnitOrderProcessor::ApplyQueuedOrders` : dépôt plus
  valable → déplacement ; en Maj, les ouvriers qui bâtissent (`IsBuilding`) sont sortis et reçoivent la file de travail
  (`IssueDropOffOrder(..., true)`, sous `FPlayerOrderScope` si `bChain` : chemin L.3 validé) ; le reste se met en file si
  une unité marche (journal « Drop-off order of slot %d queued (Shift) for %d units at %s ») ; appliqué (tout de suite
  ou démarré par `StartQueuedMoves`) : porteurs d'une ressource acceptée → `IssueDropOffOrder(..., false)` (la récolte
  passe après l'ordre dans le même pas), combat désengagé, étape de chaîne `Deposit` ; les autres → `Move` vers le point.
  `FHOLGatherFragment::bDropOrder` + `IsDroppingOff()` (posé par `StartDropOff`, effacé par `StartTask`, `Stop`, la
  décharge ; faux si `Phase` None) : `StartQueuedMoves` n'oublie plus la file d'une unité qui dépose, garde sa ligne de
  chaîne, et l'élément suivant attend (`HOLOrderChain::IsCurrentOrderOver`, 5e paramètre `bDroppingOff`, défaut faux) ;
  à l'entrée en file, une unité qui dépose (hors bâtisseur) ou reçoit un dépôt dans le même passage compte comme marchant.
  Champ `FHOLMoveOrder::Deposit` (inutilisé) retiré. Test de règle : cas « Drop-off » ajouté à
  `HeroesOfLiberty.Orders.BuildingQueue` (HOLOrderChainTests.cpp). Incertain : le dépôt **sans Maj** change de chemin
  (même effet attendu ; nécessaire, sinon la file gardée pendant un dépôt survivrait à un dépôt sans Maj) et dessine
  maintenant une ligne bleue vers le dépôt ; `Combat->Disengage()` sur les porteurs ; `TOptional<FPlayerOrderScope>`.
  Écrit (+190 lignes, non lancé jusqu'au bout) : premier lancement bloqué, le jeu figé 6 min juste après « Completed SDK
  detection » (0 % CPU), comme le `quick_check` d'Arwen lancé à 13 h 55 (même point, en même temps qu'un UBT de Celeborn
  et trois éditeurs Shiva) ; laissé tel quel (processus de jeu 65744, non tué) à la limite des 15 min. À relancer quand la machine est libre.
- 2026-10-27 (L.5, session principale) : sous-agent L.5a arrêté au minuteur (jeu figé au démarrage, machine chargée), son
  cas `build:dropoffmove` (+190 lignes) gardé. Rouge vu sur l'ancien binaire : A « closest 1963 cm » de P (déplacement
  supprimé), B « closest 1952 cm » de B (déplacement perdu après la décharge). Code L.5b compilé (PASS, 183 s) :
  `-Parts tests,build:dropoff,build:dropoffshift,build:dropoffmove` **PASS en 39 s** (tests 189/189), puis
  `-Parts production,move,gather,build,formation,combat` (toutes les parties qui donnent des ordres en Maj) **PASS en
  195 s**. Scénario `dropoff_click` : une note de plus (Maj + sol puis Maj + camp, et l'inverse). Nouvel essai de
  l'utilisateur.
- 2026-10-27 (2e essai de l'utilisateur) : Maj + déplacement puis Maj + dépôt **marche** ; mais (1) **Maj déplacement,
  Maj dépôt, Maj déplacement** : il fait le déplacement, va au dépôt, et devant le dépôt **ne fait plus rien** (le
  dernier déplacement est perdu) ; (2) **Maj déplacement puis Maj + source** : la récolte part tout de suite, le
  déplacement est supprimé (défaut **d'avant D485**, même cause que L.5 : la source en Maj va dans la file de travail).
  Même défaut probable pour Maj + chantier après un Maj déplacement. **L.6** : L.6a cas rouges (`dropoffmove` partie C :
  P, D, B ; nouveau cas `build:shiftwork` : Maj P puis Maj source → passe par P puis récolte ; Maj P puis Maj chantier →
  passe par P puis bâtit ; et noter ce que fait Maj chantier puis Maj déplacement) ; L.6b code : source et chantier en
  Maj d'un ouvrier sans file de travail passent par la file des déplacements comme le dépôt, et la cause de (1).
- 2026-10-27 (L.6a, sous-agent dans Haldir) : `quick_check.ps1` : partie C de `build:dropoffmove` (Maj P, Maj D, Maj B
  sur un porteur : passe à P, décharge à D, puis à 3 m de B et inactif) ; nouveau cas `build:shiftwork` (marques
  `shiftwork`, `shiftwork order`, `shiftwork site`, `shiftwork after`, sans groupe : `ClickUnit -1` après chaque
  apparition) : (a) Maj Pa puis Maj arbre → passe à Pa avant le premier bois ; (b) maison H posée par un bâtisseur
  renvoyé aussitôt, Maj Pb puis Maj H → passe à Pb avant « Building » ; (c) observation : Maj H puis Maj Q. Points au
  sud de la rangée de D (Pa (-15800, -8000), H (-15600, -9600), Pb (-13200, -7200), Q (-18800, -10400)). Lancement :
  `-Parts build:dropoffmove,build:shiftwork`.
  Résultat (75 s, binaire de Haldir tel quel) : **rouge pour la bonne raison** partout. `dropoffmove` C : passe près de
  P (334 cm au plus près, échantillons d'1 s, passés depuis à 0,25 s les 8 premières s, non relancé), puis au pas où
  « Queued drop-off started » paraît, « Queued move started … (-18387, -7500) » paraît **au même pas** ; il marche
  (« Moving ») jusque devant D et y reste **inactif avec son bois (8)** : ni décharge, ni B. **B aussi rouge** sur ce
  binaire (décharge à D, puis la file « Move order … queued (Shift) » vers B jamais démarrée ; aucun « Queued move
  started »), alors que L.5 l'avait vu vert. `shiftwork` (a) : part tout de suite récolter (GatherTrip), Pa jamais
  approché (1991 cm) ; (b) : va droit à H et bâtit, Pb jamais approché (2389 cm) ; (c) : marche vers H, H fini par
  l'autre à son arrivée, puis « Queued move started » vers Q, inactif à Q (pas vu bâtir : H finissait trop tôt).
- 2026-10-27 (L.6b, sous-agent dans Haldir, code seul, **ni compilé ni essayé**) : fichiers `Commands/HOLUnitCommand.h`
  (genres `EHOLOrderKind::Gather` et `Build`, champ `FHOLMoveOrder::SourceIndex`), `Commands/HOLCommandSubsystem.h/.cpp`
  (`IssueQueuedWorkOrder` : source ou chantier en Maj dans la file des déplacements, sans projection sur la navmesh),
  `Commands/HOLPlayerOrders.cpp` (`Gather` et `Build` avec Maj → `IssueQueuedWorkOrder` ; les soldats y marchent **en
  file** avec Maj, comme pour le dépôt), `Commands/HOLUnitOrderProcessor.h/.cpp`. **Cause de (1) confirmée** (hypothèse
  juste) : au pas N, `ApplyQueuedOrders` donne le dépôt démarré de la file (`IssueDropOffOrder`), `StartQueuedMoves`
  tourne avant la récolte et voit l'unité ni en dépôt ni en marche : B démarre au même pas ; au pas N+1, B arrête la
  récolte puis la marche de tâche vers D (émise au pas N) le remplace : marche à D sans tâche, inactif. **Cause du rouge
  de B** (Maj D sans rien devant, puis Maj B au même pas, les 3 images entre les deux clics tiennent dans un pas) : D
  appliqué tout de suite, B mis en file (`DroppingNow`), puis la passe « Gone » de `StartQueuedMoves` voit un ouvrier
  **encore en récolte** (la récolte n'a pas encore appliqué le dépôt) et oublie sa file : B perdu. Correctif des deux :
  `DroppingNow` devient un membre (vidé au début de chaque `ApplyQueuedOrders`) ; dans `StartQueuedMoves`, une unité de
  `DroppingNow` n'est pas « Gone », garde sa ligne de chaîne, et l'élément suivant de sa file ne démarre pas dans ce pas.
  **(2)** : `Gather`/`Build` en Maj passent par la file des déplacements : les ouvriers **avec file de travail** (ou
  mis au travail plus tôt dans la même passe, `WorkStartedNow`) le reçoivent au bout de leur file de travail tout de
  suite (comme avant) ; les autres : en file si l'un marche (journal « Gather/Build order of slot N queued (Shift) … »,
  « Queued gather/build started … »), sinon appliqué tout de suite (« … order of slot N applied for … »). Appliqué, il
  émet l'ordre de travail **avec Maj** (`IssueGatherOrder(…, true)` / `IssueBuildOrder` bQueue vrai, sous
  `FPlayerOrderScope` si bChain) et non sans Maj comme demandé : pour un ouvrier sans file de travail c'est un départ
  immédiat (le même chemin qu'avant L.6 quand rien ne marchait, donc journaux et cas existants inchangés), et ça garde
  « Maj source puis Maj chantier » en file de travail. Les ordres de travail qui le suivent **directement** dans la file
  des déplacements (source, chantier, dépôt) passent dans sa file de travail, dans l'ordre (sinon Maj P, Maj arbre, Maj
  chantier perdait le chantier) ; le reste de la file des déplacements est oublié une fois au travail, comme avant.
  `WorkStartedNow` (membre) : l'élément suivant ne démarre pas au même pas. Chaîne (D328) : `MakeChainStep` donne
  `MakeGather` / `MakeBuild` aux ordres en file. **Incertain** : Maj + pose d'un bâtiment (`Place`, ordre de
  construction) non touché, garde le défaut (2) ; un ordre de travail **sans** Maj donné au pas exact où un déplacement
  en file démarre peut encore être écrasé (marche de tâche après l'ordre démarré : rare, non traité) ; Maj chantier
  puis Maj déplacement dans la même passe : le déplacement est mis en file puis oublié (il bâtit). À vérifier à la
  compilation : `TOptional<FPlayerOrderScope>` dans la fonction statique `IssueToWorkQueue`, `UnitsOfQueued.KeySort`,
  `FHOLBuildOrder` vu depuis `HOLUnitOrderProcessor.cpp` (par `HOLCommandSubsystem.h`).
- 2026-10-27 (L.6, session principale) : sous-agent L.6b arrêté au minuteur en fin de travail (journal écrit, fonctions
  d'aide présentes). Compilation PASS (551 s). `-Parts tests,build:dropoff,build:dropoffshift,build:dropoffmove,build:shiftwork`
  **PASS en 107 s** (tests 189/189) ; puis `-Parts production,move,gather,build,formation,combat` **PASS en 522 s**
  (séquence 282 s, x3,8 ; démarrage lent, 5 éditeurs d'autres copies ; les cas L ajoutent ~80 s de séquence à `build`,
  à alléger si besoin). Scénario `dropoff_click` : une note de plus (Maj sol, Maj camp, Maj sol ; Maj sol puis Maj arbre ;
  Maj sol puis Maj chantier). Troisième essai de l'utilisateur. Restes connus (non traités) : Maj + **pose** d'un
  bâtiment après un Maj déplacement garde le défaut ; Maj chantier puis Maj déplacement : le déplacement est oublié (il bâtit).
- 2026-10-27 : **3e essai de l'utilisateur : « good »**. Branche `dropoff-click` avancée sur `main` 7c48518 (J8a.2),
  modifications réappliquées (conflit de `quick_check.ps1` avec le cas `build hero` : les deux gardés), recompilation et
  `quick_check` sur la base à jour, puis commit et fusion dans `main`. Restes notés dans `EtapeDefini.md` (retours
  d'essai) : Maj + pose d'un bâtiment après un Maj déplacement ; Maj chantier puis Maj déplacement.
- 2026-10-27 (avant commit, base `main` 7c48518) : recompilation PASS (679 s) ; `-Parts tests,production,move,gather,build,formation,combat`
  deux fois FAIL « game launch ended by itself » à des endroits différents : c'est la **limite de 960 s** de `quick_check`
  pour un lancement de jeu (le jeu tué par son propre lanceur), dépassée par la charge de la machine (85-100 % de CPU,
  compilations d'autres copies, ProjectShiva) ; aucune session n'a tué de jeu (toutes interrogées). Découpé : `tests`
  196/196, `production`, `move`, `gather` PASS (passage 2), `-Parts build` **PASS en 156 s** (cas `hero` de `main`
  compris), `-Parts formation,combat` **PASS en 114 s** (avec `UE_SKIP_UBT_SDK_SETUP=1`). Commit sur `dropoff-click`.

## M — Maj : pose d'un bâtiment et déplacement après un chantier

Restes de L, remontés au 3e essai de l'utilisateur (2026-10-27, lignes de « Retours d'essai » d'`EtapeDefini.md`) :

1. **Maj + déplacement puis Maj + pose d'un bâtiment** : le déplacement est supprimé (la pose, `Place`, ne passe pas
   par la file des déplacements). Attendu (AoE4) : le Paysan marche au point, puis va bâtir.
2. **Maj + chantier puis Maj + déplacement** : le déplacement est oublié, le Paysan bâtit puis s'arrête. Attendu : il
   bâtit, puis marche au point une fois le bâtiment fini.

Règle : Maj met **tout** en file, dans l'ordre des clics, quel que soit le genre d'ordre (déplacement, source, chantier,
pose, dépôt). Worktree `HeroesOfLiberty-Elrond`, branche `shift-queue` depuis `main` 6fdb7be.

### Plan (boucles de 15 min, sous-agents)

- **M.1** Tests rouges (`quick_check`, vraie chaîne : ordres du contrôleur avec Maj), un cas par bug, plus les
  enchaînements voisins (Maj déplacement, Maj pose, Maj déplacement ; Maj chantier, Maj déplacement, Maj source) ;
  correctif ; cas existants de L (`build:dropoff*`, `build:shiftwork`) toujours verts.
- **M.2** Scénario d'essai (`dropoff_click` complété ou `shift_queue`), répétition hors écran, essai de l'utilisateur.
- **M.3** Commit, fusion dans `main`, lignes retirées d'`EtapeDefini.md`.

### Suivi

- 2026-10-27 : étape ouverte (session principale du chantier tailles de carte, Elrond libéré). Rien codé.
- 2026-10-27 : **M.1 lancée** (sous-agent, Elrond `shift-queue`) : tests rouges des deux enchaînements et de leurs voisins,
  puis correctif ; rien commité avant l'essai.
- 2026-10-27 (M.1, sous-agent, Elrond `shift-queue`) : cas `build:shiftplace` et `build:shiftsite` **rouges** sur le
  binaire de `main` (P jamais approché, chantiers abandonnés), puis correctif : `FHOLBuildFragment::bMovesAfter` ; pose
  en Maj par `IssueQueuedWorkOrder` (la pose passe par la file des déplacements) ; Maj sur un ouvrier qui a une file de
  travail = mis en file après elle ; inactif en fin de file de travail si des déplacements suivent ; lignes de chaîne
  d'un bâtisseur avec ses déplacements après (`AppendQueuedSteps`). Compilation PASS (196 s) ; les deux cas PASS (43 s,
  puis 26 s) ; `-Parts tests,build,move,gather` **PASS en 114 s** (tests 232/232). Scénario `shift_queue` répété hors
  écran (prêt, sans erreur). Incertain, à voir à l'essai : Maj arbre puis Maj déplacement attend la fin du bosquet
  (comme Maj arbre puis Maj chantier, D309) ; une sélection mêlée attend le plus lent. Reste : essai (M.2), puis M.3.
- 2026-10-27 : **essai de l'utilisateur : « tout m'a l'air bon »** (scénario `shift_queue`). Commit 1245e98 sur
  `shift-queue`, fusion de `main` 07fe108 (R8a, sans code du jeu : tests de M.1 toujours valables), **dans `main` :
  0a45afe** (avance rapide). **Étape M close** ; lignes retirées d'`EtapeDefini.md`.

## N — Point de ralliement en plusieurs étapes

Demande de l'utilisateur (2026-10-27, ligne d'`EtapeDefini.md`) : « Maj + clic droit ajoute des points au ralliement
d'un bâtiment (chemin de sortie, par exemple pour contourner une zone) ». Worktree `HeroesOfLiberty-Elrond`, branche
`rally-steps` depuis `main` 715402c.

### Constat (code de `main` 715402c)

- `AHOLBuilding` garde **un seul** point (`RallyPoint`, `bHasRallyPoint`, `RallySource`, `RallySite` côté serveur),
  posé par l'ordre `FHOLProductionCommand::EKind::Rally` (`UHOLProductionSubsystem`), répliqué au propriétaire
  (`OnRep_Rally`, drapeau) ; à la naissance, l'unité reçoit un déplacement, une récolte ou un chantier
  (`Request.MoveTarget` / `GatherSource` / `BuildSite`) sous `FRallyOrderScope` (ligne blanche de chaîne, D328).

### Règles proposées (façon AoE4, à juger à l'essai)

- Clic droit sans Maj : remplace tout le ralliement par ce point (comme aujourd'hui).
- **Maj + clic droit** : ajoute une étape à la fin (sol, source ou chantier), 8 étapes au plus (au-delà : refusé, log).
- À la naissance, l'unité reçoit **la même file que si le joueur lui avait donné ces ordres avec Maj** (file des
  déplacements de L et M) : elle passe par chaque point dans l'ordre, puis fait la dernière étape (récolter, bâtir,
  ou s'arrêter au point).
- À l'écran (propriétaire) : le drapeau à la dernière étape, un trait entre les étapes quand le bâtiment est
  sélectionné.

### Plan (boucles de 15 min, sous-agents)

- **N.1** Tests rouges (`quick_check`, vraie chaîne : ordres du contrôleur sur le bâtiment sélectionné, avec Maj), puis
  code : étapes répliquées, ordre « ajouter une étape », naissance avec la file, affichage.
- **N.2** Scénario d'essai `rally_steps`, répétition hors écran, essai de l'utilisateur.
- **N.3** Commit, fusion dans `main`, ligne retirée d'`EtapeDefini.md`.

### Suivi

- 2026-10-27 : étape ouverte ; N.1 lancée (sous-agent, Elrond `rally-steps`).
- 2026-10-27 (N.1, sous-agent, Elrond `rally-steps`) : cas `build:rallysteps` **rouges** sur l'ancien binaire (a : point
  remplacé, Paysan droit à R2 ; b : droit à l'or ; d : aucun refus), puis code : `FHOLRallyWaypoint` et `RallyWaypoints`
  (répliqués au propriétaire ; chantiers côté serveur), `RallyPoint` garde la dernière étape (drapeau) ; ordre de
  ralliement avec `bQueue` (Maj) depuis `OrderAt` ; 8 étapes au plus (refus et log) ; naissance du côté de la première
  étape, puis `HOLIssueSpawnRallySteps` (sol = déplacement, source ou chantier = `IssueQueuedWorkOrder`, camp fini = sa
  source la plus proche, soldats = marche) sous `FRallyOrderScope` ; trait blanc bâtiment → étapes → drapeau. Verts :
  `build:rallysteps` PASS en 27 s ; `-Parts tests,production,build,move,gather` PASS (232/232) ; `net` étendu (chaque
  client voit ses 2 étapes) PASS en 163 s. Scénario `rally_steps` répété hors écran (prêt en 4,2 s, sans erreur).
  À juger à l'essai : une source au milieu du chemin est récoltée jusqu'à épuisement du bosquet (règle de M) ; courte
  halte à chaque point. Reste : essai (N.2), puis N.3.
- 2026-10-27 : **essai de l'utilisateur : « tout est bon »** (scénario `rally_steps`). Commit 1ddc9df sur `rally-steps`,
  fusion de `main` d1a7dc5 (GDD et EtapeDefini seulement), **dans `main` : 3026278** (avance rapide). **Étape N close** ;
  ligne retirée d'`EtapeDefini.md`.
