# GALADRIEL-1119ac : retrait des replis C++ des planches (construction, production)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `boards-nofallback` (depuis `net-playtest`). NON COMPILE, aucun build, aucun jeu, aucun quick_check. Branche prete, pas de push, pas de merge.

## Commits
- `01293de` construction : `Source\HeroesOfLiberty\Buildings\HOLConstructionSubsystem.cpp` (48 ajoutees / 59 supprimees, net -11 ; `git diff -w` 46/57)
- `9083320` production : `Source\HeroesOfLiberty\Buildings\HOLProductionSubsystem.cpp` (41 ajoutees / 27 supprimees, net +14 : l'aide `LogBoardAbsentOnce` de 15 lignes et les blocs de refus remplacent les copies)

LIGNES_CPP_SUPPRIMEES : 86 lignes brutes (59 + 27), dont environ 60 de copie de decision ; net du lot : -11 + 14 = +3 (les refus et logs d'erreur ajoutent des lignes). BUILDS_DEMANDES: 0.

## Construction (`HOLConstructionSubsystem.cpp`)
- `Place` : le bloc `if (Verdict < 0)` (chaine IsBuildableBy / IsBuildingOpenTo / IsAgeOpen / CheckPlacement / TryPay, 41 lignes) est supprime. Verdict -1 (classe absente ou reponse hors plage) : rien pose, rien debite, `return`, erreur `Construction: Place refused, script board absent` une fois (static bool).
- `Cancel` : repli `CanCancel` et `GetCancelRefund` supprimes. -1 : erreur une fois `Construction: Cancel refused, script board absent`, puis verdict 1 (ignore) : la ligne de journal « cancel ... ignored » reste, aucun remboursement, rien detruit.
- `RefundProductionQueue` : porte et somme C++ supprimees (la boucle `HOLProduction::Cancel` qui vide la file reste, c'est l'effet ; `AskQueueTotal` est maintenant appele sans condition). -1 : `Construction: RefundProductionQueue refused, script board absent` une fois, retour sans rien rembourser ni vider.
- `DestroyBuilding` (EXCEPTION jugee) : -1 -> le batiment est QUAND MEME detruit (sinon un batiment mort reste debout) ; seuls les garde-fous du moteur restent (`!World || !HasAuthority || !IsValid` -> return, car `*World` est dereference plus bas). Erreur une fois : `Construction: Destroy kept without rules, script board absent (no refund, no rubble)` (texte different du gabarit car il ne refuse pas). Sans planche : pas de remboursement du chantier (`AskCancelRefund` rend vide), pas de decombres (`AskRubble` -1, test devenu `RubbleVerdict > 0`), et `RefundProductionQueue` ne rembourse pas (sa propre erreur). Une reponse hors plage de `AskRubble` sur une planche presente = plus de decombres (avant : repli `!IsFoundation && Type`).
- Intacts : `DamageBuilding`, `GetCost`, les lignes de journal, l'ordre des effets.

## Production (`HOLProductionSubsystem.cpp`, `ApplyOrders`)
- Aide anonyme `LogBoardAbsentOnce(Action)` (TSet<FString> statique) : `Production: <Action> refused, script board absent`, une fois par action.
- Porte d'ordre (-1) : `continue` (ordre ignore), action `ApplyOrders`. Copie `Gate = ? :` supprimee.
- Issue du ralliement (-1) : `continue`, action `ApplyOrders rally`. Copie de 2 lignes supprimee.
- Site du ralliement (`AskRallySite` -1 avec un site) : site abandonne (le ralliement va au sol), action `ApplyOrders rally site`. Copie supprimee.
- Verdict de mise en file (-1) : `continue`, action `ApplyOrders queue`. Copie de 6 lignes (techno / siege / tier / faction / file pleine / paiement) supprimee. Le `TryEnqueue` d'effet reste (interdit, et il reverifie).
- Annulation (-1) : `continue`, action `ApplyOrders cancel`. Branche `else if (!HOLProduction::Cancel(...))` supprimee (le chemin script `AskStockAfterCancel` + `RemoveAt` est le seul ; garde-fou `IsValidIndex` garde).
- Intacts : `AdvanceQueues`, `GetCost`, naissance, porte de population, `TryEnqueue`, `Cancel` (effets, autres appelants).

## Replis gardes
Aucune regle ne reste seulement dans un repli que j'aie vu : tous les `?:` et `if` de repli etaient des jumeaux de verdicts que les planches rendent. `HOLConstruction::CanCancel / GetCancelRefund / IsAgeOpen / TryPay` ne sont plus appeles par les deux Subsystem mais restent utilises par `HOLConstructionReadLibrary` (liaison vers le script), `HOLConstructionRules.cpp` et des tests.

## Tests C++ a surveiller (non supprimes, non lances)
Tout test C++ qui pilote `Place`, `Cancel`, `ApplyOrders` dans un monde SANS planche script compilee refuse maintenant et logge une erreur (une erreur `UE_LOG` peut faire echouer un test automation). A relire apres build : `Tools`/`Tests` : `HOLAIBuildOrdersTests.cpp`, `HOLDisconnectTests.cpp`, `HOLTestWorldLibrary.cpp` (grep `Place(`, `ApplyOrders`) ; les tests de construction et de production C++ qui tombent deja en faussant le script prouvent qu'ils passent par la planche (donc monde avec planche). Les tests script des planches sont inchanges.
TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0.

## NON VERIFIE
Rien compile ni execute. A lancer par la session principale : build de `boards-nofallback`, puis les tests C++ (construction / production / `HOLAIBuild*` / `HOLDisconnect*`), les 38 cas de construction en jeu et la pose / annulation par le reseau.

## Notes
- « Une fois par partie » = une fois par processus (variables statiques) ; en editeur, une seconde partie ne relogge pas.
- `edit_cons.ps1` / `edit_prod.ps1` (scratchpad) ont servi aux remplacements par plage de lignes verifiees par debut / fin de ligne ; `git diff --stat` et `git diff -w --stat` relus (ecart de 2 lignes en construction : espaces seuls).
- PROCESSUS_DETACHES: aucun lance (stray_processes non execute). EDITS_REFUSES: 0. Pas de fichier de suivi touche (EtapeDefini.md, Auto_Journey_File.md laisses).
