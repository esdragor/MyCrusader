# [À RÉPONDRE] Garnison : une unité en garnison est-elle soignée ?

**Contexte** (F7 Garnison, D693, `FortificationsInProgress.md`) : depuis le lot 3a, une unité en garnison est « hors terrain » (corps au centre du bâtiment, ni ciblée ni combattante). Le GDD ne dit pas si un Moine à côté de la tour la soigne, ni si un Moine en garnison soigne.

- **A (défaut codé, recommandé)** : non. Une unité en garnison n'est ni soignée ni soigneuse ; elle reprend ses soins en sortant. Conséquence visible : on ne « recharge » pas une garnison avec des Moines au pied de la tour ; un Moine garni ne sert à rien.
- **B** : comme sur le terrain. Un Moine à portée du centre du bâtiment soigne les occupants, un Moine garni soigne autour de lui. Conséquence : une tour gardée par des Moines devient très dure à vider (soin sans risque pour le Moine garni).
- **C** : régénération propre à la garnison (style AoE2 : les occupants regagnent lentement des PV, sans Moine). Demande une valeur (PV/s) et du C++ en plus ; non préparé.
- **Autre** : à préciser.

**Réglage** : `hol.Garrison.HealInside` (0 = A, 1 = B), dans `Source/HeroesOfLiberty/Simulation/HOLHealProcessor.cpp`. Test : `HeroesOfLiberty.Garrison.SeparationAndHeal` (vérifie A puis B).

**Recommandation** : A (cohérent avec « hors terrain » et l'abri des occupants ; C possible plus tard comme technologie).

Posée par ELROND-1115b, 2026-11-15, branche tiers.
