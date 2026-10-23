# Chantier rochers (ridge-rock)

Retours d'essai de l'utilisateur du 2026-10-23, scénario `ridge_army` (50 Hommes d'armes entre le centre et la crête
de rochers de `Plain`, envoyés derrière elle) :

1. « ils traversent le rocher, ils ont pas le droit » : jusqu'à 5 soldats à la fois dans la pierre, le bloc figé ~2 s
   au coin (« unit N held the block 40 steps »).
2. Armée rangée derrière la crête, renvoyée devant : collée contre le bout du rocher.
3. Traversées répétées vers des points au hasard : armée coupée en deux, une moitié figée 16-17 s derrière la crête.
   « Un test d'un scénario ne suffit pas » ; « il faut vraiment que nos red tests soient hyper exigeants ».

Branche `ridge-rock`, worktree `HeroesOfLiberty-Haldir`.

## Tests

- `quick_check -Parts rocks` : 17 cas autour de la crête du joueur 0 et dans le bosquet du milieu (face, dos,
  diagonales, armée collée au rocher, Lanciers, Archers, bout à bout, armée formée aux ordres enchaînés, troncs).
  Chaque cas : personne hors du navmesh près des obstacles (relevé toutes les 0,1 s, tolérance 25 cm : une unité
  dessinée entre deux pas coupe l'angle des voxels du navmesh autour d'un tronc, 27 cm à l'écart du tronc), tout le
  monde arrêté à l'arrivée en 16 s au plus, le bloc retenu 10 pas au plus.
- `hol.Test.Crossings <N> [Seed] [Units] [Type] [quit]` (`Source/HeroesOfLiberty/Dev/HOLCrossingTest.cpp`) et
  `Tools/Tests/crossings.ps1` : la même armée traverse la crête N fois, vers un point au hasard de l'autre côté, puis
  de nouveau, par l'ordre du clic droit. À chaque image : personne dans un rocher, pas de gel (la moitié des marcheurs
  encore loin de l'arrivée qui n'avance pas, 0,6 s au plus) ; puis tout le monde arrêté près du point dans le temps
  permis (chemin du navmesh à 650 cm/s × 1,4 + 5 s). ~7 s de jeu par traversée : 60 traversées en ~8 min.

Rouges de départ : `rocks` 45 relevés dans la pierre ; `hol.Test.Crossings` 6 échecs sur 20, puis 9 à 13 sur 100.
Verts à la fin : `rocks` 17/17 ; traversées graines 1, 2, 3 (50 Hommes d'armes) et 4 (30 Lanciers) 400/400, graines 5
et 6 60/60 ; `tests` (132), `move`, `bodies`, `formation`, `combat` PASS.

## Corrections (dans le code)

1. **Aucun corps dans un obstacle fixe de la carte** (`UHOLFootprintKeepOutProcessor`) : les rochers et les troncs
   (les `StaticMeshActor` qui coupent le navmesh, relevés une fois par carte : 36 sur `Plain` à 2 joueurs) ; une unité
   près de l'un d'eux dont le centre sort du navmesh est remise sur son point le plus proche, à chaque pas, comme les
   bâtiments (D310). Compté par unité (`ObstaclePushes`).
2. **Un membre du bloc collé à un rocher quitte le bloc** (4 pas de suite remis dehors) : chemin à lui jusqu'à sa
   place. Un marcheur sur un chemin ou sur sa ligne droite collé 10 pas : nouveau chemin (comme pour un bâtiment).
3. **Reformage** : un membre dont la place est derrière un obstacle (ligne coupée sur le navmesh) quitte le bloc tout
   de suite ; tant que l'arrière du bloc tourne au dernier coin du chemin, personne ne le retient et il ralentit au
   plus de moitié (avant : arrêt net, l'arrière ne passait jamais le coin, 20 s) ; un bloc qui se traîne 0,5 s (allure
   sous le quart) laisse partir ses membres lents.
4. **Un marcheur près de sa place qui n'avance plus** (3 s sur son chemin à moins de 5 m de sa place) s'arrête là où il
   est, comme la ligne droite finale (avant : un nouveau chemin toutes les 3 s, 15 s à tourner contre les rangs arrêtés
   et le rocher).
5. **Pas de « court déplacement » à travers un obstacle** : l'ordre de groupe à moins d'une taille de bloc allait droit
   aux places, chacun seul ; si la ligne droite du centre ou d'un seul soldat coupe le navmesh, c'est une marche en
   bloc (avant : 50 soldats contournaient le bout du rocher chacun de leur côté et s'y bousculaient, 17 s).
6. **Filet de MV.4.4 ramené de 2 s à 0,5 s** (choix de l'utilisateur, 2026-10-23) : un membre qui retient le bloc qui
   se reforme le quitte au bout de 0,5 s et finit seul (`BlockMemberStuckSteps` 40 → 10 ; test `Formation` mis à jour).

Essai rejeté : faire suivre le chemin à chaque file à sa propre profondeur (le bloc n'arrivait plus, 12 membres
retenant tour à tour ; sauvegarde `scratchpad\files_follow_path.patch`, session 0efba1aa).

## Décisions à porter au GDD

Les six points ci-dessus (§ 16.6, chantier déplacement ; prochain numéro au § 22). Le n° 6 révise la valeur du filet
de MV.4.4 ; le n° 1 étend D310 aux obstacles fixes de la carte.

## Après le commit

- (2026-10-23, signalé par mycrusader-c8) `quick_check -Full` cassait en chaîne : les `hol.Units.Remove` de « rocks »
  (1800 cm) emportaient 3 Paysans du centre, `gather` n'en trouvait plus (« waitunits 21 »). Corrigé :
  `hol.Units.Remove X Y Radius [Slot] [Type]`, « rocks » n'enlève que ses soldats ; « rocks » sort de `-Full` (comme
  « bodies », par `-Parts` seulement : ~4 min de plus dépassaient la limite de 960 s). `-Parts rocks,sources,gather` PASS.

## Reprise exacte

Tout est commité (`main`). Reste : les décisions au GDD (ligne d'`EtapeDefini.md`). Pour revérifier :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\crossings.ps1 -Count 60 -Seed 7`
et `quick_check.ps1 -Parts rocks`.
