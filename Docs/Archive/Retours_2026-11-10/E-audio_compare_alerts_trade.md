# [À ESSAYER] E-audio_compare_alerts_trade : écoute l'alerte de COMMERCE : sons GRAVES, SOURDS, COURTS et DOUX, pas usants (une bourse, pas un tintement) ; tu réponds par cue, la lettre que tu préfères

STATUT : LOGIQUE SEULEMENT   (aucune oreille n'a encore écouté : l'agent a lancé les scénarios sans son)
GESTE: interne      (les sons sont joués par la commande hol.Audio.Candidate du scénario, pas par un vrai clic ; seule la touche Entrée est la tienne)
SON: non entendu   (-nosound côté agent ; passe à « entendu » quand quelqu'un a écouté)
branches: heal-sleep   (le scénario et les candidats viennent du worktree Arwen ; candidats JAMAIS commités, décision B de l'utilisateur)

Réponses : remplace le _ par la lettre préférée (A à J), plusieurs lettres possibles, ou « aucune, dix autres » et dis ce que tu aimes.
Commerce [Alert.Trade] = _

Lettres (le panneau affiche le nom du cue a cote de la lettre) :
Alert.Trade : A s_trade_purse, B s_trade_purse_b, C s_trade_leather, D s_trade_drop, E s_trade_tin, F s_trade_wood_double, G s_trade_cloth_belt, H s_trade_soft, I s_trade_coins_double, J s_trade_generic

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare_alerts_trade -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 2 min (10 sons, Entrée = variante suivante ; le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer »).
Réponse attendue : une lettre par cue, `OK`, `BUG: ...` ou `REGLAGE: ...`.
