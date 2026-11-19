# GALADRIEL-1119y : lot K1b (RefundProductionQueue et DestroyBuilding decidés par le script, repli C++ gardé)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab` (base bbc11ce). TOUT NON COMPILE : aucun build, aucun quick_check, aucun jeu. Lancé 12:16, rapport ~12:27.

## EN TETE : chemin Mass
Grep des appelants (hors tests) :
- `RefundProductionQueue` : `Match\HOLGameState.cpp:400` (chute du forum, HandleForumFall) et `DestroyBuilding`.
- `DestroyBuilding` : `Match\HOLGameState.cpp:1214` (boucle de chute du forum) et `DamageBuilding` ligne 612 (a 0 de vie).
- `DamageBuilding` est appelé par le processeur de combat (`HOLCombatProcessor.cpp:1364`, `:2528`), à chaque coup. DamageBuilding reste INTOUCHE (aucun pont par coup). Mais `DestroyBuilding` est atteint par ce chemin UNE FOIS par mort de bâtiment (pas par coup, pas par pas), et il fait déjà SpawnActorDeferred / Destroy (thread de jeu : le processeur déclare `bRequiresGameThreadExecution = true`, `HOLCombatProcessor.cpp:252`). J'ai donc branché le pont sur DestroyBuilding et RefundProductionQueue (coût : 3 appels script par bâtiment détruit). Si tu juges qu'un appel script depuis un processeur Mass reste exclu, le retrait tient en 3 lignes (les `Ask*` de `DestroyBuilding` : renvoyer -1 = repli C++).

## Commits (anglais, sans Co-Authored-By, pas de push, pas de merge)
- TEST   cf98333 : `Script\Tests\HOLConstruction_Test.as` + `Test_Construction_RefundDestroyVerdict` (fin de fichier, CRLF respecté)
- SCRIPT 6f83a38 : `Script\Buildings\HOLConstructionBoard.as` (3 enums, 4 fonctions de namespace, 4 overrides de la classe)
- C++    0b5bf30 : `Buildings\HOLConstructionBoard.h/.cpp` (+4 évènements) et `HOLConstructionSubsystem.cpp` (RefundProductionQueue, DestroyBuilding) : +84 / -9 au total

## Décision / effet
Script décide (entiers, enum script) :
- `QueueRefundVerdict(bHasWorld, bAuthority, QueueLength)` : 0 RefundQueue, 1 NothingToRefund (gate d'origine : pas de monde, pas d'autorité, file vide).
- `QueueRefund(TArray<FHOLResources> ItemCosts)` : somme des coûts (un par item, ordre de file, tête comprise). Somme champ par champ car l'opérateur `+=` de FHOLResources n'est pas vérifié exposé ; ce n'est pas une règle recopiée (HOLProduction::Cancel rend le coût plein de l'item, il n'a pas d'autre formule).
- `DestroyVerdict(bHasWorld, bAuthority, bValid)` : 0 DestroyIt, 1 LeaveIt (gate d'origine).
- Part du chantier du site : `CancelRefund` EXISTANT (K1a, `HOLConstructionRead::GetCancelRefund`) réutilisé tel quel via `AskCancelRefund` : aucune règle recopiée.
- `RubbleVerdict(bFoundation, bHasType)` : 0 NoRubble, 1 LeaveRubble (D345.2a : ni pour une fondation nue ni sans ligne de type).
Reste en C++, même ordre : lecture de la file, `HOLProduction::Cancel` qui vide la file, `NotifyProductionChanged`, `AddSlotStock` (x2), ligne de journal "Building destroyed: ..." (mots identiques, texte `finished/foundation/under construction` et "no stock" lus en C++), SpawnActorDeferred du gravât, `RecordBuildingLost`, `ReleaseLinkedSource`, `EjectAll`, `ReleaseFootprint`, `Destroy`.
Repli : `AskQueueRefund / AskDestroy / AskRubble` = -1 (pas de planche, ou `_Implementation` par défaut) -> les gates et la somme C++ d'origine. `AskQueueTotal` rend false sans planche (le C++ garde sa somme). Réponse != 0 du script = ne rien faire (idem gate d'origine). Détail : dans RefundProductionQueue la file est vidée par la boucle C++ qu'une planche réponde ou non (effet), la somme du script REMPLACE seulement `Refund` ensuite (donc AddSlotStock reçoit la valeur du script).

## Champs du brief commun
- BUILDS_DEMANDES: 1 (lot C++ K1b, à lancer par la session principale ; fusionnable avec le build K1a déjà prouvé : seul le delta K1b est neuf)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 1 (Test_Construction_RefundDestroyVerdict : 16 assertions : file x4 verdicts + somme x2, destroy x4, rubble x3, site refund x2)
- LIGNES_CPP_SUPPRIMEES: 0 (repli gardé)
- LIGNES_MIGREES_PAR_LOT: ~45 l. de portes de RefundProductionQueue + DestroyBuilding ont une source script (copie C++ gardée en repli)
- TEST_ROUGE: NON VU (rien compilé). Valeur à fausser : dans `RubbleVerdict`, inverser `bFoundation` (foundation -> LeaveRubble : 2 assertions rouges) ; ou dans `QueueRefund` boucler jusqu'à `Num() - 1` (la somme donne 100/0/0/20 au lieu de 100/10/0/20 : l'assertion "sum of the costs" rouge) ; ou dans `DestroyVerdict` retirer `!bAuthority`.
- TESTS_TAUTOLOGIQUES: 0 (le site refund passe par la vraie GetCancelRefund exposée)
- EDITS_REFUSES: 0 (un premier script python a échoué sur le chemin /tmp, rien écrit ; relancé via le scratchpad)
- PROCESSUS_DETACHES: non lancé (aucun process démarré par moi)
- TYPES_A_EXPOSER: aucun nouveau. Non migré : `HOLProduction::Cancel` du TArray<FHOLProductionItem> (FHOLProductionItem non exposé, d'où les coûts passés en TArray<FHOLResources>) ; `DamageBuilding` (hors lot, chemin Mass par coup) ; HandleForumFall.

## Risques à la compilation
1. UHT : `const TArray<FHOLResources>&` en paramètre d'un BlueprintNativeEvent ; côté script, l'override `QueueTotalAnswer(TArray<FHOLResources> ItemCosts)` : si la liaison veut `const TArray<...>&`, adapter la signature.
2. Noms d'enum script `RefundQueue / NothingToRefund / DestroyIt / LeaveIt / NoRubble / LeaveRubble` : collision possible avec un nom global existant (aucune vue).
3. `Total.Food += ...` sur propriété de struct dans une boucle script : à vérifier (écriture de champ de struct dans un TArray élément lue seulement, Total est local).
4. Ordre du journal : inchangé ; la ligne "Building destroyed: ... refund %s" reçoit Refund = file (valeur du script si planche) + site refund (valeur du script).

## Preuves à rejouer (session principale, un build groupé avec K1a déjà prouvé)
1. Build du worktree. Rouge VU une fois (une des valeurs ci-dessus) puis vert de `HOLConstruction_Test.as` (le nouveau puis tout le fichier).
2. `quick_check -Parts tests` : `HOLBuildingTests.cpp` (DamageBuilding 554, 563, 792-814, 918 ; DestroyBuilding 808), `HOLDisconnectTests.cpp:224-225`, `HOLVictoryTests.cpp` (forum, 356-1275), `HOLGarrisonOrderTests.cpp:675` doivent rester verts (les tests tournent avec ou sans planche selon le monde de test : si -1, repli C++).
3. `quick_check -Parts build` (annulation avec remboursement, bâtiment détruit) et `-Parts net` ; chute de forum (HOLVictory) avec file de production remplie : ligne de journal "Victory: forum of slot N fell ... (queue refunded ...)" et "Building destroyed: ... queue of N refunded, refund ..." identiques avec et sans planche.
4. Test d'accord : même scénario avec la planche et sans (repli) -> mêmes lignes ; ensuite seulement retrait des gates C++ et du test C++ d'accord (D839).
5. Merge main : non fait (DLL périmée, la session principale synchronise).
