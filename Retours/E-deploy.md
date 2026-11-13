# E-deploy : la Mangonnelle (substitut du Trebuchet) se monte et se demonte par la touche D (case D : Monter / Demonter)

STATUT : LOGIQUE SEULEMENT (pre-vol FAIL : NE PAS JOUER)
GESTE: interne      (hol.Select.Grid 6, equivalent de la touche D, pas une vraie touche)
SON: non entendu    (-nosound)
branches: tiers

Pre-vol 2026-11-13 13:56 (ELROND-1113c, tiers 27d91a4, build 13:52) = FAIL (6), 93 s, timeout etape 35 'waitlog D=Pack'.
- Le journal ajoute au HOLDeployProcessor montre que l'engin SE MONTE : 'Deploy: #13 phase 0 -> 1 (order 3...)' puis 8 s apres 'phase 1 -> 2 (delay over)'.
- Le defaut est dans le SCENARIO : hol.HUD.Report ne dit pas 'D=Pack' une fois monte (probablement le nom de commande fixe). A corriger dans deploy.txt (waitlog sur la ligne 'Deploy: #') ou dans le rapport HUD.
- La piste 'Maison a 5250 > portee 5000' est fausse : la Maison est a ~2200 cm.
- Correctif voisin fait : D presse pendant une marche ne s'annule plus (27d91a4).

NON VERIFIE : tir sur la Maison, demontage, PNG (seul deploy_packed produit, NON OUVERT).
