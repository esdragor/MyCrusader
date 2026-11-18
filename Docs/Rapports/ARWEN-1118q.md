# ARWEN-1118q : lot NG-1c (raccords C++ de l'ecran Nouvelle partie pour le script)

**NON COMPILE.** Aucun build, aucun quick_check, aucun lancement. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`, branche d751-root, commit `5f20788` (2 fichiers : `Source\HeroesOfLibertyUI\Screens\HOLNewGameScreen.h/.cpp`, +254/-11). Pas de push, pas de fusion. Raison du C++ (D839) : widget de base + liaison a exposer au script. Comportement C++ inchange : `Open` garde `StaticClass()`, tout ce qui est ajoute ne joue que pour une classe fille script (`GetClass() != StaticClass()` ou `bScriptOwnsGestures`) ou est une fonction appelable de plus.

Champs : BUILDS_DEMANDES: 0 (la session principale compile, groupe avec NG-1a / NG-1b) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: aucun de plus (FHOLNewGameLobbyView vient de NG-1b) ; PROCESSUS_DETACHES: aucun.

Commande de build : `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait` (apres fusion avec NG-1b : `HOLNewGameReadLibrary.h/.cpp` sont deja dans cette branche).

## 1. Recherche des widgets par nom (fait)
Dans `NativeOnInitialized`, juste apres `Super::NativeOnInitialized()` (donc apres `BP_BuildNewGame`), si `GetClass() != StaticClass()` : `FindWidget` par nom de MapDropdown, SizeDropdown, TeamsTogetherDropdown, FogDropdown, ModeDropdown, NomadDropdown, RegicideDropdown, CatastropheDropdown, SacredSitesDropdown, LaunchButton, BackButton, ReadyButton, SpectateButton, SpectatorsText, ErrorText, et **`Note` -> NoteText**, **`Heading` -> HeadingText**. Les `Slot*`, `SlotYou0`, `HeadName/HeadSlot/HeadLevel` etaient deja retrouves (cpp avant : lignes 360-372). Les abonnements existants du dessous (grisage Nomad/Regicide/Catastrophe, boutons Launch/Ready/Spectate/Back) jouent donc sur le layout du script.
Ecarts de nom confirmes (le rapport ARWEN-1118p etait inexact et ELROND-1118r a raison) : `NoteText` est le membre, le widget s'appelle `Note` ; le widget du titre s'appelle `Heading`, le membre `HeadingText` ; `SlotState0` n'existe pas (ligne 0 = `SlotYou0`). Les noms du script d'Elrond (`Heading`, `Note`, `ErrorText`, `SpectatorsText`, `HeadName`...) concordent avec le cpp (BuildLayout 237-345).
**Piege restant** : les gestes dropdown C++ (`Handle*Chosen`) restent abonnes pour la classe script. Voir `bScriptOwnsGestures` ci-dessous.

## 2. Couleur de l'erreur (fait)
`AddNewGameText` : si `Name == "ErrorText"` -> `SetColorAndOpacity(HOLUIStyle::Missing)` (comme cpp:336). Le script n'a rien a faire. (La visibilite Collapsed reste au script, comme avant.)

## 3. Textes, une seule source (fait) : `GetNewGameText`
`UFUNCTION(BlueprintPure) static FText GetNewGameText(FName Key)` (Angelscript : `UHOLNewGameScreen::GetNewGameText(n"Heading")`), vide pour une autre cle. Chaque `LOCTEXT` reprend EXACTEMENT cle et texte d'origine (meme espace `HOLNewGameScreen`, donc meme entree de traduction, pas de conflit) :
Heading, Map, Fog, Size, TeamsTogether, Player, Slot ("Emplacement", lobby), Level, Faction, Colour, Team, You, Mode, Nomad, Regicide, Catastrophe, SacredSites, GreyedTeamsOpen (note solo), LobbyGreyed (note salon), Back, LeaveLobby ("Quitter le salon"), ReadyAction, NotReadyAction ("Plus pret"), BecomeSpectator, BecomePlayer ("Jouer"), Launch, Starting ("Lancement..."), NoMatchAddress, TooManyPlayers.
+ `static FText GetNewGamePlayerName(int SlotIndex)` ("Joueur n", PlayerN, SlotIndex 0-based).
Non couverts (restent dans Refresh C++, texte compose) : Leader ("crown {0}"), Spectators ("Spectateurs : {0}"). Les listes de valeurs (Ferme/Humain/IA, niveaux, factions, brouillard, Non/Oui, tailles, Standard/Diplomatie) viennent deja de `GetNewGameOptions(Kind)` (NG-1a). Les `NSLOCTEXT` du `.as` d'Elrond sont a remplacer par ces appels (agent suivant) ; cles identiques donc rien ne casse en attendant.

## 4. Acces a l'etat pour les gestes (fait en entier, ~150 lignes)
Signatures Angelscript exactes (toutes sur `UHOLNewGameScreen`, categorie `HOL|NewGame`) :
- `bool bScriptOwnsGestures` (UPROPERTY EditAnywhere, BlueprintReadWrite ; defaut false). Dans une classe script : `default bScriptOwnsGestures = true;`. Alors `NativeOnInitialized` n'abonne PAS les `Handle*Chosen` aux dropdowns (Slot State/Color/Team/Faction, Map, Size, TeamsTogether, Fog, Mode, Options) : le script s'abonne a `UHOLDropdown::OnChosen` sans double jeu. Les boutons restent cables sur les C++ (Launch, ToggleReady, ToggleSpectator, Close).
- `FHOLNewGameLobbyView GetLobbyView() const` (BlueprintPure) = `UHOLNewGameReadLibrary::MakeLobbyView(Lobby)` ; vue par defaut (bHasMatch faux, 8 lignes vides) en solo.
- `FString GetConfigJson() const` = `Config.ToJson()` ; lecture par `HOLMatchConfigRead::Describe`, `HOLNewGameRead::DescribeGlobals`.
- `bool SetConfigJson(const FString& Json)` = `FromJson` puis `SetConfig` (normalise, remplace une carte hors liste, Refresh inclus) ; faux = rien change. ATTENTION : la normalisation clampe la couleur "Aleatoire" ; pour ce cas utiliser `SetConfigSlot`.
- `int GetConfigMapIndex() const` (-1 hors liste) ; `void SetConfigMapIndex(int Index)`.
- `void SetConfigMapSize(int Size)` (ecriture brute, ignoree si pas une taille de la table : geste salon) ; `bool ChooseNewGameSize(int Size)` (= `ChooseSize` : FitSlotsToSize + places tirees ; faux si les joueurs ne tiennent pas).
- `void SetConfigGlobals(bool bTeamsTogether, int FogMode, int Mode)` (FogMode = EHOLFogMode 0 Disabled / 1 Explored / 2 Normal = 2 - index de liste ; Mode 0 Standard / 1 Diplomacy ; clampes).
- `void SetConfigOptions(bool bNomad, bool bRegicide, bool bCatastrophe, bool bSacredSites)`.
- `void SetConfigSlot(int SlotIndex, int State, int Faction, int ColorIndex, int Team)` (State = EHOLSlotState 0 Closed / 1 Human / 2 AI ; -1 = champ inchange ; couleur "Aleatoire" permise ; indice hors 0..7 : rien).
  Les ecritures `SetConfig*` ne normalisent pas et ne font PAS de Refresh : le script appelle `Refresh()` en fin de geste (comme les `Handle*` C++).
- `void SendMyChoice(int ColorIndex, int Team, int Ready, int Spectator, const FString& Faction)` (enveloppe de `UHOLManagerClient::UpdateMe` ; -1 = non regle, Ready/Spectator 0 ou 1, Faction vide = non regle ; reponse montree par `HandleWritten` ; rien sans salon).
- `void SendSlotOpen(int SlotIndex, bool bOpen)` (enveloppe de `SetSlot`, chef).
- Deja la depuis NG-1a : `Launch()`, `Refresh()`, `ToggleReady()`, `ToggleSpectator()`, `SendSettings()`, `SendSlotAIChoice(int Slot, int Color, int Team, FString Faction)`, `ShowError(FText)`, `IsLobby()`, `IsLobbyAI(int)`, `GetMapNames()`, `GetNewGameOptions(Kind)`.

## Table des gestes : geste C++ -> ce dont il a besoin -> fonction appelable (contrat NG-4 solo / NG-5 salon)
Le script garde les regles (`HOLNewGameRules.as`, `HOLNewGameRead`) ; il lit l'etat par `GetConfigJson` / `GetLobbyView`, applique, ecrit par `SetConfig*`, puis `Refresh()`.

| Geste (cpp) | Lit | Ecrit / appelle | Fonctions appelables |
|---|---|---|---|
| HandleMapChosen (948) | indice choisi, mode salon, taille actuelle, nb de joueurs | Config.Map ; si solo et taille None : ChooseSize(GetDefaultSize(n)) ; Refresh ; salon : SendSettings | `SetConfigMapIndex`, `GetConfigJson`+`DescribeGlobals`/`Describe` (taille, joueurs), `HOLNewGameRules::MapChoiceNeedsDefaultSize/DefaultSize`, `ChooseNewGameSize`, `Refresh`, `IsLobby`, `SendSettings` |
| HandleSizeChosen (966) | taille choisie, mode salon | solo : ChooseSize ; salon : Config.MapSize si taille connue + SendSettings ; Refresh | `ChooseNewGameSize` (solo), `SetConfigMapSize` (salon), `SendSettings`, `Refresh` |
| HandleTeamsTogetherChosen (982) | valeur 0/1 | Config.bTeamsTogether ; salon SendSettings ; Refresh | `SetConfigGlobals(v==1, fog, mode)` (relire fog/mode par `DescribeGlobals`), `SendSettings`, `Refresh` |
| HandleFogChosen (992) | valeur (index liste) | Config.FogMode = 2 - v | `SetConfigGlobals` |
| HandleModeChosen (1003) | valeur 0/1 | Config.Mode | `SetConfigGlobals` |
| HandleOptionChosen (1014) | 4 dropdowns d'options (leurs `GetSelectedIndex`, que le script tient lui-meme) | Config.bSacredSites (solo aussi), Nomad/Regicide/Catastrophe (salon seulement) + SendSettings | `SetConfigOptions`, `SendSettings` |
| HandleStateChosen (1197) solo | indice de place, valeur, NumPlaces, couleurs prises | slot.State, couleur libre si prise ; Refresh | `HOLNewGameRules::ApplyStateChosen/IsColorTaken/FindFreeColor`, `SetConfigSlot(i, state, -1, color, -1)`, `Refresh` |
| HandleStateChosen salon | valeur | 0/1 : SetSlot ; IA : SetSlotAI | `HOLNewGameRules::LobbyStateAction`, `SendSlotOpen`, `SendSlotAIChoice(i,-1,-1,"")` |
| HandleFactionChosen (1266) | faction, cheat (`HOLNewGameRules::IsFactionChoosable`), ma place + CanEditAISlot (vue salon) | Config.Slots[i].Faction ; salon : IA -> SetSlotAI(faction), moi -> UpdateMe(faction) | `GetLobbyView().Slots[i].bCanEditAI`, `YourSlot`, `SetConfigSlot(i,-1,f,-1,-1)`, `SendSlotAIChoice(i,-1,-1,Name)`, `SendMyChoice(-1,-1,-1,-1,Name)`, `Refresh` ; nom : `HOLFactionChoiceRead::GetName` |
| HandleColorChosen (1304) | valeur | Config color ; IA : SetSlotAI(color) ; moi : UpdateMe(color) | `SetConfigSlot(i,-1,-1,c,-1)`, `IsLobbyAI(i)`, `SendSlotAIChoice(i,c,-1,"")`, `SendMyChoice(c,-1,-1,-1,"")`, `Refresh` |
| HandleTeamChosen (1074) | valeur | Config team ; IA : SetSlotAI(team) ; moi : UpdateMe(team) | `SetConfigSlot(i,-1,-1,-1,t)`, `IsLobbyAI`, `SendSlotAIChoice(i,-1,t,"")`, `SendMyChoice(-1,t,-1,-1,"")` |
| ToggleReady (1093) | ma place, bReady actuel | UpdateMe(ready) | deja `ToggleReady()` ; ou `GetLobbyView()` + `SendMyChoice(-1,-1,0/1,-1,"")` |
| ToggleSpectator (1110) | bYouSpectator | UpdateMe(spectator) | deja `ToggleSpectator()` ; ou `SendMyChoice(-1,-1,-1,0/1,"")` |
| Launch / StartMatch | Config, bLobby | Match->SetConfig + OpenLevel ; salon : StartMatch | deja `Launch()` (reste C++ : sous-systeme, OpenLevel) |
| Refresh / ShowLobby / ReadLobby / EnterLobby / HandleWritten / JoinMatch | Config, Lobby, widgets par nom | widgets | RESTENT C++ (reseau, timer, widgets par nom) ; le script appelle `Refresh()`. `EnterLobby` est appele par l'ecran de liste de salons (C++) |
| UpdateFactionLists (cheat) | `HOLFactionChoice::IsCheatOn` | listes de factions | reste C++ (Refresh/NativeOnActivated) |

## Suspects de compilation
1. `#include "Match/HOLNewGameReadLibrary.h"` dans le `.h` de l'ecran : le module UI voit deja `Match/HOLMatchConfig.h` du module principal, donc l'include est atteignable ; si UHT proteste sur `FHOLNewGameLobbyView` en retour de UFUNCTION, verifier l'ordre des includes (avant `.generated.h`, c'est le cas).
2. `HOLUIStyle::Missing` dans `AddNewGameText` : membre de l'ecran SANS `using namespace ...::NewGame` (BuildLayout l'utilise avec ce using ; le symbole est cense etre global via `Style/HOLUIStyles.h`, inclus). Si non resolu : qualifier.
3. Gardes `if (!bScriptOwnsGestures) X->OnSelectionChanged.AddWeakLambda(this, [..](const int32 Value) {...});` : `if` sans accolades devant une instruction sur plusieurs lignes (legal) ; le `if` englobant existant `if (SlotStates[SlotIndex]) {` reste.
4. Cles LOCTEXT repetees dans `GetNewGameText` avec meme cle et meme source que celles du layout : accepte en build unity et par le `fill_po.py` (meme entree) ; `PlayerN` et `Player` deja dupliques dans le fichier.
5. `UHOLManagerClient::UpdateMe(TOptional<int32>, TOptional<int32>, TOptional<bool>, Done, TOptional<bool>, const FString&)` : appel avec les 6 arguments dans l'ordre de la declaration (HOLManagerClient.h:58) ; le conditionnel `x >= 0 ? TOptional<int32>(x) : TOptional<int32>()` a le meme patron que `SendSlotAIChoice`.
6. `FHOLMatchConfig Parsed;` (construction par defaut) puis `FromJson` ; `Config.ToJson()` const. Une `UPROPERTY` bool dans un `UCLASS()` sans `Blueprintable` : defaut ecrit par `default bScriptOwnsGestures = true;` du script (a verifier au 1er lancement : si le plugin refuse `default` sur ce champ, passer par le constructeur ou un `UFUNCTION void SetScriptOwnsGestures(bool)` appele avant `Super`).
7. Variables nommees `SlotConfig` (pas `Slot`) pour eviter C4458 ; aucune fonction du namespace anonyme ajoutee (build unity inchange).
8. Le script ne peut pas appeler `SetConfigSlot` avec un `Faction` hors enum ; `static_cast<EHOLSlotState/EHOLFaction>` non valide : le script doit passer 0..2 / 0..5.

## Reste (agents suivants)
- NG-3 (Elrond) a rebrancher sur `GetNewGameText` et a abonner `OnChosen` avec `default bScriptOwnsGestures = true;`.
- Test de lancement : verifier que `FindWidget` trouve bien les noms crees par les aides du script (premier lancement, `Test_NewGameWidgets` d'Elrond).
