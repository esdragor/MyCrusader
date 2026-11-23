# [RÉPONDU : BUG + design, à rejouer après correction] E-wall_line_long : pose en ligne de la palissade au geste réel (Paysan, touches A puis D, glisser de souris), 9 fondations, Bois 20200 -> 20155

Reponse: BUG: Quand je relache le tracé, ils vont pas automatiquement construire + quand il s'agit de mur en pierre ou en bois, une casese divise en 4 cases plus petite de longeur est largeur et dans le tracé, ce qui est le plus important c'est le debut et la fin, le reste c'est juste un tracé (plus simple pour les ligne ni horizontale ni verticale) (utilisateur, 2026-11-23 17:00, D898)

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: main

Règle (L7-6) : touche de la grille (hol.Select.Grid 0 puis 6 = A puis D, comme le clavier) + vrai glisser de souris (hol.Mouse.Drag) = geste réel.

Pré-vol (2026-11-23 16:35, recette indépendante RECETTE-1123pb, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche integ-58 = main 86a2e15 fusionné + commit scénario 096b1a8, 0 fichier de code d'écart avec main, binaire compilé à 16:30) : `preflight.ps1 wall_line_long` PASS (28 s, 2 PNG, journal sans avertissement, audit sanity OK), 3 passages (le 1er avant retouche du scénario, le 3e après commit).
PNG OUVERTS et vus (viewport caché mesuré 1066x600, pas 1280x720) :
- wall_line_long_during : le Paysan est sélectionné (panneau « Paysan », grille de l'âge I ouverte, touche D = Palissade) ; pendant le glisser une file de boîtes vertes plates alignées : 7 boîtes plates + la dernière encadrée au curseur = 8, pas d'environ 92 px ; aucune boîte rouge ; le Bois affiche 20200 (rien payé). La capture est prise à 70 images sur 90 de glisser (à 30 images, 1er passage, on ne voyait que 4 boîtes : `.look` et scénario corrigés).
- wall_line_long_after : 9 fondations de palissade (cadres bleu-gris) alignées sur la ligne (comptage : Bois 20200 -> 20155 = 45 = 9 x 5 ; étendue du glisser 3357 cm / 400 = 8,4 cases -> 9), la dernière coupée par le bord droit, plus aucune boîte verte.
Journal : `Placement: Palisade (cost F 0 W 5 S 0 G 0)`, `Placement: Palisade wall drag started at (-17711, -1375)`, `Placement: Palisade wall line (-17711, -1375) to (-17711, 1982)`, `RpcOrder: place wall line received by the server: slot 0 type Palisade queue false`.
NON VÉRIFIÉ : la pose avec la souris matérielle d'un vrai joueur en fenêtre visible (ici hol.Mouse.Drag injecte la souris, viewport caché) ; la construction des fondations par les Paysans ; un glisser en diagonale ou croisant un obstacle (une case bloquée refuse toute la ligne : Retours\Q-ligne-case-bloquee.md) ; le son. Le point d'appui du glisser varie d'un rejeu à l'autre (y monde -2378, -1375) selon la caméra : sans effet sur le résultat.

CE QUE TU FAIS ET VOIS (fenêtre visible, environ 1 à 2 min) :
1. Lance la commande ci-dessous. Le scénario met 10 Paysans dans le champ, la Palissade est débloquée (Maçonnerie), Bois 20200.
2. Clic gauche sur un des Paysans bleus (panneau « Paysan » en bas à gauche, grille de l'âge I au centre). Appuie sur la touche A (âge I), puis D (Palissade : la souris porte un fantôme).
3. Sur le sol libre à l'ouest du Centre-ville, appuie clic gauche, GARDE le bouton et glisse horizontalement vers la droite d'environ 8 cases (environ 800 px), sans relâcher.
4. VOIS pendant : une file de boîtes vertes plates, une par case, du point d'appui au curseur ; aucune boîte rouge ; le Bois reste 20200.
5. Relâche. VOIS : 9 fondations bleu-gris alignées, plus de boîte verte, Bois 20155 (5 par case).
6. Ensuite, essaie une diagonale, puis une ligne qui croise un arbre ou un bâtiment : la ligne entière doit être refusée (boîtes rouges).
Si le scénario joue lui-même un glisser avant que tu prennes la main, ne touche pas la souris pendant ce glisser automatique.

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 1 à 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
