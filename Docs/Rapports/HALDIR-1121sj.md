# HALDIR-1121sj : plafond de population du centre secondaire + capture "pendant"

APPELS_DECLARES: 20 (arret 25 vus)
HEURE_DEBUT/HEURE_FIN: 20:38 / 20:44 (arret dur 21:03)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun code)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet : pas de defaut de code (voir ci-dessous) ; assertit l'effet : sans objet
COMMITS: 1 (c2cdee6 sur wall-line, scenario seul), dernier a 20:40, merge_main_fait: non
TESTS: preflight.ps1 secondary_centre -Root HeroesOfLiberty-Haldir : PASS (34 s, "no warning or error in the log", "sanity audit passed", 5 points OK) ; TESTS_TAUTOLOGIQUES: 0 (aucun test ajoute)
ESSAIS: secondary_centre : preflight PASS, sc_pendant et sc_apres ouverts avec Read et vus, VU A L'ECRAN (geste reel : touches Z, Z puis clic), commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   EDITS_REFUSES: 0   DIAGNOSTICS_AS: 0 erreur (aucun .as touche)

## 1. Population 5 / 175 : PAS un defaut
- Regle : plafond d'un joueur = somme des PopulationProvided des batiments FINIS (HOLProductionSubsystem.cpp:584, C++) + CheatPopulation (:588), puis HOLProduction::GetPopulationCap = Clamp(somme, 0, MaxPopulation) (HOLProductionRules.cpp:5 ; MaxPopulation = 175 en HOLProductionRules.h:61). GDD : D03 (population maximale 175 par joueur), D83.
- Cause : le scenario faisait `hol.Cheat.Population 200` : forum 10 + 200 = 210 > 175, borne a 175 ; le +10 du centre secondaire (colonne PopulationProvided = 10 de BuildingTypes.csv, ligne SecondaryCentre) etait donc invisible. Le CSV est correct, rien a changer.
- Correctif : triche retiree du scenario. Preuve a l'ecran : sc_pendant "Population 5 / 10" (forum seul, chantier non fini), sc_apres "Population 5 / 20" (forum 10 + centre secondaire 10).

## 2. Capture sc_pendant
- Scenario : apres la pose, `waitlog Construction: slot 0 first work on SecondaryCentre` (HOLBuildProcessor.cpp:942), `hol.Build.Speed 1`, `wait 2`, `hol.HUD.Report`, puis capture, puis `hol.Build.Speed 20` pour finir. Notes .look mises a jour.
- PNG vu : 5 Paysans au pied de la fondation, panneau "Paysan (5) / 5 en construction". Pas de barre de construction visible sur l'image (le panneau des Paysans donne l'etat) ; chantier partiel texturé.

FAIT / RESTE: fait = les deux points ; Retours\E-secondary_centre.md mis a jour (3 lignes + section 1121sj). Reste = rien de bloquant ; Q-secondary-centre-grid.md reste ouverte (grille A Paysan seul).
NON VERIFIE: son ; barre de construction (absente de l'image) ; jeu reel hors capture ; EtapeDefini.md / Placeholders.txt non touches (aucun provisoire ajoute ni retire).
PROCESSUS_DETACHES: voir sortie stray_processes en dernier geste
ANOMALIES_PROCESSUS: aucune vue ; un preflight lance par erreur avec un filtre grep Bash n'a rien imprime (relance par PowerShell)
GIT_STATUS_PRINCIPALE: non lance (ecritures : ce rapport et Retours\E-secondary_centre.md)
