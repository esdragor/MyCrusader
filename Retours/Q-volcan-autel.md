# [À RÉPONDRE] Q-volcan-autel : de combien l'autel avance-t-il l'éveil du volcan ?

Contexte : D380 dit que tenir l'autel ancien ~60 s (sans ennemi, comme la ruine, D348) réveille le volcan « plus tôt », alerte de 60 à 90 s inchangée, direction tirée (D379), sans dire de combien. Un site avec Catastrophe a des cycles : fenêtre de déclenchement (ouverte à 6 min, longue de 3 min la première fois), alerte, éruption, refroidissement, repos ; l'autel ne sert que pendant que le site est armé (pas pendant une éruption ni un repos).
Valeurs prises si tu ne précises pas : option A, avance de 120 s, réglage `FHOLVolcanoSettings::AltarAdvance` (changeable sans recoder, un seul nombre).

- **A (avance fixe, 120 s)** : une fois l'autel tenu, la date d'alerte du cycle en cours recule de 120 s, sans jamais passer avant l'instant présent. En jeu : tenir l'autel loin de la fenêtre fait gagner 2 min, tenir l'autel quand l'alerte est déjà proche ne change presque rien ; le gain est prévisible, le joueur sait ce qu'il achète.
- **B (alerte tout de suite)** : tenir l'autel lance l'alerte immédiatement (60 à 90 s avant la lave). En jeu : celui qui tient l'autel choisit le moment de la catastrophe ; très fort pour un joueur qui s'est préparé (armée loin de la zone), très punitif pour l'autre ; la fenêtre 6 min + 3 min devient presque sans objet.
- **C (moitié du temps restant)** : la date d'alerte recule de la moitié du temps qui la sépare encore de l'instant présent. En jeu : gros effet si on tient tôt, petit effet tard (s'adapte seul), mais le gain est moins lisible (« je ne sais pas de combien »).
- **Autre** : écris la durée ou la règle voulue.

Recommandation : A (lisible, bornée, un nombre à régler à l'essai ; B pourra être testé après en mettant AltarAdvance très grand).

Reponse:
