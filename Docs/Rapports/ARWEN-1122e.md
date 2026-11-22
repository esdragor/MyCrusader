# ARWEN-1122e : furtivité, scénario stealth_zones sur Forest (script seul)

Impact joueur : aucun changement de jeu ; la liaison des unités propres est PROUVÉE à l'exécution (une unité du joueur local passe à 0.5 en zone puis revient à 1.0). Essai LOGIQUE SEULEMENT (zone non peinte, matériau sans opacité ; pas de fiche E).

liaison prouvée : oui
branche prête : oui (integ-37, tip fd1440f ; merge main non fait, DLL non vérifiée à jour avec le Source\ de main)

APPELS_DECLARES: ~20
HEURE_DEBUT/HEURE_FIN: 17:09 / 17:13 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scénario + log, pas de règle neuve ; les rouges 7a5f4f8 / e6c76c9 sont ceux de ARWEN-1122d) ; assertit l'effet : oui (waitlog sur les deux changements d'opacité)   TEST_ROUGE_SEPARE: oui (log 4b8d822, scénario fd1440f)
COMMITS: 2 (4b8d822, fd1440f), dernier 17:12, merge_main_fait: non (DLL non vérifiée)
TESTS: scénario stealth_zones joué 2 fois sur 3 permises (passe 1 = sonde, passe 2 = scénario final, 25 s) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: stealth_zones : LOGIQUE SEULEMENT, aucun PNG ; `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 stealth_zones -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -TimeoutSec 240` (preflight non lancé)
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

FAIT
- NOM DE NIVEAU RÉEL lu par Gameplay::GetCurrentLevelName(true) sur Forest Small : "Forest_Small" (MapHasGroves l'accepte par StartsWith "forest_"). Au menu : "MainMenu", 0 zone.
- Zones réelles (centre de la carte = (0,0), demi-côté de carte 22000) : 0 centre (6600, 7700) demi 2640 ; 1 centre (-6600, -7700) demi 2640 ; 2 centre (7700, -5500) demi 2200 ; 3 centre (-7700, 5500) demi 2200. Ajout (4b8d822, HOLStealthFilter.as) : l'installateur journalise "StealthZones level <nom> zones <n>" et une ligne "StealthZone i center X Y half X Y" à l'installation.
- Scénario Tools\Scenarios\stealth_zones.txt/.json/.look (fd1440f) : ManAtArms slot 0 à (3000, 4200) hors zone (zone 0 = X 3960-9240, Y 5060-10340), hol.Units.Move 4800 6200 0 (entre), waitlog LITTÉRAL "StealthOwn opacity unit 11 = 0.5", hol.Units.Move 3000 4200 0 (sort), waitlog "StealthOwn opacity unit 11 = 1", hol.Units.Where 0 aux étapes, quit final. Id 11 = premier id neuf du slot 0 (paysans 1-5, ennemi 6-10).
- Journal de la passe 2 : 2 lignes StealthOwn exactement (une par changement : "unit 11 = 0.5" à (4011, 5323), puis "unit 11 = 1.0" à (3926, 5228)) ; 0 "Angelscript ... rror" ; 0 "requires WorldContext". Aucune des craintes (override script, TMap.Find/Remove, Log f-string) ne se confirme : les .as se chargent et tournent. Les paysans (ids 1-5, à (-17284, 0), hors zone) n'écrivent aucune ligne : une unité qui reste opaque ne coûte rien.
- Le texte du log vient du code : `Log(n"LogHOLMatch", f"StealthOwn opacity unit {UnitId} = {Opacity}")` ; le float s'imprime "0.5" / "1.0".

RESTE
- Peindre la zone et rendre l'opacité dans le matériau (hors de cette tâche) ; fiche E à écrire alors.
- Les 3 tests script de HOLStealthOpacityApply_Test.as n'ont pas été lancés (quick_check non demandé) ; le chargement du fichier est prouvé (pas d'erreur AS), pas leur résultat.
- Unités mortes : entrée de Last conservée (inoffensif).

NON VÉRIFIÉ
- Résultat des 3 tests script ; le rendu visuel (aucun) ; le comportement multijoueur (client distant, hôte) de GetLocalPlayerSlot ; preflight.ps1 stealth_zones ; DIAGNOSTICS_AS (serveur de langage non consulté, mais 0 erreur au chargement du journal).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune. IN_USE.txt : OCCUPE à l'entrée (CPP: non), libre à la sortie. Auto_Journey_File.md non touché (aucune ligne Arwen 1122 à moi). Warnings Angelscript préexistants sur HOLScenarioPanelScript.as (sans rapport). Le décompte de lignes dans stealth_zones : le waitlog "= 1" correspondrait aussi à une valeur "1.x" mais la règle ne produit que 0.5 ou 1.0.
GIT_STATUS_PRINCIPALE: docs seulement (StealthInProgress.md et EtapeDefini.md = ma ligne ARWEN-1122e ajoutée ; le reste préexistait) : oui.
