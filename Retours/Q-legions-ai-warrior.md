[A REPONDRE] Q-legions-ai-warrior : l'IA des Legions et le Guerrier damne (L4 etape 6)
Contexte : l'IA des Legions produit maintenant le Guerrier damne a la Caserne et lui fait lancer la rage necrotique en melee. Deux valeurs sont des choix par defaut, non tranches.
Q1 - Part du Guerrier damne dans le melange fixe de l'IA : A = 1 sur 8 (defaut code, HOLAIArmy.as DefaultData, CandidateRatios) ; B = 2 sur 9 ; C = 0 (l'IA n'en produit pas) ; Autre.
Q2 - Declenchement de la rage par l'IA : A = ennemi de melee a 6 m, PV du guerrier au-dessus de 60 %, au moins 40 s apres le dernier lancer (defaut code, HOLAIRage.as FHOLAIRageData) ; B = seulement si l'ennemi a plus de PV que lui ; Autre.
Consequences visibles : A/Q1 = environ un Guerrier damne tous les 8 soldats ; Q2 A = la rage part des que la melee commence, sans attendre un duel serre.
Recommandation : A et A. Changer = une ligne de HOLAIArmy.as (ratio) ou de FHOLAIRageData (valeurs).

Reponse: Q1 A (1 sur 8), Q2 A (ennemi de melee a 6 m, PV > 60 %, 40 s) - recommande, utilisateur, 2026-11-11 soir
