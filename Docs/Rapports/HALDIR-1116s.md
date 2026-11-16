# HALDIR-1116s : bascule Camp, le script est la seule source (D836 point 3)

Branche maps (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`), commit `f9a43b5` « Camp: script is the only source, C++ copy removed ». Lancement 23:54, sans build ni quick_check (consigne). C++ : oui, motif : suppression de la copie C++ de la règle, devenue doublon du script.

## Fait
- `Simulation\HOLCampBoard.cpp/.h` : plus de repli C++. `StepForCamp` sans board : état inchangé, `FStep` vide (rien changé, personne renvoyé, pas de « cleared »), erreur loggée (`Camp: no script board, the camp keeps its state`) ; `RewardsForCamp` sans board : aucune part, erreur loggée (`Camp: no script board, no reward`). Les défauts `CampStep_Implementation` (état rendu tel quel) et `CampRewards_Implementation` (vide) sont inertes et loggent une erreur, comme le pont de la garnison (Glorfindel 0196e7b). `ToRules` (conversion vers la règle C++) retiré ; `ToBoard` et tous les USTRUCT/UENUM du pont gardés.
- `Simulation\HOLCampRules.cpp` : SUPPRIMÉ. `HOLCampRules.h` : réduit aux types utilisés par `HOLCampSubsystem` (`EAggro`, `ECredit`, `EState`, `FSettings`, `FMember`, `FIntruder`, `FStep`, `FHit`, `FShare`) + `GetStateName` devenu inline (journal du sous-système) ; `Step` et `Rewards` retirés. Doc du header dit que la règle est au script seul.
- `Tests\HOLCampRulesTests.cpp` : test `HeroesOfLiberty.Neutrals.CampRules` retiré (et ses aides) ; `Neutrals.Owner` GARDÉ.
- `Tests\HOLCampBridgeTests.cpp` : dans `Camp.ServerBoard`, les 2 assertions « accord pont / règle C++ » remplacées par des valeurs attendues (état Fighting + changé + aucun renvoi ; parts 60/40 par propriétaire) ; test `HeroesOfLiberty.Camp.ServerBoardInert` AJOUTÉ (monde Inactive = pas de board : état gardé, rien changé, aucune récompense ; 2 erreurs attendues via AddExpectedError).
- `Script\Neutrals\HOLCampRules_Test.as` : commentaire de `Test_CampRulesVectors` mis à jour (équivalent du test C++ retiré). Vecteurs vérifiés par Grep : les 18 assertions de `Neutrals.CampRules` y sont (nobody near, beyond aggro, at aggro, leash, all sent back, still returning, at the post, one past the leash, on attack, blow beyond leash, a blow draws it, cleared once, cleared for good, last blow, most damage, shared ascending, no hit no share) ; le partage `shared` est couvert par les assertions « ascending by owner » / « remainder » du même test.
- Grep de chaque symbole supprimé dans `Source\` et `Script\` : `HOLCampRules::Step`, `HOLCampRules::Rewards`, `ToRules`, `FHOLCampRulesTest`, `Neutrals.CampRules` : plus aucun appelant (reste seulement le commentaire du test script, mis à jour).
- Suivi : ligne bascule Camp de `Auto_Journey_File.md` (cochée, résumé ajouté), `AngelscriptInProgress.md` (1 ligne), `EtapeDefini.md` (ligne du pont Camp remplacée), `IN_USE.txt` (libre, `CPP: oui`, ignoré par git).

## REGISTRE
- Tests C++ retirés : 1 — `HeroesOfLiberty.Neutrals.CampRules` -> équivalent script `Test_CampRulesVectors` (HOLCampRules_Test.as). Accords script/C++ du camp retirés : les 2 assertions d'accord de `Camp.ServerBoard` (« agrees with the C++ rule » pour Step et Rewards), remplacées par des valeurs ; la liste des autres accords : aucun (`Test_CampOwner` n'est pas un accord, gardé).
- Tests C++ ajoutés : 1 — `HeroesOfLiberty.Camp.ServerBoardInert`. Total quick_check attendu : inchangé (-1 +1), jamais en baisse.
- Lignes C++ retirées (git numstat) : 330 supprimées / 73 ajoutées, net -257. Détail : `HOLCampRules.cpp` -155 ; `HOLCampBoard.cpp` -45/+11 ; `HOLCampBoard.h` -5/+5 (commentaires) ; `HOLCampRules.h` -14/+17 ; `HOLCampSubsystem.h` -2/+2 (commentaire) ; `HOLCampRulesTests.cpp` -95/+1 ; `HOLCampBridgeTests.cpp` -14/+37.
- GARDÉ et listé (le sous-système les utilise) : types de `HOLCampRules.h`, `GetStateName` (inline), `MakeSettings` (cvars `hol.Camp.*`, `HOLCampSubsystem.cpp` l. 66), `HOLCamps::IsPassive/NoteHit` (HOLCombatProcessor), `Neutrals.Owner`. Les types USTRUCT/UENUM du pont gardés. Piste pour plus tard : faire utiliser par le sous-système les types du pont directement (`FHOLCampBoard*`) pour supprimer aussi `HOLCampRules.h` ; non fait ici (conversion sans build trop risquée).

## Champs
- BUILDS_DEMANDES: 0 (la session principale compile). TESTS_CPP_RETIRES: 1 (Neutrals.CampRules) + 2 assertions d'accord. TESTS_SCRIPT_AJOUTES: 0. LIGNES_CPP_SUPPRIMEES: 330 brutes, net 257. LIGNES_MIGREES_PAR_LOT: 0 (la règle script existait). TYPES_A_EXPOSER: aucun nouveau. PROCESSUS_DETACHES: aucun lancé. EDITS_REFUSES: 0 (un script d'édition a d'abord échoué : l'alias `rd` de PowerShell masquait ma fonction ; `HOLCampRules.h` restauré par `git checkout`, rien perdu).
- TEST_ROUGE: NON VU (rien compilé ; le rouge de Camp.ServerBoard est celui du bouchon 5949e51, HALDIR-1116r).

## NON VÉRIFIÉ
- Compilation C++ (UHT, `AddExpectedError` avec `Contains` et 1 occurrence, `UWorld::CreateWorld(EWorldType::Inactive)` sans InitializeActorsForPlay : `Find` doit rendre null car `DoesSupportWorldType` refuse ce type ; si le test Inert échoue, corriger le test, pas le pont). Le comportement du serveur dépend du script compilé (`HOLCampBoardScript.as`).
- Warnings possibles de paramètres inutilisés dans les défauts inertes (pas des erreurs en UE par défaut).

## À lancer (session principale)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Haldir (branche maps, `f9a43b5`).
2. `quick_check -Parts tests` : `Camp.ServerBoard` vert, `Camp.ServerBoardInert` vert, `Neutrals.Owner` vert, `Test_CampRulesVectors` / `Test_CampOwner` verts, total inchangé.
3. Branche prête, non fusionnée dans main ; `Content\Maps\Crown.umap` non touché.
