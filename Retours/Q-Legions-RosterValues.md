# [À RÉPONDRE] Légions L2 : coûts et noms du socle en reskin

**Contexte.** L2 (LegionsInProgress.md, LQ-1, D345) : les 7 unités du socle des Légions ont un coût propre, toujours inférieur à celui de l'Aube, et un nom affiché propre. Les valeurs ci-dessous sont codées par défaut dans `Tools/Data/FactionOverrides.csv` (lignes `Legions_<Unité>_Cost` et `_Name`), commit 7bef755 de la branche `net-playtest` (Glorfindel). Le temps de production reste celui de l'Aube (pas de ligne `BuildTime`, de toute façon non appliqué avant le C++ de L2).

**Question 1 (coûts).** Combien de moins que l'Aube ?

| Unité | Aube | A : -25 % (défaut codé) | B : -15 % | C : -35 % |
|---|---|---|---|---|
| Lancier | 60 N / 20 B | 45 N / 15 B | 50 N / 15 B | 40 N / 15 B |
| Homme d'armes | 100 N / 20 O | 75 N / 15 O | 85 N / 15 O | 65 N / 15 O |
| Archer | 30 N / 50 B | 25 N / 40 B | 25 N / 45 B | 20 N / 35 B |
| Arbalétrier | 80 N / 40 O | 60 N / 30 O | 70 N / 35 O | 50 N / 25 O |
| Cavalier léger | 80 N / 40 O | 60 N / 30 O | 70 N / 35 O | 50 N / 25 O |
| Bélier | 150 B / 75 O | 110 B / 55 O | 130 B / 65 O | 100 B / 50 O |
| Mangonneau | 200 B / 150 O | 150 B / 110 O | 170 B / 130 O | 130 B / 100 O |

(N = nourriture, B = bois, O = or.) **Autre** : un coût unité par unité de ton choix (D345).
Conséquence visible : l'écart d'économie entre Légions et Aube (le Zombie est gratuit, 0,49/s) ; plus l'écart est grand, plus les Légions tiennent avec moins de Zombies.
**Recommandation : A** (un quart de moins, lisible, les rapports de D66 gardés).
**Défaut codé :** A. Pour changer : les 7 lignes `_Cost` de `FactionOverrides.csv`, puis relancer `python Tools/Tests/legions_roster_test.py`.

**Question 2 (noms affichés, provisoires jusqu'à L13).** Défaut codé : Legion Spearman, Black Guard, Legion Archer, Legion Crossbowman, Dread Rider, Bone Ram, Bone Mangonel. Alternatives : B = « Légion » partout (Legion Man-at-arms, Legion Rider…), C = noms sans « Légion » (Pikeman, Warden, Bowman, Bolter, Rider, Ram, Catapult). **Recommandation : A**, ils disent la faction et le rôle dans l'infobulle. **Autre** : tes noms.

Reponse: Q1 = A (-25 %), Q2 = A (noms codes), 2026-11-05 19:50, repondu par l'utilisateur dans la conversation. Valeurs par defaut conservees : rien a changer. A reporter dans LegionsInProgress.md (D a numeroter par la session) puis supprimer ce fichier.
