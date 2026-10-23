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
