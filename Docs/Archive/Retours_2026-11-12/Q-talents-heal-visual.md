# [À RÉPONDRE] Mains du guérisseur : comment voit-on le soin renforcé ?

Contexte : le talent « Mains du guérisseur » (Capitaine, rangée 3, T4b) fait soigner les Moines 20 % plus vite quand ils sont dans l'aura du héros ou d'une bannière. Rien dans le GDD ne dit comment le joueur le VOIT ; le trait vert du soin (`HOLHealEffectSubsystem`) est le seul signe d'un soin. Position de l'aura = celle du soigneur (le GDD dit « Moines dans l'aura »).

- **A (codé par défaut)** : le trait de soin renforcé est doré (255, 215, 60) avec « ++ » au lieu de « + ». Visible en solo et chez l'hôte seulement : un client ne reçoit pas les soins sur le réseau, il devine le trait d'après la hausse de PV (il voit donc le trait vert normal). Réglage : une ligne (`BoostedHealColour` dans `HOLHealEffectSubsystem.cpp`).
- **B** : comme A, plus un petit bit répliqué avec les PV pour que les clients voient aussi le doré (C++ réseau, ~1 lot, touche le format des unités répliquées ; à refaire passer par le test réseau `-Parts net`).
- **C** : aucun signe (le soin est juste plus rapide ; l'info reste dans la bulle du talent et le journal).
- **Autre** : à préciser.

Recommandation : **A** maintenant (rien ne casse, le signe existe pour l'essai), **B** seulement si les parties en réseau doivent montrer le talent aux deux joueurs.

Valeur par défaut codée : A (changer la couleur ou la retirer = une ligne).

Reponse: A (trait dore ++ solo et hote) ; B seulement si le reseau doit le montrer - recommande, utilisateur, 2026-11-11 soir
