# [À ESSAYER] E-disconnect : le tableau des joueurs montre le départ de Bob : « Connexion perdue », puis « IA » quand la fenêtre est finie et qu'une IA de son camp garde le siège

STATUT : VU À L'ÉCRAN (tableau des joueurs, 3 captures ouvertes) ; ce que le scénario ne montre pas : voir NON VÉRIFIÉ
GESTE: interne      (le départ de Bob est simulé par la console : `hol.Match.StartWaitSeconds 1` = la partie démarre sans lui, vraie chaîne KeepMissingSlot ; `hol.Net.FakeAbsent End 1` = vraie fin de fenêtre ; aucune touche ni clic de l'utilisateur)
SON: sans objet
branches: net-playtest

Règle (L7-6) : un essai de GESTE reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ; ici l'objet de l'essai est l'AFFICHAGE (pastille du tableau), vu sur les PNG ; le départ réel d'un client est prouvé à part sur le fil par `Tools\Tests\lobby_match.ps1 -Team` et `-Reconnect` (Alice lit la pastille répliquée).

Pré-vol (2026-11-07 12:46, agent Glorfindel-15, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main 9d39223) : `preflight.ps1 disconnect_look -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice` PASS (28 s, 3 PNG). WARN : pas de `hol.HUD.Report` ni de `hol.Test.Sanity` dans ce scénario.
PNG OUVERTS et vus (C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- disconnect_present.png : tableau en haut à droite, « Vous », « Bob », « IA Normal 3 », 2 vies chacun, AUCUNE pastille (partie pas encore démarrée, le serveur attend Bob).
- disconnect_away.png : la partie a démarré sans Bob ; la ligne de Bob garde son nom (rouge) et porte la pastille « Connexion perdue » ; bandeau « Bob : connexion perdue — 3:00 ».
- disconnect_ai.png : la ligne de Bob garde son nom et porte la pastille « IA » (petite, à droite de « 2 vies ») ; le bandeau a disparu.
NON VÉRIFIÉ : le départ d'un VRAI second jeu (couvert par lobby_match, pas à l'écran ici) ; la pastille « IA » est petite (lisible mais discrète) ; le scénario n'est pas passé par un `hol.HUD.Report` (recouvrement des panneaux non contrôlé : le tableau et le panneau « Scénario en préparation » ne se touchent pas sur les captures).

Lancer UNE commande (le joueur local doit s'appeler Alice, sinon il est spectateur : le salon nomme les sièges) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_look -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice`
Durée : environ 1 minute (l'attente de 60 s avant le démarrage de la partie est raccourcie par la console). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
