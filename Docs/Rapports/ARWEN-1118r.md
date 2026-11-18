# ARWEN-1118r : bibliothèque de test d'écran (NON COMPILÉ)

NON COMPILÉ. Aucun build, aucun quick_check, aucun lancement. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` (branche d751-root).

## Fichiers neufs (module HeroesOfLibertyUI, aucun fichier existant modifié, .Build.cs inchangé)
- `Source\HeroesOfLibertyUI\Tests\HOLScreenTestLibrary.h`
- `Source\HeroesOfLibertyUI\Tests\HOLScreenTestLibrary.cpp`

Classe `UHOLScreenTestLibrary : UBlueprintFunctionLibrary` (UCLASS toujours déclarée, comme les autres `*ReadLibrary` du module). Les corps de `CreateScreenForTest` et `EndScreenTest` sont sous `#if WITH_DEV_AUTOMATION_TESTS` (hors test : nullptr / rien). Les statiques de l'état (monde, joueur local, écran) sont des `TStrongObjectPtr` dans le namespace `HOLScreenTestLibNs`, noms préfixés `HOLScreenTestLib*` (unity). Dépendances : toutes déjà dans le module (UMG, Engine, HeroesOfLiberty n'est pas nécessaire ici).

## Signatures Angelscript (namespace déduit du nom de classe, comme `HOLGameNamesRead`)
```
UUserWidget HOLScreenTest::CreateScreenForTest(TSubclassOf<UUserWidget> ScreenClass)
void        HOLScreenTest::EndScreenTest()
void        HOLScreenTest::DestroyScreenForTest(UUserWidget Screen)     // = EndScreenTest()
UWidget     HOLScreenTest::FindWidgetByName(UUserWidget Screen, FName Name)
bool        HOLScreenTest::IsWidgetEnabled(UWidget Widget)
FText       HOLScreenTest::GetDropdownText(UWidget Widget)               // UHOLDropdown::GetShownText, vide sinon
int         HOLScreenTest::GetDropdownIndex(UWidget Widget)              // UHOLDropdown::GetSelectedIndex, -1 sinon
```
Comportement : `CreateScreenForTest` appelle d'abord `EndScreenTest()` (un seul montage à la fois), puis monde de jeu + contexte + ULocalPlayer + APlayerController (copie de HOLNewGameSitesBoxTests.cpp:39-49), `NewObject<UUserWidget>(GetTransientPackage(), Class)`, `SetPlayerContext`, `Initialize()` (NativeOnInitialized joué, mise en page construite). Chaque échec : `UE_LOG(LogHOLScreenTest, Error, ...)`, nettoyage, nullptr.

## Emploi (remplace NGS_CreateScreen de HOLNewGameScreen_Test.as)
```
UHOLNewGameScreen NGS_CreateScreen(FUnitTest& T)
{
	TSubclassOf<UHOLNewGameScreen> Found = UHOLNewGameScreen::FindScriptClass();
	if (!Found.IsValid()) { T.AssertTrue(false, "the New game screen has a script class to create"); return nullptr; }
	TSubclassOf<UUserWidget> Wanted = Found;
	UHOLNewGameScreen Screen = Cast<UHOLNewGameScreen>(HOLScreenTest::CreateScreenForTest(Wanted));
	if (Screen == nullptr) T.AssertTrue(false, "the New game screen is created");
	return Screen;
}
// NGS_Find(Screen, Name) : return HOLScreenTest::FindWidgetByName(Screen, Name);
// En fin de chaque test qui a créé l'écran : HOLScreenTest::EndScreenTest();
```
Chaque test qui appelle NGS_CreateScreen doit appeler `HOLScreenTest::EndScreenTest()` avant ses `return` (ou au début du suivant : le suivant nettoie de toute façon).

## Port du test C++ « case Sites sacrés » (HOLNewGameSitesBoxTests.cpp)
Lisible maintenant : `HOLScreenTest::IsWidgetEnabled(HOLScreenTest::FindWidgetByName(Screen, n"SacredSitesDropdown"))`. NON exposé (hors périmètre, à voir si on veut porter ce test) : `FHOLMapRules::FScopedSiteCount` (le crochet « Plain sans couche de sites »), `FHOLNewGameScreen::SetConfig` / `EnterLobby` côté script si ce ne sont pas déjà des UFUNCTION ; le test C++ reste donc en C++ pour l'instant.

## Suspects de compilation
1. `TStrongObjectPtr<UWorld>` : `UWorld::DestroyWorld` après `Reset()` du pointeur fort : le code garde un `UWorld*` local avant le Reset, puis détruit. Si UE se plaint d'une référence restante, déplacer le `Reset()` après `DestroyWorld`.
2. `HOLScreenTestLibWorld.Reset(World)` posé avant `CreateNewWorldContext` : si le monde est collecté par GC entre-temps (non, il est fortement tenu) rien à faire ; sinon ordre à inverser.
3. `NewObject<UUserWidget>(..., ScreenClass)` avec `TSubclassOf<>` : l'argument `UClass*` est implicite ; si ambigu, écrire `ScreenClass.Get()`.
4. `Screen->WidgetTree->FindWidget(Name)` : la surcharge non-template `UWidget* FindWidget(const FName&)` ; si le compilateur ne la voit pas, `Screen->WidgetTree->FindWidget<UWidget>(Name)`.
5. `DestroyScreenForTest(UUserWidget* Screen)` : paramètre non utilisé (warning C4100 possible, pas une erreur ; ajouter `(void)Screen;` si les avertissements sont des erreurs).
6. Binding script : si le nom de namespace n'est pas `HOLScreenTest` (le plugin retire `U` et `Library` ; `UHOLGameNamesReadLibrary` donne `HOLGameNamesRead`), on le verra à la compilation du script ; corriger l'exemple en conséquence.
7. Hors éditeur, GetDropdownText utilise `UHOLDropdown` (code non gardé), compile partout.

## Commande à lancer par la session principale
`C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait`, puis réécrire NGS_CreateScreen/NGS_Find/EndScreenTest dans `Script\UI\HOLNewGameScreen_Test.as` et lancer `quick_check -Parts tests` (rouge vu : si le montage échoue, les tests échouent avec la ligne `LogHOLScreenTest`).

BUILDS_DEMANDES: 1 (lot C++ de la séance)
