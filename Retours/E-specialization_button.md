# [À ESSAYER] E-specialization_button : rejeu du défaut de débordement : la case D en cours n'écrit plus que « N s » et « Annuler » sur 2 lignes, sans déborder sur la case C.

STATUT : VU À L'ÉCRAN (débordement corrigé ; un défaut restant vu : l'infobulle couvre le bas de la case, ci-dessous)
GESTE: réel      (hol.Mouse.Click GridCell06 : curseur de Slate, survol/appui/relâche)
SON: sans objet  (lancé avec -nosound ; aucun son attendu pour ce bouton)
branches: tiers

Pré-vol (2026-11-16 17:18, agent ELROND-1116a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, tiers 76ab50a, main fusionné 1ede5b5) : `preflight.ps1 specialization_button -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 34 s, 3 PNG, sanity OK). Le scénario attend maintenant les cases dans le rapport du HUD (`=SpecializeT1A enabled`, `=CancelSpecializationT1A enabled`).
PNG OUVERTS et vus :
- specialization_button_open : Centre principal sélectionné ; case D « Voie A du / palier 1 », case F « Voie B du / palier 1 », 2 lignes chacune, dans la case.
- specialization_button_running : case D surlignée (or), 2 lignes seulement : « N s » (29 ou 20, illisible à 1066x600) puis « Annul… » ; plus de 3e ligne, plus rien sur la case C (avant : « Voie A du palier 1 / En cours : 29 s / Annuler » débordait). DÉFAUT RESTANT : l'infobulle (souris restée sur D) commence au milieu de la 2e ligne de la case et coupe le bas de « Annuler ».
- specialization_button_cancelled : case D revenue à « Voie A du / palier 1 » (bas de « palier » sous l'infobulle, même défaut), choix rouvert.
NON VÉRIFIÉ : lisibilité à 1080p ; valeur exacte de N ; position de l'infobulle (non corrigée : C++ arrêté le 2026-11-16) ; libellés Voie A/B provisoires ; réseau à 2 clients.

À essayer : clique la case D, puis regarde-la pendant le décompte : « N s » et « Annuler » tiennent-ils dans la case ? L'infobulle cache-t-elle le bas de la case quand la souris est dessus ?

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 specialization_button -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 2 min (3 min max). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
