# [RÉPONDU] Q-commanderie-palier1 : valeurs des 3 unités du palier 1 des voies de commanderie Templiers (D893)

Contexte : D893 demande d'abord les 3 unités du palier 1 (Fer : Sergent du Temple, Foi : Frère infirmier, Pierre : Arbalétrier à pavois), provisoires. Le GDD (§ 13.7, D133) donne les noms et le rôle, pas les coûts ni les PV : CELEBORN-1123pa a donc copié trois unités existantes (lignes de `Tools/Data/UnitTypes.csv`, faction `Templars`, mannequin, chaque ligne dans `Placeholders.txt`). La voie (Fer / Foi / Pierre) n'est pas une colonne de la table (ce serait du C++) : le test la porte en littéral ; le déblocage par la voie est la ligne suivante de la file.

Valeurs actuelles (PROVISOIRES, défaut codé = option A) :

| Ligne de table | Nom GDD | Copie de | Coût | PV | MinTier |
|---|---|---|---|---|---|
| `TempleSergeant` | Sergent du Temple | Hallebardier | 75 nourriture + 30 bois | 100 | 0 |
| `InfirmaryBrother` | Frère infirmier | Moine | 100 nourriture + 100 or | 80 | 1 |
| `PaviseCrossbowman` | Arbalétrier à pavois | Arbalétrier | 80 nourriture + 40 or | 80 | 1 |

Capacités du GDD NON codées (Ouvrir les rangs, soin hors combat rapide, Planter le pavois) : chacune est une règle à part.

Question : quelles valeurs veux-tu pour ces trois unités ?
- A (défaut codé, recommandé pour avancer) : garder les copies ci-dessus, à régler en test de jeu (GDD : « valeurs à régler en test »). Conséquence : le Sergent du Temple sort au palier 0 comme l'Hallebardier, les deux autres au palier 1 ; rien à refaire.
- B : un coût et des PV dictés par toi pour chaque unité (donne les 6 chiffres) ; conséquence : trois cellules à changer, aucun code.
- C : un autre modèle de base (p. ex. Sergent = plus léger que le Lancier, comme le GDD le dit : « moindre que le Lancier ») ; conséquence : PV et dégâts du Sergent à baisser (aujourd'hui ceux de l'Hallebardier, plus forts que le Lancier).

Recommandation : A.

Réponse : A pour les 3 (utilisateur, 2026-11-23 15:28, D896)
