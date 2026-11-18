# GLORFINDEL-1116r : bascule Garrison, la copie C++ de la regle est supprimee

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, branche net-playtest, commit 0196e7b "Garrison: script is the only source, C++ copy removed" (parent 38ab5f4 + pont 75fe739). NON COMPILE (pas de build, pas de quick_check, consigne).

## Fait
- Pont (HOLGarrisonBoard.h/.cpp) : plus de repli C++. Sans board script, AdmittedForOrder renvoie une liste vide (le serveur REFUSE l'entree, R7) avec un log Warning ; l'implementation par defaut de GarrisonAdmitted renvoie aussi vide.
- Fichiers supprimes (3) : Source\HeroesOfLiberty\Buildings\HOLGarrisonRuleLibrary.h, HOLGarrisonRuleLibrary.cpp, Script\Tests\HOLGarrisonParity_Test.as.
- HOLGarrisonRules.h : 80 -> 30 lignes, garde EKind + KindOf (utilises par HOLGarrisonSubsystem.cpp). Retires : KindMayEnter, Admitted, IsSettingOn, AdmittedBySettings, include HAL/IConsoleManager.h.
- Test Garrison.ServerBoard : retire l'include de la library et la comparaison avec la regle C++ (valeurs deja ecrites a la main).
- Commentaires mis a jour (HOLGarrisonSubsystem.h/.cpp, HOLCommandSubsystem.h:38, HOLPlayerOrders.h:86, HOLGarrisonRules.as).
- Grep Source\ et Script\ des symboles supprimes (AdmittedBySettings, GarrisonRuleLibrary, HOLGarrisonRule::, KindMayEnter, IsSettingOn, HOLGarrisonParity) : 0 appelant restant (les KindMayEnter/IsSettingOn de Script\Garrison\HOLGarrisonRules.as, AI\HOLAIGarrisonRules.as et HOLGarrisonRules_Test.as sont les fonctions du script, pas du C++).
- Les 8 tests C++ Garrison.* (Orders, OffField, SeparationAndHeal, ExitPoint, Ejection, Shelter, RightClick, Net.Garrisoned) n'utilisaient aucun symbole supprime (Grep) : intacts. Reglages hol.Garrison.* : declares par le script (HOLGarrisonRules.as), rien a redeclarer.

## REGISTRE des tests retires : 2
1. Test_GarrisonScriptAndCppAdmitTheSame (400 cas aleatoires + 8960 cas exhaustifs script contre C++) : supprime -> ce qui couvre : Test_Garrison... de Script\Tests\HOLGarrisonRules_Test.as (la regle du script, valeurs ecrites a la main) + Garrison.ServerBoard (la chaine serveur -> board -> script) + les 8 tests C++ Garrison.*.
2. Test_GarrisonScriptAndCppDefaultsAgree (defauts des trois reglages, script contre C++) : supprime -> meme couverture ; les defauts hol.Garrison.* sont verifies par HOLGarrisonRules_Test.as.
Le total quick_check baisse de 2 par ce retrait voulu (la copie C++ n'existe plus). Limite : le test C++ ne verifie plus l'egalite script/C++ (plus de C++ a comparer) ; le rouge du pont (stub) reste a faire.

## Chiffres
- LIGNES_CPP_SUPPRIMEES : 61 (RuleLibrary .h+.cpp) + 50 (Rules.h) + ~12 nets (Board repli/defaut, bridge test) ; git diff : C++ 0 ajout/-33 -28 -55 ... total commit = 22 insertions, 291 deletions (dont 152 de script).
- BUILDS_DEMANDES : 0. TESTS_CPP_RETIRES : 0 test C++ (la comparaison de Garrison.ServerBoard retiree). TESTS_SCRIPT_RETIRES : 2 (ci-dessus). TESTS_SCRIPT_AJOUTES : 0.
- TYPES_A_EXPOSER : aucun.

## NON VERIFIE (a faire par la session principale)
- Compilation C++ (HOLGarrisonBoard.cpp utilise UE_LOG(LogTemp) : verifier que le fichier compile avec CoreMinimal ; HOLGarrisonRules.h n'inclut plus HAL/IConsoleManager.h : un autre fichier qui s'appuyait dessus par ricochet pourrait casser).
- Commande : <worktree>\Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts (Garrison.*, script). Rouge par bouchon du pont (GarrisonAdmitted du script -> return TArray<int>();) : Garrison.ServerBoard doit rougir, puis vert.
- Merge main non fait.
- Si le build montre un appelant oublie : la liste ci-dessus est exhaustive pour Source\ et Script\ au grep.
