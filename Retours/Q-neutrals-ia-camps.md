# [À RÉPONDRE] Q-neutrals-ia-camps : l'IA et les camps neutres (N6)

Contexte : un camp neutre (brigands gris, D613) charge à vue (D611) ; l'IA le voit comme n'importe quel ennemi, donc il pèse dans le rapport de force qui décide de lancer une vague (marge 1,5). Rien dans le GDD ne dit si l'IA doit chercher à nettoyer un camp (XP du héros, D612) ou l'ignorer.

Options :
- **A. Ignorer** (`CampPolicy` 0) : la vague part contre les adversaires seuls ; un camp rencontré sur la route n'est qu'un combat. L'IA ne nettoie jamais de camp. Visible : une IA qui se précipite sur le forum d'en face en laissant un camp fort sur son chemin et peut se faire écraser en route.
- **B. Évaluer** (`CampPolicy` 1, **valeur actuelle, codée par défaut**) : le camp compte comme n'importe quel ennemi dans la marge ; la vague attend d'être assez forte pour battre adversaires ET camps vus (brouillard). L'IA ne va pas au camp exprès. Visible : une IA qui attend plus longtemps quand un camp fort est en vue.
- **C. Nettoyer d'abord** : si le rapport de force contre un camp vu (règle `CampRatio`, déjà écrite) atteint la marge, l'IA envoie d'abord la vague sur le camp (XP, gisement ou passage), puis sur l'adversaire. Demande une cible de vague en plus (un lot de code après cette réponse) et des camps posés par les cartes (N2, pas encore).
- **Autre** : vos mots.

Recommandation : **B** pour le prototype (aucun changement de comportement, aucune cible de plus, les camps n'étant pas encore posés par les cartes), **C** après le prototype avec les vraies créatures.

Valeur par défaut codée : `CampPolicy = 1` (`FHOLAIArmyData` de `Script/AI/HOLAIArmy.as`) ; changer la réponse = une ligne (0 pour A). Les règles `CampUnits`, `AdversaryView` et `CampRatio` sont déjà testées (`HOLAICamp_Test.as`).

Reponse (utilisateur, 2026-11-09) : B (evaluer, CampPolicy 1 = valeur deja codee). Rien a changer ; C apres le prototype.
