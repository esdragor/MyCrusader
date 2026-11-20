# SAURON-1120pa : D864 + D865, colonnes de table du passif du Chevalier (NON COMPILÉ)

**NON COMPILÉ** : aucun build, aucun quick_check, aucun éditeur (lot C++ de la session principale).

APPELS_DECLARES: 22 (arrêt à 25 vus ; seuil 80 jugé sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 15:40:45 / 15:44:57
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build ; worktree annoncé « compilé à jour sur a831674 » par l'énoncé)
TEST_ROUGE: c919d09 (test seul, antérieur au correctif f1aa981) ; assertit l'effet : oui. Vu à 15:42 : `python -I Tools\Data\unit_columns_declared_test.py` = 4 FAIL (colonnes absentes du CSV, jamais copiées par ToConstants). Le test C++ `HeroesOfLiberty.DamageRedirect.Table` (même commit) n'est pas lancé : rouge attendu sur c919d09 (ToConstants ne copie rien : 30 et 800 restent 0), vert attendu sur f1aa981.
COMMITS: 2 (+ 1 merge main 8820b8a), dernier à ~15:44, merge_main_fait: oui
TESTS: python unit_columns_declared_test.py PASS (60 colonnes CSV, 59 champs Row, 53 champs Constants, 0 faux) ; unit_stats_test.py PASS (17 lignes) ; value_consumers.py --check : 3 DEAD préexistants (honor.start, honor.kill_gain, knight.challenge_duration_ms, présents aussi sans mes changements, vérifié par stash). TESTS_TAUTOLOGIQUES: 0 (le test C++ passe par Row -> ToConstants -> Redirect, valeurs attendues à la main : 30 / 70 / 100 / un seul Chevalier).
ESSAIS: aucun (colonnes de données, rien à l'écran).
QUESTIONS: Retours\Q-chevalier-ligne.md (aucune ligne VirtuousKnight dans UnitTypes.csv ; défaut codé : option C, aucune ligne, colonnes à 0 partout = passive inactive).
FAIT / RESTE:
- FAIT : `DamageRedirectPercent` (int32, 0..100) et `DamageRedirectRadius` (float cm) en UN seul changement de `HOLUnitTypes.h` : `FHOLUnitTypeConstants` (`UPROPERTY()`), `FHOLUnitTypeRow` (`UPROPERTY(EditAnywhere)`, ClampMax 100), `ToConstants` (borne 0..100). Mode Nearest = `HOLDamageRedirect::DefaultCumulMode` (D864, déjà le défaut de la règle).
- FAIT : `Tools\Data\UnitTypes.csv` (2 colonnes en fin, `0,0` sur les 17 lignes), `make_unit_stats.py` (lecture des 2 colonnes), `Tools\Data\unit_columns_declared_test.py` (L18-a : chaque colonne affectée est dans le CSV, la Row, ToConstants et Constants ; toute colonne du CSV est un UPROPERTY de la Row), branché dans `Tools\Tests\quick_check.ps1` (BOM gardé, analyse sans erreur).
- L19-4 : identifiants cherchés dans quick_check.ps1 et Tools\Scenarios\ : aucun scénario ni test existant ne cite `DamageRedirect`, `unit_columns` ni les nouvelles colonnes.
- RESTE : (1) build + `-Parts tests` ; (2) `HOLCombatProcessor.cpp` n'appelle PAS `HOLDamageRedirect::Redirect` (nulle part dans Source) : branchement à faire à côté de `IsInAura` (:638-730), lire Constants.DamageRedirectPercent / Radius ; (3) anneau de 8 m (HOLSelectionMarkers) ; (4) ligne VirtuousKnight (Q-chevalier-ligne) ; (5) l'en-tête de HOLDamageRedirectRules.h dit encore « NOT decided » (D864 / D865 tranchés) : à mettre à jour avec le branchement.
NON VÉRIFIÉ: compilation C++ de HOLUnitTypes.h et du test DamageRedirect.Table ; exécution de tous les tests C++ ; regex du test python sur la forme exacte des UPROPERTY (vérifiée seulement sur l'état actuel du fichier) ; empreinte des constantes (hash du fragment) non revue ; DIAGNOSTICS_AS: 0 erreur (aucun fichier .as touché).
ASSETS_A_REGENERER: `Content\Units\UnitTypes.uasset` (DataTable importé de UnitTypes.csv) par l'éditeur, pour que les 2 colonnes existent dans l'asset (sans régénération elles valent 0 : sans effet).
EN-TETES_TOUCHES: Source\HeroesOfLiberty\Simulation\HOLUnitTypes.h (central, recompilation large, les 2 colonnes en un seul changement).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune ; `AubeInProgress.md` mêle UTF-8 et mojibake (lignes anciennes), non touché hors ma ligne ajoutée.
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress, Auto_Journey_File, EtapeDefini modifiés par moi + ceux des autres agents, Retours\Q-chevalier-ligne.md) ; aucun fichier de code : oui.
