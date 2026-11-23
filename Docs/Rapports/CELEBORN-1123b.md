# CELEBORN-1123b : S3-a T7 correctif (branche integ-41, worktree Celeborn)

T7 FAIT et VERT. Branche integ-41 prete a integrer (rouge 7fa5b77 + vert 998788b + journal f602459).

APPELS_DECLARES: ~24 (arret 09:24 ou 25 ; fini a 09:17 ; un appel Bash-sous-PowerShell perdu + un TaskStop)
HEURE_DEBUT/HEURE_FIN: 09:08 / 09:17
PREMIER_BUILD: 09:09   BUILD: PASS (49 s, incremental, apres file Faramir ; PASS a 09:10)   LAUNCHCHECK_DEBUT: DLL 09:02:53 plus vieille que HOLCombatProcessor.cpp (edite 09:08:53 par moi) : le build ci-dessus
TEST_ROUGE: 7fa5b77 (CELEBORN-1123a, vu 09:06 : "Expected 'every arrow a tower releases is launched on the display' to be 7, but it was 0") ; assertit l'effet : oui   TEST_ROUGE_SEPARE: oui (7fa5b77 test seul, 998788b correctif)
COMMITS: 2 (998788b correctif C++ 09:16, f602459 journal 09:16), merge_main_fait: oui (282c4ed, avant, par 1123a ; pas refait)
TESTS: UnrealEditor-Cmd UE_5.8_HOL -nullrhi, `Automation RunTests HeroesOfLiberty.Combat.BuildingShot` : 10/10 Success a 09:15 (BuildingShot, Cadence, Damage, Display, FiresEmpty, Fog, Guard, IdleVictim, NoUnit, Range) ; journal Saved\Logs\t7d.log du worktree ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (pas de scenario tower_wood, T8)
QUESTIONS: aucune
FAIT: HOLCombatProcessor.cpp ReleaseBuildingArrows : apres `++TotalArrowsReleased`, `World.GetSubsystem<UHOLArrowSubsystem>()` puis FHOLArrowLaunch {From = FVector(Step.Box.GetCenter(), Row->Height), To = Enemy.Location, Target = Enemy.Entity, FlightMs, ShooterType NAME_None} ; `(void)World;` retire. Docs : ligne S3-a d'Auto_Journey_File.md, EtapeDefini.md (ligne 57), FortificationsInProgress.md (commite), Placeholders.txt (ligne « Tir de tour » [forme du moteur]).
RESTE: T7b (champ replique sur AHOLBuilding + OnRep qui appelle Launch, test HOLNetTests.cpp rouge d'abord : pas fait, il restait 7 min, un build + tests ne tiennent pas) ; T8 (compteur de cout par pas, banc 200 unites 10 tours, scenario tower_wood). Aucun rouge connu.
NON VERIFIE: rendu en jeu (aucun essai ; Launch.From = centre + Height de la ligne, donc le toit si Height est bien la hauteur du toit, pas verifie a l'ecran) ; le lancement se fait aussi sur serveur dedie si le subsystem existe (le modele des unites fait pareil, l.3284) ; RELECTURE_CPP : diff relu (12 lignes), copie du modele l.3284 ; Row non nul (deja utilise l.~944 `Row->BuildingAttack`) ; DIAGNOSTICS_AS sans objet (aucun script) ; quick_check complet non lance (seulement BuildingShot*) ; seul HOLCombatProcessor.cpp touche (HOLHealProcessor / HOLZoneEffect* intacts).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: premier lancement des tests via Start-Process rate (commande executee par le shell bash : `$` non interprete), tache arretee par TaskStop, relancee en foreground (1 appel + 1 attente perdus) ; Write d'IN_USE refuse sans Read : remplace par Set-Content ; Edit refuse « not read » (2x) : remplace par PowerShell sur chaine exacte (EDITS_REFUSES: 2) ; build en file derriere Faramir.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, Auto_Journey_Jour.md, EtapeDefini.md, Placeholders.txt + non suivis Docs/Rapports/*, Retours/Q-* : docs seulement, inchange hors docs voulus : oui
