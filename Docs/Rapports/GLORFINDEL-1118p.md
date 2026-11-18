# GLORFINDEL-1118p : lot NG-1b (liaisons de lecture de l'ecran Nouvelle partie)

**NON COMPILE.** Aucun build, aucun quick_check, aucun lancement. Commit 7597d5e sur la branche net-playtest du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` (pas de push, pas de fusion). Pas de `git merge main` (inutile : branche deja a jour avec d4731a0). IN_USE.txt remis a « libre ».

Impact joueur : aucun visible encore ; cela debloque l'ecran Nouvelle partie en script (NG-2 et le script de l'ecran) sans ecrire de regle en C++.

## Fichiers (neufs, module principal)
- `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Source\HeroesOfLiberty\Match\HOLNewGameReadLibrary.h`
- `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Source\HeroesOfLiberty\Match\HOLNewGameReadLibrary.cpp` (~150 l. au total pour le .cpp, ~190 le .h dont commentaires)
- Aucun autre fichier touche (pas HOLNewGameScreen.*, HOLDropdown.*, Script\).

## Ce qui EXISTAIT DEJA (rien refait)
1. Cartes : `HOLMapRulesRead::GetMaps()` (TArray<FHOLMapInfoValue> : Key, DisplayName, bBudget, bGoldAtCentre, bDenseWood, NumSites), `FindByKey`, `GetAssetPath(Key)`, `FindByAsset`, `GetDisplayName` (`Map\HOLMapRulesReadLibrary.h`). Il n'y a PAS de « nombre de places » par carte : les places dependent de la taille (`HOLMapSizeRead::GetNumPlaces(Size)`).
2. Tailles et places : `HOLMapSizeRead::GetNumSizes, GetSizeAt, FindSize, FindByKey, GetNumPlaces, GetSideTiles, IsAllowed, GetAllowedSizes, GetDefaultSize, GetMapFile, GetFixedPlaces, DrawPlaces, GetConfigNumPlaces, GetConfigPlaces, CheckConfigPlaces, FitSlotsToSize` (`Map\HOLMapSizeReadLibrary.h`).
3. Factions : `HOLFactionChoiceRead::IsChoosable, IsListed, GetListed, Sanitize, GetName, TryFindByName, IsCheatOn, SetCheatOn, GetClickWindow, Click` (`Match\HOLFactionChoiceReadLibrary.h`). Point 2 entierement couvert.
4. Config : `HOLMatchConfigRead` (`Match\HOLMatchConfigReadLibrary.h`) lit/ecrit la config PAR TEXTE JSON : `MakeDefaultJson, CanRead, Describe, SetSlotJson, SetOptionsJson, GetNumOccupiedSlots, FindFreeHumanSlot, GetMaxSlots, GetNumColors, GetRandomColor, GetPaletteDWColors, GetPaletteColorName...` (FHOLMatchConfig est un `USTRUCT()` sans BlueprintType : le patron voulu est le JSON). Manquaient seulement les champs ci-dessous.

## AJOUTE : namespace script `HOLNewGameRead` (classe `UHOLNewGameReadLibrary`)
Fonctions (UFUNCTION BlueprintPure, statiques, Category `HeroesOfLiberty|NewGame`) :
- `int GetSiteCount(const FString& AssetPath)` : `FHOLMapRules::GetSiteCount` (case « Sites sacres » grisee a 0, cpp:936). Passer `HOLMapRulesRead::GetAssetPath(Key)`.
- `bool DescribeGlobals(const FString& Json, int& OutFogMode, bool& bOutTeamsTogether, FString& OutBiomeKey, int& OutStartingPeasants, TArray<int>& OutDifficulties, TArray<int>& OutAIBrains)` : ce que `Describe` ne donne pas (brouillard, equipes ensemble, biome, niveau et cerveau IA par place ; AIBrains 0/1). Faux si JSON illisible (sorties intactes).
- `FString SetGlobalsJson(const FString& Json, const FString& Map, int MapSize, bool bDrawPlaces, bool bTeamsTogether, int FogMode, int Mode, const FString& BiomeKey)` : ecrit Map, MapSize (EHOLMapSize 0..5, hors plage = None), DrawPlaces, TeamsTogether, FogMode (0 Disabled, 1 Explored, 2 Normal), Mode (0 Standard, 1 Diplomacy), Biome (vide = "default"). Vide si illisible.
- `FString SetSlotAIJson(const FString& Json, int Index, int Difficulty, bool bAIBrain)` : niveau IA (0 Easy, 1 Normal, 2 Hard, 3 Evolving) et cerveau d'une place ; vide si illisible ou Index hors 0..7.
- (C++ seulement, pas UFUNCTION) `static FHOLNewGameLobbyView MakeLobbyView(const FHOLLobby& Lobby)`.

Structs (USTRUCT(BlueprintType), champs UPROPERTY BlueprintReadWrite, valeurs par defaut = celles de FHOLLobby) :
- `FHOLNewGameLobbySlotView` : `int Index, State (0 Open, 1 Closed, 2 Player, 3 AI), FString PlayerName, int ColorIndex, Team, FString FactionName, int Faction (EHOLFaction ou -1 si nom inconnu), bool bReady, int Level (EHOLAIDifficulty), bool bIsLeader, bool bIsYou, bool bCanEdit (CanEditSlot), bool bCanToggle (CanToggleSlot), bool bCanEditAI (CanEditAISlot)`.
- `FHOLNewGameLobbyView` : `FString Id, Name; int State (0 Lobby, 1 Starting, 2 InGame); FString Leader, You, Map, Size; int MapSize, NumPlaces; bool bTeamsTogether; int FogMode; FString BiomeKey; int Mode; bool bNomad, bRegicide, bCatastrophe, bSacredSites; int YourSlot (-1 = spectateur); bool bYouLeader, bYouSpectator, bHasOpenSlot, bCanStart, bHasMatch; int NumPlayers, NumAIs, NumReady; TArray<FString> Spectators; TArray<FHOLNewGameLobbySlotView> Slots` (8 lignes).
Les regles du salon (CanStart, CanToggleSlot...) sont calculees en C++ par FHOLLobby et copiees : le script les APPLIQUE, ne les recalcule pas. Rien d'ajoute comme regle.

## CE QUI RESTE (hors de mon perimetre, a faire par NG-1a / la session principale)
- **Le point 4 n'est utilisable qu'avec un getter sur l'ecran** : `FHOLLobby` n'etant pas reflechi, un script ne peut pas le passer. Le lot NG-1a doit ajouter a `UHOLNewGameScreen` un `UFUNCTION(BlueprintPure) FHOLNewGameLobbyView GetLobbyView() const { return UHOLNewGameReadLibrary::MakeLobbyView(Lobby); }` (+ `#include "Match/HOLNewGameReadLibrary.h"`). Idem pour la config de l'ecran : le script tient un JSON ; il faut un getter/setter `FString GetConfigJson()` / `SetConfigJson(const FString&)` sur l'ecran (ou equivalent) pour relier `Config` (membre C++) a `HOLMatchConfigRead` / `HOLNewGameRead`.
- Textes LOCTEXT (noms d'etat, niveaux, tailles courtes) : prevus dans NG-1a, pas ici.
- Couleurs libres (`IsColorTaken`, `FindFreeColor`) : regles pures, voulues en script (NG-2), pas exposees ici.

## SUSPECTS DE COMPILATION (non verifies)
1. `struct FHOLLobby;` en avant-declaration dans le .h puis `MakeLobbyView(const FHOLLobby&)` statique non-UFUNCTION dans un UCLASS : UHT l'ignore normalement ; si UHT proteste, passer la declaration dans une section `#if` ou le deplacer dans un namespace C++ libre.
2. `FHOLLobby` est `HEROESOFLIBERTY_API` mais ses `FHOLLobbySlot` non ; l'usage reste dans le meme module, donc OK.
3. Build unity : j'ai prefixe les helpers du namespace anonyme (`NewGameReadConfig`, `NewGameToFogMode`, `NewGameToMapSize`, `NewGameToDifficulty`) pour ne pas heurter `ReadMatchConfig` / `ToSlotState` de HOLMatchConfigReadLibrary.cpp.
4. `FHOLMapSizeRules::Find(static_cast<EHOLMapSize>(Value))` avec Value hors enum : suppose que Find renvoie nullptr (non relu, signature `static const FHOLMapSizeInfo* Find(EHOLMapSize)`).
5. Angelscript : `TArray<FHOLNewGameLobbySlotView>` dans un USTRUCT(BlueprintType) et parametres sortants `TArray<int32>&` (meme patron que `Describe`) ; l'expose script reel n'est pas vu avant le build ; `FString` ref de sortie : `FString&`. Dans le script, `int` = `int32`.
6. Includes : `Map/HOLFogMode.h`, `Map/HOLMapRules.h`, `Map/HOLMapSizeRules.h`, `Match/HOLFactionChoice.h`, `Net/HOLManagerClientRules.h` (existent ; `Map/HOLMapRules.h` declare bien `GetSiteCount(const FSoftObjectPath&)`).
7. Enum non UENUM exposes en `int32` volontairement (pas de dependance a leur exposition script).

## Commande a lancer par la session principale
`C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` (groupe avec le lot NG-1a), puis une verification script : `HOLNewGameRead::GetSiteCount(HOLMapRulesRead::GetAssetPath("Plain"))` et un aller-retour `SetGlobalsJson` / `DescribeGlobals` (fog 1, equipes faux, biome "snow").

BUILDS_DEMANDES: 1 (lot C++ groupe) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: FHOLNewGameLobbyView, FHOLNewGameLobbySlotView ; appels d'outils vus par moi : ~14 ; arret 20:36 (< 20:51).
