# HALDIR-1116r : pont serveur Camp (bascule Camp, D836 point 3)

Branche maps (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`), sans build ni quick_check (consigne). Lancement 23:33.

## Fait
- `Source\HeroesOfLiberty\Simulation\HOLCampBoard.h/.cpp` : `UHOLCampBoard` (UWorldSubsystem abstrait, modèle `UHOLGarrisonBoard`), événements `CampStep` et `CampRewards` (BlueprintNativeEvent) dont l'implémentation par défaut appelle `HOLCampRules::Step` / `Rewards`. Statiques `StepForCamp` / `RewardsForCamp` (repli sur la règle C++ si pas de carte script). USTRUCT/UENUM minimaux du pont : `FHOLCampBoardSettings/Member/Intruder/Step/Hit/Share`, `EHOLCampBoardAggro/Credit/State` ; conversion dans le pont, types C++ existants inchangés. L'état revient dans `FHOLCampBoardStep.State` (un événement ne prend pas de référence en sortie).
- `Source\HeroesOfLiberty\Simulation\HOLCampSubsystem.cpp` : 2 appels remplacés (Step l. 238, Rewards l. 279) par le pont, plus l'include. Rien retiré du C++ (règle, `GetStateName`, `MakeSettings` intacts).
- `Script\Neutrals\HOLCampBoardScript.as` : sous-classe `UHOLCampBoardScript` qui convertit vers les types du script et appelle `HOLCamp::Step` / `HOLCamp::Rewards`.
- `Source\HeroesOfLiberty\Tests\HOLCampBridgeTests.cpp` : `HeroesOfLiberty.Camp.ServerBoard` (aggro au rayon inclus/au-delà, retour de tous les membres, camp vidé une seule fois, partage 60/40, LastBlow, accord pont/règle C++ pour Step et Rewards).
- Commits : `5949e51` (pont + test, SANS script : le test est rouge à l'exécution, `Find` renvoie null) puis `70ebae7` « Camp: server bridge to the script rule » (script = vert attendu).
- Suivi : ligne bascule Camp de `Auto_Journey_File.md`, `AngelscriptInProgress.md` l. 254, `EtapeDefini.md` l. 9. `IN_USE.txt` est ignoré par git (rempli : libre, `CPP: oui`).

## Champs
- BUILDS_DEMANDES: 0 (la session principale compile). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0 (test de pont C++ : Camp.ServerBoard). LIGNES_CPP_SUPPRIMEES: 0. LIGNES_MIGREES_PAR_LOT: 0 (pont seul, la règle script existait déjà). TYPES_A_EXPOSER: les 3 UENUM et 6 USTRUCT ci-dessus (déjà dans le header). PROCESSUS_DETACHES: aucun. EDITS_REFUSES: 0.
- TEST_ROUGE: NON VU (bouchon = commit 5949e51 sans le script ; à lancer après build).

## NON VÉRIFIÉ
- Compilation C++ et UHT (USTRUCT contenant `FVector2D`, constructions agrégées `FHOLCampBoardMember{...}` dans le test : un USTRUCT avec valeurs par défaut reste agrégat en C++20, mais à confirmer).
- Compilation Angelscript : `EHOLCampState(int(State))` (conversion int <-> enum), `FVector2D.X/Y`, méthode `RuleSettings` non-UFUNCTION dans la sous-classe, surcharge BlueprintOverride avec `FVector2D` et `const TArray<struct>&`.
- Les deux commits ne sont pas testés ; le script est la source dès que la carte existe, donc le comportement du serveur dépend du script compilé.

## À lancer (session principale)
1. Build : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Haldir, sur le commit `5949e51` d'abord si on veut voir le rouge (retirer temporairement `Script\Neutrals\HOLCampBoardScript.as`), puis `70ebae7`.
2. `quick_check -Parts tests` : attendre `HeroesOfLiberty.Camp.ServerBoard` vert et les tests Camp existants inchangés.

## Reste [ ]
- Retirer le C++ de la règle (`HOLCampRules.h/.cpp`, ses tests) seulement après le vert ; `GetStateName` (journal) et `MakeSettings` (cvars) restent à trancher (points 1 à 3 de la ligne de la file).
- Aucune question ouverte.
