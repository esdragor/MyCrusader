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
Lancer UNE commande (à jour le 2026-11-19, worktree Arwen = main) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 specialization_button -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Pré-vol rejoué le 2026-11-19 22:03 (session principale, worktree Arwen = main e4605c4, binaire à jour) : PASS (37 s, audit Sanity OK). À 21:55 il échouait : le scénario relisait la case 10 images après l'annulation, avant que la grille (maintenant en script) ait reçu la réponse du serveur ; corrigé dans le scénario seul (`wait 2`, fc10863) + `mapSize Small` (D820) ; les cases de spécialisation existent bien dans la grille en script (`HOLCommandGridRules.as:100-107`). Les 3 captures ouvertes et vues : case D « Voie A du palier 1 » et case F « Voie B du palier 1 » dans leurs cases ; après le clic, case D dorée « 28 s » puis « Annul… » sur 2 lignes ; après le second clic, case D revenue à « Voie A du palier 1 ». Défauts toujours là, connus : l'infobulle (souris sur D) couvre le bas de la case ; le libellé de la case W « Cloche d'alarme améliorée / Bientôt » déborde.
Durée : environ 2 min (3 min max). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse (2026-11-19 ~22:55) : OK (essai joue sur main, scenario fc10863). Reporte dans SpecializationInProgress.md. Fiche archivee dans Docs\Archive\Retours_2026-11-19\.
