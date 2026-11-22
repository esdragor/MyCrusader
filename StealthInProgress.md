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
- 2026-11-21 ARWEN-1121sc (C++ liaison D839, NON COMPILE, integ-37) : UHOLVisionSubsystem::GetCampUnitPositions(Camp) (UFUNCTION BlueprintCallable, lit UHOLUnitSubsystem::ForEachLivingUnit, camp par GetCampIndex) + static FilterPositionsByCamp(OwnerCamps, Positions, Camp) (coeur pur). Commit 1 537c6ec (bouchon vide + Test_StealthBindingPicksCampPositions, rouge attendu : ce test, qui appelle ensuite IsVisibleToCamp), commit 2 2b0c986 (liaison). Build a lancer par la session principale (Tools\build.ps1 -LiveCoding=0 -Wait) puis quick_check -Parts tests. Suite = filtre reseau UHOLFogNetSubsystem (ligne de file).
- 2026-11-21 ARWEN-1121sd (C++ reseau D839, NON COMPILE, integ-37) : UHOLStealthFilter (UCLASS, BP_IsVisibleToCamp, defaut vrai) + static MaskCamps(Filter, Camps, NumCamps, UnitCamp, Pos) retire les camps ennemis que la regle script cache ; UHOLVisionSubsystem::SetStealthFilter / GetCampsSeeingUnit ; HOLUnitReplicationSubsystem.cpp appelle GetCampsSeeingUnit (1 ligne). Commit 1 9049921 (bouchon + Test_StealthNetFilterMasksHiddenEnemyCamp), commit 2 28dddc1 (correctif). Reste : un script qui pose le filtre (sous-classe de UHOLStealthFilter alimentee par les zones de la carte + GetCampUnitPositions) et appelle SetStealthFilter ; aucune carte ne pose de zone, donc aucun effet en jeu avant. Build a lancer par la session principale puis quick_check -Parts tests.
- 2026-11-21 ARWEN-1121se (Angelscript, sans build, integ-37) : script qui pose le filtre : Script\Fog\HOLStealthFilter.as = UHOLStealthRuleFilter (sous-classe de UHOLStealthFilter, BP_IsVisibleToCamp -> HOLStealth::IsVisibleToCamp avec Zones + Vision.GetCampUnitPositions ; liste de zones vide = vrai sans lire d unite), HOLStealthInstall::ZonesOfMap (liste VIDE, aucune carte ne pose de zone : aucun effet en jeu), UHOLStealthInstaller : UScriptWorldSubsystem (1er Tick ou UHOLVisionSubsystem::Get() existe : SetStealthFilter). Commit 1 1f1c86a (bouchon vrai + 2 tests Test_StealthRuleFilter*, 1 rouge attendu : Test_StealthRuleFilterMasksEnemyCampForHiddenUnit), commit 2 = filtre + installateur. Non lance ; a verifier : UHOLVisionSubsystem::Get() depuis un UScriptWorldSubsystem, Cast<> de NewObject.
- 2026-11-21 ARWEN-1121sf (Angelscript, sans build, integ-37) : regle d'aspect HOLStealth::OwnUnitOpacity(Zones, UnitPosition) (D824 Q3 B : unite propre en zone = demi-transparente, 0.5 via hol.Stealth.OwnOpacityPercent 50, 1.0 hors zone). Commit 1 bc656dd (bouchon 1.0 + 3 tests Test_StealthOwnUnit*, 2 rouges attendus : ...InZoneIsHalfTransparent, ...InZoneLessOpaqueThanOutside), commit 2 = regle. Aucun point d'accroche script pour l'opacite d'une unite (les SetRenderOpacity de Script\UI sont des widgets) : application a l'affichage = liaison C++ nommee dans TYPES_A_EXPOSER (SetOwnStealthOpacity sur la vue/acteur d'unite, valeur de OwnUnitOpacity). Non lance.
ARWEN-1122b : opacite lue UNE fois par execution du processeur (FOpacitySnapshot) ; c69b0e4 rouge (bouchon + SnapshotInZone/Stable), 1b1d194 vert ; NON COMPILE ; ids : aucun appelant de ApplyOwnUnits en production (a brancher avec FHOLUnitStateFragment::UnitId). Rapport Docs\Rapports\ARWEN-1122b.md
- 2026-11-22 ARWEN-1122c (integ-37, NON COMPILE) : liaison C++ UHOLStealthOwnUnitsLibrary (Presentation\HOLStealthOwnUnits.h/.cpp : GetPlayerUnits(monde, slot) -> ids State.UnitId + positions, GetLocalPlayerSlot) ; l'installateur (HOLStealthFilter.as, ApplyOwnUnitOpacity) pose 0.5/1.0 a 20 Hz, rien sans zone ; test rouge 7314bed (Tests\HOLStealthOwnUnitsTests.cpp), vert 4ee4116 ; reste : build, test HeroesOfLiberty.Stealth.OwnUnits, chargement des .as, jeu en zone.
- ARWEN-1122e (integ-37, 17:1x, script) : scenario stealth_zones joue sur Forest_Small (niveau reel Forest_Small, 4 zones), liaison PROUVEE : StealthOwn unit 11 = 0.5 a l entree puis = 1.0 a la sortie, 2 lignes, 0 erreur AS, 0 WorldContext ; commits 4b8d822 (log des zones) fd1440f (scenario) ; LOGIQUE SEULEMENT
- 2026-11-22 ARWEN-1122d (integ-37, script seulement, NON CHARGE a l editeur) : passe des unites propres allegee : 7a5f4f8 rouge (bouchon UHOLStealthOwnApplier + 3 tests Fog\HOLStealthOpacityApply_Test.as), e6c76c9 vert (4 Hz = OwnOpacityPeriodSeconds, rien lu sans zone, distance au carre, ecriture au changement + ligne LogHOLMatch StealthOwn). Reste : charger Forest (essai logique), lancer les 3 tests.
ARWEN-1121sg (20:22) : setter de rendu UHOLUnitOpacityLibrary (Presentation/HOLUnitOpacity.h/.cpp, 358a948 stub+tests rouges, 81aae44 setter, 8f03b93 HOLStealthApply + test). RESTE : (1) lier le dessin : HOLInterpolatedInstancesProcessor.cpp ~l.582 (AddBatchedCustomData) doit lire GetStealthOpacity + materiau M_UnitPlayerColor avec une entree d opacite (5e float) ; (2) point d appel UHOLStealthInstaller.Tick : binding C++ listant ids+positions des unites du joueur local. Non compile, non teste.
ARWEN-1122a (16:3x, C++ NON COMPILE, integ-37) : DESSIN LIE cote C++ : la donnee personnalisee d une instance d unite porte un 5e float = opacite (HOLUnitDraw::FInstanceCustomData, HOLUnitOpacity.h ; fonction testee MakeUnitInstanceCustomData(Color, Prop, UnitId) = GetStealthOpacity(UnitId)), appelee l.582 de HOLInterpolatedInstancesProcessor.cpp ; corps (l.656) ecrivent 1.0 par MakeInstanceCustomData. Commit 1 rouge 09868d9 (bouchon 1.0 + 3 tests HOLUnitOpacityDrawTests.cpp : InZone et Layout rouges attendus), commit 2 vert b620434. RESTE : (1) build + quick_check tests ; (2) ENTREE D OPACITE DU MATERIAU M_UnitPlayerColor (uasset, editeur : CustomData index 4 -> Opacity/Dithered, voir Docs\Rapports\ARWEN-1122a.md) : sans elle le rendu est inchange ; (3) point d appel UHOLStealthInstaller.Tick.
- 2026-11-22 ARWEN-1122g (integ-37, C++ NON COMPILE) : table d opacite de furtivite vidée par partie (REVUE-1122 defaut 1) : UHOLStealthOpacityResetSubsystem (UWorldSubsystem, HOLUnitOpacity.h/.cpp) compte les mondes de jeu vivants, vide la table quand le premier monde naît et quand le dernier meurt (deux mondes PIE : jamais d effacement d un monde vivant). b0c7f81 rouge (bouchon + test HOLUnitOpacityResetTests.cpp NewWorldForgetsOld), 3efc630 vert. RESTE : build + quick_check tests (rouge/vert jamais vus a l execution).
- 2026-11-22 ~22:05 (réponse de l'utilisateur, aucun code) : Q-stealth-rendu = A si la mesure à 2000 unités passe, sinon C (GDD D887) ; ligne [ ] dans la section Arwen.
