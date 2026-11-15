# [À ESSAYER] E-talents_icons_compare : les talents de l'Arbre se prennent maintenant par de vrais clics souris (case puis « Choix conseillé »), les images des talents restent dans l'Arbre et le bloc du héros.

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: sans objet
branches: galadriel-tab

À l'utilisateur : cet essai (talents_icons_compare, images des talents) a déjà été joué OK le 2026-11-14 avec le geste interne (hol.HUD.Click). Seul le geste change : la touche T ouvre l'Arbre (vraie touche), puis la case « Boucliers levés » et deux fois « Choix conseillé » sont pris par de vrais clics souris (hol.Mouse.Click : curseur virtuel de Slate, survol, appui, relâche). Rejouer n'est utile que pour confirmer le geste ; l'OK du 2026-11-14 reste valable pour les images.

Pré-vol (2026-11-15 09:36, agent GALADRIEL-1115a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, galadriel-tab dd47594) : `preflight.ps1 talents_icons_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (52 s, 4 PNG). Test rouge feaf662 (waitlog « Mouse click ... x1 at » : rouge tant que hol.HUD.Click), vert dd47594 ; journal : 3 « Mouse click ... on SButton » suivis chacun de « Hero talent order: slot 0 family 1 at step N taken » (55, 77, 120).
PNG OUVERTS et vus :
- talents_icons_window : Arbre ouvert, Boucliers levés, Mains du guérisseur, Pas de l'Aube pris (teintés), images à gauche des noms ; constat neuf : le curseur virtuel resté sur « Choix conseillé » montre la vraie infobulle de Slate dans la capture (« Pas de choix conseillé tant qu'aucun talent n'est pris ou aucun point n'est à dépenser »), sous le bouton, sans rien cacher d'important.
- talents_icons_hero_block : bloc du héros, Niveau 4, trois images de talents sous la liste, à la taille des capacités.
- talents_icons_hero_block_1080 : même bloc en 1920x1080, entier, rien de coupé.
- talents_icons_window_1080 : Arbre rouvert par T en 1080p, cases lisibles, trois cases prises teintées, fenêtre entière.
NON VÉRIFIÉ : le clic avec le vrai curseur du bureau (le verbe passe par le curseur factice de Slate, pas par la souris Windows) ; `session_gate.ps1 -Phase Essay` non lancé.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 talents_icons_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
