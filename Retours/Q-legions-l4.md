# [À RÉPONDRE] Q-legions-l4 : le Guerrier Damné (L4) et sa rage nécrotique

Contexte : L4 crée la ligne `DamnedWarrior` (Légions, Caserne, palier 1, D395) et sa capacité *rage nécrotique*, premier usage du cadre de capacités d'unités (LQ-3 = A, D678). Le GDD ne donne aucun chiffre (§ 13.3 : « frontline / berserker, dégâts élevés, se consume au combat »), le dossier dit « valeurs à proposer » (D345 : tu tranches). Rien n'est codé ; l'agent de L4 démarre dès ta réponse, ou avec les valeurs par défaut ci-dessous si tu ne réponds pas.

Voisins de référence (`Tools\Data\UnitTypes.csv`, `UnitStats.md`) : Homme d'armes 155 PV, attaque 12, cadence 1,375 s (8,7 dégâts/s), vitesse 697, armures 4 / 4 / 0 / 2 (mêlée / distance / siège / héros), 100 N + 20 O, 22 s, palier 1. Lancier 80 PV, 4,3 dégâts/s, vitesse 777. Cavalier léger 120 PV, 6 dégâts/s, vitesse 875. Paladin-Commandant 450 PV, 24 dégâts/s. Garde noire (Homme d'armes des Légions) : 75 N + 15 O (-25 %, D66).

## Q1 : les chiffres du Guerrier Damné (unité de base, avant la rage)

Source commune à toutes les lignes : catégories Infanterie lourde + Lourd (L4 du dossier, D428) + Profane (D393) ; Caserne (D395, D128) ; palier 1 (`MinTier` 1 comme l'Homme d'armes) ; 1 place de population ; sans bonus de catégorie ; attaque contre bâtiment 25 par coup toutes les 2,1 s, portée 400 (l'Homme d'armes : 20) ; rayon d'agressivité, de vue, corps (rayon 60) identiques à l'Homme d'armes.

| Valeur | A (recommandé) | B | C | Source GDD / voisin |
|---|---|---|---|---|
| Profil | berserker fragile | bruiser solide | canon de verre | § 13.3 « frontline / berserker, dégâts élevés » |
| PV | 130 | 170 | 100 | un peu sous l'Homme d'armes (155) car « se consume » |
| Attaque / cadence | 16 / 1,5 s (10,7 dégâts/s) | 14 / 1,5 s (9,3) | 22 / 1,5 s (14,7) | « dégâts élevés » : plus que l'Homme d'armes (8,7) |
| Armures mêlée / distance / siège / héros | 2 / 2 / 0 / 1 | 3 / 3 / 0 / 2 | 0 / 0 / 0 / 0 | l'Homme d'armes 4 / 4 / 0 / 2 ; les tireurs et l'Arbalétrier (+10 contre Lourd) le contrent |
| Vitesse | 740 | 697 | 800 | entre l'Homme d'armes (697) et le Lancier (777) |
| Coût | 90 N + 25 O | 100 N + 30 O | 70 N + 20 O | D66 (Légions moins chères) mais emblématique (D82) : au-dessus de la Garde noire (75 N + 15 O) |
| Temps de production | 24 s | 26 s | 20 s | Homme d'armes 22 s |
| Contre un Homme d'armes de l'Aube, sans rage | l'Homme d'armes gagne de peu (environ 18 s contre 19 s) | le Guerrier gagne de peu | l'Homme d'armes gagne de peu | calcul à la main, non simulé |
| Contre 10 Archers | fond en quelques secondes (4 dégâts par flèche) | tient un peu plus | fond tout de suite | l'Archer fait 5, moins l'armure de distance |

Options :
- **A (recommandé)** : berserker fragile. Plus offensif que l'Homme d'armes, moins protégé : avec la rage (Q2) il gagne son duel contre l'Homme d'armes avec environ 20 % de ses PV, sans elle il perd de peu. Il oblige l'Aube à garder des tireurs ou des Arbalétriers.
- B : bruiser. Presque un Homme d'armes plus cher qui frappe un peu plus fort ; la rage devient un bonus plutôt qu'une nécessité ; moins de contraste avec le socle.
- C : canon de verre. Très violent, très fragile ; le moindre Archer le punit ; le scénario `legions_warrior` sera plus spectaculaire mais l'équilibre dépendra du nombre.
- Autre : tes chiffres (même tableau, ligne par ligne).

## Q2 : la rage nécrotique (premier usage du cadre de capacités, LQ-3)

Le cadre (technique, déjà tranché par D678) : une table de capacités en données (`Tools\Data\UnitAbilities.csv`, une ligne par capacité), des règles pures en Angelscript, un bouton dans la grille de la sélection, la file d'ordres existante. Ce qui se décide ici : ce que fait la rage et ce qu'elle coûte.

| Valeur | A (recommandé) | B | C | Source GDD / voisin |
|---|---|---|---|---|
| Déclenchement | à la demande (bouton de la sélection, tous les Guerriers sélectionnés) | automatique, sans bouton | bascule marche / arrêt | § 13.3 « rage nécrotique », LQ-3 (le cadre existe pour les boutons) |
| Durée | 10 s | tant qu'il combat | jusqu'à l'arrêt ou 20 PV | D127 : la *Fosse des damnés* la rend « plus longue » (A : 15 s avec la Fosse, +50 %) |
| Effet | dégâts +40 %, cadence +25 % (intervalle 1,5 s devient 1,2 s) | dégâts +15 % | dégâts +25 %, cadence +15 % | effets de moral existants (`Effects.csv`, colonnes Dealt / Rate) |
| Coût en PV | 3 PV par coup porté (environ 25 PV sur 10 s) | 1 PV par seconde de combat | 1,5 PV par seconde | § 13.3 « se consume au combat » ; D122 (la rage du héros, Seigneur Damné, est un autre sujet, L8b) |
| Recharge | 40 s (depuis le début) | aucune | aucune | longue recharge comme la Dématérialisation du Spectre (D396) |
| Garde-fous | refusée sous 30 PV ; s'arrête à 20 PV ; ne tue jamais le Guerrier | s'arrête à 20 PV | s'arrête à 20 PV | pas de suicide involontaire |
| L'IA | lance si un ennemi de mêlée est à 2 cases et PV au-dessus de 60 % | rien à faire | décide comme un joueur (plus complexe) | § 14.4 : toute règle lisible par l'IA |

Options :
- **A (recommandé)** : rage à la demande, 10 s, 40 s de recharge, 3 PV par coup. Le joueur choisit le moment (un Guerrier lancé au mauvais moment s'use pour rien), l'IA suit la même règle ; c'est ce qui fait réellement tester le cadre (bouton, recharge, refus, effet, durée) avant le Nécromancien et le Spectre.
- B : rage automatique sans bouton. Plus simple à coder (pas de bouton ni de recharge) et fidèle à « se consume au combat », mais aucune décision pour le joueur et le cadre LQ-3 n'est pas exercé à L4 (il le serait à L6 avec le Nécromancien).
- C : bascule marche / arrêt. Contrôle total, mais le joueur doit surveiller les PV de chaque Guerrier ; l'IA doit savoir arrêter ; plus de cas à tester.
- Autre : tes chiffres.

## Recommandation

Q1 = A, Q2 = A. Résultat visible en jeu : le Guerrier Damné est la masse d'assaut des Légions, qui coûte plus qu'une Garde noire, perd un duel isolé contre un Homme d'armes sans la rage et le gagne avec ; l'Aube répond par des tireurs ou des Arbalétriers et en refusant le corps à corps prolongé.

## Valeur par défaut qui serait codée (si pas de réponse)

Q1 = A dans la ligne `DamnedWarrior` de `Tools\Data\UnitTypes.csv` ; Q2 = A dans une ligne de `Tools\Data\UnitAbilities.csv` (durée, recharge, coût par coup, plancher de PV, une ligne à changer) et une ligne `NecroticRage` de `Tools\Data\Effects.csv` (dégâts et cadence). Les chiffres sont indicatifs, à régler à l'essai (scénario `legions_warrior`). Le détail de l'implémentation est dans `LegionsInProgress.md`, étape L4.

Reponse (utilisateur, 2026-11-09) : Q1 = A (berserker fragile) ; Q2 = A (rage a la demande, 10 s, +40 % degats, +25 % cadence, 3 PV par coup, recharge 40 s). L4 a coder au prochain Auto_Journey (plan en 8 etapes dans LegionsInProgress.md).
