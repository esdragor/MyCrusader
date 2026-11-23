# [RÉPONDU : OK + doute sur la volée] E-ai_garrison : tes 8 Hommes d'armes frappent la Tour de guet en bois de l'IA ; les Archers de l'IA arrivent et 3 d'entre eux entrent dans la tour (ils disparaissent de l'écran)

Reponse: OK mais je crois que la tour ne tirait qu'une fleche et pas (1 + nombre d'occupants) * fleche (utilisateur, 2026-11-23 17:00, D899)

STATUT : VU À L'ÉCRAN   (l'entrée se voit par la DISPARITION : plusieurs Archers rouges serrés au pied de la tour sur « during », un seul dehors sur « after » ; l'intérieur de la tour n'est pas montré au joueur)
GESTE: interne      (c'est l'IA qui joue : commandes `hol.*` du scénario pour poser la tour, les 8 Hommes d'armes et l'attaque-déplacement ; pas de clic réel)
SON: non entendu   (lancé avec -nosound)
branches: main

Pré-vol (2026-11-23 16:34, recette indépendante RECETTE-1123pb, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche integ-58 = main 86a2e15 fusionné, 0 fichier de code d'écart avec main, binaire compilé à 16:30) : `preflight.ps1 ai_garrison` PASS (37 s, 3 PNG, journal sans avertissement, audit sanity OK).
Journal vu : `Garrison order: slot 1, 3 unit(s) queued (AI)` puis `AI slot 1 step 149: 1 garrison order(s)` puis `Garrison: slot 1 puts 3 of 3 units in TowerWood (3 places, 0 inside before)` (entrée effective ; le scénario attend CE texte, pas une durée).
Ce que le jeu montre d'une garnison : un soldat garnisonné sort de la liste des unités dessinées : il DISPARAÎT. Les occupants et le compteur « n / places » ne sont donnés qu'au PROPRIÉTAIRE : le joueur ne voit pas la garnison d'une tour ENNEMIE, seule la disparition des archers fait preuve.
PNG OUVERTS et vus (viewport caché 1066x600) :
- ai_garrison_before : la Tour de guet en bois de l'IA (toit rouge, socle blanc, environ 105 x 130 px) SEULE au centre, aucun défenseur autour ; le Forum de l'IA en haut à droite avec 5 à 6 silhouettes rouges à son pied (c'est le Forum, pas la tour).
- ai_garrison_during : gros plan, la tour remplit le centre (environ 300 x 260 px) ; des Hommes d'armes bleus à son flanc gauche ; au pied de la tour 2 à 3 silhouettes rouges serrées qui se chevauchent : le compte exact n'est PAS lisible (pas 4 nettes ; le `.look` a été corrigé en « 2 à 4 »). Pris juste avant l'ordre de garnison (ordre 12 images plus tard).
- ai_garrison_after : même cadre ; UNE seule silhouette rouge dehors à gauche de la tour (le 4e archer, la tour n'a que 3 places), les Hommes d'armes bleus toujours là. Le panneau de notes affiche les notes 1 et 2 (en retard d'une note).
NON VÉRIFIÉ : le compte exact des archers sur « during » (superposés) ; l'avant-entrée en marche (non capturée) ; aucun compteur ni affichage de garnison pour une tour ennemie (par règle) ; le son.

CE QUE TU FAIS ET VOIS (environ 40 s, tu regardes) :
1. Lance la commande ci-dessous (fenêtre visible, rien à cliquer : l'IA joue).
2. VOIS d'abord la Tour de guet en bois de l'IA seule (toit rouge), puis tes 8 Hommes d'armes arriver et la frapper.
3. VOIS ensuite les Archers rouges de l'IA apparaître et se grouper au pied de la tour.
4. VOIS enfin 3 des 4 Archers disparaître d'un coup (entrés dans la tour) : il n'en reste qu'un dehors. Rien n'est affiché sur la garnison ennemie : c'est voulu.

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 ai_garrison -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 40 s. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
