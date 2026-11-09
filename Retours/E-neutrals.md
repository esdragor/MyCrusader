# [LOGIQUE SEULEMENT] E-neutrals : un camp de 3 Lanciers neutres charge mes 6 Hommes d'armes (Q puis clic), est détruit, et mon héros gagne 100 XP

STATUT : LOGIQUE SEULEMENT (jamais compté prêt) : le point « le camp gris est visible au repos » n'est PAS vu à l'écran (le camp est posé au milieu d'un bosquet de la carte, les 3 Lanciers sont cachés derrière les houppiers sur `neutrals_camp`). Vus à l'écran : la charge (`neutrals_fight`, 3 Lanciers gris qui courent sur mes bleus sélectionnés) et la fin (`neutrals_cleared`, un Lancier gris à terre, mes bleus debout, un houppier cache le reste). Les mêmes bugs de mêlée connus (soldats qui tournent ou se figent, D641) ne sont pas jugés ici.
GESTE: réel      (Q = `hol.Select.Grid 4` puis clic gauche au sol = `hol.Select.LeftClickGround`, par les gestionnaires d'entrée comme `neutrals_qground` ; avant ce correctif le scénario donnait l'ordre par `hol.Units.Move` = interne ; souris physique non testée)
SON: non entendu    (-nosound, rien entendu par l'agent)
branches: visual    (le scénario corrigé est sur la branche `visual`, worktree Sauron ; le jeu lui-même = main)

Pré-vol (2026-11-09 16:36, agent Sauron-E, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main e9a349a, binaire du 13:20) : `Tools\Scenarios\preflight.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (53 s, 3 PNG, audit Sanity PASS, aucun HUD qui en recouvre un autre).
Défaut du scénario trouvé et corrigé (commit d5df96a sur `visual`) : AVANT, `neutrals_cleared` était pris 0,4 s après « Camp 1: cleared » et montrait 3 Lanciers gris encore debout (corps en train de tomber) ; il attend maintenant 150 images ; l'ordre est donné par Q puis clic ; la capture du camp est dézoomée (Zoom -3, le fight remonte à Zoom 6).
PNG OUVERTS et vus (Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- neutrals_camp : mes bleus en bas (5 visibles sur 6), le bosquet au milieu ; UN seul Lancier gris à peine visible entre les troncs : le camp est caché. NON conforme au point à voir.
- neutrals_fight : 3 Lanciers gris qui chargent vers mes 6 bleus (sélection ronde, barres de vie vertes), un houppier couvre le coin ; conforme.
- neutrals_cleared : un Lancier gris à terre, mes bleus debout, un houppier cache le centre ; les 2 autres Lanciers ne se voient pas, la mort est prouvée au journal (« Camp 1: cleared for good », « reward: slot 0 +100 XP »).
- Cosmétique sur les 3 captures : le panneau « en préparation… » reste affiché (défaut connu).
NON VÉRIFIÉ : la visibilité du camp au repos (arbres) ; la souris physique ; les 100 XP à l'écran (journal seulement) ; la mêlée à l'œil (mêmes BUG ouverts : spin, stuck).

Sauron-4 (2026-11-09 16:5x, sans C++) : réponse C de Q-neutrals-camp-cache IMPOSSIBLE par commande de test : les arbres du bosquet sont du décor de la carte, pas des sources (`hol.Sources.TakeNear 0 0` n'en coupe aucun). Repli B essayé (plan serré, zoom +6) : les Lanciers gris se voient entre les troncs mais 1 à 2 sont à moitié cachés ; deux pré-vols avec le plan serré ont échoué à l'audit Sanity (spin de mêlée, unités #8 et #9, bug D641) : scénario NON modifié, essai reste LOGIQUE SEULEMENT.
Question (valeur par défaut codée) : voir `Retours\Q-neutrals-trees-decor.md` (clairière impossible sans C++ : commande de décor, plan serré ou camp déplacé) et `Retours\Q-neutrals-camp-cache.md`.
Commande (à NE PAS donner avant le cadrage) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 neutrals -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
