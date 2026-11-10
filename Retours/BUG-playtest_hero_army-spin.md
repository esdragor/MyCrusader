# BUG-playtest_hero_army-spin : l'audit Sanity échoue à chaque passage de playtest_hero_army (soldat qui tourne sur place en mêlée)

Découvert par : agent d'intégration n°2, 2026-11-05 21:19 et 21:23, main eb7b5fe (binaire 21:07). Même famille que BUG-melee-spin-sanity.md (D641 « boucles mêlée »), mais ici 2 passages sur 2.

GESTE EXACT : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\preflight.ps1 playtest_hero_army -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` (le scénario appelle `hol.Test.Sanity` en fin de run, après les combats A et B).
VU (journal du pré-vol) :
- passage 1 : `Sanity: FAIL unit #18 (slot 0) at (-15562, 665) spins over 8 s: path 312 cm, net 144 cm, turn 20.2 rad (net 10.6, to and fro 1.3), 3 reversals, ordered 100%`
- passage 2 : `Sanity: FAIL unit #16 (slot 0) at (-15541, 709) spins over 8 s: path 126 cm, net 95 cm, turn 16.2 rad (net 15.5, to and fro 0.4), 5 reversals, ordered 100%`
Contexte : slot 0 = mes Hommes d'armes (6 + héros), mêlée contre 10 Lanciers près du forum (x ~ -15500, y ~ 700), combat B (héros mort) en cours quand l'audit mesure ; unité « ordonnée à 100 % » = elle a un but et ne l'atteint pas.
Le check lui-même (« ready », aucun warning) est PASS ; seul l'audit Sanity échoue : le preflight rend FAIL.
Pas de correctif tenté (classe sans C++, simulation). Pas de question de design.
NON VÉRIFIÉ : si le tour sur place se voit à l'écran (PNG : pas de défaut visible, la mêlée est une grappe) ; si c'est l'unité du combat A (non finie quand B commence) ou du B.

## Cadrage corrige (agent scenarios, 2026-11-05 21:34, commit c5969ef sur tiers) : attente 45 s par combat (rapports : A kills 10 fighting 0, B kills 20 cumules fighting 0), combat B a y=6000 hors de la vue de A. PNG vus : b_start dans une zone propre, a_end/b_end montrent survivants et corps. Le spin Sanity RESTE (defaut du jeu, D641) : preflight FAIL, passage 3 : unites #28/#27 (slot 1) et #18 (slot 0) apres la fin des combats ; essai reste LOGIQUE SEULEMENT.

## Rejeu 2026-11-10 17:43-17:49 (session principale, sans C++, binaire du 13:10 = a6022e1) : toujours 0/4
Passes 1, 2, 4 : #18 slot 0 « stalls over 2.2 s out of reach of #19 (ou #21), 108-120 cm short of its edge, with a free place » : c est l audit de STALL (cible hors de portee avec une place libre), pas un spin, et il est deterministe (meme position (-15546, 673) aux passes 1 et 2). Passe 3 : #32 slot 0 spin a y = 6024 (combat B). Piste pour un agent C++ : #18 s arrete a ~110 cm de la portee devant une cible alors qu une place est libre.
