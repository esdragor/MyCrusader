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
