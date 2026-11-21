# [À RÉPONDRE] Q-secondary-centre-grid : que propose la grille du Centre secondaire ?

Contexte : D384 dit « un vrai forum de plus » (produit des Paysans, dépôt, garnison, tir), D879 dit « Paysan seulement » sans préciser si cela vaut pour la grille de commandes. Le forum principal propose A Paysan, D/F (voies de palier) et W (cloche d'alarme) ; le Centre secondaire fini ne propose que A Paysan (vu dans sc_apres, essai E-secondary_centre).

Valeur actuelle codée : **A = Paysan seul** (ligne `SecondaryCentre` de `Tools\Data\BuildingTypes.csv`, une ligne à changer).

Options :
- **A. Paysan seul (valeur actuelle).** En jeu : le centre secondaire ne sert que de dépôt, de production de Paysans, de population et de lieu de retour du héros ; la montée de palier ne se lance qu'au forum principal (donc cible unique à défendre pour progresser). Boom économique pur, sans surcoût de code.
- **B. Paysan + cloche d'alarme (W).** En jeu : le joueur peut sonner la garnison depuis le centre secondaire (utile en expansion attaquée), la montée de palier reste au forum principal. Petite ligne de grille en plus ; dépend des garnisons du centre secondaire (D384 : garnison et tir).
- **C. Mêmes commandes que le forum (Paysan, voies de palier D/F, cloche W).** En jeu : on peut monter de palier depuis n'importe quel centre (ne s'arrête plus quand le forum principal est attaqué), mais le boom devient encore plus fort (risque signalé par D384 « le boom ne doit pas devenir systématiquement optimal ») et les voies de palier doivent rester uniques (un seul palier en cours à la fois).
- **Autre** : à préciser.

Recommandation : **A** pour le prototype (le boom est déjà un pari fort, D384), ajouter **B** quand les garnisons du centre secondaire sont jouables ; C seulement si les essais montrent que la montée de palier au forum principal est trop fragile.
