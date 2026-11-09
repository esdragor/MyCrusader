# [À ESSAYER] E-duel_challenge_accept : la fenêtre « Défi envoyé au héros du joueur 2 » (barre et secondes) puis le duel des deux héros seuls au centre, l'IA acceptant d'office.

STATUT : VU À L'ÉCRAN (fenêtre de défi, anneau et PV du duel, par captures du pré-vol ; objet = l'AFFICHAGE, pas le geste)
GESTE: interne      (le défi est la commande `hol.Duel.Challenge 1` : le bouton du défi n'existe pas encore ; l'acceptation est forcée par `hol.Duel.ForceAnswer 1 1` ; Entrée est une vraie touche attendue par `waitkey`, le pré-vol la passe en « go on (auto) »)
SON: sans objet
branches: aucune    (seul main compte ; le scénario et la fenêtre de défi sont dans main 60013e8)

Règle (L7-6) : l'essai ne juge aucun geste de souris ; le défi par bouton reste à faire, donc ce qui est vu est l'affichage (fenêtre, anneau du duel).

Pré-vol (2026-11-09 10:54, agent Elrond-integration, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 60013e8, rejoué sur le build de main fusionné) : `preflight.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (68 s, 3 PNG).
PNG OUVERTS et vus :
- duel_heroes_face : héros bleu et héros rouge face à face au centre, 5 Paysans bleus derrière le mien, couronne d'arbres autour, anneau clair du décor ; note 1 du scénario affichée (« elles ne doivent pas trembler »), mini-carte
- duel_window : « Défi envoyé au héros du joueur 2 » en haut au centre, barre jaune presque pleine, « 6 s » ; panneau du scénario caché ; les deux héros au contact dans l'anneau
- duel_started : anneau ROUGE autour des deux héros et de leur duel (et sur la mini-carte), PV du héros 399 / 450, les Paysans bleus restent en dehors de l'anneau
Constat : le panneau du scénario affiche « en préparation… » même une fois prêt (sans rapport avec l'objet) ; les arbres du décor sont gros et entourent bien l'anneau.
NON VÉRIFIÉ : le tremblement des unités autour du héros pendant la marche de 25 s (aucun œil humain) ; les Paysans de l'IA ne sont pas visibles ; la fin du duel (vainqueur, 90 s) n'est pas capturée ; les récompenses (D7) ne sont pas branchées.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min (Entrée au milieu). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse (utilisateur, 2026-11-09, joue par lui) : OK.
