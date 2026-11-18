# ELROND-1118s : NG-3 corrige (ecran Nouvelle partie, script)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers, commit `44dce75` (2 fichiers : `Script\UI\HOLNewGameScreen.as`, `HOLNewGameScreen_Test.as`). Pas de push, pas de fusion. Aucun C++.

## Etat de verification
- Compilation script : 4 essais sur 6 faits. Au dernier essai compile (21:07) il ne restait comme erreur de MES fichiers que le `CreateWidget` du test ; la correction finale (`WidgetBlueprint::CreateWidget(Wanted, Controller)`) a COMPILE dans une sonde (aucune erreur sur cette ligne a 21:07), mais le module complet n'a PAS ete relance apres l'avoir mise dans `NGS_CreateScreen` et retire la sonde.
- Les 2 derniers lancements de `quick_check.ps1` ont ete REFUSES par le classifieur d'autorisation (auto mode) : aucun compte de tests, aucun rouge vu. NON VERIFIE : les 3 tests (Test_NewGameScriptClass, Test_NewGameWidgets, Test_NewGameSlotCount) n'ont jamais tourne.
- Les 3 fichiers Stats ont ete supprimes du disque pour les essais puis restaures par `git checkout` avant le commit (`git status` propre hors mes 2 fichiers).

## Erreur -> correction (ce qui n'existe pas -> ce qu'on ecrit)
| Erreur | Correction |
|---|---|
| `FText::AsNumber(int)` n'existe pas | inutile ici : `UHOLNewGameScreen::GetNewGamePlayerName(SlotIndex)` (base). Sinon `FText::FromString("" + n)` (f-string `f"{n}"` aussi) |
| `FText::GetEmpty()` n'existe pas | `FText()` (usage du projet) |
| NSLOCTEXT / chaines en dur | tous remplaces par `UHOLNewGameScreen::GetNewGameText(n"Cle")` (22 appels, memes cles que les LOCTEXT d'origine) |
| `Widget::CreateWidget(APlayerController, TSubclassOf)` (aussi `(Class, PC)`, `(PC, Class)`, `(PC, Class, PC)`) : aucune signature | `WidgetBlueprint::CreateWidget(TSubclassOf<UUserWidget> Wanted, APlayerController Controller)` : le namespace est `WidgetBlueprint::`, pas `Widget::`, ordre (classe, controleur) ; il faut une variable `TSubclassOf<UUserWidget>` (copie de `TSubclassOf<UHOLNewGameScreen>`) puis `Cast<UHOLNewGameScreen>(...)`. Seule la ligne `WidgetBlueprint::CreateWidget(Wanted, Controller)` n'a donne aucune erreur dans la sonde |
| `UHOLNewGameScreen::GetWidgetFromName(FName)` n'existe pas (ni `WidgetTree`, ni `UWidgetBlueprintLibrary::GetWidgetFromName`) | `Screen.GetAllWidgets(TArray<UWidget>& Out)` (liaison de `UUserWidget`, Bind_UUserWidget.cpp) puis boucle sur `Out[i].GetName() == Name` (helper `NGS_Find` du test) ; compile |

## Tests : jamais verts a vide
`NGS_CreateScreen(T)` fait `T.AssertTrue(false, ...)` si la classe script manque, s'il n'y a pas de controleur de joueur (`Gameplay::GetPlayerController(0)`), ou si la creation echoue ; les deux tests de widgets sortent alors en echec, pas en silence. Risque : sans monde de test, `GetPlayerController(0)` peut etre nul et les 2 tests seront ROUGES (a voir au premier lancement ; ce serait le vrai constat, pas un bug du test).

## Manques de la base C++ (A AJOUTER A LA BASE C++)
- Aucun : `GetAllWidgets` suffit pour retrouver par nom. Si un controleur de joueur n'existe pas en test, prevoir `static APlayerController UHOLNewGameScreen::GetTestController()` ou un test dans un monde ; a juger apres le premier lancement.

## Reste / commande
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Tests\quick_check.ps1 -Parts tests` puis `Select-String ... quick_tests.log -Pattern "Angelscript: Error"`. Pour voir un rouge : changer `8` en `9` dans `T.AssertEquals(Lines, 8, ...)` de `Test_NewGameSlotCount`, relancer, remettre. Pas de geste ajoute (`bScriptOwnsGestures` : lot suivant). Placeholders.txt non touche (aucun provisoire ajoute ou retire).
