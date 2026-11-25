# LOGIQUE SEULEMENT (pas prêt, ne pas lister) : E-ai_tower_easy : l'IA Facile pose et achève toute seule une Tour de guet en bois (D918)

STATUT : LOGIQUE SEULEMENT   (la tour n'est pas identifiable sans doute sur le PNG, voir ci-dessous ; jamais listé prêt)
GESTE: interne      (aucun geste : on regarde l'IA jouer ; Lanciers posés par `hol.Units.SpawnHere`, caméra par `hol.Camera.FocusNear`)
SON: sans objet
branches: morale-look-fix-1125m

Pré-vol (2026-11-25 16:03, agent GALADRIEL-1125d, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, tip 1acb6a2) : `preflight.ps1 ai_tower_easy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (111 s, 1 PNG, audit Sanity OK : 1536 pas, 18 unités suivies, 0 arrêt > 0,5 s).
Journal : `AI slot 1 step 1114: build TowerWood at (14800.0, 0.0) by #11 (level 0, 0 already, an army of 5)` puis `Construction: slot 1 finished TowerWood at (14800, 0)` ; `Camera near: (14800, 0)` (le repère `enemy` de `hol.Camera.FocusNear` est MIROIR : 16400 - dx, donc `1600 0 enemy`).
PNG OUVERTS et vus :
- ai_tower_easy_built : caméra centrée sur (14800, 0) ; au centre un bloc à base grise et toit rouge (couleur de l'IA), entre trois Lanciers rouges (bas) et le Forum de l'IA (haut). Il a le même gabarit provisoire que les maisons voisines : la tour n'est reconnue que par sa position (journal), pas par une silhouette propre.
NON VÉRIFIÉ : que ce bloc est bien la tour (position seule) ; le spin du Paysan (BUG-ai_peasant-spin) s'est vu au run 1125b de 14:47 mais PAS à ce run (wait 55 au lieu de 110) : intermittent, non corrigé (ligne C++ PushedBacks en tête de `Auto_Journey_File.md`) ; aucun geste réel ; son non concerné.

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 ai_tower_easy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
