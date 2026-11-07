# [À RÉPONDRE] Biomes : look appliqué au monde (B1 visuel) - Sauron-2, 2026-11-07

Reponse: Q1 A puis B, Q2 B (utilisateur 10:36)

Contexte : l'applicateur (Script/Biomes/HOLBiomeApplier.as) pose sur la carte chargée le soleil, le ciel, la brume et les
couleurs du sol du biome du salon. Les valeurs de la table sont PROVISOIRES (jamais jugées à l'écran) ; deux choix restent à toi.

## Q1. Que fait le biome « par défaut » (le salon n'en choisit pas) ?

- A (codé) : la carte garde son propre look, celui que son fichier porte (ambience.py) ; l'applicateur ne touche à rien.
- B : chaque carte a un biome par défaut (colonne de Tools/Data/Maps.csv, B4) et l'applicateur le pose toujours (Plaine = herbe fraîche, Steppe = steppe sèche...). Les cartes changent de look dès B4.
- C : herbe fraîche partout.
- Autre : ...

Recommandation : A tant que les cinq looks ne sont pas jugés à l'écran, puis B avec B4 (D660).
Valeur par défaut codée : A (`HOLBiomeApply::WantedBiome` renvoie -1 pour « default » ; une ligne à changer).

## Q2. Faut-il juger les cinq looks maintenant ?

- A : oui, essai `biome_apply_tour` (un PNG par biome, Entrée = biome suivant) : tu dis aimé / pas aimé par biome, je règle la table.
- B : après B3 (décor générique), pour juger sol + lumière + décor ensemble.
- Autre : ...

Recommandation : B (un biome se juge avec son décor). Valeur par défaut codée : les valeurs provisoires de HOLBiomeRules.as, rien ne change.
