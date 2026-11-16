# [À ESSAYER] E-arquebusier : le premier tir de l'Arquebusier fait passer un Homme d'armes de 155 à 117 PV (pénétration d'armure), sur une carte générée (sol d'herbe, décor)

STATUT : VU À L'ÉCRAN
GESTE: interne      (les clics de la préparation passent par `hol.Select.ClickUnit` ; l'essai porte sur les PV, pas sur le geste ; à toi ensuite : vrais clics, note 4)
SON: sans objet     (l'Arquebusier n'a pas encore de son de tir : Placeholders.txt)
branches: visual

Pré-vol (2026-11-16 15:39, agent SAURON-1116b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, visual b1e97e3) : `preflight.ps1 arquebusier -AllowUncommitted` PASS (~1 min, 2 PNG) ; le fichier non commité au moment du pré-vol (arquebusier.json) est commité depuis (b1e97e3).
Changements depuis le dernier essai : l'Arquebusier naît APRÈS la lecture des PV de l'Homme d'armes (a1f559f : posé à portée, il tire tout seul) ; la partie se joue sur la Plaine générée petite (`mapSize Small`, D820) et non plus sur la carte de test grise.
PNG OUVERTS et vus :
- arq_before : sol d'herbe de la Plaine générée, un arbre en haut à gauche ; l'Homme d'armes rouge seul, sélectionné, panneau « Points de vie 155 / 155 » ; pas encore d'Arquebusier.
- arq_hit : l'Arquebusier bleu au contact de l'Homme d'armes (celui-ci a couru vers lui) ; panneau « Points de vie 117 / 155 ».
NON VÉRIFIÉ : vrais clics souris de la note 4 (geste interne dans la préparation) ; l'avis « Shaders en préparation (2) » apparaît en haut à gauche sur arq_before (carte générée, premier lancement : il disparaît seul) ; le duel complet de la note 5 n'est pas rejoué sur la carte générée.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 arquebusier -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: OK (utilisateur, 2026-11-16 16:3x, dans la conversation)
