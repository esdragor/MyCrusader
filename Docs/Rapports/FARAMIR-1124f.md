# FARAMIR-1124f : colonne « faction » separee dans FPartHashes (D894)

BUILD FAIL (11:51, 625 s, exit 6, resultat final ; les 3 commits NE COMPILENT PAS). Erreurs : (a) HOLZoneEffectTests.cpp : `OpenWorld`/`CloseWorld` introuvables (helpers hors portee, a chercher dans HOLZoneEffectTests.cpp, peut-etre un namespace anonyme ou une autre signature) ; (b) `AHOLGameState::FactionResource`, `PowerCooldownEndMs`, `SetFactionResource`, `SetPowerCooldownEndMs` sont PRIVES (HOLGameState.h:461-488 : le « public » du commentaire est faux, ou un `private:`/`protected:` precede) : HashFaction doit passer par les getters publics (`GetFactionResource(Slot)`, `GetPowerCooldownEndMs(Slot, PowerIndex)`, boucle sur `FHOLMatchConfig::MaxSlots` et `MaxFactionPowers`) ; le test doit atteindre les setters par reflexion (`FindFunctionByName` + `ProcessEvent`, comme `HOLHealProcessor::CreditHonorPoints`) ou par les fonctions publiques d'un autre appelant. Un agent suivant corrige ces deux points puis rebuild. Journal : C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Saved\Logs\build.log.

(Texte initial, avant le resultat du build :) NON COMPILE (au moment du rapport) : le build `Tools\build.ps1 -LiveCoding=0 -Wait` du worktree Faramir est EN FILE derriere le build de Galadriel (lance 11:40:25, en file a 11:43, sortie : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\0584c8ca-a9ce-4837-a887-ba2185dcd361\tasks\bmtwyw21h.output). L'arret dur 11:53 est tombe pendant l'attente : la passe d'integration compilera (`Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Faramir, puis `quick_check -Parts tests` avec le quick_check.ps1 du worktree, filtre `HeroesOfLiberty.Simulation.FactionHashColumn`).

Impact joueur : aucun visible ; le journal de determinisme (hol.Simulation.StepHashes >= 2) nomme desormais la jauge d'Honneur et les recharges de pouvoirs quand deux parties divergent.

APPELS_DECLARES: 24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:38:05 / ~11:45
PREMIER_BUILD: 11:40:25 (en file)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: binaire plus vieux que HOLSimulationSubsystem.cpp (2026-11-23T17:56:58 contre sources 11:40) : build necessaire (pris par la file)
TEST_ROUGE: aac9d9a (bouchon HashFactionState constant + test HeroesOfLiberty.Simulation.FactionHashColumn dans HOLZoneEffectTests.cpp), assertit l'effet : oui (hash change quand jauge/recharge change, revient quand remis a 0, depend du slot)   TEST_ROUGE_SEPARE: oui (commit a part) ; rouge NON VU a l'execution (un seul build en file, pas deux)
COMMITS: 3 sur zone-effect : aac9d9a (rouge), ea2c420 (correctif), ef5f1ef (membre Faction oublie dans ea2c420 : ea2c420 seul ne compile pas, ef5f1ef le complete), dernier 11:40 ; merge_main_fait: oui (premier geste, 11:38)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (le hash est calcule par HashFaction sur le vrai game state, pas recopie)
ESSAIS: aucun
QUESTIONS: aucune (D894 tranche : B)
FAIT : `FPartHashes::Faction` ; `HashFaction(World)` (FNV-1a : tailles puis valeurs de FactionResource et PowerCooldownEndMs de AHOLGameState) ; `UHOLSimulationSubsystem::HashFactionState()` public (pour le test) ; RunStep pose `Parts.Faction` et le journal `Step hash: ... after <proc>` gagne ` faction %016llx` (StepHashes >= 2). HashUnits : AUCUNE ligne modifiee (la colonne est calculee au site d'appel de RunStep).
RESTE : compiler, lancer le test (rouge a verifier en mettant le bouchon si on veut le voir), RELECTURE_CPP ci-dessous, ligne D894 d'Auto_Journey_File a cocher apres vert.
RELECTURE_CPP : (1) HashFaction lit le GameState du monde sur le serveur seulement (RunStep ne tourne pas sur client) ; GetGameState<AHOLGameState> peut etre null en test sans GameState : renvoie la graine. (2) Le test insere un `#include` au milieu de HOLZoneEffectTests.cpp (avant le test) : correct mais a remonter en tete au prochain passage. (3) ea2c420 reference Parts.Faction ajoute dans ef5f1ef : a squasher a la fusion. (4) PowerCooldownEndMs/FactionResource sont des champs UPROPERTY publics lus directement : pas de reflexion.
NON VERIFIE : tout (compilation, test, journal reel).
PROCESSUS_DETACHES: build en file (non arrete, regle 3e) ; aucun autre.
ANOMALIES_PROCESSUS: build en file derriere Galadriel (3 min+ d'attente) ; le 8e appel / premier Edit depasse (premier Edit au 17e appel) car le code a lire (HashUnits, GameState, helpers de test) demandait 12 appels.
GIT_STATUS_PRINCIPALE: non verifie (aucun fichier de code touche dans la copie principale).
