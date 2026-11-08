# [À ESSAYER] E-forum_review : voir dix variantes du Forum (K à T), une par une, et dire lesquelles tu aimes ou pas

STATUT : VU À L'ÉCRAN
GESTE: interne      (la touche Entrée passe à la variante suivante par un waitkey du scénario ; aucune souris, aucun geste de jeu)
SON: sans objet
branches: visual

Préflight (2026-11-08 11:5x, agent Sauron-2, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main c158640 fusionné) : `preflight.ps1 forum_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (66 s, 10 PNG, 9 waitkey, 0 s d'attente morte).
PNG OUVERTS et vus (en montage de 10 vues, Saved_Tests\Screenshots\WindowsEditor\HOL\fr_K.png à fr_T.png) :
- fr_K : donjon symétrique, toit en pyramide, 4 tourelles d'angle, mât et drapeau sur chacune, « FORUM K » en jaune
- fr_L : tourelles hautes, lettre lisible
- fr_M : enceinte basse avec tourelles d'angle
- fr_N : toit pointu plus haut
- fr_O : toit plus bas et large
- fr_P : portail et escalier sur l'axe (l'escalier cache un peu la fin de l'étiquette)
- fr_Q : trois étages
- fr_R : plate-forme crénelée et petite pyramide
- fr_S : portique de colonnes, sans tourelle
- fr_T : maisons de chaque côté
Chaque vue : le Forum est seul au milieu d'un sol dégagé, loin du Forum de la partie ; la lettre est écrite en jaune sur le sol devant lui.
NON VÉRIFIÉ : jugé à la main uniquement sur des captures de 1066 x 600 px (le jeu en grand écran peut montrer plus de détail) ; K, N et O se ressemblent vus d'en haut ; le message du moteur « Champs de distance des maillages en préparation » peut apparaître en haut à gauche sur les premières vues ; les 10 modèles sont des provisoires de revue (hors jeu, rien n'est branché).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 forum_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min (Entrée = variante suivante, la lettre est écrite à l'écran). Les lettres : K, L, M, N, O, P, Q, R, S, T.
Réponse attendue : `aimé : <lettres> / pas aimé : <lettres>` (ex. `aimé : K, S / pas aimé : N, O`), ou `BUG: ...`.
