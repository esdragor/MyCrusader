# ARWEN-1122d : furtivité, passe des unités propres allégée (script seul)

Impact joueur : la lecture des unités du joueur passe de 20 Hz à 4 Hz, ne lit rien sans zone, n'écrit l'opacité que quand elle change. NON CHARGÉ à l'éditeur, NON TESTÉ à l'exécution.

APPELS_DECLARES: ~25 (premier Edit au 14e appel environ : consigne « avant le 8e » non tenue, lecture du code d'abord ; l'essai éditeur n'a pas été fait, budget d'appels épuisé)
HEURE_DEBUT/HEURE_FIN: 17:05 / ~17:09 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: sans objet (script seul)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7a5f4f8 (bouchon + 3 tests script) ; rouge NON VU à l'exécution (pas de quick_check ni d'éditeur) ; assertit l'effet : oui (écritures et lectures comptées par une sous-classe, valeurs comparées à HOLStealth::OwnUnitOpacity)   TEST_ROUGE_SEPARE: oui (7a5f4f8 puis e6c76c9)
COMMITS: 2 (7a5f4f8 rouge, e6c76c9 vert), dernier ~17:08, merge_main_fait: non
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (Forest non chargée ; scénario stealth_zones.* non écrit)
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 3 (Test_StealthOwnApplierTwoPassesWithoutChangeWriteOnce, Test_StealthOwnApplierUnitEnteringThenLeavingIsWrittenTwice, Test_StealthOwnApplierWithoutZonesReadsNoUnit, dans Script\Fog\HOLStealthOpacityApply_Test.as)   TYPES_A_EXPOSER: aucun

FAIT
- Script\Fog\HOLStealthOpacityApply.as : classe UHOLStealthOwnApplier (Zones, ReachSquared = rayon² du cercle englobant de chaque zone calculé une fois par SetZones, Last = TMap<int,float> des seules unités non opaques, WorldObject). Pass() : retourne 0 sans zone et SANS lire d'unité ; sinon une lecture, puis par unité : NearAZone (distance² à chaque centre contre ReachSquared, sans allocation) ; hors de tout cercle = 1.0 sans appeler la règle ; la règle HOLStealth::OwnUnitOpacity seulement pour les survivantes ; écriture seulement si la valeur diffère de Last (défaut 1.0, donc une unité qui n'entre jamais en zone ne coûte aucune écriture) ; une ligne Log(n"LogHOLMatch", "StealthOwn opacity unit <id> = <valeur>") par écriture. ReadOwnUnits et WriteOpacity sont des méthodes virtuelles de script, remplacées par le test.
- Script\Fog\HOLStealthFilter.as : constante unique `OwnOpacityPeriodSeconds = 0.25` (4 Hz), l'installateur crée l'Applier au premier passage et appelle Pass() ; ApplyPeriod supprimé.
- Coût attendu : sans zone zéro ; avec zones, par passage 2000 comparaisons de distance² (pas d'appel de règle ni d'écriture pour les unités éloignées ni inchangées) à 4 Hz au lieu de 2000 appels de règle + écritures à 20 Hz. Estimation, non mesurée.

RESTE
- Charger les .as (risque connu : `override` sur méthode de script n'est employé nulle part ailleurs dans Script\ ; si « override » est refusé, retirer le mot-clé des deux méthodes de test, la déclaration de même signature suffit).
- Lancer les 3 tests (rouge attendu sur 7a5f4f8, vert sur e6c76c9).
- Essai Forest logique : jamais fait ; demande une position connue d'une zone (zones = centre de AHOLMapBounds ± fractions de ForestGroveHalf, bornes non connues) : prévoir un Log des zones à l'installation, puis un scénario stealth_zones.txt/.look qui déplace une unité à soi dans une zone puis dehors. Chercher dans le journal : 0 « Angelscript ... rror », 0 « requires WorldContext », lignes « StealthOwn opacity unit <id> = ... » (preuve que la liaison rend des unités ; absence = rouge : id ≠ State.UnitId ou slot local < 0 à GetLocalPlayerSlot).
- Les unités mortes gardent leur entrée dans Last (id jamais recyclé, inoffensif).

NON VÉRIFIÉ
- Tout : compilation des .as, `override`, TMap<int,float>.Find(clé, valeur) / Remove / Add, `Log(n"...", f"...")` (forme vue ailleurs dans Script\AI), Cast de NewObject(this, ...) dans un UScriptWorldSubsystem, la liaison HOLStealthOwnUnits côté jeu (toujours non compilée au dernier rapport, build PASS 17:04 selon l'énoncé).

TEST_ROUGE_SEPARE: oui (voir ci-dessus)
DIAGNOSTICS_AS: non relus (serveur de langage non consulté)
LISTE ORDONNÉE des rouges et verts de la branche integ-37 (du plus ancien) : 7314bed rouge (C++ : liaison OwnUnits, bouchon à 0 unité + test) ; 4ee4116 vert (liaison + installateur) ; 7a5f4f8 rouge (script : bouchon de l'Applier + 3 tests) ; e6c76c9 vert (Applier + cadence 4 Hz). La fusion integ-49 (925ed88) est avant.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune. Aucun éditeur lancé (la fenêtre 17:11 non atteinte et appels épuisés). IN_USE.txt : entré OCCUPE (CPP: non), rendu libre. Auto_Journey_File.md non touché (aucune ligne à moi trouvée).
GIT_STATUS_PRINCIPALE: docs seulement (StealthInProgress.md, EtapeDefini.md = mes lignes ajoutées ; le reste préexistait) ; oui.
