# HALDIR-1117d : D810 / D841 premier lot (etude + plan + regles pures C++)

APPELS_DECLARES: 12 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 09:28 / ~09:33
PREMIER_BUILD: sans build (la session principale compile)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: aucun rouge vu (pas de build possible) ; test et regles dans le meme commit e6fa3a4 ; assertit l'effet : oui (vitesse mise a l'echelle, degats, arret de charge)
COMMITS: 1 (e6fa3a4, branche maps, Haldir), merge_main_fait: non (pas demande)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculees par les regles : GetScaledSpeed(500, GetWalkerSpeedPercentAt) = 400)
ESSAIS: aucun. QUESTIONS: aucune (un point de design a defaut : forets et marais se composent par le plus lent, voir ci-dessous)
NON VERIFIE: TOUT le C++ (aucune compilation, aucun test lance) ; Crown.umap non commite laisse tel quel.
PROCESSUS_DETACHES: aucun lance. ANOMALIES_PROCESSUS: aucune.

## Sens retenu (D836 / D841)
UN SEUL sens : la regle est C++ (`FHOLTerrainRules`, `Map\HOLTerrainRules.h/.cpp`), appelee par le pas Mass ; le script la lit par
`UHOLTerrainLibrary` (namespace `HOLTerrain::` en script). Jamais de copie script. Les fonctions de `Script\Terrain\HOLTerrainRules.as`
(`ForestSpeedPercent`, `FordDamageTaken`, `StopsCharge`, `Contains`, `FHOLTerrainSettings`, `FHOLTerrainZone`) sont donc a SUPPRIMER au
lot suivant, avec `HOLTerrainRules_Test.as` (leurs tests sont repris par `Terrain.Rules` C++). Tant que les deux copies existent (jusqu'a ce
lot), c'est un jumeau provisoire : le retirer dans le lot qui expose la bibliotheque.

## Fait (commit e6fa3a4, non compile)
`Map\HOLTerrainRules.h/.cpp` : `HOLTerrainKind::Forest = 3`, `Ford = 4` (2 = ancienne zone de couvert D796, non reutilise) ;
`HOLTerrainWalker::{OnFoot,Cavalry,Siege}` ; defauts `DefaultForestCavalrySpeedPercent 80`, `DefaultForestSiegeSpeedPercent 70`,
`DefaultFordDamageTakenPercent 120` ; `IsInZoneOfKind`, `GetForestSpeedPercent`, `GetWalkerSpeedPercentAt` (zones existantes puis, en
zone Forest, le PLUS LENT de la zone et de la foret : choix par defaut, a faire confirmer si besoin), `GetFordDamageTakenPercent`,
`GetFordDamageTaken`, `StopsCharge(bMarshStopsCharge, Zones, X, Y)`. `Tests\HOLTerrainTests.cpp` : `Terrain.Rules` etendu (20 assertions :
80/70/100, bord +1 cm, gue ne ralentit pas, marais sous foret, 120 % / 100 %, 0 reste 0, charge arretee en marais seulement).

## Lecture des zones (donnees de la carte)
La carte peint des `FHOLPlacedTerrainZone {Kind, Location, Radius}` dans `AHOLTerrainLayout` (rempli par `Tools\Editor\maps\common.py`
~ligne 1213, section « Terrain zones »), lues par `UHOLTerrainSubsystem::OnWorldBeginPlay` (HOLTerrainSubsystem.cpp ~l.50-70) dans
`Zones`. Aucun changement de format : seul `Kind` prend 3 (foret) et 4 (gue). Restera cote carte : peindre les zones Forest / Ford dans
`common.py` (les cartes Foret, Hinterland, Les Gues D496 ; sur Les Gues le gue ne ralentit pas, la defense reduite est gardee), puis
regenerer les cartes (seul, file de build vide, regle 13).

## Reste (appelants Mass, lot C++ suivant, avec build)
1. `UHOLTerrainSubsystem` : ajouter `GetWalkerSpeedPercentAt(FVector, Categorie)`, `IsOnFord(FVector)`, `IsInMarsh(FVector)` (appellent les regles ci-dessus ; la categorie via un reglage cvar des pourcentages comme `hol.Terrain.MarshSpeedPercent`).
2. `Commands\HOLUnitOrderProcessor.cpp` : `GetTerrainShare` (l.151) prend la categorie ; appels l.499, 2584, 3773, 3843 ; categorie = `Constants.HasArmorCategory(EHOLArmorCategory::Siege / Cavalry)` (comme `HOLGarrisonSubsystem.cpp` l.143), sinon a pied.
3. `Simulation\HOLCombatProcessor.cpp` : `GetDamageTakenPercent(const FFighter& Victim)` (l.719) est le point unique des degats recus (fleches l.1391 et 1453, melee l.3083) : y multiplier par `GetFordDamageTakenPercent(Terrain->IsOnFord(Victim.Location))` (`FFighter.Location` existe).
4. `StopsCharge` : aucun appelant (pas de charge de cavalerie lourde en C++ : D345.3 ouvert) ; reste libre jusqu'a la charge.
5. `UHOLTerrainLibrary` : exposer `ServerForestSpeedPercent`, `ServerFordDamageTaken`, `ServerStopsCharge` ; supprimer la copie script et son test ; tests Mass reels (cavalerie 80 % en foret sur la chaine ordre de mouvement, gue 120 % sur un coup) rouges d'abord, comme C3b (HOLTerrainTests.cpp ~l.110-170).
6. Commande a lancer par la session principale : `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Haldir`, puis `quick_check -Parts tests` (attendu : `HeroesOfLiberty.Terrain.Rules` vert ; sinon une assertion fautive de ce commit).

LIGNES_CPP_SUPPRIMEES: 0   TESTS_CPP_RETIRES: 0   TESTS_SCRIPT_AJOUTES: 0   BUILDS_DEMANDES: 1 (lot suivant)   TYPES_A_EXPOSER: voir point 5
