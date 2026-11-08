# [À ESSAYER] E-forum_review2 : voir dix nouvelles variantes du Forum (U à AD, mélanges de tes préférées), une par une, et dire lesquelles tu aimes ou pas

STATUT : VU À L'ÉCRAN
GESTE: interne      (la touche Entrée passe à la variante suivante par un waitkey du scénario ; aucune souris, aucun geste de jeu)
SON: sans objet
branches: visual

Contexte : tu avais aimé K, L, M, N, O, Q, T (pas aimé P, R, S) avec le drapeau à remonter sur L et Q. Ces dix variantes mélangent les traits aimés (tourelles hautes de L, trois étages de Q, toit pointu de N, toit bas de O, enceinte de M, maisons de T), sans portail, sans plate-forme crénelée, sans portique. Le drapeau est posé sur la couronne de la tourelle et monte au-dessus du donjon sur U, V, W, X, Y, AB, AD ; Z, AA, AC gardent le drapeau ordinaire de K.

Préflight (2026-11-08 12:03, agent Sauron-6, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual = main fc601b7 + commit du tour 2) : `preflight.ps1 forum_review2 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (10 PNG, 9 waitkey, 0 s d'attente morte). WARN connu : pas de ligne « HUD overlap ».
PNG OUVERTS et vus (en montage de 10 vues, Saved_Tests\Screenshots\WindowsEditor\HOL\fr2_U.png à fr2_AD.png) :
- fr2_U : tourelles hautes (3 niveaux), toit en pyramide, drapeaux sur la couronne des tourelles, au-dessus d'elles
- fr2_V : donjon de trois étages, tourelles hautes, drapeaux très hauts (le haut des mâts sort un peu du cadre)
- fr2_W : tourelles hautes, toit pointu
- fr2_X : trois étages et toit très pointu, mâts les plus hauts (sortent du cadre)
- fr2_Y : tourelles hautes et deux maisons de chaque côté du donjon
- fr2_Z : toit pointu, une maison de chaque côté, tourelles basses, drapeau ordinaire
- fr2_AA : enceinte basse avec tourelles d'angle et toit pointu
- fr2_AB : enceinte à tourelles hautes, donjon de trois étages, drapeaux hauts
- fr2_AC : toit bas et large, maisons de chaque côté, drapeau ordinaire
- fr2_AD : trois étages, deux maisons de chaque côté, drapeaux hauts
Chaque vue : le Forum est seul au milieu d'un sol dégagé, loin du Forum de la partie ; la lettre est écrite en jaune sur le sol devant lui. Les dix sont distincts (hauteur des tourelles, hauteur et pente du toit, enceinte, maisons).
Lancement par play.ps1 : le jeu démarre, la scène s'affiche et le scénario attend la touche Entrée (journal : « Forum review: letter=U shown=1 », « Scenario waits for the key »). Le panneau du scénario est caché pendant la revue (panel hide) : rien n'indique « en préparation ».
NON VÉRIFIÉ : jugé sur des captures de 1066 x 600 px ; le haut des mâts de V, X, AB et AD sort légèrement du cadre de la caméra ; les drapeaux sont gris (la couleur du joueur n'est pas appliquée dans cette revue) ; la capture d'écran de mon lancement par play.ps1 montrait la fenêtre d'un autre jeu (fog_trees d'un autre agent, même worktree) : l'état de mon jeu est prouvé par son journal, pas par une image ; les 10 modèles sont des provisoires de revue (rien n'est branché dans le jeu).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 forum_review2 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 2 min (Entrée = variante suivante, la lettre est écrite à l'écran). Les lettres : U, V, W, X, Y, Z, AA, AB, AC, AD.
Réponse attendue : `aimé / pas aimé` par lettre (ex. `aimé : U, AB / pas aimé : Z, AC`), plus ce qui doit changer (drapeau, toit, tourelles...), ou `BUG: ...`.
