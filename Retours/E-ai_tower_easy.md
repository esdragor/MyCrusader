# VU À L'ÉCRAN (GALADRIEL-1125f 16:43, tip 4f3c147) : E-ai_tower_easy : l'IA Facile pose et achève toute seule une Tour de guet en bois (D918)

STATUT : VU À L'ÉCRAN   (GALADRIEL-1125f 2026-11-25 16:4x, tip 4f3c147 compilé : preflight PASS x2 (109 s et 110 s), PNG ouvert x2, bloc de la tour au centre du cadrage 1600 0 enemy = (14800, 0) du journal ; reconnu par la position, même gabarit provisoire que les maisons)
GESTE: interne      (aucun geste : on regarde l'IA jouer ; Lanciers posés par `hol.Units.SpawnHere`, caméra par `hol.Camera.FocusNear`)
SON: sans objet
branches: morale-look-fix-1125m

Pré-vol (2026-11-25 16:03, agent GALADRIEL-1125d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, tip 1acb6a2) : `preflight.ps1 ai_tower_easy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (111 s, 1 PNG, audit Sanity OK : 1536 pas, 18 unités suivies, 0 arrêt > 0,5 s).
Journal : `AI slot 1 step 1114: build TowerWood at (14800.0, 0.0) by #11 (level 0, 0 already, an army of 5)` puis `Construction: slot 1 finished TowerWood at (14800, 0)` ; `Camera near: (14800, 0)` (le repère `enemy` de `hol.Camera.FocusNear` est MIROIR : 16400 - dx, donc `1600 0 enemy`).
PNG OUVERTS et vus :
- ai_tower_easy_built : caméra centrée sur (14800, 0) ; au centre un bloc à base grise et toit rouge (couleur de l'IA), entre trois Lanciers rouges (bas) et le Forum de l'IA (haut). Il a le même gabarit provisoire que les maisons voisines : la tour n'est reconnue que par sa position (journal), pas par une silhouette propre.
NON VÉRIFIÉ : que ce bloc est bien la tour (position seule) ; le spin du Paysan (BUG-ai_peasant-spin) s'est vu au run 1125b de 14:47 mais PAS à ce run (wait 55 au lieu de 110) : intermittent, non corrigé (ligne C++ PushedBacks en tête de `Auto_Journey_File.md`) ; aucun geste réel ; son non concerné.
Rejeu 4f3c147 (GALADRIEL-1125f, 16:34-16:43) : 2 runs preflight ai_tower_easy, PASS les deux, Sanity OK les deux (1531 et 1534 pas, 18 unités, aucun spin) ; journal : build TowerWood at (14800.0, 0.0) step 1114 puis finished TowerWood at (14800, 0) ; Camera near: (14800, 0). Le spin n'est pas apparu en Facile (intermittent) : le correctif n'est pas prouvé par ce scénario, voir Retours\BUG-ai_peasant-spin.md (ai_tower Normale : spin encore vu).

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 ai_tower_easy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
recette 8d79aaa : PASS 17:22 (GLORFINDEL-1125d, Haldir integ-85 = main 8d79aaa) : preflight ai_tower_easy PASS (sanity OK, 0 spin au journal), PNG ai_tower_easy_built ouvert : bloc a toit rouge (gabarit provisoire des maisons) au centre du cadrage (14800, 0) entre 3 Lanciers et le Forum ; journal step 1114 build TowerWood at (14800,0) level 0, foundation payee W 100, finished apres 352 pas. Tour reconnue par la position seule (pas de silhouette propre).
