# Stats des unités — valeurs demandées par l'utilisateur

> **Pour reprendre : lire la section « Reprise » en bas de ce fichier.** Valeurs dans main (7bda727) ; les règles de
> D345 restent à coder. Les sections suivent l'ordre du chantier ; les portées de mêlée des tableaux ci-dessous sont
> remplacées par la portée par arme (Piquier 130 cm, Homme d'armes 70 cm, voir plus bas).

Questionnaire du 2026-10-22, une unité à la fois. **Vitesses et portées fixées en cases** (choix de l'utilisateur, 2026-10-22) : elles suivent la grille. Une case = **4 m** (D311 revu par la session mycrusader-4b le 2026-10-22, branche tile-4m ; avant 3 m). Portées entre les bords des corps.

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
- [x] Dans Haldir (sous-agent, 2026-10-22) : compilation PASS, `/Game/Units/UnitTypes` réimporté, tests mis à jour
      (HOLCombatTests, HOLAnimationTests, HOLProductionTests), `quick_check -Parts tests,production,gather,build,anims,combat`
      **PASS** (tests 90/90, 467 s). Scénarios de quick_check adaptés : build cut (fenêtre 12 → 17 s, Paysans plus
      rapides en phase), combat et attack-move (7 coups de 12 au lieu de 10 de 8 ; groupes écartés de plus de 16 m,
      l'agression plus longue faisait partir le Lancier tout seul), arrows flee (Lancier à 20 m, tir attendu 1800-2120).
      À savoir : les scénarios doivent espacer les groupes de plus de 16 m (21,3 m pour les Archers) ; vitesse répliquée
      jusqu'à 12,8 m/s (800 cm/s tient), commentaire « 3 m/s » de HOLUnitReplicationRules.h périmé.
- [ ] Essai de l'utilisateur (`play.ps1 j7_army -Root ..\HeroesOfLiberty-Haldir`), puis validation et commit sur
      unit-stats, fusion dans main (retirer d'abord les copies du CSV, du script et du tableau dans la copie principale).
- [ ] Tests touchés (combat, récolte) après l'import, puis validation de l'utilisateur et commit.
- [ ] Code de D345 (types d'attaque, 4 armures, attaque contre bâtiment, catégories multiples, bonus en liste,
      deuxième attaque) ; avec lui `HOLCombat::AttackMoveRadius` 1200 → 1600 (4 cases).

## Portée de mêlée par arme (choix de l'utilisateur, relayé par mycrusader-43, J7c.4, 2026-10-22)

La portée de mêlée reste **propre à chaque arme**, comme la mêlée validée à l'essai de J7c.4 : **Piquier 130 cm,
Homme d'armes 70 cm** (bord à bord). Remplace les 0,29 case (116 cm) des soldats de mêlée ; **le Paysan garde 116 cm**.
Ces valeurs arrivent dans main avec la branche j7c-shot (avec ses AttackWindUpPercent) : à la fusion de main dans
unit-stats, garder dans UnitTypes.csv la portée et l'élan de j7c-shot, et le reste de unit-stats ; réimporter, puis
tests et combat. Copies de la copie principale (CSV, script, tableau) retirées par mycrusader-43 à la demande de
l'utilisateur (identiques à Haldir).

## Essai 1 (2026-10-22, j7_army dans Haldir) et fusion de main 48ffa00

- Vitesses : **bonnes** (utilisateur). Portée non testée : le scénario s'arrêtait avant l'ennemi (ordre de bâtir la
  caserne hors de l'écran, caméra ailleurs) ; corrigé dans Haldir (`hol.Camera.FocusNear -3800 4600` avant l'ordre).
- Superposition d'un Lancier, d'un Homme d'armes et d'un Archer envoyés au même point : à revoir après la fusion
  (MV.3 « immobiles superposées » avait été accepté, pas corrigé) ; sinon, au chantier déplacement.
- Ligne ajoutée à EtapeDefini (retours d'essai) : unité repoussée par un chantier posée dans ses murs.
- unit-stats avancée jusqu'à main 48ffa00 (j7c-shot, MV.5, moulin), travail remis par-dessus (stash gardé en réserve
  jusqu'à la fin) ; CSV : portées de main gardées (Piquier 130, Homme d'armes 70), le reste de unit-stats ; test
  « Spearman: reach » → 130. Sous-agent : compilation, réimport, quick_check court puis `-Parts tests,combat`.
- Après la fusion (sous-agent) : compilation PASS, réimport PASS, `quick_check` court PASS (21 s, tests 92/92),
  `-Parts tests,combat` PASS (165 s), aucune retouche de test. À reprendre avec le code de D345 : dans quick_check,
  la file d'attente (B à 15 m, désormais dans l'agression de 16 m : la vérification ne distingue plus Maj de
  l'agression ; éloigner B au-delà de 16 m), seuil « Peasant out of reach » de la tenue de position à 50 cm (Homme
  d'armes 70 cm), commentaires « 12 m » périmés. Essai 2 (j7_army corrigé) lancé.

## Essai 2 (2026-10-22, j7_army corrigé) : « tout a l'air bon », deux demandes

1. **Flèches 1,5 fois plus rapides** : `HOLCombat::DefaultArrowSpeed` 2500 → **3750 cm/s** (37,5 m/s) ; test de la
   vitesse par défaut suivi, tests du vol gardés sur une vitesse ronde de 25 m/s ; `UnitStats.md` régénéré.
2. **Règle de formation absolue** (à coder dans unit-stats, choix de l'utilisateur) : un tireur est **toujours
   derrière la mêlée** ; la **première ligne n'a jamais de tireur** ; dès la ligne 2, une ligne peut avoir des tireurs
   **au centre, flanqués de mêlée sur ses ailes** (« l'inverse » de la proposition « mêlée au centre, tireurs aux
   ailes »). Même règle pour le **siège derrière les tireurs**. Exemple 1 Lancier + 1 Homme d'armes + 1 Archer :
   ligne 1 `L H`, ligne 2 `A` (aujourd'hui : une seule ligne de 3).
   **Confirmé par l'utilisateur sur exemples** (largeurs des lignes : `MakeMixedRows`) :
   - 5 Lanciers + 2 Hommes d'armes + 6 Archers : `L L L L L` / `H A A A H` / `A A A` (tireurs au centre, flanqués par la
     mêlée restante dès la ligne 2) ;
   - 5 Lanciers + 7 Archers + 1 engin : `L L L L L` / `A A A A A` / `A A` / `M` — **le siège n'est pas flanqué** : il
     prend des lignes à lui, derrière toutes les lignes de tireurs (correction de l'utilisateur) ;
   - 1 Lancier + 8 Archers : `L` / `A A A` / `A A A` / `A A` — règle stricte : la première ligne n'a que la mêlée,
     même réduite à une unité.
- Flèches à 37,5 m/s : compilation PASS, `quick_check -Parts tests,combat` PASS après mise à jour de la vérification
  « each flight = distance / 37.5 m/s » (`$ArrowSpeed` 3750 dans quick_check.ps1), 171 s.
- Règle de formation : confiée à un sous-agent dans Haldir (HOLFormationRules, tests, scénario `formation_ranks`).
- Règle de formation codée (sous-agent) : `PlaceKinds` reçoit et rend les lignes (ligne 1 sans tireur ni siège,
  raccourcie ; siège en lignes à lui ; débordement en lignes de plus, larges comme l'avant-dernière donnée ; dans une
  ligne la classe la plus en arrière au centre) ; `GetRowPriority` 0-6 (Lancier, Homme d'armes, autre mêlée, Archer,
  autre tireur, siège, Paysan : l'Infanterie légère passe devant les Archers) ; `bRanged` / `bSiege` dans la vue des
  unités (non répliqué) ; aperçu du tracé aligné. Test `HeroesOfLiberty.Formation.Classes` ; tests 93/93. Exemples
  obtenus conformes (1 L + 1 H + 1 A : `H L` / `A`). quick_check formation : seuil de vitesse du bloc codé en dur à
  523 cm/s → lu dans le journal (`pace`, 697 cm/s). Scénario `formation_ranks` (4 groupes, 30 m d'écart). Choix du
  sous-agent à faire confirmer : largeur des lignes de débordement ; Paysans mêlés aux soldats au centre des Archers.
  Essai 3 lancé (formation_ranks).
- Formation : quick_check formation PASS (168 s). **Essai 3 (formation_ranks) : « ça fonctionne »** (validé, choix
  du sous-agent acceptés). Commit eb73ead sur unit-stats, **fusionné dans main 7bda727**. EtapeDefini : ligne du
  tableau des stats remplacée par « D345 à coder » ; ligne du chantier qui pose une unité dans ses murs.

## Reprise (passation du 2026-10-22, pour une autre session)

**État** : la demande de départ (tableau complet des stats) est faite : valeurs choisies une à une par l'utilisateur
(sections ci-dessus), reportées dans `Tools/Data/UnitTypes.csv`, lisibles dans `UnitStats.md` (produit par
`python Tools/Data/make_unit_stats.py`, ne pas l'écrire à la main) ; flèches à 37,5 m/s ; rangs de formation (tireurs
derrière la mêlée, siège derrière les tireurs). Tout est validé à l'essai et dans main (eb73ead fusionné en 7bda727).
Les **règles** de D345 (GDD, journal § 21 et § 7.2 « Fiche de combat ») restent à **coder** : c'est la suite.

**Où travailler** : worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir` (nom choisi par l'utilisateur),
branche `unit-stats` à eb73ead, **en retard sur main** : la remettre au niveau (`git merge main`) avant tout. Son
`IN_USE.txt` dit « en pause » : la session qui reprend y écrit son nom. La copie principale garde les journaux
(`UnitStatsInProgress.md`, `GDD.md`, `EtapeDefini.md`). Le `git stash` de réserve du chantier a été supprimé
(demande de l'utilisateur) ; le stash « mv2 wip » est celui d'une autre session.

**Règles de travail** (de l'utilisateur) :
- compiler par `powershell -ExecutionPolicy Bypass -File Tools\build.ps1 -Root <copie>`, sans attendre les autres
  sessions ; tout lancement Unreal scripté avec `-LiveCoding=0`, depuis PowerShell (jamais Git Bash) ;
- réimporter la table après chaque changement du CSV : `UnrealEditor-Cmd.exe <uproject> -run=pythonscript
  -script=<copie>\Tools\Editor\make_unit_types.py -unattended -nop4 -nosplash -LiveCoding=0` (de même
  `make_building_types.py` pour les bâtiments) ;
- **quick_check court** (sans argument, < 1 min) à chaque correction ; les parties touchées une seule fois avant le
  commit (l'utilisateur a trouvé 8 min trop long) ;
- sous-agents pour le code et les tests ; chaque essai demandé vient avec son scénario (`Tools/Scenarios/<nom>.txt`,
  `play.ps1 <nom> -Root <copie>`) ; une question à la fois, avec options et recommandation ;
- commit seulement après validation de l'utilisateur, en anglais, sans ligne Co-Authored-By ; mettre ce fichier à
  jour à chaque étape.

**Code existant à connaître** :
- Table des unités : `Simulation/HOLUnitTypes.h` (`FHOLUnitTypeRow`, ligne du CSV ; `FHOLUnitTypeConstants`, fragment
  partagé de Mass : **chaque champ doit être un UPROPERTY**, sinon deux types partagent une copie). Combat
  aujourd'hui : `ArmorCategory` (une seule, `EHOLArmorCategory` : LightInfantry, HeavyInfantry, Ranged, Cavalry, Siege,
  Peasant), `Attack`, `AttackInterval`, `AttackWindUpPercent`, `AttackRange`, `MeleeArmor`, `RangedArmor`,
  `BonusCategory` + `BonusDamage` (un seul bonus), `AggressionRadius`.
- Règles pures : `Simulation/HOLCombatRules.h/.cpp` (`GetDamage` : attaque + bonus − armure de mêlée ou à distance,
  1 au moins ; le type vient de `IsRangedAttack(AttackRange)`, portée > 2 m ; cadence, élan, choix de cible,
  `AttackMoveRadius` = 1200, flèches). Processeur : `Simulation/HOLCombatProcessor.cpp`. Tests :
  `Tests/HOLCombatTests.cpp`, parties `combat` et `orders` de quick_check.
- Bâtiments : `Buildings/HOLBuildingTypes.h` (`MaxHealth`, table `Tools/Data/BuildingTypes.csv`). **Aucun bâtiment
  ne peut encore être abîmé ni détruit** : la vie sert au chantier (`HOLConstructionRules::GetHealth`, D312). L'attaque
  contre bâtiment demande donc d'abord les dégâts aux bâtiments et leur destruction (navmesh rouvert, dépôt et
  population retirés, réplication) : à poser comme question à l'utilisateur avant de coder.
- Pas de héros dans la simulation légère : le ×0,3 et le type Héros n'ont pas encore de porteur.
- Rangs de formation : `Simulation/HOLFormationRules` (`PlaceKinds`, `GetRowPriority` 0-6, `ERowClass`), test
  `HeroesOfLiberty.Formation.Classes`.

**Plan proposé** (à présenter à l'utilisateur ; une partie jouable à la fois, chacune avec essai et scénario) :
1. **D345.1 — Types et armures** : par attaque un `AttackType` (Melee, Ranged, Siege, Hero ; Mêlée ou Distance pour
   les unités d'aujourd'hui) ; quatre armures (`SiegeArmor` et `HeroArmor` en plus) ; **catégories multiples** (liste
   sans plafond, ex. `(Ranged,GunpowderInfantry)` ; un masque de bits sur `EHOLArmorCategory` suffit si le fragment
   partagé l'exige) ; **bonus en liste** `(LightInfantry=5,GunpowderInfantry=5)` qui s'additionnent ; nouvelle
   catégorie `GunpowderInfantry`. Valeurs : Homme d'armes `HeroArmor` 2 ; Archer +5 Infanterie légère et +5 Infanterie
   à poudre ; armures Siège et Héros à 0 ailleurs. `GetDamage` et ses tests ; `UnitStats.md` à étendre (nouvelles
   colonnes). Peu d'effet visible (le bonus de l'Archer est inchangé) : essai léger.
2. **D345.2 — Attaque contre bâtiment** : colonnes par unité (dégâts, portée, vitesse : Paysan 10, 1 case, 2,1 s ;
   Piquier et Homme d'armes 20, 1 case, 2,1 s ; Archer 5, 5 cases, 1,625 s, type Distance) ; `RangedArmor` 40 dans
   `BuildingTypes.csv` (tous les bâtiments, Moulin compris) ; cibles : ordre (clic droit sur un bâtiment ennemi),
   attaque-déplacement après les unités, **au repos** si un bâtiment ennemi est dans le rayon d'agressivité sans unité
   ennemie (choix de l'utilisateur, contre AoE4) ; une unité ennemie qui arrive fait lâcher le bâtiment ; le Paysan
   seulement sur ordre. D'abord la question des dégâts et de la destruction des bâtiments (ci-dessus).
3. **D345.3 — Deuxième attaque** à déclencheur, premier déclencheur **Charge** (premier coup après X cases de course,
   sa vitesse = recharge) : colonnes et règle pure ; aucune unité ne l'utilise avant la cavalerie (tests seulement, ou
   une ligne d'essai).
4. **Plus tard** : ×0,3 des troupes contre un héros sauf type Siège, attaques de type Héros (avec les héros) ; tours.

**Petits restes à reprendre avec D345.1** : `HOLCombat::AttackMoveRadius` 1200 → 1600 (4 cases, en cases comme le
reste) ; quick_check : file d'attente (B à 15 m, dans l'agression de 16 m : l'éloigner au-delà), seuil « Peasant out
of reach » de la tenue (50 cm alors que l'Homme d'armes a 70 cm), commentaires « 12 m » périmés ; commentaire
« 3 m/s » de `HOLUnitReplicationRules.h` (la vitesse répliquée va jusqu'à 12,8 m/s). Les scénarios doivent écarter
les groupes ennemis de plus de 16 m (21,3 m avec des Archers).

## Code de D345 (reprise du 2026-10-22, nouvelle session)

- Haldir : `unit-stats` avancée en avance rapide jusqu'à main 275581a ; `IN_USE.txt` repris (« EN COURS »).
- **D345.1 (types et armures) confiée à un sous-agent** dans Haldir (décisions déjà tranchées, aucune question) :
  `AttackType`, armures Siège et Héros, catégories multiples, bonus en liste, catégorie Infanterie à poudre,
  `UnitStats.md` étendu, petits restes (AttackMoveRadius 1600, quick_check file d'attente et seuil, commentaires).
- En parallèle : question à l'utilisateur pour D345.2 (dégâts et destruction des bâtiments).

### Questions de D345.2 (attaque contre bâtiment), une à la fois

1. **À 0 point de vie — B** (choix de l'utilisateur, contre la recommandation A « détruit net ») : le bâtiment est
   détruit et laisse des **décombres** quelques secondes, qui ne bloquent pas le passage ; vie répliquée sur le
   bâtiment, barre de vie visible quand il est abîmé ; navmesh rouvert, il cesse de servir (population, dépôt,
   production). Pas de réparation pour l'instant. Écartés : A (disparaît net), C (abîmé seulement, vie arrêtée à 1).
2. **File de production d'un bâtiment détruit — A** (choix de l'utilisateur, la recommandation) : tout est
   remboursé, comme une annulation. Écartés : B (tout perdu), C (tête perdue, reste remboursé).
3. **Chantier détruit — au prorata** (réponse de l'utilisateur : « au prorata et annuler est la même chose
   désormais ») : un chantier détruit rend le coût × la part qui restait à construire ((TotalWork − Work) / TotalWork,
   arrondi vers le bas), et **l'annulation suit la même règle** (remplace « annuler un chantier commencé rend la
   moitié » de D311 ; une fondation nue rend donc tout). Une fondation nue reste invisible à l'ennemi (pas de cible).
4. **Centre principal — A** (choix de l'utilisateur, la recommandation) : détruit comme les autres bâtiments, la partie
   continue ; la défaite (écran de fin, joueur éliminé) devient une ligne d'EtapeDefini (au commit de D345.2).
   Écartés : B (défaite tout de suite), C (indestructible pour l'instant).
- Choix de Claude, à corriger au besoin : décombres pendant **10 s**.

**Découpage de D345.2** (après la fin de D345.1, même worktree) : 2a — vie des bâtiments (répliquée, barre de vie),
dégâts, destruction et décombres, navmesh rouvert, service arrêté, remboursements (file, chantier au prorata, annulation
au prorata), `RangedArmor` 40 dans `BuildingTypes.csv` ; 2b — colonnes d'attaque contre bâtiment des unités et cibles
(ordre, attaque-déplacement après les unités, au repos dans le rayon d'agressivité, lâcher pour une unité ennemie,
Paysan sur ordre seulement).

### D345.1 fait (sous-agent, 2026-10-22, Haldir, non commité)

- CSV : `ArmorCategories` (liste, ex. `(Ranged,GunpowderInfantry)`), `AttackType` (Melee, Ranged, Siege, Hero),
  `SiegeArmor`, `HeroArmor` (Homme d'armes 2), `Bonuses` (`((Category=LightInfantry,Damage=5),(Category=GunpowderInfantry,Damage=5))`).
  Fragment partagé : `ArmorCategoryMask` (masque int32), `AttackType`, armures, `TArray<FHOLAttackBonus>`, tout en UPROPERTY.
- `HOLCombat::GetDamage(Attacker, Defender)` : attaque + bonus de chaque catégorie visée − `GetArmor(Defender, type)`,
  1 au moins. `IsRangedAttack` (portée > 2 m) ne sert plus qu'au comportement (flèche, choix de cible, rang, affichage).
- `AttackMoveRadius` 1600 ; quick_check : file B à 20 m, seuil de la tenue > 70 cm, commentaires 16 m ; commentaire
  3 m/s corrigé. `UnitStats.md` étendu et régénéré (matrice inchangée).
- Compilation PASS (35 s), réimport PASS, quick_check court PASS (23 s, 93/93), `-Parts tests,combat,orders` PASS (169 s).
- Reste : commentaires « 12 m » dans 8 scénarios ; `order_lines.txt` groupes à 15 m bord à bord (dans les 16 m).
- Pas d'essai propre (effet invisible en partie) : à valider avec l'essai de D345.2.
- **D345.2a lancée** (sous-agent, Haldir).
- Scénarios (Claude, Haldir) : ennemis écartés au-delà de l'agression — `archers` 6500 → 7000 (bords ~24 m, Archers
  21,3 m), `attack_move_q`, `attack_move_queue`, `order_lines` 6200 → 6800 (bords ~21 m), caméras suivies ;
  commentaires « 12 m » → 16 m dans 8 scénarios.

### D345.2a fait (sous-agent, 2026-10-22, Haldir, non commité)

- Vie : `DamageTaken` (uint16 répliqué) sur `AHOLBuilding` ; `GetHealth()` = vie du chantier (D312) ou MaxHealth,
  moins les dégâts. `RangedArmor` 40 dans `BuildingTypes.csv` (réimporté) ; `HOLConstruction::GetBuildingDamage`
  (Distance − 40, 1 au moins ; autres types entiers, 1 au moins).
- `UHOLConstructionSubsystem::DamageBuilding` / `DestroyBuilding` : file remboursée en entier, chantier au prorata,
  `AHOLRubble` (nouveau, 10 s, sans collision ni navmesh), `ReleaseFootprint`, puis `Destroy()`. Annulation au prorata
  (`GetRefund(Cost, Work, TotalWork)`), info-bulle « Rend la part du coût non bâtie » (`RefundRest`, en/ja dans fill_po.py).
- Barre de vie des bâtiments (abîmé ou sélectionné), vie dans le panneau de sélection.
- Triches : `hol.Cheat.DamageNear <n> <dx> <dy> [enemy|own] [melee|ranged|siege|hero]`, `hol.Cheat.DamageSelected`.
- Tests : `Construction.Damage`, `Buildings.Damage` ; 95/95. quick_check court PASS (22 s) ; `-Parts build` PASS
  (285 s, deux vérifications du remboursement passées au prorata) ; tests, production PASS. `gather` non relancé.
- Choix du sous-agent à confirmer : fondation nue détruite sans décombres (remboursée en entier) ; barre si abîmé ou
  sélectionné ; 1 dégât au moins pour tout type ; l'annulation ne prévient pas le navmesh (déjà le cas avant).
- Restes : .po/.locres non régénérés pour `RefundRest`.
- **Essai 4 lancé : `building_destroy`** (jamais joué avant).

### Essai 4 (building_destroy) : « works » — validé, corrections de l'utilisateur

- Choix du sous-agent acceptés : fondation nue détruite sans décombres (remboursée en entier) ; 1 dégât au moins pour
  tout type. **Corrigés** : barre de vie sur le **seul bâtiment sélectionné** (plus quand il est abîmé) ; infobulle
  d'annulation « Annuler la construction » avec le montant rendu, **sans ligne d'effet** (traductions RefundAll,
  RefundHalf, RefundRest retirées de fill_po.py). Compilation PASS, quick_check court PASS (95/95, 22 s).
- Commits sur unit-stats : 95afb99 (D345.1 + D345.2a), cd091c8 (corrections) ; **fusionnés dans main 2e086ed**.
- GDD : D345 complété (point 8 : destruction, décombres, remboursements, annulation au prorata, centre provisoire
  avant D347) ; D311 et le résumé de J6 renvoient au prorata. EtapeDefini : ligne D345 réduite à ce qui reste.
- À savoir : D347 (review du GDD, même jour) fait du forum un bâtiment à vies (ruine qui se reconstruit) ; notre
  « centre détruit comme les autres » est provisoire, jusqu'au code des conditions de victoire.

## Reprise après D345.2a

Suite : **D345.2b** — colonnes d'attaque contre bâtiment des unités (`BuildingAttack`, `BuildingAttackRange`,
`BuildingAttackInterval`, type : Paysan 10 / 1 case / 2,1 s ; Piquier et Homme d'armes 20 / 1 case / 2,1 s ; Archer
5 / 5 cases / 1,625 s, Distance), cibles : ordre (clic droit sur un bâtiment ennemi), attaque-déplacement après les
unités, au repos dans le rayon d'agressivité sans unité ennemie, une unité ennemie fait lâcher le bâtiment, Paysan
sur ordre seulement ; dégâts par `UHOLConstructionSubsystem::DamageBuilding`. Puis D345.3 (deuxième attaque, charge).
