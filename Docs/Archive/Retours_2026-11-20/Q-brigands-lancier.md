# [À RÉPONDRE] Brigands : garder le Lancier (Spearman) ou passer au Hallebardier ?

Question de SAURON-1120d (2026-11-20). Valeur par défaut codée : A (rien n'est changé, `Camps.csv` garde `Spearman`).

## Contexte (2 lignes)
`Tools/Data/Camps.csv` ligne 2 : `Brigands,Brigands,Spearman;ManAtArms;Archer;Crossbowman`. Or depuis D859 le Lancier (`Spearman`, UnitTypes.csv ligne 7) est une unité de la faction Légions, et l'Aube a le Hallebardier (`Halberdier`, ligne 18, faction Dawn, 75 nourriture + 30 bois, 100 PV, bonus contre la cavalerie).

## Options
- **A. Garder le Lancier chez les Brigands** (défaut). Les Brigands sont neutres (aucune faction de joueur) : leur liste de membres est une liste de types, le serveur n'y filtre pas par faction (NON VÉRIFIÉ : lecture de `Camps.csv` seule, le code de naissance des camps n'a pas été relu). En jeu : le joueur de l'Aube voit des Lancier brigands (80 PV, mêlée simple) qu'il ne peut pas lui-même produire ; les Légions les voient comme leurs propres Lancier. Aucun correctif, aucun test à changer.
- **B. Passer au Hallebardier.** Les Brigands deviennent un camp mixte « Aube » (Hallebardier, Homme d'armes, Archer, Arbalétrier) : plus de Lancier neutre. En jeu : le Hallebardier a 100 PV et un bonus contre la cavalerie, donc un camp de brigands plus dur pour la cavalerie du joueur ; leur butin et leur puissance de camp changent (équilibrage des camps à revoir, puissance de camp lue par l'IA via `FHOLCampTable`). Un test de données et une ligne de CSV à changer.
- **Autre** : un type neutre dédié aux Brigands (ni Lancier ni Hallebardier), à nommer.

## Recommandation
**A** pour l'instant : la ligne est cohérente avec une règle de neutres sans faction, et le Hallebardier est encore neuf (sa traduction EN/JA et son nom français ne sont pas encore dans `HOLGameNames.cpp`) ; changer les camps avant d'avoir joué le Hallebardier mélange deux réglages. À rejouer après le premier essai du Hallebardier de l'Aube.

## Réponse de l'utilisateur
Reponse: (2026-11-20 ~12:30, interface de choix) A : les Brigands gardent le Lancier (aucun changement, `Camps.csv` garde `Spearman`). Voir GDD D863.
