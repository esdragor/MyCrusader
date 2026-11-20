APPELS_DECLARES: 9 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 10:06:41 / 10:09
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun commit de test (le test Test_AudioCandidatesEveryCueHasTen attend deja Select.BuildingStable = Building.CutStable, ajoute par a72ae48 ; rouge vu par l'utilisateur) ; assertit l'effet : oui (CueCount de la table et nombre de variantes)
COMMITS: 1 (da2d910), dernier a 10:08, merge_main_fait: non (merge integ-26 fait au premier geste, a2 merge sans conflit)
TESTS: check_audio_soft.py PASS (141 sons) ; check_audio_candidates.ps1 PASS (42 cues, 389 stems) ; tests AS NON LANCES ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
 Cause : a72ae48 a mis Add(List,"Select.BuildingStable",...) dans BuildBuildings() (listes de familles, NON comptees par CueCount) au lieu de Build() (une liste par cue de la table, comme Select.BuildingRange/Barracks). Donc CueCount() = 32 contre HOLAudio::Count() = 33, et la 1re assertion de Test_AudioCandidatesEveryCueHasTen echouait (ForCue trouvait quand meme la liste via BuildBuildings, donc les autres assertions passaient).
 Correctif : ligne deplacee dans Build() apres Select.BuildingBarracks (memes 2 stems : s_horse6_snort, s_horse6_vocal_full ; aucun son choisi a la place de l'utilisateur). Building.CutStable reste dans BuildBuildings (comparatif).
 Autres tests/outils verifies par lecture : HOLAudioElected_Test (Count 33), HOLAudioBuildingFamilies_Test (cues de table != cues de famille : OK), check_audio_candidates.ps1 (accepte deja Select.BuildingStable = 2), check_audio_soft.py (CLEAN_CUES l'inclut). Aucun autre test ne compte CueCount.
 RESTE : session principale lance quick_check -Parts tests ; import de SW_s_horse6_snort (hors de ma tache).
NON VERIFIE: les tests AngelScript n'ont pas ete executes (pas d'editeur) ; l'ordre de Build() n'est pas assert par un test.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune ; Content/Maps/Crown.umap non suivi dans le worktree (pas de moi, merge integ-26)
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress, AudioBuildingsInProgress, Auto_Journey_File, EtapeDefini, Placeholders + Docs non suivis) : oui