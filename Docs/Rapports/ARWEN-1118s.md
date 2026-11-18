# ARWEN-1118s : lot NG-1d (base C++ de l'ecran Nouvelle partie : ce qu'il manque au script pour porter le MODE SALON)

**NON COMPILE.** Aucun build, aucun quick_check, aucun lancement. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`, branche d751-root, 1 commit (voir `git log -1`), 2 fichiers : `Source\HeroesOfLibertyUI\Screens\HOLNewGameScreen.h/.cpp` (~+206/-43, dont le bloc deplace). Rien dans `Script\`, `HOLButton` non touche. Pas de push, pas de fusion.
Raison du C++ (D839) : widget de base + liaison a exposer. BUILDS_DEMANDES: 0 (la session principale compile) ; TESTS_*: 0.
Commande de build : `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\build.ps1 -LiveCoding=0 -Wait`.

## Comportement C++ d'aujourd'hui : inchange
`ShowLobby` : apres `Refresh();`, `bLeader` et `bSpectator` sont calcules (le journal de fin les lit), puis `if (bScriptOwnsLobbyDisplay) BP_ShowLobby(); else ShowLobbyWidgets();`. `ShowLobbyWidgets()` (nouvelle fonction privee, `.cpp` avant `GetColumns`) contient telle quelle l'ancienne partie d'affichage (ex-lignes 648-739 : titre, 8 lignes, listes grisees, Pret, Spectateur, texte des spectateurs, Lancer). RESTE EXECUTE PAR LA BASE dans tous les cas : la lecture du salon, la copie vers `Config`, `Refresh()`, l'etat Starting (`ShowError("Lancement...")`), `JoinMatch`, le journal « Lobby: ... », les envois, `HandleWritten`.
**Ecart a la consigne** : le declencheur est un drapeau `bScriptOwnsLobbyDisplay` (defaut faux) et non `GetClass() != StaticClass()`. Raison : une classe script qui n'a pas encore ecrit `ShowLobby` (cas d'aujourd'hui) garderait l'affichage C++ au lieu d'un salon vide. Le script le met avec `default bScriptOwnsLobbyDisplay = true;` EN MEME TEMPS qu'il ecrit l'evenement.

## Signatures Angelscript EXACTES (toutes sur `UHOLNewGameScreen`, categorie HOL|NewGame)
```
bool bScriptOwnsLobbyDisplay;                         // default = false ; default bScriptOwnsLobbyDisplay = true;
UFUNCTION(BlueprintOverride) void ShowLobby()         // surcharge de BP_ShowLobby (sans argument ; lire GetLobbyView())
static FText GetNewGameAIName(int SlotIndex, int Level) // "IA Difficile 3" ; Slot 0-based ; Level = EHOLAIDifficulty 0 Facile,1 Normal,2 Difficile,3 Evolutive (clampe)
static FText GetNewGameLeaderName(const FString& Name)                                  // "♛ Nom"
static FText GetNewGameSpectatorsText(const TArray<FString>& Spectators, const FString& Leader) // "Spectateurs : A, ♛ B" ; vide si aucun
FText GetNewGameLobbyLineName(int SlotIndex) const    // colonne nom du salon courant : vide (IA), "Joueur n" (place sans joueur), "♛ Nom" (chef), sinon le nom
FText GetNewGameLobbyAIName(int SlotIndex) const      // "IA <niveau> n" de la ligne du salon courant ; vide si ce n'est pas une IA
static FString GetNewGameFactionName(int Faction)     // nom manager d'un EHOLFaction par entier ("DawnOrder"...) ; vide hors 0..5
void SetNewGameButtonLabel(FName ButtonName, const FText& Label)  // bouton par nom de widget (LaunchButton, BackButton, ReadyButton, SpectateButton ou un du script) ; rien si inconnu
bool HasClient() const                                // le UHOLManagerClient du GameInstance existe
bool CanToggleLobbySlot(int SlotIndex) const          // FHOLLobby::CanToggleSlot ; faux en solo
bool CanEditLobbySlot(int SlotIndex) const            // FHOLLobby::CanEditSlot
bool CanEditLobbyAISlot(int SlotIndex) const          // FHOLLobby::CanEditAISlot
bool IsLobbyYouSpectator() const
bool IsLobbyYouLeader() const
bool LobbyHasOpenSlot() const
bool CanStartLobby() const                            // FHOLLobby::CanStart
```
Rappel : le script appelle `UHOLNewGameScreen::GetNewGameAIName(...)` pour les statiques, les autres sur `this`. Les libelles Pret / Plus pret / Spectateur / Jouer / Lancer / Quitter le salon etaient DEJA dans `GetNewGameText(n"ReadyAction" | n"NotReadyAction" | n"BecomeSpectator" | n"BecomePlayer" | n"Launch" | n"LeaveLobby")` (NG-1c) ; « Joueur n » dans `GetNewGamePlayerName(i)`. Les memes valeurs sont aussi lisibles dans `GetLobbyView()` (`bYouLeader`, `bYouSpectator`, `bHasOpenSlot`, `bCanStart`, `Slots[i].bCanToggle/bCanEdit/bCanEditAI/Level/bIsLeader/bIsYou`, `Spectators`, `Leader`) : les predicats ci-dessus sont des raccourcis equivalents (avec le garde `bLobby`).
Cles de traduction : `Leader` et `Spectators` gardent exactement l'espace `HOLNewGameScreen` et le texte d'origine ; `GetAIName` garde son espace `HOLForumMessages` : aucune nouvelle entree.

## Table : ce que ShowLobby affichait -> appel depuis le script (dans `ShowLobby()` du script)
| C++ (ShowLobbyWidgets) | Script |
|---|---|
| `HeadingText->SetText(Lobby.Name)` | `Heading.SetText(FText::FromString(View.Name))` (`View = GetLobbyView()`) |
| nom de ligne `SlotNames[i]` (IA vide / « Joueur n » / « ♛ Nom » / nom) | `LobbySlotNames[i].SetText(GetNewGameLobbyLineName(i))` |
| `State` = 0 ouvert/ferme (Row null ou Open), IA : `LobbyAIState`, 1 place sans joueur, 3 pret, 2 pas pret | meme calcul sur `View.Slots[i]` (State/bReady) ; valeurs des listes : `GetNewGameOptions(n"LobbyState")`, `HOLNewGameRules::LobbyAIState` ; regle pure `HOLNewGameRules::LobbyListIndex(true, State, bReady)` |
| `SlotStates[i]->SetSelectedIndex / SetIsEnabled(Lobby.CanToggleSlot(i)) / SetOptionEnabled(2,false), (3,false)` | `SetSelectedIndex`, `SetIsEnabled(CanToggleLobbySlot(i))`, `SetOptionEnabled` (UFUNCTION de `UHOLDropdown`, a verifier) |
| IA : `SlotStates[i]->SetShownText(AIName)` ou ligne 0 `SlotYou->SetText(AIName / LobbyStateNames()[State])` | `SetShownText(GetNewGameLobbyAIName(i))` (ou `GetNewGameAIName(i, Level)`) ; ligne 0 : `LobbyYouText.SetText(...)` |
| `bEditable = bMine || CanEditAISlot(i)` -> `SetIsEnabled` des listes faction / couleur / equipe | `View.Slots[i].bIsYou \|\| CanEditLobbyAISlot(i)` (= `HOLNewGameRules::LobbyLineEditable`) |
| reglages du chef (Map, Taille, Equipes ensemble, Brouillard, Mode, Nomad, Regicide, Catastrophe) `SetIsEnabled(IsYouLeader())` | `SetIsEnabled(IsLobbyYouLeader())` (note : SacredSites n'etait PAS dans la liste C++ : a l'identique, ne pas l'ajouter sans le dire) |
| `ReadyButton` : libelle, `Yours != NONE && State == Lobby`, Collapsed si spectateur | `SetNewGameButtonLabel(n"ReadyButton", GetNewGameText(bReady ? n"NotReadyAction" : n"ReadyAction"))`, `SetIsEnabled`, `SetVisibility` (regles pures `ReadyButtonEnabled/Visible/ShowsUndo`) |
| `SpectateButton` : libelle Jouer / Spectateur, actif si `State == Lobby && (!bSpectator \|\| HasOpenSlot())` | `SetNewGameButtonLabel(n"SpectateButton", GetNewGameText(IsLobbyYouSpectator() ? n"BecomePlayer" : n"BecomeSpectator"))`, `SetIsEnabled(...LobbyHasOpenSlot())` (regle `SpectateButtonEnabled/ShowsPlay`) |
| `SpectatorsText` : texte, Collapsed si aucun (sinon HitTestInvisible) | `SetText(GetNewGameSpectatorsText(View.Spectators, View.Leader))`, visibilite selon `View.Spectators.Num()` |
| `LaunchButton` : visible si chef ; actif si `CanStart() && State == Lobby` | `SetVisibility`, `SetIsEnabled(CanStartLobby() && View.State == 0)` |
| faction d'une ligne (nom manager) | `GetNewGameFactionName(int)` ou `HOLFactionChoiceRead::GetName(EHOLFaction(i))` |
| garde des envois : `GetClient()` | `HasClient()` (a la place de `IsLobby()` pour `bHasClient` de `LobbyStateAction`) |
Reste dans la base (le script ne l'ecrit pas) : `State == Starting` -> erreur « Lancement... » ; `InGame && bHasMatch` -> `JoinMatch()` ; le journal.

## Qui cable quoi (Ready / Spectateur) : LA BASE, toujours
`NativeOnInitialized` cable `ReadyButton->OnClicked()` sur `ToggleReady` et `SpectateButton->OnClicked()` sur `ToggleSpectator`, SANS condition (`bScriptOwnsGestures` ne les touche pas, comme documente dans ARWEN-1118q). Donc le script ne s'abonne PAS a ces deux boutons (double envoi sinon) : il ne fait que leur AFFICHAGE (libelle, actif, visible). Idem Launch -> `Launch()` et Back -> `Close`. Seuls les 9 jeux de listes (etats, couleurs, equipes, factions de ligne ; carte, taille, equipes ensemble, brouillard, mode, options) sont au script quand `bScriptOwnsGestures`.

## Suspects de compilation
1. `ShowLobbyWidgets()` redeclare `using namespace UE::HeroesOfLiberty::UI::NewGame;` (necessaire : `LobbyStateNames()`, `LobbyAIState`) ; si le build unity a un probleme de nom, c'est la.
2. `bLeader` / `bSpectator` sont calcules dans `ShowLobby` ET redeclares dans `ShowLobbyWidgets` (locaux distincts, pas de conflit) ; `bLeader` doit rester utilise par le journal (il l'est) sinon C4189.
3. `static_cast<EHOLAIDifficulty>(...)` : l'enum est `uint8`, `FMath::Clamp(int,0,3)` ok. `Lobby.Slots[i].Level` est bien un `EHOLAIDifficulty` (passe tel quel a `GetAIName` dans l'ancien code).
4. `UFUNCTION(BlueprintPure)` sur des `const` membres rendant `FText` / `bool` : ok ; `static FText ... (const TArray<FString>&, const FString&)` BlueprintPure : parametres const-ref de TArray acceptes (entrees).
5. (CORRIGE) le parametre de `GetNewGameAIName` etait nomme `Slot` (masque `UWidget::Slot`, C4458) : renomme `SlotIndex`, 2e commit.
6. `GetClient()` est prive mais `HasClient()` est dans la classe : ok. `WidgetTree->FindWidget(FName)` : meme appel que dans `NativeOnInitialized` (ici avec un `FName`, surcharge existante).
7. `BP_ShowLobby` : `BlueprintImplementableEvent` non-const sans parametre, cote script `UFUNCTION(BlueprintOverride) void ShowLobby()` ; si l'AS refuse parce que `ShowLobby` existe deja en C++ privee (nom different : `ShowLobby(const FHOLLobby&)` n'est pas UFUNCTION donc invisible), sinon utiliser `BP_ShowLobby` en C++.

## A regarder a deux instances (avec le script)
Chef : 8 listes de reglages actives, Lancer visible, actif quand tous prets ; invite : reglages grises, sa ligne et les IA du chef editables ; Pret -> « Plus pret », Spectateur -> « Jouer » et Pret masque ; « Spectateurs : ... » avec couronne ; une seule trame envoyee par clic sur Pret / Spectateur (pas de doublon).
