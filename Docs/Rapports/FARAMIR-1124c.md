# FARAMIR-1124c (REVUE-1123pm genant 4 : retrait du relais Script/Morale/HOLMoraleRules.as)

Impact joueur : aucun (refactor script, meme regle C++ derriere). Branche zone-effect, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, commit ed8cbc9 (non pousse, merge main non fait).

APPELS_DECLARES: 16 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:02:57 / ~11:11
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: vide (Get-StaleBinaryProblem sans sortie : DLL du 2026-11-23 17:56 a jour, ClearMalusAround deja dans HOLMoraleLibrary.h)
TEST_ROUGE: sans objet (refactor, pas de regle neuve) ; assertit l'effet : sans objet   TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (ed8cbc9, 11:08), merge_main_fait: non (DLL du worktree plus vieille que le Source de main non verifiee ; la session principale synchronise)
TESTS: aucun test lance ; les 6 fichiers de test migres ont ete COMPILES au chargement du jeu (0 erreur) mais PAS EXECUTES ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 0
ESSAIS: aucun (pas d'E-*.md)
QUESTIONS: aucune
TYPES_A_EXPOSER: aucun   BUILDS_DEMANDES: 0

## FAIT
- Verifie d'abord : FARAMIR-1124b (HOLFactionPowerServer.as:177) appelle `HOLMorale::ClearMalusAround` (natif, UHOLMoraleLibrary), PAS le relais.
- HOLMoraleTable.as:177 -> `HOLMorale::NativeMakeEffect` ; UI/HOLMoraleBlock.as:40 et :61 -> `HOLMorale::NativeIsMalus`.
- Grep : plus aucun .as (tests compris) n'appelle un nom du relais hors Native* ; `git rm` de Script/Morale/HOLMoraleRules.as et Script/Tests/HOLMoraleRules_Test.as.
- ECART A LA LIGNE 51 (non prevu par la ligne) : 6 fichiers de TEST utilisaient aussi le relais (HOLMoraleBlock_Test, HOLMoraleRecipients_Test, HOLMoraleTable_Test, HOLMoraleView_Test, HOLSocleLibraries_Test, HOLUnitAbilityRules_Test : Apply, Tick, Total, Multiplier, GrantImmunity, RemoveMalus, HasIntensity, IsMalus, constantes Lever*/Apply*). Sans correction le retrait cassait la compilation du script. Migres par script (FARAMIR_relay.ps1, scratchpad) : `HOLMorale::X(` -> `HOLMorale::NativeX(` ; `NativeApply` n'a pas de valeur par defaut : `, 0` ajoute aux appels a 2 arguments ; `HOLMorale::Count(State)` -> `State.Effects.Num()` (HOLMoraleTable_Test.as:73 et :105).
- Les constantes Lever*/Apply* n'ont pas d'equivalent natif (aucun appelant hors tests) : nouveau fichier Script/Tests/HOLMoraleCodes.as (namespace HOLMorale, constantes seules, tests seuls). C'est un choix de repli, pas une decision de design : si l'utilisateur prefere les exposer en C++ (lot C++), on peut supprimer ce fichier.
- Jeu charge : `play.ps1 ai_economy_start -Hidden -Root <worktree> -TimeoutSec 100` (journal Saved\Logs\HeroesOfLiberty.log du worktree, 11:05-11:07) : 0 ligne « Angelscript ... Error » (grep strict `Angelscript.*\bError\b` = 0), uniquement des Warning de division entiere (HOLAIMarket.as, HOLAudioPlan.as, anterieurs). Tout le Script de la branche zone-effect (HOLFactionPowerServer, ClearMalusOrderOf, tests) a compile. DIAGNOSTICS_AS: 0 erreur (par le journal, pas par le serveur de langage VS Code).
- Ligne 51 d'Auto_Journey_File.md cochee `[x]` ; ligne EtapeDefini.md ajoutee ; IN_USE.txt du worktree remis a « libre ».

## RESTE
- Le test Script/Tests/HOLMoraleRules_Test.as (regle de plafonds, cumul, attenuation, immunite) est supprime comme demande : sa couverture vient de HOLMoraleRulesTests.cpp. Verifier a la prochaine session C++ que ce test C++ couvre bien ClearAuras / RemoveMalus / CombineAttenuation (non verifie ici).
- Commentaires de Source\...\HOLMoraleLibrary.h, HOLMoraleRules.h/.cpp et Tools\Data\ValueSheet.csv citent encore Script/Morale/HOLMoraleRules.as : a nettoyer au prochain lot C++ (aucun effet).
- PlaceholdersInProgress / Placeholders.txt : aucun provisoire ajoute ni retire.

## NON VERIFIE
- Les tests script migres n'ont pas ete EXECUTES (quick_check -Parts tests non lance, regle du brief) : seule leur compilation est vue.
- Le scenario ai_economy_start a fini « failed » apres 60 s (`Scenario panel: scenario ai_economy_start failed`), cause non cherchee (hors tache : choisi seulement comme chargement court ; il ne se termine pas par quit).
- Le journal contient des `LogAutomationTest: Error: Condition failed` au demarrage (UE::UnifiedErrorTest, tests internes du moteur, 10:06:00), pas du Script.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune (play.ps1 a ferme lui-meme son UnrealEditor 61252 a 100 s)
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File.md, EtapeDefini.md, Docs/Rapports, Retours, Placeholders.txt, SessionEnd.txt) : oui, aucun fichier de code modifie par moi dans la copie principale.
