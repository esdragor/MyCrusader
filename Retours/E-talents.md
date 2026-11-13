# [À ESSAYER] E-talents : niveau 4, dépenser trois talents dans la fenêtre Arbre de talents (T), puis le cri Serrez les rangs

STATUT : VU À L'ÉCRAN
GESTE: interne (clics et touche T joués par le scénario ; objet = affichage)
SON: non entendu
branches: aucune

Pré-vol (2026-11-13 16:2x-16:33, agent FARAMIR-INTEG intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 8d13f46, dll 11-13 13:27/13:29) : `preflight.ps1 talents -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (117 s, 7 PNG).
PNG OUVERTS et vus :
- talents_points : « Talents à dépenser : 2 » sous les capacités, sans bouton de famille
- talents_window : fenêtre au milieu, Gardien / Capitaine / Champion, rangées 2 à 9 nommées, 7 à 9 grisées, Fermer et Choix conseillé
- talents_first : Boucliers levés teinté doré, ligne dorée « Talent pris : Boucliers levés », icône B
- talents_second : Mains du guérisseur teinté, « Moines dans l'aura : soins +20 % », icônes B M
- talents_dawn : niveau 4, Pas de l'Aube teinté, icônes B M P
- talents_closed : fenêtre fermée, trois icônes restent
- talents_cry : Hommes d'armes arrêtés contre le mur du forum, cercle d'aura, case du cri en recharge (55, look : 56)
NON VÉRIFIÉ : le panneau du scénario dit « en préparation... » sur toutes les captures (le journal dit ready) ; population 11/10 sur talents_cry ; son ; vrai clic de la souris de l'utilisateur.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
