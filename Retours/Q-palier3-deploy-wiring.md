# [À RÉPONDRE] Palier 3 G6 : quelle unité porte l'état monté / démonté, et comment les ordres le touchent

**Contexte.** Le fragment d'état (`FHOLDeployFragment`) et son processeur existent (G6 a). Reste à dire QUELLE unité le porte, et où se prennent les décisions d'ordre et de tir. (Le fichier `Q-trebuchet-deploy.md` cité dans le lancement n'existe pas dans `Retours\` ; les valeurs proposées sont dans `HOLDeployRules.h` : montage 8 s, démontage 5 s, portée minimale 12 m, portée maximale 50 m.)

**Question 1 : qui porte le fragment ?**
- **A (codée, défaut)** : substitut : réglage `hol.Siege.Deployable` (0 par défaut = aucune unité du jeu ne se monte ; 1 = tout Mangonnelle naît démontée avec le fragment). Réversible, aucune colonne. Conséquence en jeu : rien ne change tant que le réglage est à 0 ; le Trébuchet (G7) devra brancher sa ligne.
- **B** : colonne `Deployable` (et délais / portées) dans `UnitTypes.csv`, lue à la naissance. Conséquence : un seul mécanisme pour Trébuchet et Canon, mais migration des CSV, du test `Siege.Table`, de l'empreinte réseau.
- **C** : A maintenant, B à l'ouverture de G7 (le Trébuchet), quand une vraie unité existe.
- **Autre** : à préciser.
**Recommandation : C.** Une colonne sans unité réelle à lire n'a pas de test d'effet honnête.

**Question 2 : où s'applique « pas de tir hors Monté » ?**
- **A (codée, défaut)** : dans `HOLCombatProcessor` aux deux points de tir (unité, bâtiment) : démonté, l'engin s'arrête, laisse son ordre d'attaque au processeur de déploiement et ne tire pas. La vitesse nulle hors Démonté est posée par le processeur de déploiement (il passe après `HOLUnitOrderProcessor`, avant le combat).
- **B** : tout dans le processeur de déploiement (il repousse le délai de coup de l'engin). Conséquence : combat intact, mais le délai de frappe (montée en puissance) se mêle au déploiement : visible en jeu comme un premier coup plus lent.
- **Autre** : à préciser.
**Recommandation : A** (une seule source de vérité pour « peut tirer »).

**Valeur par défaut codée** : Q1 = A (`hol.Siege.Deployable` 0), Q2 = A.
