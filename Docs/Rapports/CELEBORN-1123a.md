# CELEBORN-1123a : S3-a T7/T8 (branche integ-41, worktree Celeborn)

TOUT T7/T8 EXIGE HOLCombatProcessor.cpp (interdit) : ARRET, correctif non fait.

APPELS_DECLARES: 26 (arret 25 vus ; depassement de 1 : la verification finale)
HEURE_DEBUT/HEURE_FIN: 08:58 / 09:07
PREMIER_BUILD: 09:01   BUILD: PASS (38 s, incremental, apres file Faramir)   LAUNCHCHECK_DEBUT: DLL du 22/11 22:24 plus vieille que les sources apres le merge main : build fait d'abord (c'est le build PASS ci-dessus)
TEST_ROUGE: 7fa5b77 (test HeroesOfLiberty.Combat.BuildingShotDisplay seul, pas de bouchon : Launch existe deja) ; rouge vu a 09:06 : "Expected 'every arrow a tower releases is launched on the display' to be 7, but it was 0" ; assertit l'effet : oui (compte lance de UHOLArrowSubsystem contre flèches relâchées par le processeur)   TEST_ROUGE_SEPARE: oui
COMMITS: 1 (7fa5b77, 09:06), merge_main_fait: oui (282c4ed, avant mon arrivée)
TESTS: BuildingShot* x10 lancés par UnrealEditor-Cmd (UE_5.8_HOL, -nullrhi) : 9 Success (Range, BuildingShot, Cadence, Damage, FiresEmpty, Fog, Guard, IdleVictim, NoUnit = T1-T6 rejoués apres merge main : verts), 1 Fail attendu (Display = T7) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (pas de scenario tower_wood : T8 bloque)
QUESTIONS: aucune
FAIT: test rouge T7 ; T1-T6 reverifies verts sur le build merge main.
RESTE (bloque par HOLCombatProcessor.cpp, a faire quand l'autre agent l'a libere) :
 T7 correctif : dans le bloc tour (Source\HeroesOfLiberty\Simulation\HOLCombatProcessor.cpp l.942-957, boucle `for Shot`), apres `++TotalArrowsReleased;` ajouter, comme l.3284-3292 : `if (UHOLArrowSubsystem* ArrowDisplay = World.GetSubsystem<UHOLArrowSubsystem>()) { FHOLArrowLaunch Launch; Launch.From = FVector(Step.Box.GetCenter(), Row->Height); Launch.To = Enemy.Location; Launch.Target = Enemy.Entity; Launch.FlightMs = FlightMs; Launch.ShooterType = NAME_None; ArrowDisplay->Launch(Launch); }` (World est une reference dans cette fonction, l.847 `(void)World;` a retirer ; `Row->Height` = toit : champ de la ligne a verifier, NON VERIFIE). Le test BuildingShotDisplay passe alors vert.
 T7b reseau : champ replique sur AHOLBuilding (compteur de tirs + id cible) + OnRep qui appelle Launch ; le processeur doit incrementer le champ (donc HOLCombatProcessor.cpp aussi) ; test HOLNetTests.cpp.
 T8 : compteur de cout (tours tirant / recherches par pas) dans le processeur (HOLCombatProcessor.cpp), banc 200 unites 10 tours, scenario tower_wood (donnees/script), Placeholders.txt (ligne tir de tour a ajouter avec T7).
NON VERIFIE: Launch.From a la hauteur du toit (champ Height non lu) ; Placeholders.txt, EtapeDefini.md, Auto_Journey_File.md, FortificationsInProgress.md NON mis a jour (budget d'appels epuise) ; RELECTURE_CPP sans objet (aucun C++ de production touche) ; DIAGNOSTICS_AS sans objet.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: 1er Write d'IN_USE refuse (Read requis) : premier Edit effectif au 9e appel ; premier essai de test avec le moteur UE_5.4 du launcher (mauvais moteur, le projet utilise UE_5.8_HOL via registre, EngineAssociation) : 1 appel perdu ; build en file derriere Faramir.
GIT_STATUS_PRINCIPALE: non verifie