# GALADRIEL-1120e (worktree HeroesOfLiberty-Galadriel, branche specialization-trial)

NON COMPILE, aucun test lance (sans C++, ni build, ni quick_check).

APPELS_DECLARES: 14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:06 / 10:11
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: e00c4a4 (faction) et 62286bc (recharges), assertit l'effet : oui. Rouge NON VU a l'execution (rien ne se lance dans l'agent) : a voir au premier lancement des tests script sur e00c4a4 puis 62286bc.
COMMITS: 4 (e00c4a4, efa29c6, 62286bc, 90be4f5), dernier a 10:10 ; merge integ-26 fait (premier geste), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les valeurs assertees sont calculees par FromGameStateAt / CooldownsLeft depuis un vrai AHOLGameState)
ESSAIS: aucun (pas de scenario, consigne)
QUESTIONS: aucune

## FAIT
(1) Faction par le state. `Script\UI\HOLFactionSlot.as` : `FactionOfText` supprime ; `FromGameState(State, Slot, bHasSpecializationButton, bTierOpen)` lit `EHOLFaction(State.GetSlotFaction(Slot))` ; `RefreshFactionSlot` (HOLGameScreenScript.as, la fonction qui appelle FromGameState) n'a plus l'argument texte. Stub rouge de e00c4a4 : faction = BlackLegions en dur (le test « slot de l'Aube visible, jauge = valeur du state » est rouge), correctif efa29c6.
Tests : `Test_FactionSlotFromGameStateValue` adapte. Le « texte inconnu » est retire. Le cas Legions est RETIRE aussi : le monde de test n'a pas de LaunchConfig, `GetSlotFaction` y rend toujours l'Aube, et `LaunchConfig` n'a aucun setter exposable au script. Remplace par « slot 1 lit la faction du state (Aube) et sa propre valeur de jauge ». Le spectateur est conserve. La regle Legions sans jauge reste couverte par `Make(EHOLFaction::BlackLegions, ...)` (tests existants).
(2) Recharges. Ajouts dans `HOLFactionSlot` : `CooldownLeft(EndMs, NowMs)` (pure : 0 si EndMs <= 0 ou <= NowMs, sinon EndMs - NowMs), `CooldownsLeft(State, Slot, Count, NowMs)` (lit `GetPowerCooldownEndMs`), `FromGameStateAt(State, Slot, NowMs, ...)`. `FromGameState` appelle `FromGameStateAt` avec `int(State.GetServerWorldTimeSeconds() * 1000.0)`. Tests neufs : `Test_FactionSlotCooldownLeftRule` (pure), `Test_FactionSlotCooldownFromGameState` (vrai state : 3000 ms restantes a 2000, 0 a 6000, autre slot 0 ; ligne codee : Cooling puis Ready, Describe « a cost 10 cooling 3 s »). Stub 62286bc (CooldownLeft rend 0), correctif 90be4f5.

## ECART avec l'enonce
L'enonce dit « recharge en cours -> `soon`, finie -> `ready` ». Dans `HOLPowerBookRules.as`, `Soon` = pouvoir SANS effet code (`bEffectCoded` false), et l'unique pouvoir de l'Aube (SacredLight, HOLDawnPowers.as) est non code : son etat reste `Soon` quelle que soit la recharge. Les etats de recharge sont `Cooling` (en cours) et `Ready` (finie, si ressource suffisante). Les tests assertent Cooling / Ready sur une ligne codee et la valeur `CooldownLeftMs` sur la vraie ligne de l'Aube. Pas de question ouverte (c'est la regle existante D601).

## RESTE
- Lancer les tests script (`HOLFactionSlot_Test`) : voir le rouge sur e00c4a4 puis 62286bc, puis le vert sur le tip.
- Personne n'ECRIT `PowerCooldownEndMs` cote serveur (aucun lanceur de pouvoir ne l'appelle) : la recharge reste 0 en jeu jusque-la.
- Scenario `faction_slot` + preflight (session principale).

BUILDS_DEMANDES: 0
TESTS_SCRIPT_AJOUTES: 2 (Test_FactionSlotCooldownLeftRule, Test_FactionSlotCooldownFromGameState) ; 1 adapte (Test_FactionSlotFromGameStateValue)
TYPES_A_EXPOSER: un setter de test pour la faction d'un emplacement du LaunchConfig (p. ex. dans HOLTestWorldLibrary) pour retablir un test Legions de bout en bout. Verifier aussi que `AGameStateBase::GetServerWorldTimeSeconds` est visible du script (BlueprintCallable) : si la compilation script refuse, remplacer par un petit accesseur C++ sur AHOLGameState.

NON VERIFIE: tout (aucune compilation script ni test). Points a risque : `GetServerWorldTimeSeconds()` en script ; l'unite de `PowerCooldownEndMs` (le commentaire C++ dit « match time (ms) », on le compare a l'heure du serveur en ms ; a aligner avec le futur ecrivain) ; `TArray<int>` passe par valeur a `Make` ; `Math` non utilise.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune. `LaunchCheck` non applicable. Une premiere edition PowerShell a echoue (fins de ligne CRLF dans le remplacement multi-ligne), refaite avec normalisation CRLF/LF et conservation du BOM : EDITS_REFUSES 1 (sans effet).
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress.md, AudioBuildingsInProgress.md, Auto_Journey_File.md, DuelInProgress.md, EtapeDefini.md, Placeholders.txt + Docs\ non suivis) : aucun fichier de code touche dans la copie principale : oui. Placeholders.txt non touche par moi.
