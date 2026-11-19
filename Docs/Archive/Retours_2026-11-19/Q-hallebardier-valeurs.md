# [À RÉPONDRE] Hallebardier de l'Aube : quelles valeurs ?

**Contexte.** Le Hallebardier est la première unité propre de l'Ordre de l'Aube (D79) : un « mur de piques » anti-cavalerie, avec un bonus contre l'infanterie lourde, et plus cher que le Lancier (60 nourriture + 20 bois). Aucun chiffre n'est écrit nulle part et la règle D345 veut que tu choisisses : il faut trancher maintenant pour que la ligne de données soit écrite à la prochaine séance (étape A1 de `AubeInProgress.md`).

Les chiffres du Lancier viennent de `Tools\Data\UnitTypes.csv` (ligne `Spearman`). Un « bonus » du jeu est une valeur **ajoutée** à chaque coup contre la catégorie visée (pas un multiplicateur), puis l'armure de la cible est retranchée. Pour garder le test de A1 « autant de dégâts à la cavalerie que le Lancier », les trois propositions gardent la même attaque (8), la même cadence (1,875 s), la même portée (130) et le même bonus contre la cavalerie (+17). Elles ne diffèrent que par le coût, les PV, la vitesse et le bonus contre l'infanterie lourde.

## Question 1 : quelles valeurs pour le Hallebardier ?

| | Coût (nourriture + bois) | Temps | PV | Attaque (cadence) | Vitesse | Portée | Bonus cavalerie | Bonus infanterie lourde |
|---|---|---|---|---|---|---|---|---|
| **Lancier actuel** (CSV) | 60 + 20 (= 80) | 15 s | 80 | 8 (1,875 s) | 777 | 130 | +17 | aucun |
| **A. Sobre** | 70 + 25 (= 95, +19 %) | 17 s | 85 | 8 (1,875 s) | 777 | 130 | +17 | +4 |
| **B. Équilibrée** | 75 + 30 (= 105, +31 %) | 18 s | 100 | 8 (1,875 s) | 740 | 130 | +17 | +8 |
| **C. Mur de piques** | 90 + 40 (= 130, +63 %) | 22 s | 120 | 8 (1,875 s) | 700 | 130 | +17 | +12 |

Aucun coût en or : l'Aube garde une infanterie légère payée en nourriture et en bois. « Infanterie lourde » vise la catégorie `HeavyInfantry` (Homme d'armes, Guerrier damné), pas l'étiquette `Heavy` (qui inclurait un futur Cavalier lourd).

Ce que tu verras en jeu (temps de combat 1 contre 1, calculés à la main avec les valeurs du CSV, approche et techniques non comptées ; Homme d'armes = 155 PV, armure 4, 100 nourriture + 20 or ; Cavalier léger = 120 PV) :

| | Contre un Homme d'armes seul | Contre un Cavalier léger | Contre un Lancier |
|---|---|---|---|
| **Lancier actuel** | le tue en 73 s, tombe en 9 s | le tue en 9 s | (miroir) |
| **A. Sobre** | le tue en 36 s, tombe en 10 s | le tue en 9 s, tombe en 14 s | gagne de justesse (20 s contre 19 s) pour 19 % de plus |
| **B. Équilibrée** | le tue en 24 s, tombe en 12 s | le tue en 9 s, tombe en 17 s | gagne nettement (23 s contre 19 s) |
| **C. Mur de piques** | le tue en 18 s, tombe en 14 s | le tue en 9 s, tombe en 20 s | gagne largement (28 s contre 19 s) |

- **A. Sobre** : presque un Lancier avec un petit supplément ; l'Homme d'armes reste très dur à tuer pour lui. Le coût ralentit peu l'armée de l'Aube, mais la différence se voit à peine.
- **B. Équilibrée** : bat le Lancier à coût proche, tue un Homme d'armes en deux fois moins de temps que le Lancier, mais perd encore ce duel seul ; il reste lent (740) et se fait rattraper par la cavalerie, qu'il tient mieux grâce à ses PV.
- **C. Mur de piques** : presque à égalité avec l'Homme d'armes et très solide face à la cavalerie, mais il coûte plus que beaucoup d'unités de palier 1 et il avance à la vitesse de l'Homme d'armes (700 contre 697) : l'armée de l'Aube monte lentement en nombre.

**Ma recommandation : B.** Elle respecte D79 (« plus cher », +31 %), donne un vrai rôle (contre la cavalerie et en appui contre les armures lourdes) sans en faire une unité qui remplace l'Homme d'armes, et laisse de la place pour la technologie *Hallebardes trempées* (bonus accru, palier 1) sans casser l'équilibre.

Autre : (dis tes propres valeurs, ou « A mais plus de PV », etc.)

Valeur par défaut codée : aucune (rien n'est codé tant que tu n'as pas répondu)

Reponse:

## Question 2 : le Hallebardier remplace-t-il le Lancier pour l'Aube, ou s'y ajoute-t-il ?

- **A. Il remplace le Lancier** (comme écrit dans D79 et D128 : « Lancier ou Hallebardier »). La Caserne de l'Aube propose le Hallebardier et plus le Lancier ; celle des Légions garde le Lancier (le Lancier squelette). Pour l'IA, l'Aube produit des Hallebardiers à la place des Lanciers dans son mélange fixe (même part de 2 sur 7 environ), et le Lancier doit être marqué « Légions seulement » dans la table pour que l'IA de l'Aube ne le planifie plus. Une case en moins dans la grille, un choix de moins pour le joueur, identité de l'Aube plus nette.
- **B. Il s'ajoute au Lancier.** La Caserne de l'Aube propose les deux (plus de cases à lire dans la grille, un choix « bon marché ou solide »). Pour l'IA, le Hallebardier entre comme un candidat en plus (part à choisir, par exemple 1 sur 8) à côté du Lancier, sur le modèle du Guerrier damné ; elle garde donc des Lanciers. Plus simple à coder (aucun filtre à retirer au Lancier) et moins risqué pour les tests existants, mais cela s'écarte de D79 et du compte « 2 variantes maximum » du § 7.1.

**Ma recommandation : A (il remplace)**, c'est ce que le GDD a décidé (D79, D128, A1) et cela donne à l'Aube une identité propre dès la première unité.

Autre : (dis ta propre règle)

Valeur par défaut codée : aucune (rien n'est codé tant que tu n'as pas répondu)

Reponse (2026-11-19 ~22:55) : D858. Valeurs : « B. Équilibrée » = 75 nourriture + 30 bois, 18 s, 100 PV, vitesse 740, attaque 8, cadence 1,875 s, portée 130, +17 contre la cavalerie, +8 contre l'infanterie lourde (catégorie HeavyInfantry). Lancier : « Il remplace le Lancier » pour l'Aube (les Légions gardent le Lancier). Reporte dans AubeInProgress.md (A1) et Auto_Journey_File.md. Fiche archivee dans Docs\Archive\Retours_2026-11-19\.
