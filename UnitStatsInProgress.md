# Stats des unités — valeurs demandées par l'utilisateur

Questionnaire du 2026-10-22, une unité à la fois. Rien n'est encore appliqué au CSV (`Tools/Data/UnitTypes.csv`) :
on applique une fois toutes les unités passées. **Vitesses et portées fixées en cases** (choix de l'utilisateur, 2026-10-22) : elles suivent la grille. Une case = **4 m** (D311 revu par la session mycrusader-4b le 2026-10-22, branche tile-4m ; avant 3 m). Portées entre les bords des corps.

Colonnes nouvelles (règles à coder, décision au GDD) : armure contre Siège, armure contre Héros, attaque contre
bâtiment (dégâts, portée, vitesse), deuxième attaque (dégâts, bonus, portée, vitesse).

## Paysan (fait)

| Stat | Avant | Demandé |
|---|---|---|
| Santé | 50 | 50 |
| Armure CaC | 0 | 0 |
| Armure Distance | 0 | 0 |
| Armure Siège | — | 0 |
| Armure Héros | — | 0 |
| Vitesse | 2 cases/s (6 m/s à 3 m) | **2 cases/s (8 m/s à 4 m)** |
| Attaque | 3 | 3 |
| Bonus | aucun | aucun |
| Portée | 0,5 m | **0,29 case (1,16 m)** |
| Vitesse d'attaque | 2 s | **3,7 s** |
| Attaque bâtiment | — | **10** |
| Portée bâtiment | — | **1 case (4 m)** |
| Vitesse bâtiment | — | **2,1 s** |
| Deuxième attaque | — | aucune |

## Piquier, Homme d'armes, Archer (premiers jets de Claude, corrigés par l'utilisateur le 2026-10-22)

Logique : le modèle AoE4 (D21), d'où viennent déjà les valeurs actuelles. Ce qui existe est gardé (santé, attaque,
armures, cadence, vitesse, déjà au niveau AoE4) ; les colonnes nouvelles suivent tes choix pour le Paysan :
- portée de mêlée 0,29 case pour tous les soldats de mêlée, comme pour le Paysan ;
- attaque contre bâtiment : portée 1 case, vitesse 2,1 s pour tous, comme le Paysan ; dégâts 15 pour les soldats
  de mêlée, 10 pour l'Archer (moins bon à la torche) : **estimation**, à confirmer ;
- armure Siège et armure Héros : 0 partout (pas d'unité anti-héros, D62 ; aucun soldat du socle ne résiste au siège) ;
- deuxième attaque : aucune pour ces unités d'infanterie, elle viendra avec la cavalerie.

| Stat | Piquier | Homme d'armes | Archer |
|---|---|---|---|
| Santé | 80 | 155 | 70 |
| Armure CaC | 0 | 4 | 0 |
| Armure Distance | 0 | 4 | 0 |
| Armure Siège | 0 | 0 | 0 |
| Armure Héros | 0 | **2** | 0 |
| Vitesse | **1,94 case/s (7,77 m/s)** | **1,74 case/s (6,97 m/s)** | **1,95 case/s (7,79 m/s)** |
| Attaque | 8 | **12** (avant 8) | 5 |
| Bonus | +17 contre Cavalerie | aucun | +5 contre Infanterie légère **et +5 contre Infanterie à poudre** |
| Portée | **0,29 case (1,16 m)** | **0,29 case (1,16 m)** | **5 cases (20 m)** |
| Vitesse d'attaque | 1,875 s | 1,375 s | 1,625 s |
| Attaque bâtiment | **20** | **20** | **5** (comme son attaque) |
| Portée bâtiment | **1 case (4 m)** | **1 case (4 m)** | **5 cases (20 m)** (comme son attaque) |
| Vitesse bâtiment | **2,1 s** | **2,1 s** | **1,625 s** (comme son attaque) |
| Deuxième attaque | aucune | aucune | aucune |

En gras : ce qui change ou ce qui est nouveau.

**Infanterie légère** : laissée telle quelle, comme unité de test. Les stress tests S1 et S2 la font marcher par
milliers : changer sa vitesse ou sa portée changerait leurs mesures, et elle ne coûte rien. Elle garde donc ses valeurs **en mètres** (3 m/s, 0,5 m), même avec la case à 4 m. Elle ne serait pas
produite en partie.

## Tranché : case de 4 m

Valeurs en cases (choix de l'utilisateur, la recommandation) : les vitesses et portées grandissent en mètres avec la
case (un tiers de plus), les corps restent à 60 cm. Le CSV restant en cm, l'application convertira au pas de 4 m.

## Retours de l'utilisateur sur les premiers jets (2026-10-22)

- Attaque bâtiment : 20 pour l'Homme d'armes et le Piquier ; l'Archer, la même que son attaque simple.
- Homme d'armes : 12 de dégâts de base, 2 d'armure contre Héros.
- Archer : +5 contre l'infanterie à poudre. Catégorie d'armure nouvelle : le GDD range l'Arquebusier dans « distance »
  (§ palier 3, D58) ; à créer (« Infanterie à poudre ») et à y ranger l'Arquebusier.
- Tranché : les deux bonus de l'Archer s'additionnent sur leurs cibles respectives (plusieurs bonus par unité, à
  coder) ; « Infanterie à poudre » devient une catégorie d'armure à part, l'Arquebusier y passe (GDD à corriger, D343).

## Suite (plan validé par l'utilisateur le 2026-10-22)

1. **Sans code** : reporter dans `Tools/Data/UnitTypes.csv` ce que le jeu lit déjà (santé, attaque, armures CaC et
   Distance, vitesse, portée, cadence, bonus existants), en cm à 4 m la case, **une fois tile-4m fusionné dans main** (après l'essai et la validation de l'utilisateur, au moins une heure ; la session tile-4m prévient à la fusion)
   (session mycrusader-4b, worktree HeroesOfLiberty-J5c). Puis relancer `python Tools/Data/make_unit_stats.py`
   (le passer en cases) et réimporter la table (`Tools/Editor/make_unit_types.py`).
2. **Avec code** : armures Siège et Héros, attaque contre bâtiment, plusieurs bonus par unité, catégorie
   « Infanterie à poudre », deuxième attaque. D'abord une décision au GDD (**D345**, réservé avec la session tile-4m : D343 = état d'unité sur 59 bits, D344 = case de 4 m et cartes de 220 cases),
   en vérifiant que la review du GDD n'écrit pas en même temps.

Rien n'est commité : `UnitStats.md`, `Tools/Data/make_unit_stats.py` et ce fichier attendent la validation.

## Préparation de D345 (règles de combat), une question à la fois

Rien n'est écrit au GDD avant la fusion de tile-4m. Plan : 1 type d'une attaque, 2 armure Héros et ×0,3, 3 quand une
unité frappe un bâtiment, 4 armure des bâtiments, 5 plusieurs bonus, 6 deuxième attaque, 7 catégorie du Paysan
(la matrice du GDD donne le bonus de l'Archer contre « Lancier, Paysan… », le jeu range le Paysan à part).

1. **Type d'une attaque — A** (choix de l'utilisateur, la recommandation) : chaque attaque porte son type (Mêlée,
   Distance, Siège, Héros), écrit dans la table à côté de ses dégâts ; on soustrait l'armure de ce type. Une deuxième
   attaque peut avoir un autre type. Soldats d'aujourd'hui : Mêlée (Paysan, Piquier, Homme d'armes), Distance (Archer).
   Écartés : B (type déduit de l'unité, la deuxième attaque ne peut pas changer de type), C (type déduit de la cible,
   la torche deviendrait du siège et l'armure Siège ne servirait plus).
2. **Armure Héros et ×0,3 — A** (choix de l'utilisateur, la recommandation) : les attaques d'un héros (attaque simple
   et dégâts de ses compétences) sont de type Héros ; la cible retire son armure Héros, 1 au moins. Des troupes vers un
   héros : dégâts normaux (armure du héros comprise), puis ×0,3 (D17), sauf une attaque de type Siège (dégâts
   normaux) : le type Siège porte l'exception de D17. Tours (plus tard) : attaque de type Siège ou ajoutées à
   l'exception. Écartés : B (compétences hors armure Héros, l'armure ne pèserait presque rien), C (armure Héros en
   pourcentage, seule armure hors du modèle AoE4).
3. **Quand une unité frappe un bâtiment — B** (choix de l'utilisateur, **contre la recommandation A**) : contre un
   bâtiment, l'unité se sert toujours de son attaque contre bâtiment. Sur ordre (clic droit sur un bâtiment ennemi),
   en attaque-déplacement (unités ennemies d'abord, bâtiments s'il n'en reste aucune) **et au repos** : une unité qui
   a un bâtiment ennemi dans son rayon d'agressivité, sans unité ennemie, va le frapper d'elle-même. Une unité
   ennemie qui arrive lui fait lâcher le bâtiment. Le Paysan (rayon 0) ne frappe un bâtiment que sur ordre.
   Écartés : A (AoE4 : jamais au repos), C (seulement sur ordre).
4. **Armure des bâtiments — une armure contre les dégâts à distance seulement** (choix de l'utilisateur, ni A, ni B,
   ni C) : colonne « armure Distance » dans `Tools/Data/BuildingTypes.csv`, retirée des attaques contre bâtiment de
   type Distance (l'Archer), 1 au moins. La torche (Mêlée), le siège et les héros n'ont pas d'armure en face ; la
   solidité se règle par les points de vie et l'attaque contre bâtiment de chaque unité. Valeur : voir ci-dessous.
   Valeur : **40 partout** (choix de l'utilisateur, en points ; propositions 2, 3 ou 4 écartées). Effet : tout tir de
   moins de 41 dégâts ne fait qu'1 dégât à un bâtiment (Archer seul contre une maison : ~16 min ; 10 Archers ~1 min 40 s).
5. **Plusieurs bonus — B** (choix de l'utilisateur, contre la recommandation A) : une unité peut avoir **plusieurs
   catégories d'armure** (l'Arquebusier : Distance et Infanterie à poudre) et les bonus d'une attaque
   **s'additionnent** pour chaque catégorie de la cible qu'ils visent. Chaque attaque garde une liste de bonus (Archer :
   Infanterie légère +5, Infanterie à poudre +5). Écartés : A (une catégorie par unité, un seul bonus par coup),
   C (un bonus par attaque, le second porté par la deuxième attaque).
   *Précisé par l'utilisateur* : une unité a **de 1 à autant de catégories qu'on veut, sans plafond** (la table
   écrit une liste ; pas de nombre fixe de colonnes).
6. **Deuxième attaque — A** (choix de l'utilisateur, la recommandation) : ses propres dégâts, bonus, type, portée et
   vitesse, et une colonne **déclencheur** à liste courte. Premier déclencheur : **Charge** — le premier coup sur une
   cible après au moins X cases de course vers elle est porté par la deuxième attaque, puis l'attaque simple reprend ;
   sa « vitesse » est le temps avant de pouvoir recharger. X par unité dans la table (premier jet avec la cavalerie).
   Autres déclencheurs ajoutés au besoin (ex. javelot à distance, mêlée au contact). Écartés : B (selon la distance de
   la cible, pas de charge), C (deux attaques en même temps, pas de charge).
7. **Catégories — B pour le Paysan** (choix de l'utilisateur, contre la recommandation A) : le Paysan reste dans la
   seule catégorie Paysan ; l'Archer n'a pas son bonus contre lui (5 dégâts, 10 flèches, ~15 s). **Matrice du GDD à
   corriger** (§ 7.2 : « infanterie légère (Lancier, Paysan…) » → sans le Paysan). Autres unités, proposées sans
   objection : Piquier = Infanterie légère ; Homme d'armes = Infanterie lourde ; Archer = Distance ; Arquebusier
   (plus tard) = Distance + Infanterie à poudre. Écarté : A (Paysan + Infanterie légère, comme la matrice).

**Les 7 questions sont tranchées.** À écrire au GDD comme **D345** après la fusion de tile-4m (§ 7.2 matrice et
mise en œuvre, § 6.1 siège, § 9.1 bis résistance héroïque, tableau des décisions), puis le code.

## Avancement après la fusion de tile-4m (main e5815db, 2026-10-22)

- [x] `Tools/Data/UnitTypes.csv` : valeurs que le jeu lit déjà, à 4 m la case. Paysan vitesse 800, cadence 3,7 s,
      portée 116 ; Piquier vitesse 777, portée 116 ; Homme d'armes vitesse 697, attaque 12, portée 116 ; Archer vitesse
      779, portée 2000. Rayons d'agressivité aussi passés en cases (×4/3 : 1600, Archer 2133) et vitesses de lecture de
      la marche ×4/3 (pieds sans glissement, D234). Infanterie légère inchangée.
- [x] `UnitStats.md` en cases (lit `TileSize` de `Config/DefaultGame.ini`).
- [x] **D345 écrit au GDD** (ligne du journal entre D344 et D346, § 7.2 matrice et fiche de combat, § 9.1 bis,
      numéro suivant du § 22) ; non commité.
- [x] **Worktree `HeroesOfLiberty-Haldir`** (nom choisi par l'utilisateur), branche `unit-stats` depuis main 0886717
      (IN_USE.txt posé) : la session du moulin travaille dans la copie principale. Le CSV, `make_unit_stats.py` et
      `UnitStats.md` y sont copiés ; les copies de la copie principale (CSV modifié, deux fichiers non suivis) sont à
      retirer avant la fusion (le classificateur a refusé de les effacer : demander à l'utilisateur).
- [ ] Dans Haldir (sous-agent) : compilation par `Toolsuild.ps1 -Root`, réimport de `/Game/Units/UnitTypes`, tests
      mis à jour (HOLCombatTests, HOLAnimationTests, HOLProductionTests), `quick_check -Parts tests,production,gather,build,anims,combat`.
- [ ] Tests touchés (combat, récolte) après l'import, puis validation de l'utilisateur et commit.
- [ ] Code de D345 (types d'attaque, 4 armures, attaque contre bâtiment, catégories multiples, bonus en liste,
      deuxième attaque) ; avec lui `HOLCombat::AttackMoveRadius` 1200 → 1600 (4 cases).
