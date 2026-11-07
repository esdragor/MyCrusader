# [À ESSAYER] E-audio_compare_martial : écoute l'alerte d'entrainement fini, deuxieme tour : sons MARTIAUX (cor, trompette, timbales, trombone, epee, bouclier) ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son ; le PNG montre l'écran et le panneau, pas le son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep (83bc85d dans main 80e3c07 ; le scénario et les 153 candidats viennent du worktree Arwen, abb3335, candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Alert.TrainingDone = _

Pré-vol (2026-11-07 10:19, agent ELROND-9 intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 80e3c07, binaire de main 10:10) : `preflight.ps1 audio_compare_martial -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (33 s, 1 PNG, sanity audit passed, 10 lignes « candidate played » sur 10). Premier passage FAIL : les candidats (Content\Audio\Candidates, non suivis par git) n'existent que dans le worktree Arwen ; copiés dans Elrond le temps du pré-vol puis retirés (Elrond propre). Donc l'essai se lance DANS Arwen, pas dans Elrond.
PNG OUVERTS et vus :
- audio_compare_martial_start : l'écran du jeu (forum, 5 Paysans, forêt, panneau du scénario avec les 3 notes : écoute l'alerte d'entrainement fini, sons MARTIAUX, Entrée = variante suivante, comment réécouter avec hol.Audio.Candidate, réponds par cue) ; aucune fenêtre ne cache le panneau.
NON VÉRIFIÉ : SON non entendu (-nosound, aucune oreille) : que les 10 sons soient audibles, distincts et bien nommés par le panneau ; l'avance par la touche Entrée (waitkey passés en -unattended) ; le lancement par play.ps1 dans Arwen (binaire du 09:00, branche heal-sleep abb3335, pas le build de main : non rejoué avec le son) ; le panneau après la 1re variante (un seul PNG, au départ).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_martial -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 3 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
