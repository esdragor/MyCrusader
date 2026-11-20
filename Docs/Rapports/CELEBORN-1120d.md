# CELEBORN-1120d : hol.Cheat.Honor fermee en script (2026-11-20)

Joueur : la triche `hol.Cheat.Honor <valeur>` est maintenant branchee de bout en bout (commande, RPC serveur, jauge de l'Aube) ; rien n'est compile, donc pas encore jouable avant le build du lot.

```
APPELS_DECLARES: 12 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 10:06 / 10:10
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: e8f53d9 + assertit l'effet : oui (Test_HonorCheatServerApply lit AHOLGameState::GetFactionResource apres ServerApply ; Test_ControllerCommandsC2CheatHonor voit la route ENVOYEE via IsRouted) ; rouge NON VU a l'execution
COMMITS: 2 (e8f53d9 test + stubs, 2de465d correctif), dernier a 10:09, merge_main_fait: non (integ-26 fusionne en premier geste, aucun build)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (la valeur lue vient de la jauge de l'etat apres le code teste)
ESSAIS: aucun
QUESTIONS: aucune
```

## FAIT (branche duel-rewards, worktree HeroesOfLiberty-Celeborn, NON COMPILE)

- `git merge integ-26` fait au premier geste (GetSlotFaction present).
- `HOLHonorCheat::ServerApply(State, bCheatsAllowed, Sender, Value)` (`Script\Factions\HOLHonorCheat.as`) : faction = `EHOLFaction(State.GetSlotFaction(Sender))`, refus par `HOLRpc::CheatHonorRefusal`, puis `Apply`. Retour : valeur stockee, ou -Refus (-1 triches coupees, -2 sans siege, -3 sans jauge), ou -4 (rien ecrit).
- `ServerCheatHonor(int Value)` dans `Script\Net\HOLRpcPlayer.as` (avant `ServerCheatSpecialization`), sur le modele de `ServerCheatTier` ; journalise `RpcCheat: honor refused (N)` ou `honor set to V`.
- `HOLControllerCommandsC2::Send` : branche `ServerCheatHonor` ; nouvelle `IsRouted(Route)` pure (liste des routes que Send sait envoyer).
- `FactionOfText` retire de `Script\Duel\HOLDuelRewards.as` (plus d'appelant ; `HOLFactionSlot.as` a sa propre `FactionOfText` a 2 arguments, non touchee).
- Tests : `Test_HonorCheatServerApply` (neuf, sur un vrai `AHOLGameState`), `Test_ControllerCommandsC2CheatHonor` (assertion SENT) et `Test_ControllerCommandsC2EachNameOnceAndRouted` (chaque commande valide a une branche Send). TESTS_SCRIPT_AJOUTES: 1 (Test_HonorCheatServerApply) + 2 modifies. BUILDS_DEMANDES: 0. TYPES_A_EXPOSER: aucun.
- Journaux : EtapeDefini.md (ligne socle factions morceau 2), DuelInProgress.md (D7), Auto_Journey_File.md. AubeInProgress.md non touche.

## RESTE

- Compiler (lot), puis `quick_check -Parts tests` (rouge attendu au 1er commit : `IsRouted` sans Honor, `ServerApply` stub -9).
- Essai reseau du RPC (client envoie, serveur ecrit la jauge) : non fait, `-Parts net`.
- Refus 3 (faction sans jauge) non testable sur un etat de test (pas de configuration de slot) : couvert seulement par `Test_HonorCheatRefusal` en pur.

## NON VERIFIE

Rien n'a ete compile ni lance. Points a surveiller a la premiere analyse Angelscript : `HOLRpc::CheatHonorRefusal` appelee depuis le namespace `HOLHonorCheat` (autre fichier), `EHOLFaction(int)`, `ServerApply(nullptr, ...)` dans le test, ligne `RpcCheat:` avec `{-Result}` dans une f-string.

## ANOMALIES

- `eol_check -Fix` a remis en CRLF 4 fichiers (HOLDuelRewards_Test.as, HOLHonorCheat_Test.as, check.ps1, TestSettings.ps1) : aucun diff git.
- `Tools/Scenarios/c28_*`, `c31_*` non suivis dans le worktree : pas a moi, non commites.

```
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini, DuelInProgress, Auto_Journey_File + fichiers deja modifies par d'autres sessions) : oui
```
