# [RÉPONDU 2026-11-12] Centre secondaire : coût et population (D384, § 5.4)

Contexte : le centre secondaire (D384) est un vrai forum de plus, façon centre-ville d'AoE4 : il produit des Paysans, prend toutes les
ressources, tire. Le GDD dit seulement « coûteux, surtout en pierre » et « une base de population », sans chiffre.

Journal : `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\SecondaryCentersInProgress.md`. Règles : `Script/SecondaryCentres/HOLSecondaryCentreRules.as`.

## Q1 : combien coûte un centre secondaire ?

- **A : 300 bois + 300 pierre** (le centre-ville d'AoE4 coûte 400 bois + 300 pierre ; ici un peu moins de bois). En jeu : un premier
  centre vers la 8e-10e minute si l'on mine la pierre tôt ; deux centres = un vrai pari.
- **B : 150 bois + 400 pierre** (presque tout en pierre). En jeu : sans camp de mineurs sur la pierre, pas de centre ; la pierre devient
  le goulot de l'expansion et une cible de raid.
- **C : 200 bois + 250 pierre** (moins cher). En jeu : boom plus tôt et plus fréquent, le rush a plus à punir mais le boom risque d'être
  toujours le bon choix (la vigilance de § 5.4).
- **Autre** : vos chiffres.

**Recommandation : A** (lisible pour un joueur d'AoE4, laisse le palier 1 respirer). Codé par défaut : A (`CostWood`, `CostStone`, une ligne chacun).

Reponse: B (150 bois + 400 pierre ; le défaut codé A 300 + 300 est à changer) - 2026-11-12

## Q2 : combien de population donne un centre secondaire ?

- **A : 10, comme le forum principal.** En jeu : chaque centre vaut une maison, le boom gagne aussi de la population.
- **B : 5.** En jeu : le centre sert surtout aux Paysans et au dépôt, il faut toujours des maisons.
- **C : 0.** En jeu : population par les maisons seules ; contraire au texte actuel du GDD (« une base de population »), à éviter.
- **Autre** : votre valeur.

**Recommandation : A** (le GDD dit « un vrai forum de plus »). Codé par défaut : A (`PopulationBase = 10`).

Reponse: A (population 10, comme le forum principal ; déjà codé) - 2026-11-12
