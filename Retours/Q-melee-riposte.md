# [À RÉPONDRE] Q-melee-riposte : un soldat frappé pendant sa marche vers son poste doit-il riposter ?

Contexte (Celeborn-12, diplomacy_qground_watch, run5 #10) : un soldat qui marche vers son poste (phase FinalLeg, après un ordre de marche ou de contournement) ne cherche aucune cible (HOLCombat::MaySeek, choix D345/J7b.3) ; frappé à 127 cm par un ennemi, il ne riposte pas tant que sa marche dure (jusqu'à 2 s). Rien n'est changé dans le code : le comportement actuel est gardé.

Options
- A : garder (pas de riposte pendant une marche d'ordre, comme aujourd'hui). Visible : un soldat peut encaisser des coups 1 à 2 s sans répondre quand son poste est barré.
- B : riposter dès qu'on est frappé en mêlée pendant la phase 3 (la marche est abandonnée, le soldat combat). Visible : plus aucun soldat « qui se laisse frapper » ; mais un groupe qui doit traverser une ligne ennemie (marche d'ordre explicite) s'arrête au premier coup reçu.
- C : riposter seulement si la marche ne progresse plus (poste barré, immobile depuis N pas). Visible : la marche normale n'est pas coupée, le cas figé de #10 est réglé ; un peu plus de code.
- Autre : à préciser.

Recommandation : C (traite le cas figé sans changer le sens des ordres de marche). Note : le correctif Celeborn-12 (poste dans le corps d'un ennemi immobile = fin de marche immédiate, hol.Orders.SlotBlockedStopDistance 250, 0 = avant) règle déjà le cas de #10 et #11 sans changer cette règle ; la question ne porte que sur le cas général.

Défaut codé : A (comportement actuel). Aucune cvar de riposte n'est écrite (non testée).
