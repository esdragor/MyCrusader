# [À ESSAYER] E-ai_garrison : tes 8 Hommes d'armes frappent la Tour de guet en bois de l'IA ; les Archers de l'IA arrivent et 3 d'entre eux entrent dans la tour (ils disparaissent de l'écran)

branche integ-25, pas encore dans main

STATUT : VU À L'ÉCRAN   (l'entrée se voit par la DISPARITION : 4 Archers rouges serrés au pied de la tour sur « during », un seul dehors sur « after » ; l'intérieur de la tour n'est pas montré au joueur)
GESTE: interne      (c'est l'IA qui joue : commandes `hol.*` du scénario pour poser la tour, les 8 Hommes d'armes et l'attaque-déplacement ; pas de clic réel)
SON: non entendu   (lancé avec -nosound)
branches: integ-25 (commit bbd8947, scénario seul, aucun C++)

Pré-vol (2026-11-23 15:21, agent ELROND-1123pb, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond) : `preflight.ps1 ai_garrison` PASS entier (31 s, 3 PNG, journal sans avertissement, audit sanity OK), 2 rejeux concordants.
Journal vu : `Garrison order: slot 1, 3 unit(s) queued (AI)` (ordre) puis `Garrison: slot 1 puts 3 of 3 units in TowerWood (3 places, 0 inside before)` (entrée effective, HOLGarrisonSubsystem.cpp:193 ; le scénario attend CE texte, pas une durée).
Ce que le jeu montre d'une garnison : un soldat garnisonné sort de la liste des unités dessinées (HOLUnitSubsystem::GetUnitViews) : il DISPARAÎT. Les occupants répliqués (AHOLBuilding::GarrisonOccupants, COND_OwnerOnly) et le compteur « n / places » ne sont donnés qu'au PROPRIÉTAIRE : le joueur ne voit pas la garnison d'une tour ENNEMIE (information cachée), seule la disparition des archers fait preuve.
PNG OUVERTS et vus (dernier rejeu) :
- ai_garrison_before : la Tour de guet en bois de l'IA (toit rouge, socle blanc, ~105 x 130 px) SEULE au centre, aucun défenseur ; le Forum de l'IA en haut à droite (silhouettes rouges devant).
- ai_garrison_during : gros plan, la tour fait ~300 x 260 px ; les Hommes d'armes bleus à son flanc gauche, un groupe de silhouettes rouges serrées (environ 4, qui se chevauchent : le compte exact n'est pas lisible) au pied de la tour. Pris au moment de l'ordre (les archers apparaissent à ~3 m, on ne les voit pas marcher).
- ai_garrison_after : même cadre ; plus qu'UNE silhouette rouge dehors (le 4e archer : la tour n'a que 3 places), les Hommes d'armes bleus toujours là. Le panneau de notes est en retard d'une note (affiche 1 et 2, pas la 3).
NON VÉRIFIÉ : le compte exact des archers sur « during » (superposés) ; l'avant-entrée en marche (non capturée) ; aucun compteur ni affichage de garnison pour une tour ennemie (par règle, COND_OwnerOnly).

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 ai_garrison -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 35 s. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
