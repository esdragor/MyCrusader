# [À ESSAYER] E-wall_line_skip : tracer une ligne de palissade qui croise une Maison : la case bloquée reste un trou rouge, les autres sont posées (D901)

STATUT : VU À L'ÉCRAN (recette indépendante HALDIR-1124l, 2026-11-24 12:08, sur main 9d40517 fusionné et compilé : PASS)
GESTE: réel
SON: sans objet
branches: integ-41 (dans main 9d40517 : scénario 7644229 et correctif présents, vérifié par fusion de main dans le worktree Haldir ; le tip integ-41 n'est pas ancêtre de main, l'intégration a repris le contenu)

Impact joueur : une case occupée sur le tracé n'annule plus toute la ligne ; seules les cases libres sont posées et payées.

Recette HALDIR-1124l (2026-11-24 12:07, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche integ-58 = main 9d40517 fusionné, build PASS 92 s) : `preflight.ps1 wall_line_skip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` PASS (34 s, 2 PNG), 0 erreur au journal. PNG OUVERTS et vus par l'agent de recette (sans se fier aux fiches précédentes) :
- wall_line_skip_during : PASS. Une file de dalles vertes plates (une par case) le long de la ligne ; DEUX dalles rouges/orangées au milieu, sous un cadre filaire gris (la Maison posée) ; les autres dalles restent vertes, la ligne n'est pas refusée.
- wall_line_skip_after : PASS. 7 fondations de palissade (cadres filaires) de part et d'autre de la Maison, trou net à la place de la Maison, plus aucune dalle verte ou rouge ; Bois 20150 -> 20115 = 7 x 5.
NON VÉRIFIÉ : son (sans objet) ; le cas « toutes les cases bloquées = refus Occupied » n'est pas rejoué à l'écran ; la Maison est posée par `hol.Build.PlaceNear` (interne), seul le glisser est un geste réel ; `quick_check -Parts tests` non lancé.

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 wall_line_skip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
(le worktree Haldir contient main 9d40517 : le correctif est dans main 9d40517, compilé)
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Recette RECETTE-1125mc sur main 7922539 (2026-11-25 13:50, worktree Celeborn, binaire 12:37, preflight PASS, 2 PNG ouverts) : PASS. wall_line_skip_during : file de dalles vertes plates, 2 dalles rouges sous la Maison (cadre filaire), ligne acceptee ; wall_line_skip_after : 7 fondations (4 a gauche, 3 a droite), trou net a la place de la Maison, plus de dalle verte ou rouge ; Bois 20150 -> 20115 = 7 x 5.
Reponse: OK (2026-11-25 14:0x, joue par l utilisateur sur main 7922539, dans la conversation)
