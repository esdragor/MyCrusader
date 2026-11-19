# GLORFINDEL-1119m : reprise 14, etape 5, lot S1 (pause + diplomatie de HOLGameState)

Impact joueur : aucun attendu (memes refus, memes delais, memes lignes de journal) ; rien n'est compile ni execute.
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, branche net-playtest (base main 3b52f7a). Etape atteinte : 4 sur 4 en branchement, SANS retrait de la copie C++ (voir Reste).

## Commits (un par etape, anglais, sans Co-Authored-By, pas de push)

1. `1d51c51` test d'abord : `Script\Match\HOLMatchFlow_Test.as`
2. `8dd9447` regle : `Script\Match\HOLMatchFlowBoard.as` (hors Dev / Editor / Examples)
3. `bba3920` pont : `Source\HeroesOfLiberty\Match\HOLMatchFlowSubsystem.h/.cpp` (~110 l. avec les 6 evenements et leurs `_Implementation`, plus 45 l. de classe script dans la planche) ; raison D839 : liaison a exposer ; la replication des champs de GameState reste C++.
4. `1cafa5f` branchement : `HOLGameState.cpp` (+18 / -5 lignes, 7 remplacements exacts)

## Signatures

Script (namespace `HOLMatchFlow`, enums globaux `EHOLPauseAnswer { Allowed, NotInMatch, NoPauseLeft, AlreadyPaused, NotPaused, TimerRunning }` dans l'ordre du C++ `FHOLPauseRules::EAnswer`, `EHOLDiplomacyGate { Open, NotDiplomacyMode, NoSuchPlayer }`) :
`DefaultPausesPerMatch()`, `DefaultTimerSeconds()`, `CanPause(bSolo, PausedBySlot, PausesLeft, bInMatch)`, `TimerLeft(PausedBySlot, PausedAt, Now)`, `CanResume(bSolo, PausedBySlot, PausedAt, RequesterSlot, bInMatch, Now)`, `PausesLeftAfterPause(bSolo, PausesLeft)`, `PausedSeconds(PausedAt, Now)`, `ShouldLiftPause(bAuthority, bPaused, PausedBySlot, LeavingSlot)`, `StanceGate(bAuthority, bDiplomacyMode, bFromOccupied, bToOccupied)`, `VisionGateOpen(bAuthority, bDiplomacyMode)`, `ShouldEvaluateMatchEnd(bAnnounce, bFormed)`.

Pont C++ `UCLASS(Abstract) UHOLMatchFlowSubsystem : UWorldSubsystem` (Game et PIE), sous-classe script `UHOLMatchFlowBoardScript` (dans `HOLMatchFlowBoard.as`). Statiques : `AskCanPause`, `AskCanResume`, `AskPausesLeftAfterPause`, `AskStanceGate`, `AskVisionGate`, `AskEvaluateMatchEnd` ; chacune renvoie un entier ou -1 (pas de planche script : GameState garde alors la regle C++). Evenements natifs proteges : `PauseAnswer`, `ResumeAnswer`, `PausesAfterPause`, `StanceGateAnswer`, `VisionGateAnswer`, `EvaluateEndAnswer` (defaut -1).

## Tests ajoutes (script, 6)

`Test_MatchFlowPauseAllowedAndRefused`, `Test_MatchFlowResume`, `Test_MatchFlowTimerAndCount`, `Test_MatchFlowLiftPauseOfLeaver`, `Test_MatchFlowStanceGate`, `Test_MatchFlowVisionAndAllianceEnd`.

## Tests C++ couverts (rejoues valeur pour valeur = test d'accord tant que deux copies existent)

- `HeroesOfLiberty.Net.PauseLimits` (`FHOLNetPauseLimitsTest`, `Tests\HOLNetTests.cpp:1130-1181`) : refus hors match, deja en pause, timer 60 / 45 s / 0, resume par celui qui a pause, autre joueur a 159 s refuse puis a 160 s accepte, spectateur, 3 pauses puis la 4e refusee, solo sans limite ni minuterie.
- `HeroesOfLiberty.Match.DisconnectPause` (`FHOLDisconnectPauseTest`, `Tests\HOLDisconnectTests.cpp:800`) : la levee de la pause du joueur qui part (`ShouldLiftPause`, compte non touche). La levee elle-meme (`LiftPauseOf`) reste en C++, non branchee.
- Diplomatie : pas de test C++ direct de `ApplyStance` / `ApplyVision` (seul `HOLDiplomacyStateTests.cpp:140` consomme des ordres). Les portes sont couvertes par les cas du code lu (voir ci-dessous), pas par un test C++ existant. Les regles des stances (`HOLDiplomacy::SetStance` / `SetVision`) sont deja testees par `HOLDiplomacyRules_Test.as`.

## Lignes C++ retirees : 0 (net +13 dans HOLGameState.cpp)

La copie C++ (`FHOLPauseRules`, portes en ligne) reste : le test d'accord est le fichier de test script (memes vecteurs) ; elle sert aussi de repli quand le monde n'a pas de planche. Pas retiree : le retrait suppose de voir le test script vert puis de supprimer `FHOLNetPauseLimitsTest` (regle du brief : rouge VU puis vert), ce que je ne peux pas faire sans compiler.

## Table des journaux lus par les outils (grep de Tools\, lignes inchangees, niveau Display inchange)

| Ligne | Lue par | Etat |
|---|---|---|
| `Pause: slot 1 (Bob) paused` (prefixe de `Pause: slot %d (%s) paused, %d left`) | Tools\Tests\lobby_match.ps1:470 | inchangee |
| `Pause: lifted, Bob (slot 1) left` | lobby_match.ps1:471 | inchangee (`LiftPauseOf` non touchee) |
| `Pause:` (filtre large) | lobby_match.ps1:440 | inchangee |
| `Pause: this machine's world paused/running` | Tools\Tests\log_lock_test.ps1:188 | inchangee (`NotifyPauseChanged` non touchee) |
| `Pause: refused to %s (slot %d): %s` | aucun outil trouve | inchangee (le texte vient toujours de `FHOLPauseRules::ToString`, sur la reponse du script) |
| `Diplomacy: slot N now <stance> toward slot M`, `Diplomacy: slot N shares its vision with slot M` | Tools\Scenarios\diplomacy_compare.txt:14,79-97, diplomacy_window.txt:12 | inchangees |
| `Diplomacy report: slot A toward slot B` | une trentaine de scenarios (diplomacy*.txt, c31_f*.txt, melee_spin*.txt) | inchangee (`ReportDiplomacy` non touchee) |
| `RpcDiplomacy ...`, `Diplomacy order: slot N ... queued` | Tools\Scenarios\rpc_diplomacy_run.ps1 | hors lot (script de RPC, non touche) |
| rapports de pause `hol.Match.PauseReport` | Tools\Tests\quick_check.ps1:473-496 | commande non touchee |

## Resultat par rapport au C++ : points d'equivalence a revoir a la relecture

- Porte de stance : le C++ journalise « not the Diplomacy mode » si `!IsDiplomacyMode()`, sinon « no such player » (y compris client sans autorite) ; `StanceGate` donne les memes trois cas, le texte est encore choisi en C++ par `!IsDiplomacyMode()`, donc identique.
- `CanPause` en solo : la regle C++ teste `AlreadyPaused` avant `Standalone` ; repris dans cet ordre.
- `PausesLeftAfterPause` est appele AVANT `FHOLPauseRules::Pause` (avec l'ancien compte) puis ecrase `State.PausesLeft`.

## NON VERIFIE

Rien compile ni execute. En particulier : syntaxe Angelscript (enums globaux, `int(EHOLPauseAnswer::X)`, `float64` dans les surcharges de `ResumeAnswer`, `Math::Max` sur `float`), generation UHT des 6 `BlueprintNativeEvent` avec `double` (declares sans `UFUNCTION` pour les `_Implementation`, comme `UHOLUnitTierSubsystem`), collision de noms (`Gate`, `EvaluateEnd`, `ScriptAnswer`, `bSolo` dans `HOLGameState.cpp`), que la planche soit bien nee sur le serveur dedie ET en solo (sinon repli C++ : aucun changement mais rien de teste), que le monde des tests C++ `FHOLDisconnectPauseTest` ne cree pas de planche (repli) ou la cree (script). Le commit 1d51c51 seul ne compile pas cote script (la regle arrive au commit suivant) : c'est le rouge voulu, ne pas construire sur ce commit.

## Reste / questions (aucune decision prise)

1. Retrait de la copie C++ : apres le build, voir le rouge (fausser une valeur attendue de `Test_MatchFlowResume`, p. ex. 160.0 -> 159.0 sur `CanResume`), puis le vert, puis seulement supprimer `FHOLNetPauseLimitsTest` et les corps `CanPause` / `CanResume` / `TimerLeft` de `FHOLPauseRules` (le client a encore son propre calcul de minuterie, `GetPauseTimerLeft`, non branche).
2. Non migres dans ce lot (petits) : `GetPauseTimerLeft`, `LiftPauseOf` et le cumul `WindowPausedSeconds` appellent encore le C++ ; `ShouldLiftPause`, `TimerLeft`, `PausedSeconds` existent et sont testes en script mais ne sont pas branches.
3. Question : `HOLDiplomacy::SetStance` / `SetVision` (regles des stances) sont en C++ (`Match\HOLDiplomacyRules.cpp`) exposees en lecture a `HOLDiplomacyRead` ; le plan les croyait « deja en script ». Les migrer (ou les laisser, source unique en C++) : decision de design a prendre ; ce lot ne migre que les portes et la decision d'evaluer la fin de manche.
4. Placeholders.txt : aucun provisoire ajoute ou retire.

## Champs

- BUILDS_DEMANDES: 1 (lot C++ : HOLMatchFlowSubsystem.h/.cpp + HOLGameState.cpp)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 6 (Test_MatchFlowPauseAllowedAndRefused, Test_MatchFlowResume, Test_MatchFlowTimerAndCount, Test_MatchFlowLiftPauseOfLeaver, Test_MatchFlowStanceGate, Test_MatchFlowVisionAndAllianceEnd)
- LIGNES_CPP_SUPPRIMEES: 0 (net +13 ; 5 supprimees, 18 ajoutees dans HOLGameState.cpp, plus ~110 l. de pont neuf)
- LIGNES_MIGREES_PAR_LOT: 0 retirees ; ~60 l. de decision recopiees en script (pause et portes de diplomatie)
- TYPES_A_EXPOSER: aucun nouveau (entiers et booleens seulement) ; UHOLMatchFlowSubsystem (UCLASS Abstract) neuf
- TEST_ROUGE: non vu (aucune execution) ; a voir selon la section commandes
- TESTS_TAUTOLOGIQUES: 0 (les tests passent par la regle de pause et de portes avec les vecteurs du test C++ existant)
- PROCESSUS_DETACHES: aucun lance

## Commandes a lancer par la session principale

1. Binaire : `. C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\LaunchCheck.ps1; Get-StaleBinaryProblem C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` puis `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` (un build : un `.h` neuf avec UCLASS).
2. Rouge : dans `HOLMatchFlowBoard.as`, `CanResume`, remplacer `<= 0.0` par `<= 5.0`, `quick_check -Parts tests` (attendu : `Test_MatchFlowResume` rouge), remettre, relancer (vert, total +6 script, aucun test C++ retire).
3. Tests : `quick_check -Parts tests` ; C++ qui touchent GameState : `HeroesOfLiberty.Net.PauseLimits`, `HeroesOfLiberty.Match.DisconnectPause`, `HeroesOfLiberty.Diplomacy` (tests HOLDiplomacy*Tests.cpp).
4. Preuve reseau : `quick_check` parties net (pause : Bob pause, Alice refusee tant que la minuterie court, seconde pause de Bob refusee, reprise de Bob ; diplomatie : Bob Ally, Alice Ally, Bob rompt) ; `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Tests\lobby_match.ps1` (lignes `Pause: slot 1 (Bob) paused` et `Pause: lifted, Bob (slot 1) left`) ; `Tools\Scenarios\rpc_diplomacy_run.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` ; scenario `diplomacy_compare`. Verifier dans le journal serveur que la planche est nee (aucune ligne `Angelscript: Error`) et, si possible, que `RequestPause` passe par le script (ajouter un log temporaire ou observer un refus force par une valeur fausse).
