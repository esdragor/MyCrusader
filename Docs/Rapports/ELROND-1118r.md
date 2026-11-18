# ELROND-1118r : lot NG-3 (mise en page de l'écran Nouvelle partie en script)

**NON COMPILÉ, NON LANCÉ** (aucun build, aucun quick_check, par consigne). Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers, commit `c3a3506` (2 fichiers neufs, rien dans `Source\`). Branche prête, pas de merge, pas de push.

Champs : BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_NewGameScriptClass, Test_NewGameWidgets, Test_NewGameSlotCount) ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: aucun de plus ; PROCESSUS_DETACHES: aucun lancé ; TEST_ROUGE: non vu (rien lancé).

Commande à lancer par la session principale (après le build groupé qui compile NG-1a) : `quick_check -Parts tests` (ou le lanceur de tests script habituel), puis lire les 3 tests.

## Fichiers
- `Script\UI\HOLNewGameScreen.as` : `class UHOLNewGameScreenScript : UHOLNewGameScreen`, surcharge `BP_BuildNewGame`. Construction seule, aucun événement abonné.
- `Script\UI\HOLNewGameScreen_Test.as` : 3 tests + aide `NGS_CreateScreen`.

## Widgets créés (noms vérifiés dans HOLNewGameScreen.cpp 224-345)
Heading ; ligne carte : MapDropdown, FogDropdown ; ligne taille : SizeDropdown, TeamsTogetherDropdown ; en-têtes HeadName, HeadSlot, HeadLevel ; pour n = 0..7 : SlotName<n>, SlotLevel<n>, SlotFaction<n>, SlotSwatch<n>, SlotColor<n>, SlotTeam<n>, et SlotYou0 (n = 0) ou SlotState<n> (n = 1..7) ; ModeDropdown, NomadDropdown, RegicideDropdown, CatastropheDropdown, SacredSitesDropdown ; SpectatorsText (masqué), Note, ErrorText (masqué), BackButton, ReadyButton (masqué), SpectateButton (masqué), LaunchButton. Même ordre et mêmes largeurs que le C++ (largeurs lues par `GetNewGameWidth`, listes par `GetNewGameOptions`, cartes par `GetMapNames()`).

## ÉCARTS AVEC L'ÉNONCÉ (vérifiés dans le C++, à lire)
1. **`NoteText` n'est pas un nom de widget** : le C++ crée la note sous le nom `Note` (`NoteText` est le membre). Le script crée `Note`. Un scénario qui vise `NoteText` ne trouverait rien aujourd'hui non plus.
2. **`SlotState0` n'existe pas dans le C++** : la ligne 0 est « Vous » (`SlotYou0`), seules `SlotState1..7` existent (cpp:303-309). Le script fait pareil. `Test_NewGameSlotCount` vérifie donc `SlotName0..7` (8 lignes), `SlotState1..7` et `SlotYou0`.
3. **Le rapport ARWEN-1118p est faux sur un point important** : il dit que `NativeOnInitialized` retrouve par nom `MapDropdown`... `LaunchButton`, `NoteText`, `ErrorText`, `SpectatorsText`, `Heading`. Faux : dans le cpp seuls `SlotYou0`, `HeadName/HeadSlot/HeadLevel` et les 7 familles `Slot*` sont retrouvés par `FindWidget` (cpp:360-372). Tous les autres membres (`MapDropdown`, `SizeDropdown`, `TeamsTogetherDropdown`, `FogDropdown`, `ModeDropdown`, `NomadDropdown`, `RegicideDropdown`, `CatastropheDropdown`, `SacredSitesDropdown`, `LaunchButton`, `BackButton`, `ReadyButton`, `SpectateButton`, `SpectatorsText`, `NoteText`, `ErrorText`, `HeadingText`) ne sont affectés QUE dans `BuildLayout` C++ (cpp:237-345). Avec un écran script ils resteront nuls : `Refresh`, les abonnements `HandleMapChosen` (cpp:418) et le bouton Lancer ne marcheront pas.

## À AJOUTER À LA BASE C++ (bloquant pour les lots suivants)
- Dans `NativeOnInitialized` (après `Super::` donc après `BP_BuildNewGame`), retrouver par nom, comme les `Slot*`, tous les membres ci-dessus : `X = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("MapDropdown")))` etc., avec `Note` pour `NoteText` et `Heading` pour `HeadingText`. À faire seulement quand `GetClass() != StaticClass()` pour ne pas changer l'écran C++.
- Couleur de l'erreur : `ErrorText->SetColorAndOpacity(HOLUIStyle::Missing)` (cpp:336) : la couleur n'est pas exposée au script. Souhaité : l'aide `AddNewGameText` pose cette couleur pour le style 2 nommé `ErrorText`, ou `static FLinearColor GetNewGameMissingColor()`. Marqué `// MANQUE:` dans le script ; l'erreur s'affichera sans cette couleur.
- Textes : les libellés sont recréés en `NSLOCTEXT("HOLNewGameScreen", <clé>, <texte>)` avec les clés et textes du `LOCTEXT` du cpp (même espace, même clé, même source : pas de conflit, même entrée `fill_po.py`). Option plus propre : `static FText GetNewGameText(FName Key)` côté C++ pour une seule source.
- Pas d'accès aux widgets du script par membre (les `UPROPERTY` protégés ne sont pas lisibles) : les lots suivants retrouveront les widgets par `GetWidgetFromName`.

## Mutations pour voir chaque test rouge
- Test_NewGameScriptClass : renommer la classe en `UHOLNewGameScreenScriptX` (le nom attendu devient faux), ou supprimer le fichier .as.
- Test_NewGameWidgets : renommer `n"FogDropdown"` en `n"FogDropdownX"` dans `BP_BuildNewGame` (ou retirer la ligne `AddNewGameDropdown(Options, n"SacredSitesDropdown", ...)`).
- Test_NewGameSlotCount : changer `const int SlotCount = 8;` en `7` (SlotName7 et SlotState7 disparaissent).
Aucune n'a été jouée.

## Suspects de compilation / d'exécution
1. `NGS_CreateScreen` : `Gameplay::GetPlayerController(0)` + `Widget::CreateWidget(Controller, Found)` : sans monde de jeu dans la passe de tests, le contrôleur est nul et `Test_NewGameWidgets` / `Test_NewGameSlotCount` RETOURNENT SANS RIEN VÉRIFIER (vert vide, précédent : le test Réglages sans objet de réglages). À constater au premier lancement ; si vide, il faut un scénario (hol.UI) ou un monde de test. Aucun test du dépôt ne crée de widget : pas de patron.
2. `Screen.GetWidgetFromName(FName)` : peut ne pas être exposé à Angelscript (`UUserWidget::GetWidgetFromName` n'est pas une UFUNCTION). Sinon : aide C++ `UWidget* FindNewGameWidget(FName)` (BlueprintCallable) sur la base.
3. `FText::Format(NSLOCTEXT(...), FText::AsNumber(n))` pour « Joueur {0} » : surcharge à confirmer.
4. `const int SlotCount = 8;` membre const de classe script et `int(EHOLAIDifficulty::Normal)` (Normal = index 1 de la liste Level : Facile, Normal, Difficile, Évolutive).
5. `NAME_None` comme nom de cellule de texte sans nom (le C++ passe `NAME_None`).
6. Un texte « Joueur » apparaît deux fois (HeadName et HeadSlot) avec la même clé NSLOCTEXT et la même source, comme le C++.
7. Classe script sans la bascule NG-6 : `Open` garde `StaticClass()`, donc cet écran ne s'ouvre pas encore en jeu ; seul `FindScriptClass` le voit.
