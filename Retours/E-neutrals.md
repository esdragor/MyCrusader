# [À ESSAYER] E-neutrals : un camp de 3 Lanciers neutres charge ce qui entre à 15 m, est nettoyé pour toujours et donne 100 XP

STATUT : VU À L'ÉCRAN (le camp gris, la charge, le camp détruit). GESTE : les Hommes d'armes entrent dans le rayon par `hol.Units.Move ... a` (attack-move injecté), pas par Q + clic : le geste réel Q + clic sur un camp est dans E-qground (neutrals_qground).

Pré-vol (2026-11-05 20:45, agent Elrond-sans-C++, worktree HeroesOfLiberty-Elrond = branche tiers, main b39fafd + 88b0813, SANS build) : `preflight.ps1 neutrals -Root ...Elrond` PASS (check ~55 s, HUD sans recouvrement, audit Sanity OK, 16 unités suivies). `neutrals.look` créé (3 points) et trois `capture` ajoutés à neutrals.txt (neutrals_camp, neutrals_fight, neutrals_cleared) avec caméra et `hol.HUD.Report`.
PNG OUVERTS et vus :
- neutrals_camp : 3 petits Lanciers GRIS en haut (parmi des arbres), mes 6 Hommes d'armes bleus en bas ; ils sont petits (zoom par défaut) mais gris, pas bleus ; personne ne bouge.
- neutrals_fight : les Lanciers gris courent sur mes Hommes d'armes bleus (la charge) ; le contact n'est pas encore là et un arbre cache une partie de la scène (décor aléatoire de la carte Plain).
- neutrals_cleared : les Lanciers gris sont au sol (2 corps visibles), mes hommes bleus debout avec leurs barres de vie ; journal « Camp 1: cleared for good », « Camp 1 reward: slot 0 +100 XP ».
DÉFAUT INTERMITTENT (pas de ce scénario) : à un premier passage du pré-vol, l'audit Sanity a donné FAIL (« unit #8 (slot 0) spins over 8 s: path 1136 cm, net 150 cm, turn 12.4 rad » près de (-916, 391)) ; aux deux passages suivants il est OK. Voir BUG-melee-spin-sanity.md.
Bruit du scénario : le Paysan posé à 3 m du camp 0 est tué par un Lancier (« Peasant of slot 0 killed by Spearman of slot 8 ») dès que `hol.Camp.Aggro 0` repasse en agressif : voulu par le scénario (le camp 0 n'est passif qu'avant ce réglage), non corrigé.
NON VÉRIFIÉ : l'XP sur le héros à l'écran (pas de héros dans ce scénario, seul le journal le dit) ; le camp sous brouillard (neutrals_fog, sans .look).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min (tout se déclenche d'office). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
