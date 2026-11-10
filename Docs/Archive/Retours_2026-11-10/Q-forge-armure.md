# [À RÉPONDRE] Forge F2 : que couvrent Maille (armure infanterie) et Barde (armure cavalerie) ?

Contexte : F2 (FortificationsInProgress.md, D695) branche les armures de la Forge dans le calcul des dégâts (C++, pas lancé : aucun agent C++ libre, et le dossier dit « les lourds et les héros : à demander à l'utilisateur »). La fiche de combat (D345) retire à la cible « son armure du type de l'attaque » (Mêlée, Distance, Siège, Héros) ; une unité a une ou plusieurs catégories d'armure. Le GDD § 11.1 dit seulement « Maille (armure infanterie), Barde (armure cavalerie) ».

Deux choses à fixer :
1. QUELLES UNITÉS : Maille couvre quelles catégories d'armure ? (Infanterie légère, Infanterie lourde, Distance, Infanterie à poudre, Paysan, héros ; Barde = Cavalerie, avec ou sans Lourd.)
2. QUEL TYPE D'ATTAQUE est réduit : la Maille réduit-elle les dégâts de mêlée seulement, de tir seulement, ou les deux ?

Options :
- **A (défaut codé)** : Maille = les unités d'Infanterie légère, d'Infanterie lourde et d'Infanterie à poudre (donc Lancier, Homme d'armes, Moine, Arquebusier) ; PAS les tireurs « Distance » (Archer, Arbalétrier), PAS le Paysan, PAS les héros. Barde = la catégorie Cavalerie (légère et lourde). Le bonus s'ajoute à l'armure de MÊLÉE ET de DISTANCE (+pourcentage de la table : I = 10, II = 20). Conséquence visible : l'infanterie de mêlée résiste mieux aux Archers et aux Lanciers ; les tireurs ne gagnent rien (ils restent fragiles, ce qui garde leur rôle).
- B : comme A mais Maille couvre aussi les tireurs « Distance » (toute l'infanterie à pied sauf Paysan et héros). Plus simple à dire (« toute l'infanterie »), mais les Archers deviennent plus durs à tuer.
- C : comme A mais le bonus ne réduit QUE les dégâts de mêlée (Maille) ; Barde réduit mêlée et tir. Plus de nuance, plus de texte d'infobulle.
- Autre : par exemple inclure les héros, ou compter le Paysan.

Recommandation : A (suit les catégories d'armure du GDD § 7 sans en inventer ; les héros ont leur propre armure et leurs capacités, les ajouter changerait leur équilibre).

Valeur par défaut codée (à venir avec F2, rien n'est codé aujourd'hui) : une table `ForgeArmorCategories` (une ligne par technologie dans les données, une ligne à changer) ; tant que F2 n'est pas lancée, `InfantryArmorI/II` et `CavalryArmorI/II` restent `Implemented=False` (grisées « Bientôt », D601).

Reponse (utilisateur, 2026-11-09) : option B = Maille couvre aussi les tireurs Distance (toute l'infanterie a pied sauf Paysan et heros) ; Barde = cavalerie. Sens des 10/20 : A (pourcentage de coup retire apres l'armure plate, minimum 1, plafond 90 %). A faire au prochain Auto_Journey : passer ForgeArmorCategories de A a B (une ligne de table) + test.
