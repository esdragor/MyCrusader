# [À RÉPONDRE] Q-ai-arquebusier : comment l'IA du palier 3 choisit-elle l'Arquebusier ? (Palier 3, G14, D716)

Contexte : D716 dit « Arquebusiers dans la composition (contre Homme d'armes) » mais ne donne ni part ni règle. La composition de l'IA (`Script\AI\HOLAIArmy.as`, `ChooseComposition`) choisit, contre une armée ennemie, le candidat au meilleur rapport duel / coût ; sans armée ennemie, la part fixe (`CandidateRatios`, 0 = jamais).
Mesure à la main sur les vraies lignes de `UnitTypes.csv` (coût Or x 1,5, exposition 0,5, plafond 5 ; NON lancée dans un test) : contre 5 Hommes d'armes, l'Arbalétrier marque 0,0084 (duel 1,18 / coût 140), l'Arquebusier 0,0059 sans pénétration dans les chiffres de l'IA (duel 1,06 / coût 180) et 0,0063 avec elle. **Ajouter l'Arquebusier aux candidats tel quel ne change donc rien : l'IA ne le produirait jamais** ; il faut une règle.

Q1. Quelle règle ?
- A. **Part fixe dans le mélange du palier 3** : l'Arquebusier entre avec une part (1 pour 7, proposée) quand il est ouvert (palier 3, faction qui le produit) ; la règle de contre-attaque ne change pas. Script pur (`DefaultData` + `Test_Army*`), mais il faut aussi filtrer la faction de l'IA (les Légions n'ont pas d'Arquebusier : le serveur le refuserait en boucle).
- B. **Contre-attaque seulement** : la règle compte la pénétration (champ de plus dans `FHOLAITypeFigures`, C++) ET donne un bonus de coût contre les Lourds ; l'IA n'en fait que si l'ennemi a des Hommes d'armes (valeur du bonus à trancher). C++ + script.
- C. A + B.
- Autre : ...

Conséquences visibles : A, l'IA Normal / Difficile aligne quelques Arquebusiers dès le palier 3, même contre des Lanciers ; B, seulement quand elle voit des Hommes d'armes (le reste du temps l'Arbalétrier garde sa place).
Recommandation : **A** (une ligne, aucune formule à inventer) avec le filtre de faction ; B plus tard si l'essai montre des Arquebusiers inutiles contre des Lanciers.
Valeur par défaut codée : **aucune** (rien n'est changé ; l'IA ne produit pas d'Arquebusier). Quand A est tranchée : une ligne `CandidateRatios` dans `DefaultData`.
Facile : joue les unités que le palier ouvre (D716), donc l'Arquebusier aussi, sans technologie.

Reponse (utilisateur, 2026-11-09 17:14) : A (part fixe 1 pour 7 dans le melange du palier 3 + filtre de faction). Applique apres la fusion 2 (tiers dans main) par un agent script.
