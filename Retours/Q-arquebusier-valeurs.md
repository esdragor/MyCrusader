# [À RÉPONDRE] Q-arquebusier-valeurs : chiffres de l'Arquebusier (Palier 3 G3) et pénétration contre un bâtiment (G2)

Contexte : G3 crée la ligne `Arquebusier` (Champ de tir, palier 3, catégories Distance + Infanterie à poudre, pénétration d'armure D713) ; le dossier dit « valeurs à proposer ». Références du tableau actuel : Archer 70 PV, attaque 5, intervalle 1,625 s, portée 2000, 30 N / 50 B ; Arbalétrier 80 PV, attaque 12 (+10 Lourd), intervalle 2,125 s, portée 2000, 80 N / 40 O ; Homme d'armes armure distance 4.

## Q1 : les chiffres de l'Arquebusier
- **A (recommandé)** : 70 PV, vitesse 720, attaque 30, intervalle 4,0 s, portée 1500 (plus courte que l'Archer et l'Arbalétrier), coût 60 N / 80 O, temps 25 s, population 1, pénétration 50 %. Contre un Homme d'armes (armure 4 -> 2) : 28 par tir, 7 par seconde, contre 8 + 10 = 18 par tir, 8,5 par seconde pour l'Arbalétrier (bonus Lourd compris) ; contre une cible sans armure 7,5 par seconde (Arbalétrier 5,6). Gros coup rare, fragile, cher en or : fait pour finir un Lourd, pas pour tenir une ligne.
- B : version plus douce : attaque 24, intervalle 3,5 s (6,9 par seconde sans armure), portée 1700, coût 60 N / 60 O, pénétration 40 %.
- C : version lourde : 90 PV, attaque 40, intervalle 5,0 s, portée 1500, coût 70 N / 100 O, pénétration 60 %.
- Autre : tes chiffres.
Conséquence visible : un groupe d'Arquebusiers au palier 3 fait fondre les Hommes d'armes (en A, 6 tirs de 28 tuent un Homme d'armes de 155 PV : 2 Arquebusiers en deux salves, soit environ 12 s ; 6 Arquebusiers en une seule salve) mais meurt vite à la charge du Cavalier léger (§ 7.2).

## Q2 : la pénétration compte-t-elle contre un bâtiment ? (G2, `GetBuildingDamage`, armure de tir 40)
- **A (recommandé)** : oui, c'est une arme à poudre : 50 % de l'armure de tir 40 ignorés, donc un Arquebusier abîme un bâtiment nettement plus qu'un Arbalétrier.
- B : non, la pénétration ne vaut que contre les unités (le siège reste le rôle du Canon et du Trébuchet).
- Autre.

Recommandation : Q1 = A, Q2 = B si tu veux garder le siège aux engins (le dossier Palier 3 y tient : « finir ou renverser » passe par le Trébuchet et le Canon) ; A si tu veux que la poudre soit utile contre les murs. Ma préférence mesurée : Q2 = B (un seul rôle pour les bâtiments).

Valeur par défaut codée derrière un réglage (une ligne à changer) : Q1 = A dans `Tools\Data\UnitTypes.csv` (ligne `Arquebusier`, G3, à écrire avec le lot C++ de G2/G3), Q2 = B (le coup contre un bâtiment ignore la pénétration) dans `HOLConstructionRules.cpp:335-339` derrière une constante. Rien n'est encore codé : G2 et G3 attendent leur lot C++.

Reponse (utilisateur, 2026-11-09) : Q1 = A MAIS attaque 40 (au lieu de 30 : 70 PV, vitesse 720, attaque 40, intervalle 4,0 s, portee 1500, 60 N + 80 O, 25 s, penetration 50 %) ; Q2 = B (la penetration ne compte pas contre un batiment). Palier 3 G2 (penetration d'armure, C++) AUTORISE. A coder au prochain Auto_Journey (G2 puis G3 puis G4), test rouge d'abord.
