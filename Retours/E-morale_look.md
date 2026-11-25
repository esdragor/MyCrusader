# [À ESSAYER] morale_look : l'effet du moral visible sur une unité

GESTE: réel (touche E par un événement clavier Slate pour produire puis lancer la rage ; clic sur le corps par hol.Select.ClickUnit, chemin du clic souris)
SON: non entendu (-nosound)
branches: morale-look-1125m, morale-look-fix-1125m
statut: VU À L'ÉCRAN (3 PNG ouverts par l'agent 14:05 : Guerrier ~80 px, violet pendant la rage, bleu avant/après ; GALADRIEL-1125mf, commit 90bad92)

Joueur : quand le Guerrier damné lance sa rage nécrotique (un effet de moral), le bloc de sélection montre une pastille de moral verte « Rage nécrotique », sans secondes ; elle disparaît à la fin de la rage.

Ce qu'on voit :
- morale_look_before : Guerrier damné sélectionné, aucune pastille de moral.
- morale_look_during : après la touche E, pastille « ● Rage nécrotique » (vert) sous les statistiques du bloc de selection, case E grisée « Rage 10 s ».
- morale_look_after : rage finie, plus de pastille.

Rejouer (1 appel, ≈ 1 min) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 morale_look -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -TimeoutSec 120`

Réserve : le worktree Galadriel (branche morale-look-fix-1125m) ; mapSize Small ajouté (clic Caserne corrigé : -13800 2600, point de la Caserne sur la carte Small), caméra Zoom +12 (Guerrier ~80 px, teinte violette sur le bleu du joueur).
Recette RECETTE-1125md sur main 7922539 (2026-11-25 13:49, worktree HeroesOfLiberty-Faramir, branche recette-1125md) : FAIL. preflight morale_look FAIL (1) : 'morale_look.json has no mapSize, the trial plays on the grey test map (D820)' ; check PASS (44 s) ; les 3 PNG ouverts et vus : before OK (Guerrier damne selectionne, bloc du bas avec stats, aucune pastille), during : pastille verte 'Rage necrotique' bien visible sous les stats + case E 'Rage 10 s', MAIS le Guerrier est minuscule (~15 px, violet) : la teinte rouge sombre du .look n'est pas discernable ; after : pastille disparue, E 'Rage 27 s', unite minuscule sans teinte lisible. Raison du FAIL : carte de test grise (mapSize absent du .json) + cadrage trop lointain pour voir la teinte. Correctif : ajouter mapSize Small au .json et cadrer FocusNear/Zoom sur le Guerrier. Rien corrige. GESTE: reel, SON: non entendu.
