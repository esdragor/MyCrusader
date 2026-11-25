# [À RÉPONDRE] Le Défi du Chevalier provoque-t-il les héros ennemis ?

Contexte : D919 fait du Défi du Chevalier une provocation seule (8 s, 8 m, 45 s). Le texte ne dit rien des héros ennemis.
Valeur codée : les héros ne sont pas provoqués (`FSeeker::bIsHero`, `HOLCombat::IsChallengedBy`, `HOLCombatRules.cpp`) ; une ligne à changer.

- A (défaut codé) : les héros ennemis ignorent le défi et gardent leur cible (un héros ne se laisse pas détourner par un Chevalier).
- B : les héros sont provoqués comme les soldats (le Chevalier protège aussi contre un héros adverse, mais peut le tanker à sa place).
- Autre.

Recommandation : A (un héros qui choisit ses duels est déjà géré par le mode duel).
