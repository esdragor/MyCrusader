# Cavalerie : Écurie, Cavalier léger, Cavalier lourd

Ouvert le 2026-11-04 (Claude, session principale, pendant l'absence de l'utilisateur). **Rien n'est codé.** Ce journal pose l'état des lieux, ce qui peut se faire sans C++, et les questions qui attendent l'utilisateur. Les valeurs ci-dessous sont des PROPOSITIONS : D345 veut des stats choisies une par une par l'utilisateur.

## État des lieux (vérifié dans le dépôt le 2026-11-04)

- **Il n'y a aucune cavalerie dans le jeu** : ni ligne dans `Tools/Data/UnitTypes.csv` (Paysan, Piquier, Homme d'armes, Archer, Arbalétrier, Roi, héros, Moine, Bélier, Mangonneau), ni Écurie dans `Tools/Data/BuildingTypes.csv` (Caserne, Champ de tir, Atelier de siège, Chapelle…). Seuls existent : la catégorie d'armure `Cavalry` (`HOLUnitTypes.h`), le bonus du Piquier (+17 contre Cavalerie), les bonus prévus dans le GDD (§ 7.1, § 7.2, D345, D428) et la ligne « Forge : Barde » des technologies.
- **GDD** : Écurie = bâtiment du palier 1 (D128, § 7.3 tableau des paliers) ; Cavalier léger au prototype (§ 8 « proposition », § 15 Unités), Cavalier lourd **hors prototype** (palier 2, D428 : Cavalerie + Lourd). Cavalier léger : raid, harcèlement, éclairage ; fort contre tireurs et siège, faible contre Lancier et Cavalier lourd. Cavalier lourd : charge, fort contre infanterie légère et tireurs, faible contre Lancier et Arbalétrier. La forêt ralentit la cavalerie, le marais **empêche la charge** (§ 9).
- **Charge (D345 point 6, D345.3)** : deuxième attaque à déclencheur (« le premier coup après au moins X cases de course »), remise à la cavalerie par l'utilisateur ; X par unité. C'est du C++ de simulation (jamais commencé).

## Ce qui est de la donnée, du script ou du C++

| Morceau | Nature | Sans C++ ? |
|---|---|---|
| Ligne Écurie (`Stable`) dans `BuildingTypes.csv`, `Produces` | donnée (CSV, asset refait par le script d'éditeur) | oui, mais l'éditeur doit tourner et les tests C++ qui comptent les bâtiments / types changent |
| Ligne Cavalier léger dans `UnitTypes.csv` (PV, attaque, vitesse, bonus contre Distance et Siège, armure, coût, temps) | donnée | oui |
| Bouton de la grille de l'Écurie, texte fr / en / ja, icône | script + `fill_po.py` | oui (icône : art) |
| IA : production de cavalerie, table de contres (D662 pour les talents, D424) | script (`HOLAI*.as`) | oui |
| Forme du cheval : maillage, squelette, animations (marche, galop, frappe, chute) | ART : rien dans `Content/Units` hors Mannequin ; chaîne `AnimationsInProgress.md` | non, demande des assets ; **placeholder possible** (Mannequin agrandi ou boîte, comme le Bélier) |
| Vitesse de galop, ralentissement en forêt, collisions de grand corps (rayon plus large) | simulation (Mass) | **non : C++** |
| Charge (deuxième attaque à déclencheur), marais anti-charge | simulation | **non : C++** |
| Tests (stats, production, matrice des contres) | C++ (`HOL*Tests.cpp`) ou script (`*_Test.as`, D484) | en script oui |

Conclusion : **un Cavalier léger sans charge est faisable presque entièrement en donnée + script** (une fois l'éditeur lancé pour refaire les assets), avec un placeholder pour le cheval ; la charge, la forêt et le marais sont du C++ à part (le Cavalier lourd en dépend, lui, et il est hors prototype).

## Propositions de valeurs (à trancher une par une, comparables aux lignes de `UnitStats.md`)

Cavalier léger (Écurie, palier 1) : catégories `Cavalry` ; PV 120 ; attaque 9, Mêlée, cadence 1,5 s ; bonus +10 contre `Distance` (Archer, Arbalétrier) et +20 contre `Siège` ; armure mêlée 1, distance 2 (couvre à peine une flèche d'Archer à 5) ; portée mêlée 0,17 case ; vitesse ≈ 875 cm/s (la valeur déjà citée comme « la vitesse d'un Cavalier léger » dans `JourneyInProgress.md`, fuite ×1,5 du Lancier) ; rayon d'agressivité 5 cases ; attaque contre bâtiment 10 (il ne sert pas à raser) ; coût 80 nourriture + 40 or ; production 20 s. Contres à vérifier au test : Piquier (80 PV, +17 contre Cavalerie : 25 par coup) le tue en 5 coups ; l'Archer à 5 cases le perce lentement (5 - 2 = 3 ; 1 dégât minimum).

## Questions pour l'utilisateur

- **CV-Q1** : le Cavalier léger a-t-il une charge au prototype ? A = non, la charge arrive avec le Cavalier lourd (recommandé : le GDD réserve la charge au lourd, et cela rend le léger faisable sans C++) ; B = oui, une charge légère (X cases de course, premier coup à +50 %), donc C++ d'abord.
- **CV-Q2** : le cheval au prototype : A = placeholder (Mannequin sur un volume de cheval, ou une boîte, assumé) pour tester les contres, vrai modèle plus tard (recommandé) ; B = d'abord une dizaine d'échantillons de chevaux à choisir (méthode `assets-ten-samples`), puis les animations.
- **CV-Q3** : les stats du Cavalier léger ci-dessus : les valider d'un bloc, ou une par une (D345) ?
- **CV-Q4** : l'IA produit-elle de la cavalerie au prototype (Difficile seulement, ou tous les niveaux) ? Recommandé : Normal et Difficile, après un test où elle perd contre des Lanciers sans en faire.
- **CV-Q5** : l'Écurie a-t-elle sa couleur de joueur par bannière (VI-Q3 = B / D666) dès le premier modèle ?

## Prochaine étape quand l'utilisateur répond

CV-Q1 = A, CV-Q2 = A : (1) test rouge d'abord : une ligne `Stable` et une ligne `LightCavalry` attendues par `Tools/Data/make_unit_stats.py` et le test de production ; (2) CSV + script d'éditeur + `UnitStats.md` ; (3) bouton de l'Écurie, textes fr / en / ja ; (4) scénario `cavalry` (essai prêt, D-trial) : Cavalier léger contre Archers, contre Piquiers, contre Mangonneau ; (5) IA.

## Réponses de l'utilisateur (2026-11-04, D672 au GDD)

CV-Q1 = **B** (le Cavalier léger a une charge au prototype), CV-Q2 = **A** (cheval placeholder), CV-Q3 = **en bloc** (stats ci-dessus validées d'un bloc, à retoucher à l'essai), CV-Q4 = **tous les niveaux d'IA**, CV-Q5 = **oui** (bannières sur l'Écurie). Lecture : les cinq réponses « Oui / placeholder / bloc / tous / Oui » suivent l'ordre des questions ; CV-Q1 « Oui » est lu comme « charge oui » (B), à corriger si l'utilisateur voulait dire autre chose.

## Découpage

- **C1 (sans C++)** : Écurie + Cavalier léger en données et script, cheval placeholder, bannières, textes, IA, scénario d'essai ; la charge existe comme colonnes de la table mais sans effet.
- **C2 (C++, demande un build : à lancer seulement quand l'utilisateur le permet)** : deuxième attaque à déclencheur (charge) dans la simulation, marais anti-charge, ralentissement en forêt, rayon de grand corps.

## C1 faite, commitée, NON COMPILÉE, NON EXÉCUTÉE (2026-11-04, sous-agent dans Glorfindel, branche `cavalry`, 07758ae, ae961a8, 3dbf714)

Aucun fichier C++ touché (0 fichier `Source/` dans la différence), aucun build, aucun éditeur, aucun test lancé. Pas fusionnée dans `main`.

- **Données** : `Stable` (Écurie, 3x3, 1500 PV, palier 1 = page II de la grille, case D, 150 bois, 20 s, produit `LightCavalry`, boîte provisoire) dans `BuildingTypes.csv` ; `LightCavalry` dans `UnitTypes.csv` (120 PV, attaque 9 mêlée / 1,5 s, bonus +10 Distance et +20 Siège, armure 1 / 2, vitesse 875, 80 nourriture + 40 or, 20 s, `SM_Mannequin` x 2,25 sans animation comme le Bélier) ; `UnitStats.md` régénéré (Python pur) ; clés fr / en / ja dans `fill_po.py`.
- **Script** : `Script/AI/HOLAICavalry.as` (règles pures : Écurie voulue dès le palier 1 avec >= 6 soldats, plafond de cavalerie Facile 2 / Normal 4 / Difficile 6, le niveau joué par l'Évolutive), 8 tests, `HOLCavalryTable_Test.as` (3 tests), `HOLAIArmy.as`, `HOLAIBrain.as` (étape 3b4).
- **Essai** : `Tools/Scenarios/cavalry.txt` (+ .json, .look), jamais joué ; coordonnées des couloirs non vérifiées sur Plain ; `quick_check.ps1` attend `D=Stable greyed`.
- **Écarts** : portée 70 cm ; vue 4000 cm ; rayon 90, évitement 300 ; le Piquier fait 24 et non 25 (armure 1) ; aucune colonne de charge dans la table ; **pas de bannière sur l'Écurie** (le Forum à bannières n'est que dans la branche `visual`, V3b non faite : CV-Q5 reste à faire avec le chantier visuel) ; la règle « ne pas faire de cavalerie contre une armée de Lanciers » n'est PAS codée (lue comme un essai de référence, à faire en partie IA avec le build).
- **Dépendances C2 (C++)** : (1) `HOLGameNames.cpp` : LOCTEXT `Stable` et `LightCavalry` (sinon le nom reste en français dans toutes les langues), puis `fill_po.py` ; (2) la charge (aucune colonne dans `FHOLUnitTypeRow`, traitement dans `HOLCombatProcessor` ; `bMarshStopsCharge` existe déjà dans `HOLTerrainRules.as`) ; (3) forêt qui ralentit, galop, grand corps à vérifier en simulation ; (4) tests C++ qui deviendront rouges à la régénération des assets : `HOLCommandGridTests.cpp` (ligne 387 : 11 -> 12 types, case 6 de la page II, `Stable` parmi les bâtiments d'âge II, grille de l'Écurie finie) et `HOLConstructionTests.cpp` (bloc de la Chapelle à copier pour la Stable) ; à vérifier : `HOLNetTests`, `HOLTechnologyTableTests`.
- **À lancer quand le build sera permis** (depuis Glorfindel, PowerShell, chemins absolus) : fusionner `main` ; `Toolsuild.ps1` ; régénérer `UnitTypes.uasset` et `BuildingTypes.uasset` avec l'éditeur (`make_unit_types.py`, `make_building_types.py`, `-LiveCoding=0`) ; corriger les tests C++ et ajouter les LOCTEXT ; `quick_check -Parts tests`, puis `-Parts build`, puis `-Parts aiduel` (chercher « build Stable » et « queue a LightCavalry ») ; `preflight.ps1 cavalry` puis `play.ps1 cavalry` ; mesures S1 / S2 seulement à une clôture.

## Fusion et libération (2026-11-04, sur ordre de l'utilisateur : « ne build rien, mets tout à jour, merge dans main, libère Glorfindel, on fera tout ça plus tard »)

`cavalry` fusionnée dans `main` = 9f9c591 (docs GDD D663-D672 commitées avant : 035cecb) ; rien compilé, rien exécuté, rien poussé. Branche `cavalry` supprimée ; Glorfindel libre, sur `net-playtest` = `main`. **Reprise plus tard** : la liste « À lancer quand le build sera permis » de la section C1, puis C2 (LOCTEXT, charge, forêt, tests C++ de la grille).
## Pré-vol de `cavalry` joué (2026-11-05 19:43-20:05, Glorfindel, branche `net-playtest`, sans build : les assets `UnitTypes` et `BuildingTypes` contenaient déjà `LightCavalry` et `Stable`)
- `preflight.ps1 cavalry -Root` : **PASS** (100 s, 70 s d'attentes explicites, 0 `waitkey`, 6 notes, audit de sanité passé, aucun panneau ne recouvre un autre). Journal : l'Écurie sort, la production donne un Cavalier léger, 3 Archers tués par 4 coups chacun (Cavalier à 42 PV sur 120), le Cavalier meurt sous 2 Piquiers, le Mangonneau tombe en 7 coups : **les chiffres attendus des notes sont ceux du journal**.
- **Premier passage : 8 PNG ouverts, mais les combats NE SE VOYAIENT PAS** (caméra Zoom 7 trop proche : 35 m ne tiennent pas dans l'image ; le panneau du scénario, grand de 5 notes, cachait le côté ennemi). Corrigé dans `cavalry.txt` (commit c513da9) : Zoom 0, caméra au milieu du couloir décalée de côté, `panel hide` pendant les captures de départ et de fin puis `panel show`. **Deuxième passage : PASS, PNG rouverts** : Écurie bleue sélectionnée avec la case A « Cavalier léger » (vu), Cavalier léger sorti (un Mannequin agrandi, vu, petit), départ des trois couloirs (Cavalier en bas, ennemis en haut, vu), fin du couloir 1 (Cavalier à 42/120 entouré de 3 corps rouges, vu), fin du couloir 2 (Cavalier mort, un Piquier debout, vu), fin du couloir 3 (Mangonneau détruit, Cavalier 120/120, vu).
- **Statut : VU À L'ÉCRAN pour l'Écurie, la production et les trois résultats ; à surveiller à l'essai de l'utilisateur** : en couloir le Cavalier léger paraît **couché ou à plat** (Mannequin agrandi, sans animation, probablement une pose de déplacement), les cadavres d'un couloir restent dans l'image du couloir suivant (30 s) ; la charge n'existe pas (C2). Jamais joué par l'utilisateur : l'essai peut être listé après la fusion de `net-playtest` dans `main`, commande `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 cavalry -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` (vérifier la syntaxe de `play.ps1` avant de la donner).
