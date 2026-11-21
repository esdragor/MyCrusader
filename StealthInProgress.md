# Zones de furtivité (GDD § 8.2, D383) : chantier en cours

Ouvert le 2026-11-15 10:20 par ARWEN-1115c (worktree `HeroesOfLiberty-Arwen`, branche `d751-root`), relevé B3 de
`Auto_Journey_Jour.md` (« chapitres du GDD non commencés »). SANS C++ à ce stade : règle pure Angelscript et ses tests.

## Ce que dit le GDD (D383, § 8.2)

La furtivité vient, pour l'instant, de **zones dédiées posées par la carte** (ni forêt ni hautes herbes) : les unités qui
s'y tiennent sont **cachées, sauf pour un ennemi tout proche ou entré dans la zone** (comme les bosquets d'AoE4). Placement
symétrique (level design). Le Spectre Assassin n'a pas de furtivité propre, il profite des zones comme tout le monde (D396).

## Règle pure (`Script\Fog\HOLStealthRules.as`, tests `Script\Tests\HOLStealthRules_Test.as`)

Une unité **dans une zone** est cachée pour un camp qui la regarde, **sauf si** une unité de ce camp est dans la même zone ou à
moins de N (distance « tout proche », `hol.Stealth.RevealDistance`, cm, défaut 800 = 2 cases, réglage en attente de
`Retours\Q-stealth.md`). Hors zone : la règle ne cache rien (la vue normale du brouillard s'applique). La règle ne remplace
pas la vue : visible = case en vue ET pas cachée par la furtivité.
Zone provisoire = rectangle aligné (centre, demi-étendue), comme une case de bloqueur ; la forme vraie dépend de la carte.

## Étapes (15 min chacune)

1. [fait] Journal, question `Retours\Q-stealth.md` (N, cartes, aspect).
2. [fait] Test rouge : vu à N - 1, caché à N + 1 (ennemi hors zone), vu si ennemi dans la zone loin de l'unité, jamais
   caché hors zone. Bouchon qui ne cache rien = ROUGE VU à l'exécution (10:23, `quick_check -Parts tests` 1270/1272, KO
   `Test_StealthHiddenJustBeyondRevealDistance` et `Test_StealthSeenByEnemyInsideTheZone`) ; commit 3b370fa.
3. [fait] Vert : la règle (distance au carré <= N², ennemi dans une zone qui contient l'unité). N est lu par la règle
   (`HOLStealth::RevealDistance`), jamais recopié dans les tests. `quick_check -Parts tests` 1272/1272 PASS (10:26).
   Prochaine étape (agent neuf) : attendre `Retours\Q-stealth.md` (changer le défaut du cvar si N diffère), puis l'étape 4
   (C++, agent C++) ; rien d'autre à faire sans C++.
3b. [fait, ARWEN-1115d 2026-11-15] Registre des valeurs : la fusion groupée n°1 avait exclu d751-root (`quick_check -Parts
   values` ROUGE vu 11:02 : « ORPHELIN Script/Fog/HOLStealthRules.as », marqueur « provisional shape »). Ligne
   `stealth.reveal_distance_cm` ajoutée à `Tools\Data\ValueSheet.csv` (code, motif sur le cvar `hol.Stealth.RevealDistance`,
   provisoire, D383/D396, § 8.2 ; la note cite aussi la forme rectangulaire provisoire), `Valeurs.md/.csv` régénérés par
   `make_value_sheet.py` (valeur lue 800). `-Parts values` PASS ; `-Parts tests` 1286/1286 PASS (sans rebuild après merge main). main fusionnée (ad6395c).
   Commit 162b28d. Prochaine étape inchangée : réponse Q-stealth puis étape 4 (C++). Fusion dans main : agent de fusion n°2.
4. (Plus tard, C++, autre agent) Branchement dans la vue par camp, voir ci-dessous.

## Ce qu'il faudra lire côté C++ ensuite (vision par camp)

- `UHOLVisionSubsystem::IsVisible(Slot, Location)` (`Source\...\HOLVisionSubsystem.h`) dit si la **case** est en vue pour un
  camp ; il faudra en plus, pour une unité en zone, les **positions des unités du camp qui regarde** (ou la distance au plus
  proche observateur de ce camp) pour appeler `HOLStealth::IsHiddenFrom`.
- `UHOLFogNetSubsystem` (réplication filtrée par camp, D518) : une unité cachée par la furtivité ne doit pas être envoyée au
  camp qui ne la voit pas (sinon un client modifié la lit), avec le délai de grâce D526.
- La texture du brouillard côté client (D525) n'a pas à changer : la case reste en vue, seule l'unité est cachée.
- L'auto-acquisition et l'IA (D519) passent par la même requête « caché pour ce camp ».
- Point d'accroche prévu (`FogInProgress.md` § 11) : champ `StealthZone` du bloqueur ; aucune carte n'en pose encore
  (`MapsInProgress.md`, après C6).

## Décompte des appels (un trait par appel)

||||||||||||
ARWEN-1115d : ||||||||||||||||||| (19 à l'écriture)
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D824 (Q-stealth) = Q1 A (800 cm, défaut) + Q2 B (certaines cartes à bosquets) + Q3 B (bosquet + contour discret au sol) ; lignes [ ] Arwen (aspect, branchement vue par camp) et Haldir (zones sur quelques cartes).
- 2026-11-21 ARWEN-1121sa (L61, Angelscript, sans build) : HOLStealth::IsVisibleToCamp(Zones, UnitCamp, UnitPos, ViewerCamp, ViewerPositions, Distance) ajoute dans HOLStealthRules.as (allie = toujours vu, ennemi = !IsHiddenFrom). Commit 1 990a147 (bouchon renvoie true + 3 tests Test_StealthCampView*, 1 rouge attendu : Test_StealthCampViewEnemyBeyondRevealDistanceDoesNotSee), commit 2 = regle. Reste : liaison C++ (UHOLVisionSubsystem positions des unites par camp, UHOLFogNetSubsystem filtre reseau) = TYPES_A_EXPOSER ; aspect (bosquet, contour) ; non lance par l'agent.
- 2026-11-21 ARWEN-1121sb (Angelscript, sans build, branche integ-37) : regle d'aspect HOLStealth::ShowsOutline(Zone, bHovered, OwnPositions) (D824 Q3 B : contour au survol ou quand une unite du joueur est dans la zone). Commit 1 c68a4ba (bouchon false + 3 tests Test_StealthOutline*, 2 rouges attendus : ...OnHover, ...WhenOwnUnitEnters), commit 2 c674622 (regle). Reste : demi-transparence des unites propres en zone (meme esprit), dessin (C++/Slate ou materiau), branchement vue par camp (liaison C++). Non lance.
