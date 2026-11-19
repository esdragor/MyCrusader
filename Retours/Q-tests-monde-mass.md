# [RÉPONDU 2026-11-19 ~10:10 : A, ils restent en C++ ; écrit au GDD, D854] Tests de monde qui font naître leurs unités par Mass : les garder en C++ ou les porter ? (2026-11-19, reprise 14)

**Contexte.** Tu as décidé ce matin (D853) de porter en script tous les tests C++ « de monde » (GameState, Victory, Disconnect et voisins). Le plan de portage (`Docs\Rapports\PLAN-TESTS-1119.md`) a compté : sur 22 fichiers (11 306 lignes), 10 fichiers (5 345 lignes) font naître leurs unités ou leurs héros par la simulation Mass (`RequestUnits` + `ServeBirthRequests`) : FarmBuilders, ProductionPopulationEnd, DuelOrder, GarrisonOrder, Gather, DeployButton, DiplomacyBranch, DiplomacyAllies, HeroReturn, Regicide. D838 dit que les tests de la simulation Mass restent en C++. Aucun effet en jeu dans les deux cas : c'est le périmètre du travail qui change.

**Options.**
- **A (recommandée)** : ces 10 fichiers restent en C++ (lecture large de D838). Le portage vise alors ~3 500 lignes de tests (12 fichiers, en entier ou en partie), avec ~700 lignes de liaison « monde de test » en 5 lots. Les tests gardés continuent de protéger les règles pendant que le code sous eux migre.
- **B** : on ajoute ~400 lignes de liaison de plus (`SpawnUnit`, `RunMassStep` exposés au script) pour pouvoir porter ~5 000 lignes de plus. Conséquences : plusieurs builds en plus, et un test script qui pilote Mass par une liaison ne prouve plus tout à fait la même chose que le test C++ (il passe par une couche de plus).

**Par défaut, en attendant ta réponse** : A (rien n'est retiré, le lot 1 de la liaison de monde de test est en cours et sert aux deux options).

Reponse: A (interface de choix, 2026-11-19 ~10:10) : les 10 fichiers à naissance Mass restent en C++ ; D854.
