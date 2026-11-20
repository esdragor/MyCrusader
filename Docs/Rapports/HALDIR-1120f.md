# HALDIR-1120f (wall-line, sans C++, Angelscript) : D834 Ecurie = s_horse6_snort

APPELS_DECLARES: 14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 09:50:12 / 09:54 (arret dur 10:08)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 8a413fe (Test_AudioElectedStableRingsTheCutSnort, Script/Audio/HOLAudioElected_Test.as) + assertit l'effet : oui (le plan joue /Game/Audio/Soft/SW_<stem> au gain de selection pour 27 graines). Le rouge n'est PAS vu a l'execution (pas de quick_check dans l'agent) : au commit 8a413fe, SelectEventFor("Stable") rend -1, donc `Event >= 0` echoue. Pas de stub : le test compile sur l'existant (Building.CutStable, IndexOfLabel).
COMMITS: 2 (8a413fe test, a72ae48 correctif), dernier a 09:53, merge_main_fait: non (non demande ; branche prete)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (le stem est calcule par HOLAudioCandidates::ForCue("Building.CutStable") + IndexOfLabel("A"), le plan par HOLAudio::Plan ; seule recopie : Stem.Contains("snort"), garde sur le choix)
ESSAIS: aucun
QUESTIONS: aucune

FAIT:
- HOLAudioTable.as : HOLAudioEvent::SelectBuildingStable = 32 ; Add(Cues, ..., "Select.BuildingStable", "soft", "s_horse6_snort", BuildingSelectGainPercent, 97, 103, 60, Order).
- HOLAudioBuildings.as : SelectEventFor("Stable") -> SelectBuildingStable.
- HOLAudioCandidates.as : liste "Select.BuildingStable" = les 2 variantes entendues (s_horse6_snort, s_horse6_vocal_full) ; HOLAudioCandidates_Test.as : attendu = 2 pour ce cue (>= 2).
- HOLAudioElected_Test.as : les deux listes "Silent" (Stable) remplacees par "Spaceship" (inconnu = silencieux) ; compte de la table 32 -> 33.
- Tools/Audio/check_audio_candidates.ps1 (Select.BuildingStable = 2 stems, analyse PS OK) et check_audio_soft.py (CLEAN_CUES).
- Auto_Journey_File.md ligne D834 passee [x] ; EtapeDefini.md ligne ajoutee ; AudioBuildingsInProgress.md ligne ajoutee.

RESTE:
- ASSET MANQUANT : Content/Audio/Soft/SW_s_horse6_snort.uasset n'existe pas dans le worktree (seulement Content/Audio/Candidates/soft/SW_s_horse6_snort.uasset dans le worktree Galadriel, non commite). A importer par l'editeur (pas d'editeur dans l'agent) : HOL_AUDIO_ONLY=s_horse6_snort, Tools/Editor/import_audio_elected.py (source C:\Users\baudo\Documents\MyCrusader_Audio\Candidates\soft\s_horse6_snort.wav, presente), puis commit de l'asset et check_audio_table.ps1 -Assets. Les tests script ne lisent pas l'asset (chaine de chemin) mais l'Ecurie serait muette en jeu tant qu'il manque.
- Lancer quick_check -Parts tests (session principale) : attendu rouge sur Test_AudioElectedStableRingsTheCutSnort a 8a413fe, vert a a72ae48.

NON VERIFIE: tout (aucun test, aucune compilation script ; `git grep` seulement pour les comptes en dur : seul HOLAudioElected_Test.as:58 comptait les evenements). Nom de variable `Expected` rendu non const dans HOLAudioCandidates_Test.as.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: copie principale : docs voulus (AubeInProgress, AudioBuildingsInProgress, Auto_Journey_File, EtapeDefini) modifies par moi/autres ; Script/Tests/HOLFactionSlot_Test.as et Script/UI/HOLFactionSlot.as modifies dans la copie principale : PAS de moi (autre session) : inchange par moi, oui pour le reste. Placeholders.txt non touche (l'election ne cree ni ne retire de provisoire).
