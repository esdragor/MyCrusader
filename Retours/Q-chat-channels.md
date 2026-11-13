# Canaux de discussion : deux cas du spectateur (A4, D385, D376, D357)

Contexte : la règle pure des canaux est écrite (Glorfindel, `Script\Chat\HOLChatRules.as`, branche net-playtest). D376 dit « le spectateur ne parle qu'aux spectateurs » ; il reste deux cas que le GDD ne tranche pas.

## Q1. Le « Tous » d'un joueur vivant arrive-t-il aux spectateurs ?
- **A (codé par défaut, recommandé)** : oui, un message à tous ne fuit rien ; le spectateur lit le fil public.
- B : non, les spectateurs ne lisent que leur canal.
- Autre.
Conséquence visible : avec A, l'éliminé voit les provocations et les annonces publiques ; avec B, son fil ne montre que les spectateurs.

## Q2. En partie par équipes, l'« Alliés » d'un joueur vivant arrive-t-il à son coéquipier spectateur ?
- **A (codé par défaut, recommandé)** : oui (D357 : vision et discussion de l'équipe).
- B : non, seul le spectateur parle à son équipe, il ne lit pas les plans.
- Autre.

Reponse: 1 A, 2 A (2026-11-13, tranché : D792 au GDD, défauts codés confirmés, rien à changer dans le code). Fiche à archiver par l'agent après report dans son journal.

Réglage : `bTeamGame` et les deux lignes commentées « Defaults coded » en tête de `HOLChatRules.as` (une ligne à changer chacune).
