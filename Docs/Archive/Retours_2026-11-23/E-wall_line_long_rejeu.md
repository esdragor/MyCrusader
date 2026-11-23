# E-wall_line_long (rejeu) : trace une ligne de palissade au glisser de souris, relâche, et vois ton Paysan sélectionné aller bâtir les fondations

Reponse: OK (utilisateur, 2026-11-23 17:4x)

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: sans objet
branches: main

Rejeu après correction du BUG du 2026-11-23 17:00 (« quand je relâche le tracé, ils ne vont pas automatiquement construire », correctif ARWEN-1123pe : le RPC de la ligne envoie maintenant les Paysans sélectionnés). Fiche neuve : l'ancienne est archivée.

Pré-vol (2026-11-23 17:35, agent RECETTE-1123pd, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main b5299e5 + commit de scénario 66cb4c9 sur la branche du worktree) : `preflight.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` PASS (28 s, 3 PNG), rejoué après le commit du scénario.
Journal (Saved_Tests\Logs\scenario_wall_line_long.log) après le relâché : « Placement: 1 units sent to build the line » ; « RpcOrder: place wall line received by the server: slot 0 type Palisade builders 1 queue false » ; 9 fondations posées (5 Bois chacune) ; « Build order of slot 0: 1 units to Palisade at (-17800, -1400) » puis 8 ordres « queued » ; ~5,4 s plus tard « Construction: slot 0 first work on Palisade at (-17800, -1400), 1 builders » puis « finished ... instant », puis la 2e (-1000) et la 3e (-600) : le Paysan enchaîne les fondations l'une après l'autre.
PNG OUVERTS et vus :
- wall_line_long_during : 7 boîtes vertes plates alignées du point d'appui au curseur (la dernière en perspective à droite), aucune boîte rouge, Bois 20200 (inchangé).
- wall_line_long_after : 9 fondations (cadres bleutés) sur la ligne, plus de boîte verte, Bois 20155 (−5 par case) ; le Paysan sélectionné est en marche (panneau « En marche ») vers la ligne, trajet blanc visible.
- wall_line_long_build (5 s après le relâché) : le Paysan est au bout gauche de la ligne, contre les 3 premières cases devenues pleines (blocs bleus pleins) ; les autres restent en cadres (fondations en attente).
NON VÉRIFIÉ : (1) la construction RÉELLE avec durée et barre d'avancement : le scénario pose `hol.Build.Instant 1`, donc chaque fondation est finie au premier coup du Paysan ; on voit le trajet et l'enchaînement, pas la barre ni la hauteur qui monte ; (2) un seul Paysan est sélectionné : le cas de plusieurs Paysans sélectionnés (tous reçoivent tous les ordres, règle de répartition par défaut d'ARWEN-1123pe, non tranchée par le GDD) n'est pas rejoué ; (3) l'ordre de relâché avec Maj (ligne suivante) et la ligne diagonale / bloquée (note 4, à toi) ; (4) rien n'est vu en réseau (client contre serveur dédié) ; (5) le scénario a été retouché (attente + capture « build »), le correctif est dans le binaire compilé à 17:30 de ce worktree, pas dans une copie de main différente.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Ce que tu fais : le scénario sélectionne le Paysan et lance le glisser tout seul (ne bouge pas la souris pendant ~20 s) ; ensuite, à toi : une ligne en diagonale, puis une ligne qui croise un arbre ou un bâtiment (refusée en entier). Regarde surtout si ton Paysan sélectionné part tout seul bâtir au relâché (avant : il restait « Inactif »).
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
