# [À ESSAYER] E-duel_challenge_accept : la fenêtre « Défi envoyé au héros du joueur 2 » (barre et secondes) puis le duel des deux héros seuls au centre, l'IA acceptant d'office.

STATUT : VU À L'ÉCRAN (fenêtre de défi et duel, par captures du pré-vol ; jamais joué avec une fenêtre par l'agent)
GESTE: interne      (Entrée est une vraie touche attendue par `waitkey`, mais le défi lui-même est la commande `hol.Duel.Challenge 1` : le bouton du défi n'existe pas encore ; l'acceptation est forcée par `hol.Duel.ForceAnswer 1 1`. Le pré-vol a passé la touche en « go on (auto) »)
SON: sans objet
branches: aucune   (seul main compte ; le scénario et la fenêtre de défi sont dans main)

Règle (L7-6) : l'essai ne juge aucun geste de souris ; le défi par bouton reste à faire, donc ce qui est vu est l'AFFICHAGE (fenêtre, anneau du duel), pas le geste.

Pré-vol (2026-11-09 09:32, agent Sauron-tools, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, main d22905c au lancement) : `preflight.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` PASS (72 s, 3 PNG ; le scénario seul dure 51 s) ; audit « sanity » passé, aucun panneau de HUD ne couvre un autre.
PNG OUVERTS et vus :
- duel_heroes_face : les deux héros côte à côte au centre dans l'anneau de décor, mes 5 Paysans bleus derrière le mien, le héros rouge de l'IA en face ; panneau du scénario affiché (il porte encore la note 1 : la note 2 s'affiche une fois la capture prise) ; mini-carte, 2 vies chacun.
- duel_window : « Défi envoyé au héros du joueur 2 » en haut au centre, barre jaune presque pleine, « 6 s » ; panneau du scénario caché ; les deux héros dans la vue.
- duel_started : anneau rouge autour des deux héros (et sur la mini-carte), mon héros à 399 / 450 PV (il avait 450), les Paysans en dehors de l'anneau ; panneau du scénario revenu à la fin.
NON VÉRIFIÉ : le tremblement des unités autour du héros pendant la marche de 25 s (audit `hol.Test.SanityMotion` passé, mais aucun œil humain) ; les 5 Paysans de l'IA ne sont pas visibles dans les captures ; la fin du duel (vainqueur, 90 s) n'est pas capturée ; les récompenses (D7) ne sont pas branchées.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min (Entrée au milieu). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
