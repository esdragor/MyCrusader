# [À ESSAYER] E-deploy : D monte puis démonte le Mangonneau (la case D change de libellé avec une barre de progression), un clic droit au sol le démonte puis le fait marcher, un clic droit sur la Maison ennemie le monte puis il tire

STATUT : VU À L'ÉCRAN
GESTE: interne      (hol.UI.Key D = événement clavier Slate, le chemin d'une vraie touche D ; mais les clics droits sont `hol.Select.OrderNear`, un appel interne : l'essai reste LOGIQUE SEULEMENT pour le geste tant que la ligne dit interne)
SON: non entendu
branches: main, integ-25 24c73b7

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-20 00:00, agent ELROND-1120sc, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main c57e12c) : `preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (40 s, 13 PNG). Correctif de scénario (L18-f) : `wait 2` / `wait 1` avant la relecture du HUD après chaque `delay over` (le HUD disait encore Mounting7 / Dismounting6 douze ms après), commit 24c73b7 sur integ-25.
PNG OUVERTS et vus (deux planches de contact, 13 captures, ordre de lecture gauche-droite puis haut-bas) :
- deploy_packed : Mangonneau sélectionné, case D = « Monter », Maison ennemie intacte.
- deploy_deploying : case D = « Montage… [#-------] » (1 dièse, PENDANT le montage), engin pas encore monté.
- deploy_deployed : case D = « Démonter », engin monté.
- deploy_fire : Maison sélectionnée, 555/600 (un coup de 45).
- deploy_fire_hit : Maison 510/600 (un second coup de 45).
- deploy_packing : case D = « Démontage… [##------] » (2 dièses, PENDANT le démontage).
- deploy_packed_again : case D redit « Monter » (retour au 2e cycle).
- deploy_move_start : engin monté, case D = « Démonter ».
- deploy_move_unmounting : trait de destination jaune, case D lit encore « Démonter » (capture 125 ms après l'ordre, le libellé « Démontage… » n'apparaît qu'ensuite, journal 22:56.48.934) ; l'engin n'a pas bougé.
- deploy_move_walked : engin plus bas et à gauche de move_start, case D = « Monter » (déplacement modeste, visible).
- deploy_attack_mounting : clic droit sur la Maison, case D = « Monter » (pas encore « Démonter »), trait de visée vers la Maison.
- deploy_attack_mounted : case D = « Démonter », engin monté.
- deploy_attack_hit : Maison sélectionnée, 465/600 (trois coups de 45).
NON VÉRIFIÉ : le son (-nosound) ; une vraie touche D puis un vrai clic droit à la souris (les ordres au sol et sur la Maison sont des appels internes hol.Select.OrderNear) ; l'ordre exact des planches (étiquettes absentes de l'image, déduit des horodatages) ; move_unmounting : le libellé « Démontage… » n'est pas visible sur la capture (délai de rafraîchissement de la case, non investigué).

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
