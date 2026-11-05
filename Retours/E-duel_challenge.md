# [À ESSAYER] E-duel_challenge : ton héros défie celui de l'IA, la fenêtre de défi, la réponse (refus), les unités ne tremblent pas

STATUT : VU À L'ÉCRAN pour la marche, la fenêtre « Défi envoyé au héros du joueur 2 » (barre + secondes, sans bouton) et la disparition après la réponse. GESTE : le défi part par la console (`hol.Duel.Challenge 1`, Entrée dans le scénario) car le bouton de défi n'est pas encore branché : donc pas un geste réel au sens de la règle 18, à dire à l'utilisateur.

Pré-vol (2026-11-05 20:39, agent Elrond-sans-C++, worktree HeroesOfLiberty-Elrond = branche tiers, main b39fafd + 88b0813, binaire du 20:12, SANS build) : `preflight.ps1 duel_challenge -Root ...Elrond` PASS (check 52 s, 31 s de jeu, 25 s de marche + 1 Entrée, HUD sans recouvrement, audit « Sanity: OK (160 steps, 12 units followed) » = aucun tremblement ni tournoiement, aucun avertissement au journal). Plus d'attente d'une minute : la marche dure 25 s, la caméra suit, l'utilisateur regarde les unités autour du héros.
PNG OUVERTS et vus :
- duel_heroes_face : mon héros (bleu) avec ses 5 Paysans, le héros de l'IA (rouge) en face, au centre dans un cercle d'arbres ; panneau du scénario affiché (note 1) ; mini-carte à jour.
- duel_window : la fenêtre en haut au centre « Défi envoyé au héros du joueur 2 », barre jaune, « 6 s », sans bouton ; panneau du scénario caché ; les héros se touchent.
- duel_answer : fenêtre disparue ; journal « AI duel answer: refuse (odds 50, believed 46, bar 60 ...) » ; PV de mon héros 399/450 (les héros se battent après le refus) ; anneau de duel absent (« Duel rings: none »).
NON VÉRIFIÉ : les unités des deux héros vues à l'œil nu pendant la marche réelle (seul l'audit mécanique et les PNG de départ/arrivée) ; le bouton de défi ; l'acceptation (scénario duel_challenge_accept, non rejoué ici) ; son.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 duel_challenge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min 30 (25 s de marche, Entrée pour défier, 6 s de fenêtre).
Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: OK (2026-11-05 22:3x, utilisateur, fenetre reelle : 25 s de marche sans tremblement puis defi a Entree).
