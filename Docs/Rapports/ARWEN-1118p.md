# ARWEN-1118p : lot NG-1a (base C++ de l'écran Nouvelle partie)

**NON COMPILÉ.** Branche d751-root, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`, commit `b337797` (4 fichiers, +294/-10). Raison du C++ (D839) : widget de base Slate/UMG et liaison à exposer au script. Comportement inchangé : `Open` garde `StaticClass()`, la classe reste concrète (`UCLASS()`), l'écran C++ continue de jouer.

Champs : BUILDS_DEMANDES: 0 (la session principale compile) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: FHOLMatchConfig, FHOLLobby, EHOLMapSize (lot NG-1b) ; PROCESSUS_DETACHES: aucun lancé.

Commande de build à lancer : `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait` (build groupé avec les autres lots C++ de la séance).

## Fichiers touchés

- `Source\HeroesOfLibertyUI\Screens\HOLNewGameScreen.h/.cpp`
- `Source\HeroesOfLibertyUI\Widgets\HOLDropdown.h/.cpp`

## Comportement

- `BuildLayout()` : si `GetClass() != StaticClass()` (classe script ou Blueprint dérivée), appelle `BP_BuildNewGame()` et rend la main ; sinon le layout C++ d'origine (inchangé).
- `NativeOnInitialized` n'est pas touché : il remplit `Maps` AVANT `Super::NativeOnInitialized()` (donc avant `BP_BuildNewGame`), puis retrouve les widgets PAR NOM (`MapDropdown`, `SizeDropdown`, `TeamsTogetherDropdown`, `FogDropdown`, `ModeDropdown`, `NomadDropdown`, `RegicideDropdown`, `CatastropheDropdown`, `SacredSitesDropdown`, `LaunchButton`, `BackButton`, `ReadyButton`, `SpectateButton`, `SpectatorsText`, `NoteText`, `ErrorText`, `SlotName0..7`, `SlotYou0`, `SlotState1..7`, `SlotLevel0..7`, `SlotFaction0..7`, `SlotSwatch0..7`, `SlotColor0..7`, `SlotTeam0..7`, `HeadName`, `HeadSlot`, `HeadLevel`, `Heading`). Le script doit donc créer les widgets avec EXACTEMENT ces noms (les aides prennent un `FName Name`).
- Les noms de widgets existants ne sont pas modifiés.

## Liste exacte des fonctions appelables du script (signature Angelscript attendue)

Toutes sur `UHOLNewGameScreen` (le `this` d'une classe script dérivée), sauf mention. Catégorie `HOL|NewGame`.

Base et cycle :
- `static TSubclassOf<UHOLNewGameScreen> UHOLNewGameScreen::FindScriptClass()` (BlueprintPure). Exclut la classe C++ elle-même : renvoie la classe dérivée la plus récente non abstraite, null sans script. Ne sert pas encore à `Open`.
- `void BP_BuildNewGame()` : BlueprintImplementableEvent, à surcharger (`UFUNCTION(BlueprintOverride) void BP_BuildNewGame()`), sans paramètre (pas de struct).

Actions (BlueprintCallable) :
- `void Launch()`
- `void Refresh()`
- `void ToggleReady()`
- `void ToggleSpectator()`
- `void SendSettings()`
- `void SendSlotAIChoice(int SlotIndex, int ColorIndex, int Team, const FString& Faction)` : -1 = colonne ou équipe non réglée, `Faction` vide = non réglée (enveloppe de `SendSlotAI` à `TOptional`, nom distinct pour éviter la collision avec la méthode C++ non-UFUNCTION, piège 3 du brief).
- `void ShowError(const FText& Text)`

Lectures (BlueprintPure) :
- `bool IsLobby() const`
- `bool IsLobbyAI(int SlotIndex) const`
- `TArray<FText> GetMapNames() const`
- `static TArray<FText> GetNewGameOptions(FName Kind)` avec Kind parmi : `SlotState` (Fermé/Humain/IA), `LobbyState` (Ouvert/Fermé/Pas prêt/Prêt/IA), `Level`, `Faction` (6, Templiers en dernier), `Team` (Aucune, 1..), `Color` (couleurs puis « Aléatoire »), `Fog`, `OffOn`, `Size` (index = EHOLMapSize, « — » en premier), `Mode` (Standard/Diplomatie) ; liste vide pour une autre clé.
- `static FText GetShortSizeNameOf(int Size)` (EHOLMapSize en entier ; vide pour 0).
- `static FText GetSizeNameOf(int Size)` (« Petite (2 joueurs) » ; « — » pour 0).
- `static TArray<FLinearColor> GetNewGameColorSwatches()` (pour `SetOptionSwatches`).
- `static float GetNewGameWidth(FName Key)` : `Name`140, `State`220, `Faction`280, `Swatch`28, `Color`200, `Team`160, `Level`140, `Option`200, `Toggle`120, `OptionLabel`170, `TeamsTogetherLabel`236, `MapList`508, `FogList`220 ; 0 pour une autre clé. Les valeurs viennent des constantes du cpp, pas de copies.

Aides de mise en page (BlueprintCallable) :
- `UVerticalBox MakeNewGameColumn()`
- `UCommonTextBlock AddNewGameText(UVerticalBox Column, FName Name, const FText& Text, int Style)` : 0 titre (24 dessous), 1 note (4), 2 corps (4).
- `UHorizontalBox AddNewGameRow(UVerticalBox Column, float TopPadding)` : ligne alignée à gauche, 4 dessous (= `AddLine` du layout C++).
- `UCommonTextBlock AddNewGameCellText(UHorizontalBox Row, FName Name, const FText& Text, int Style, float Width)` : texte en cellule de largeur fixe (Style 1 note, 2 corps ; 0 donne le style titre).
- `UHOLDropdown AddNewGameDropdown(UHorizontalBox Row, FName Name, const TArray<FText>& Options, int SelectedIndex, float Width)` (= `UHOLScreen::AddDropdown`).
- `UImage AddNewGameSwatch(UHorizontalBox Row, FName Name)` : pastille 28 dans sa cellule.
- `UCommonTextBlock AddNewGameHint(UVerticalBox Column)`
- `UHorizontalBox AddNewGameButtonRow(UVerticalBox Column)`
- `UHOLButton AddNewGameButton(UHorizontalBox Row, FName Name, const FText& Label)` (clics : `OnClicked()` natif non liable du script ; utiliser `OnButtonBaseClicked` comme le patron Réglages).

`UHOLDropdown` (BlueprintCallable / BlueprintPure, catégorie `HOL|Dropdown`) :
- `void SetOptions(const TArray<FText>& InOptions, int InSelectedIndex = 0)`, `void SetOptionSwatches(const TArray<FLinearColor>& InSwatches)`, `void SetOptionEnabled(int Index, bool bEnabled)`, `void SetSelectedIndex(int Index)` (ne diffuse rien), `int GetSelectedIndex() const`, `FText GetShownText() const`, `void SetShownText(const FText& Text)`, `bool IsListOpen() const`, `void OpenList()`, `void CloseList()`, `void Choose(int Index)`.
- Événement : `FHOLDropdownChosen OnChosen` (dynamique multicast, `UPROPERTY(BlueprintAssignable)`), signature du récepteur `void f(UHOLDropdown Dropdown, int Index)`. Abonnement : `Dropdown.OnChosen.AddUFunction(this, n"OnDropdownChosen");` avec `UFUNCTION() void OnDropdownChosen(UHOLDropdown Dropdown, int Index)`. Diffusé juste après le natif `OnSelectionChanged`, dans `Choose`. Aucun moyen existant : aucun `.as` ne s'abonne aujourd'hui à un dropdown (Réglages passe par `UHOLRotator` + `BP_SettingChanged` appelé depuis le C++). Pour distinguer l'émetteur, le script compare `Dropdown` à ses champs ou lit `Dropdown.GetName()`.

## Non exposé et pourquoi

- `SetConfig(const FHOLMatchConfig&)` / `GetConfig()` : struct non UFUNCTION-exposable ici ; lecture/écriture à passer par la bibliothèque du lot NG-1b (module principal).
- `EnterLobby(const FHOLLobby&)`, `Lobby` (lecture du salon) : `FHOLLobby` n'est pas exposé ; la lecture du salon pour `ShowLobby` en script dépend de NG-1b. `IsLobbyAI` seul est exposé.
- `SendSlotAI` d'origine (TOptional) : remplacé par `SendSlotAIChoice`.
- `GetMapName(FSoftObjectPath)`, `GetSizeName(EHOLMapSize)`, `GetLobbyMapName` statiques : conservés tels quels pour les appelants C++ ; équivalents script `GetMapNames`, `GetSizeNameOf`. `GetLobbyMapName(FString, FString)` n'est pas exposé (non demandé).
- `HandleXxxChosen`, `IsColorTaken`, `FindFreeColor`, `UpdateFactionLists`, `ChooseSize`, `StartMatch`, `JoinMatch`, `GetClient` : restent privés (le script réécrit les gestes ; réseau reste C++).
- Les widgets `UPROPERTY` protégés (`MapDropdown`... `SlotStates`) ne sont pas lisibles du script : le script garde ses propres références aux widgets qu'il construit, mais le C++ les retrouve par nom pour `Refresh` et `ShowLobby`.

## Pièges et suspects de compilation

1. Abonnements doublés à la bascule : `NativeOnInitialized` abonne déjà les `Handle*Chosen` C++ aux dropdowns trouvés par nom ; une classe script qui s'abonnerait aussi à `OnChosen` ferait jouer deux fois chaque geste. Ce n'est pas un problème de ce lot (l'écran script n'existe pas) mais NG-6 doit retirer les abonnements C++ des dropdowns (ou le script ne s'y abonne pas tant que les `Handle*` C++ existent).
2. `GetClass() != StaticClass()` : une classe Blueprint (si un jour) prend aussi le chemin `BP_BuildNewGame`. Une classe script sans surcharge de `BP_BuildNewGame` donne un écran vide.
3. `UFUNCTION` sur `Refresh`, `ToggleReady`, `ToggleSpectator`, `SendSettings`, `ShowError`, `IsLobbyAI` : déplacés de `private:` à `public:` ; les `AddUObject(this, &UHOLNewGameScreen::ToggleReady)` etc. restent valides. À surveiller : `OnFactionCheatChanged()` inline (`Refresh()`) défini avant la déclaration, légal dans la classe.
4. `UCLASS()` sans `Blueprintable` : l'héritage de `UHOLScreen` suffit normalement (flag hérité) ; si le plugin Angelscript refuse de dériver, passer en `UCLASS(Blueprintable)` (inoffensif).
5. Build unity : fonctions ajoutées dans l'espace nommé `UE::HeroesOfLiberty::UI::NewGame` : `AddNewGameCell`, `NewGameTextStyle` (préfixées). Les alias locaux `namespace NG = ...` sont dans le corps de deux fonctions seulement.
6. `GetNewGameOptions` compare un `FName` à `TEXT("...")` (opérateur `FName == const TCHAR*`, existant) et renvoie `{...}` `TArray<FText>` : si le compilateur proteste sur le retour par liste, remplacer par `TArray<FText>({...})`.
7. `DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHOLDropdownChosen, UHOLDropdown*, Dropdown, ...)` déclaré avant la classe avec une déclaration anticipée `class UHOLDropdown;` : si UHT refuse le type incomplet, déplacer la déclaration juste avant `UCLASS()` de `UHOLDropdown` (sans changer le nom).
8. Doublon de clés LOCTEXT : `Standard` et `ModeDiplomacy` sont redéclarés dans `GetNewGameOptions` avec le même texte que `BuildLayout` : même clé, même source, donc sans conflit.
9. Argument `static_cast<EHOLMapSize>(Size)` : suppose `EHOLMapSize` énumération sans sous-type incompatible avec un `int32`.
10. Le script appelle ces fonctions sous leurs noms C++ ; Angelscript n'ajoute pas de préfixe. Les paramètres `const FText&` et `const TArray<FText>&` s'appellent avec des temporaires sans souci.
