# [À ESSAYER] morale_look : l'effet du moral visible sur une unité

GESTE: réel (touche E par un événement clavier Slate pour produire puis lancer la rage ; clic sur le corps par hol.Select.ClickUnit, chemin du clic souris)
SON: non entendu (-nosound)
branches: morale-look-1125m
statut: VU À L'ÉCRAN (3 PNG ouverts par l'agent, preflight -TestMap PASS 10:32)

Joueur : quand le Guerrier damné lance sa rage nécrotique (un effet de moral), le bloc de sélection montre une pastille de moral verte « Rage nécrotique », sans secondes ; elle disparaît à la fin de la rage.

Ce qu'on voit :
- morale_look_before : Guerrier damné sélectionné, aucune pastille de moral.
- morale_look_during : après la touche E, pastille « ● Rage nécrotique » (vert) sous les statistiques du bloc de selection, case E grisée « Rage 10 s ».
- morale_look_after : rage finie, plus de pastille.

Rejouer (1 appel, ≈ 1 min) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 morale_look -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -TimeoutSec 120`

Réserve : le worktree Galadriel (branche morale-look-1125m) ; carte de test grise (json sans mapSize : avec mapSize Small le clic sur la Caserne rate, voir le journal).
