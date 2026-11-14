# [À RÉPONDRE] Q-nomad-net : en réseau, le Forum du Nomade n'apparaît pas chez un joueur distant (agent HALDIR-1114b, 2026-11-14)

Contexte : le Forum premier du Nomade (âge I, touche C, prix du centre secondaire) est dans la grille du joueur qui héberge la partie (vérifié : tests 1258/1258). Mais l'état « siège Nomade sans forum » n'est écrit que sur le serveur (`SetNomadNow`, dans `SpawnSlotStart`) et l'option `bNomad` n'est pas répliquée : la machine d'un joueur DISTANT croit son siège standard, donc sa grille n'a PAS la case Forum et l'infobulle affiche le prix normal (lu dans le code, pas joué en réseau).

Options :
- **A (recommandée)** : le serveur réplique l'état Nomade de chaque siège (un booléen par siège dans l'état de partie, comme la chute du forum) ; la machine du joueur le lit pour la grille et l'infobulle. Conséquence visible : le joueur distant voit la case Forum et son vrai prix, comme l'hôte.
- **B** : répliquer seulement l'option `bNomad` de la partie ; chaque machine en déduit « Nomade sans forum » en regardant si le siège a déjà un Centre-ville achevé. Même effet visible, mais deux façons de calculer la même chose (risque d'écart).
- **C** : ne rien changer tant que le Nomade n'est pas jouable en ligne. Conséquence : en ligne, un joueur distant Nomade ne peut pas poser son premier Forum (partie perdue d'avance).
- **Autre** : à préciser.

Valeur par défaut codée : aucune (rien de réseau n'est changé ; l'hôte et les parties contre l'IA marchent). Rien n'est bloqué en attendant.
Reponse:
