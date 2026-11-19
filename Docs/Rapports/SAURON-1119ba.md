# SAURON-1119ba : lot C10-1 (commandes console de pointage par fraction d'ecran et sol -> script), NON COMPILE

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `pilots-2` (depuis integ-17 848ce80). Aucun build, aucun quick_check, aucun jeu lance. Branche prete, merge main non fait. Lot absent du `git log` avant (verifie).

NON COMPILE : la liaison C++ et les scripts n'ont jamais ete compiles.

## Commit (anglais, sans Co-Authored-By, non pousse)
| sha | contenu |
|---|---|
| 4d72982 | `Script\Player\HOLControllerCommandsC10a.as` + `_Test.as` ; liaison `HOLControllerCommandLibrary.h/.cpp` (section C10a, +146 lignes) ; retrait des 8 commandes de `HOLPlayerController.cpp` (-259 lignes net du fichier) |

## Commandes migrees (8, script ; meme nom, memes arguments, memes lignes de journal, meme niveau, categorie LogHOLMatch)
`hol.Select.Click`, `ClickUnit`, `Box`, `BoxGround`, `OrderGround`, `OrderLineGround`, `ClickGround`, `OrderNear`.
Lignes reprises au caractere pres : Usage (Warning) de Click, ClickGround, Box, BoxGround, OrderGround, OrderLineGround ; Warning `hol.Select.ClickUnit: no <own|enemy|ally|such> unit N on the screen`, `hol.Select.BoxGround: the rectangle is not wholly on the screen`, `<Cmd>: the point is not on the screen` (OrderGround, ClickGround, OrderNear), `hol.Select.OrderLineGround: a point is not on the screen` ; Display `Click on unit #U at (f, f)` / `Right click on unit ...`, `Box on the ground from ... to ...`, `Right click on the ground at (f, f)[, Shift]`, `Right drag on the ground from ... to ...`, `Click on the ground at (f, f)`, `Right click near the centre at (X, Y)`. `%.3f` devient `Fixed3` (arrondi entier, demi vers le haut), `%.0f` devient `Math::RoundToInt`. Hol.Select.Click et Box n'ont pas de ligne Display (comme avant). Aucune ligne de journal de ces commandes n'est lue par `Tools\` (grep sur `Tools\` : seule la ligne `quick_check.ps1` contient des commentaires, aucun motif de ces textes) ; les scenarios les utilisent comme commandes, pas par `waitlog`.

## Liaison C++ (raison D839 : liaison a exposer ; wrappers, une seule regle)
`UHOLControllerCommandLibrary` (UFUNCTION) : `ProjectGroundFraction`, `SelectAtFraction`, `SelectInBoxFraction`, `OrderAtFraction`, `OrderLineAtFractions`, `UnitScreenPoint`, `PointNearOf` ; `PickBodyHeight` (constexpr) deplace dans la bibliotheque. Non UFUNCTION : `ProjectGroundFractionOf`, `PointNearFor`.
Une seule source : le `ProjectGroundPoint` et le `GetPointNear` du controleur (qui servent aux commandes restees en C++ : LeftClickGround, TestCursorGround, Banner, AttackMove(Near), FocusNear) APPELLENT maintenant la bibliotheque ; la constante `PickBodyHeight` du controleur est celle de la bibliotheque. La projection reste donc en C++ (le script ne connait pas le viewport) : pas de jumeau.

## Retire de HOLPlayerController.cpp
8 `FAutoConsoleCommand` + `HasLineFlag` (n'avait plus d'autre appelant). Reste en C++ comme demande : 3 cvars, TestCursorGround, LeftClickGround, Banner, AttackMove(Near), les 5 rapports, C10-3, `GetLocalControllers`, `ProjectGroundPoint`, `GetPointNear`, `AimAtSource` (C10-2).

## Tests script (4 fonctions, `HOLControllerCommandsC10a_Test.as`)
`Test_ControllerCommandsC10aEachNameOnceAndKnown` (8 noms, uniques, absents de C2b/C9a/C9b), `...Arguments` (Usage exacts, add/double/shift/ctrl/enemy/slot=, drapeau OrderLine seulement apres les 4 nombres, ClickUnit negatif = depuis le dernier), `...Texts` (Fixed3 et chaque ligne de journal), `...BoxOfCorners` (boite de 4 coins non ordonnes + ligne BoxGround). Les valeurs attendues sont ecrites en clair, calculees par le code teste (pas de test tautologique). Limite : la projection sol -> fraction et la regle « hors ecran = Warning » sont en C++ et ne sont pas testees en script (le test script ne voit pas l'ecran, plan 6) : preuve par scenarios.

## Scenarios a rejouer (grep -l `hol.Select.<cmd>` sur `Tools\Scenarios\*.txt`) : 67 scenarios distincts, par commande
Click 14 (deploy, ecran_hud_file, ecran_hud_grille, ecran_hud_groupes...), ClickUnit 27 (arquebusier, arquebusier_gestes, build_walkers, crossing...), Box 1 (ecran_hud_selection_puces), BoxGround 14 (blocked_bodies, build_walkers, crossing, dropoff_click...), OrderGround 16 (building_queue, build_walkers, farms, fog_orders...), OrderLineGround 0 scenario (quick_check « formation » seulement), ClickGround 5 (legions_roster, legions_roster_gesture, legions_roster_gesture_dawn, legions_warrior...), OrderNear 26 (archers, arquebusier_gestes, blocked_bodies, building_attack...). Union : 67 ; 5 noms : archers, arquebusier, arquebusier_gestes, blocked_bodies, building_attack. Preuves du plan : `ecran_hud_selection_puces` (Box), quick_check `Select.ClickUnit`, `Select.OrderGround`, `Select.BoxGround`, partie « formation » (OrderLineGround).

## A lancer par la session principale (je n'ai rien execute)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Sauron` (C++ : liaison + retrait ; risques : `FHOLUnitView`/`HOLUnitView::Gather`/`HOLUnitPicking::GetSide` dans la bibliotheque (include ajoute `Player/HOLUnitPicking.h`), `GetHOLPlayerState` public, `static constexpr double` dans un UCLASS, include de la bibliotheque dans le controleur).
2. Tests script (compilation des `.as` incluse) puis `quick_check -Parts` des parties `Select.ClickUnit`, `Select.OrderGround`, `Select.BoxGround`, `formation`, puis 2 scenarios de chaque commande ci-dessus.
3. Rouge unique : dans `HOLControllerCommandsC10a.as` `Fixed3`, remplir a 2 chiffres au lieu de 3 -> `Test_ControllerCommandsC10aTexts` rouge sur « 0.050 keeps its zeros », puis remettre.

## Champs
- BUILDS_DEMANDES: 1 (lot C++ liaison + retrait)
- TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 4 ; TESTS_TAUTOLOGIQUES: 0
- TEST_ROUGE: NON VU (aucun build autorise)
- LIGNES_CPP_SUPPRIMEES: ~285 dans HOLPlayerController.cpp (-259 net apres les 4 lignes de delegation et l'include) ; LIGNES_CPP_AJOUTEES (liaison) : 146 (bibliotheque .h/.cpp) ; net C++ : ~ -113
- LIGNES_MIGREES_PAR_LOT: ~285 -> 330 lignes script (code + test)
- TYPES_A_EXPOSER: aucun nouveau (AHOLPlayerController, AHOLBuilding deja exposes)
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance

## NON VERIFIE
- Compilation C++ et script (rien lance). Fonctions script supposees : `FString::Mid`, `ToLower`, `StartsWith`, `Math::Abs/Min/Max/RoundToInt`, `HOLControllerCommands::ParseReal` (accepte « -1 » et un nombre decimal ; ClickUnit tronque en `int(...)` la ou le C++ faisait `Atoi`), parametres `double&` / `int32&` en sortie.
- `Fixed3` vs `%.3f` : identiques sauf egalite exacte sur la valeur binaire ; `Math::RoundToInt` vs `%.0f` : idem (demi vers le haut, printf demi pair).
- `ClickUnit` : unite d'identifiant passe en `int32` (le C++ affichait `%u`) : identique tant que l'identifiant < 2^31.
- `OrderLineGround` : le C++ evaluait `From` puis `To` avec court-circuit (To non projete si From echoue) ; le script projette les deux (pas d'effet de bord, meme Warning).
- BoxGround : le C++ accumulait `Box += Fraction` meme hors ecran ; le script de meme (boite inutilisee dans ce cas).
- Les 3 commandes dont le plan dit « seulement citees » ne sont pas concernees. Pas de `git merge main` (DLL perimee).

## Anomalies
- Un premier script PowerShell de retrait a plante (surcharge `Insert` a 3 arguments) APRES avoir ecrit la liaison (.h/.cpp) mais AVANT d'ecrire le controleur ; le retrait a ete refait seul, rien de duplique (diff verifie : 8 commandes retirees, 0 enregistrement restant, accolades equilibrees 1037/1037).
- Aucune ecriture dans `EtapeDefini.md`, `Auto_Journey_File.md`, `IN_USE.txt` (consigne).
