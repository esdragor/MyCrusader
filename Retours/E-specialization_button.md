# [À ESSAYER] E-specialization_button : Forum sélectionné au palier 1, un vrai clic sur la case D lance « Voie A du palier 1 » (En cours : N s / Annuler), un second clic l'annule.

STATUT : VU À L'ÉCRAN (avec un DÉFAUT vu sur la case en cours, ci-dessous)
GESTE: réel      (hol.Mouse.Click GridCell06 : curseur de Slate, survol/appui/relâche, modèle talents_icons_compare)
SON: non entendu (lancé avec -nosound ; aucun son attendu pour ce bouton)
branches: tiers

Pré-vol (2026-11-15 14:09, agent ELROND-1115i, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 6a77f35 fusionné) : `preflight.ps1 specialization_button -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 37 s, 3 PNG, sanity OK, aucun chevauchement de panneaux signalé).
Journal du serveur vu : « Specialization: slot 0 starts tier 1 option 0, answer 0 » puis « Specialization: slot 0 cancels, answer 0 ».
PNG OUVERTS et vus :
- specialization_button_open : Centre principal sélectionné ; grille en bas à gauche (à droite du cadre de sélection) : A = Paysan, case D (2e rangée, 3e colonne) = « Voie A du palier 1 », case F = « Voie B du palier 1 », même taille que les autres cases, texte entier sur 2 lignes ; W = « Cloche d'alarme améliorée Bientôt » (grisée). Aucune file de production affichée, donc pas de chevauchement avec une file.
- specialization_button_running : case D surlignée (or) ; DÉFAUT : le texte à 4 lignes « Voie A du palier 1 / En cours : 29 s / Annuler » ne tient pas dans la case : « En cou… » coupé, « 29 s » et « Annul… » débordent sur la case C en dessous. L'infobulle (souris restée sur D) « Voie A du palier 1 / Déblocage en cours au Forum. Cliquer annule sans coût ; le choix reste ouvert. » couvre la 3e rangée et le bas de F.
- specialization_button_cancelled : case D revenue à « Voie A du palier » (le « 1 » est sous le bord de l'infobulle) ; infobulle « Voie A du palier 1 / Gratuit. Se débloque au Forum en 30 / 45 / 60 s (palier I / II / III), en parallèle de sa file. Définitif une fois appliqué. » : le choix est rouvert.
NON VÉRIFIÉ : lisibilité de la case en cours à 1080p (capture en 1066x600 seulement) ; libellés Voie A/B provisoires (GDD 13 par faction à venir) ; chevauchement avec une file de production réelle (le Forum ne produisait rien) ; réseau à 2 clients ; son.

À essayer : regarde la case D pendant le décompte : le texte « En cours : N s / Annuler » est-il lisible ? (défaut attendu : il déborde ; correctif = besoin C++ de la case, Q-specialization-ui reste ouverte sur l'emplacement).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 specialization_button -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 2 min (3 min max). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
