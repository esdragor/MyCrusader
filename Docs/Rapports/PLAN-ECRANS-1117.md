# PLAN-ECRANS-1117 : les 3 premiers écrans passés en script, un par un, avec un essai de 3 min (D840)

Auteur : GALADRIEL-1117c, 2026-11-17 09:28-09:40, lecture seule du code (aucun build, aucun fichier de code touché). Sources : `Docs\Rapports\PLAN-1117a.md` partie 1.c,
`Source\HeroesOfLibertyUI\Screens\*`, `Script\UI\HOLEndBannerScreen.as`, `HOLConfirmDialog.as`, `HOLGameMenuScreen.as`, `Tools\Scenarios\*`.
Rien ci-dessous ne tranche un visuel : le script reproduit ce que le C++ construit aujourd'hui, l'utilisateur juge « identique ? » à l'essai.

## 0. Ce que montre le patron (à suivre à l'identique)

Le modèle est le même pour `UHOLEndBannerScreen` (+ `HOLEndBannerScreen.as`, 78 l.), `UHOLGameMenuScreen` (105 l.) et `UHOLConfirmDialogScreen` (`HOLConfirmDialog.as`, 40 l., déjà en
script, commit 47e222a). Quatre pièces :

1. **Base C++ gardée**, `UCLASS(Abstract, Blueprintable)`, qui tient ce que le script n'atteint pas : ouverture (couche, `PushMenu`), actions qui touchent le monde ou le réseau,
   remplacements d'événements Slate (`FReply`), textes localisés (`LOCTEXT`, pour que `HeroesOfLiberty.Localization.Translations` reste vert, ex. `YesLabel()` du dialogue).
2. **`static TSubclassOf<Base> FindScriptClass()`** `BlueprintPure` (itère les `UClass` enfants, saute `CLASS_Abstract | CLASS_NewerVersionExists`, comme `UHOLScriptScreen::FindScriptClass`)
   et l'appelant C++ pousse cette classe (pas `StaticClass()`) ; sans classe, un `Warning` et `nullptr`.
3. **Aides `BlueprintCallable`** qui enveloppent les aides de `UHOLScreen` (`MakeCenteredColumn`, `AddText`, `AddButton`, `AddRotatorRow`...) : `MakeXColumn`, `AddXText`, `AddXButton`...
   plus un `BP_Build<X>` `BlueprintImplementableEvent` appelé à la place de `BuildLayout` (le script le surcharge avec `UFUNCTION(BlueprintOverride)`).
4. **Script** `Script\UI\HOL<X>Screen.as` (`class UHOL<X>ScreenScript : UHOL<X>Screen`) : construit la colonne, branche `OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked")`,
   appelle les actions de la base. Les noms de widgets sont **gardés tels quels** (les scénarios, `hol.UI.Click <nom>`, `hol.Mouse.Click <nom>`, les `waitlog` s'y accrochent).
   Test de structure `Script\UI\HOL<X>Screen_Test.as` (modèle `HOLEndBannerScreen_Test.as` : `FindScriptClass` valide et nommée `HOL<X>ScreenScript`).

Piège vu en lisant `HOLUIOpen.cpp` et `HOLMenuPlayerController.cpp` : l'écran titre est poussé par `RootLayout->PushMenu(UHOLTitleScreen::StaticClass())` (ligne 49) ; les écrans Réglages et
Multijoueur sont poussés par leurs `Open()` statiques, appelés par le titre, `HOLUIOpen` (`hol.UI.Open`) et `HOLMenuPlayerController` : ces `Open()` restent en C++ (un seul endroit à changer par écran : `StaticClass()` devient `FindScriptClass()`).

## 1. Choix des 3 écrans

Candidats lus : Titre (123+46 l.), ConfirmDialog (déjà fait), Stats (400+68), Réglages (538+89), Multijoueur (432+73), NewGame (1323+196, exclu), GameScreen (exclu, étape 4 en panneaux), CheatPanel/MinimapView (hors liste « salon, titre, options »).

| Écran | Lignes C++ | Slate/UMG pur ? | Appelants | Réseau / monde | Risque visuel | Verdict |
|---|---|---|---|---|---|---|
| **Titre** `HOLTitleScreen` | 169 | oui (colonne, 4 boutons, texte, version) | 1 `PushMenu`, `AskToQuit` (HOLUIOpen), 1 scénario qui clique `PlayButton` | aucun | faible (colonne centrée, aucun dessin propre) mais c'est le premier écran vu : tout scénario lancé du menu y passe | **1er** |
| **Réglages** `HOLSettingsScreen` | 627 | oui (onglets, rotateurs) | titre (bouton), `hol.UI.Open Settings[:Tab]` | aucun ; lit/écrit `UGameUserSettings` / `UHOLGameUserSettings` | moyen (4 onglets, 15 à 20 rotateurs, grandes pages) ; effet de bord : change des réglages sauvegardés | **2e** |
| **Multijoueur** `HOLMultiplayerScreen` | 505 | oui (liste de lignes, champ de nom) | titre, `HOLMenuPlayerController` (retour de partie), `hol.UI.Open Multiplayer` | **oui** : `UHOLManagerClient`, `FHOLLobbySummary`, retours asynchrones | moyen, mais logique de liste et d'erreurs | **3e** (le « salon ») |
| Stats `HOLStatsScreen` | 468 | non : peinture Slate `UHOLStatsGraph`, abonnements `UHOLMatchStatsSubsystem` | bannière de fin, partie (spectateur), `hol.UI.Open Stats` | monde (tableaux de la partie) | élevé (courbes, 4 onglets) | réserve : après le lot L17-11 (bibliothèque de lecture MatchStats) |

Pourquoi cet ordre : le Titre est le plus petit et sans dépendance, il valide le patron sur un écran « poussé par la racine du menu » (cas jamais fait : les 3 modèles sont des modales ou des bannières). Les Réglages sont
purs et locaux (pas de réseau), bien plus gros mais sans risque de transport ; l'essai de 3 min est riche (l'utilisateur y change des valeurs). Le Multijoueur vient en dernier parce qu'il est le seul à toucher
le réseau (gestionnaire de salons, callbacks asynchrones à exposer) : si le patron a une faille, les deux premiers l'auront montrée. L'écran « salon » au sens de la file d'attente d'une partie
en réseau est `HOLNewGameScreen` en mode lobby (1 519 l.) : trop gros pour cette vague (PLAN-1117a : « non maintenant »).
Question possible pour l'utilisateur (non posée : défaut codé = l'ordre ci-dessus) : « salon = liste Multijoueur (505 l.) ou écran de salon NewGame (1 519 l.) ? ». Je pars de la liste.

## 2. Fiche par écran

### 2.1 Titre (lot ECR-1)

- **Patron** : `UHOLTitleScreen` devient `UCLASS(Abstract, Blueprintable)` ; `Script\UI\HOLTitleScreen.as` (`UHOLTitleScreenScript : UHOLTitleScreen`, ~55 l.) construit la colonne `Title` (48, style titre, `SetVisibility(Visible)`
  pour que le clic atteigne l'écran), `PlayButton`, `MultiplayerButton`, `SettingsButton`, `QuitButton`, `Version` (coin bas droit).
  Le **triple clic de triche** (`NativeOnMouseButtonDown`/`DoubleClick`, `HOLFactionChoice::FClickCounter`) **reste en C++** (remplacement de `FReply`, rien à gagner à le passer en script).
  `bIsBackHandler = false`, `NativeGetDesiredFocusTarget` restent en C++ (la base lit `PlayButton`/`QuitButton`, désormais `UPROPERTY(BlueprintReadWrite)`).
- **Liaisons C++ à exposer** (~100 lignes ajoutées, ~70 retirées : `BuildLayout`, `NativeOnInitialized` et les 4 `Handle*` en C++) :
  `FindScriptClass`, `MakeTitleColumn` (= `MakeCenteredColumn`), `AddTitleText(Column, Name, Text, bTitle)`, `AddTitleButton(Column, Name, Label)`, `AddVersionText()` (le calque `UOverlay` et `HOLVersion::Display`),
  `static FText ButtonLabel(FName)` (les 4 `LOCTEXT` restent en C++), `OpenNewGame()`, `OpenMultiplayer()`, `OpenSettings()`, `AskQuit()` (`BlueprintCallable`, 3 lignes chacun, appellent `Open`/`AskToQuit` existants),
  `UPROPERTY(BlueprintReadWrite)` sur `Title`, `Version`, 4 boutons ; `BP_BuildTitle`. Appelant : `HOLMenuPlayerController.cpp:49` pousse `FindScriptClass()` (+ log d'erreur si nul).
  `HOLUIOpen.cpp:81` (`AskToQuit`) et `HOLFactionChoice` ne changent pas.
- **Scénario d'essai** : aucun scénario n'ouvre le titre pour le regarder. Un seul scénario clique les boutons du titre : `sites_option.txt` (`hol.Mouse.Click PlayButton`, plus loin dans son `.txt`) ;
  les 220 autres passent par `hol.Scenario` depuis le menu et n'ont pas de `.json` de menu. À écrire : `Tools\Scenarios\ecran_titre.txt` (+ `.look`, `.json` minimal si le préflight l'exige ; carte `Plain`) :
  titre ouvert, `capture ecran_titre_menu`, `note` des 3 points, `waitkey` (l'utilisateur joue lui-même : clique Jouer / Multijoueur / Réglages / Quitter / triple clic), `hol.UI.Open Quit` puis `capture ecran_titre_quitter`,
  `hol.Mouse.Click CancelButton`. Comment rester sur le titre en fin de scénario : **NON VÉRIFIÉ** (le README de `Tools\Scenarios` ne le dit pas dans mes extraits ; l'agent du lot lit d'abord `README.md` et `sites_option.txt`, sinon `Retours\Q-ecran-titre-scenario.md`).
  Commande : `powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 ecran_titre -Root <worktree ECR-1>`.
- **Test de structure** : `Script\UI\HOLTitleScreen_Test.as` : `Test_TitleScreenScriptClass` (la classe poussée par le menu est `HOLTitleScreenScript`) et `Test_TitleScreenWidgets` (l'écran ouvert contient `Title`, 4 boutons nommés `Play/Multiplayer/Settings/QuitButton`, `Version`, et le libellé de chaque bouton vaut `ButtonLabel(<nom>)`, valeur calculée par la base). 
  **Rouge à voir** : attendre `HOLTitleScreenScriptX` (ou « Jouez » au lieu de `ButtonLabel`) une fois, VU rouge (compte `quick_check` N-1/N), puis vert. Le test C++ n'existe pas ; `Localization.Translations` doit rester vert (les `LOCTEXT` n'ont pas bougé).
- **L'utilisateur regarde** : (1) le titre ressemble à celui d'aujourd'hui (nom, 4 boutons dans l'ordre Jouer / Multijoueur / Réglages / Quitter, version en bas à droite, rien de décalé à 1920x1080 et en fenêtre) ;
  (2) chaque bouton ouvre le bon écran et « Quitter » pose « Quitter le jeu ? » avec Non au focus (Entrée seule ne quitte pas ; Échap sur le titre ne fait rien) ; (3) le triple clic sur le nom marche encore (cheat des factions : Nouvelle partie propose les Templiers après).

### 2.2 Réglages (lot ECR-2)

- **Patron** : `UHOLSettingsScreen` Abstract/Blueprintable ; `HOLSettingsScreen.as` (~300 l.) construit les 4 onglets (Vidéo, Son, Commandes, Langue), le `UWidgetSwitcher`, les rotateurs et les gestes `Handle*`.
  Le C++ garde : `Open`, `Find` (statiques appelés par `hol.UI.Open Settings:<Onglet>`), `GetColumns` (colonnes de navigation, `virtual`), `NativeGetDesiredFocusTarget`, les libellés `LOCTEXT` (les 8 groupes de qualité, volumes, langues : via une `ButtonLabel`/`TextOf(FName)`).
  Le script utilise directement les `UFUNCTION(BlueprintCallable)` du moteur sur `UGameUserSettings` (résolution, mode fenêtre, VSync, cadence, échelle de rendu, qualité globale et par groupe, `ApplyResolutionSettings`, `SaveSettings`) et `UKismetInternationalizationLibrary::SetCurrentCulture`, `UKismetSystemLibrary::GetSupportedFullscreenResolutions`.
- **Liaisons C++ à exposer** (~150 lignes ajoutées, ~450 retirées sur 627) : `FindScriptClass` ; `MakeSettingsColumn`, `AddSettingsText`, `AddSettingsRotatorRow(Column, Name, Label, Options, Index)` (enveloppe `AddRotatorRow`, options en `TArray<FText>`), `AddSettingsTab`, `AddSettingsPages` (le switcher) ;
  un **événement dynamique** de `UHOLRotator` (`OnValueChanged` natif aujourd'hui : un `DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHOLRotatorChanged, UHOLRotator*, int32)`, ~12 l., fichier `HOLRotator.h/.cpp`, base) ;
  `UHOLGameUserSettings` (pas de `UFUNCTION` aujourd'hui) : `UFUNCTION(BlueprintCallable)` sur `Get()`, `GetVolume/SetVolume(int32 Volume)`, `GetCameraSpeed/SetCameraSpeed(int32)`, `IsEdgeScrollEnabled/SetEdgeScrollEnabled`, `GetOverallScalabilityLevel/SetOverallScalabilityLevel` (~40 l.) ;
  `SelectTab(int32)` en `BlueprintCallable`, `BP_BuildSettings`. Fichiers : `HOLSettingsScreen.h/.cpp`, `HOLRotator.h/.cpp`, `Settings\HOLGameUserSettings.h` (module principal). Aucun conflit prévu avec ECR-1 ni ECR-3.
- **Scénario d'essai** : aucun scénario n'ouvre les Réglages. La commande `hol.UI.Open Settings[:Tab]` existe (`HOLUIOpen.cpp`), `hol.UI.Click <widget>` et `hol.UI.Key` aussi. À écrire : `Tools\Scenarios\ecran_reglages.txt` (+ `.look`) : ouvre `Settings:Video`, capture, clique `SoundTab`, capture, `ControlsTab`, `LanguageTab` (sans changer la langue : `waitkey`, c'est l'utilisateur qui essaie), notes des 3 points.
  **Risque de bord** : l'essai change `GameUserSettings.ini` de la copie : l'agent du lot vérifie que `play.ps1 -Root <worktree>` utilise un `Saved\Config` du worktree (NON VÉRIFIÉ ici) et dit à l'utilisateur de ne pas changer la résolution pendant l'essai ou de la remettre.
- **Test de structure** : `Script\UI\HOLSettingsScreen_Test.as` : `Test_SettingsScreenScriptClass` ; `Test_SettingsTabs` (4 onglets nommés, `SelectTab(2)` met le switcher sur la page 2 : l'**effet** est l'index actif lu sur le switcher) ; `Test_SettingsVolumeEffect` (un rotateur de volume change `UHOLGameUserSettings::GetVolume` : la valeur lue est calculée par la vraie chaîne rotateur -> réglage, puis remise à l'état de départ).
  **Rouge à voir** : attendre l'index 3 au lieu de 2 ou un volume faussé, une fois, rouge VU puis vert. `HeroesOfLiberty.Settings.Volumes` (C++, sur `UHOLGameUserSettings` seul, pas sur l'écran) reste.
- **L'utilisateur regarde** : (1) les 4 onglets et leurs pages ressemblent à ceux d'aujourd'hui (aucun rotateur manquant, rien de coupé en 1280x720 comme en 1920x1080) ; (2) un changement s'applique tout de suite et se garde en rouvrant l'écran (un volume, la cadence, la vitesse de caméra) ;
  (3) clavier/manette : la navigation entre colonnes (flèches) et Retour/Échap ramènent au titre sans rien perdre.

### 2.3 Multijoueur, liste des salons (lot ECR-3)

- **Patron** : `UHOLMultiplayerScreen` Abstract/Blueprintable ; `HOLMultiplayerScreen.as` (~220 l.) : titre, champ du nom (`UEditableTextBox`), état, liste `LobbyList`, lignes (nom, joueurs, état, `Join/Rejoin/Watch<i>`), `Refresh`, `Create`, `Back`.
  Le C++ garde : `Open` (3 appelants), `IsValidPlayerName` (règle unique, testée par le script via la liaison), `RefreshLobbies`, `ShowLobbyLoss` (appelés par `HOLMenuPlayerController`), `UHOLManagerClient` (transport HTTP/WebSocket, `HOLManagerClient` 857 l.), `EnterAs`/`OpenMatchOf` (ouvrent NewGame en mode salon, lancent la partie), libellés d'erreur `GetManagerErrorText`.
- **Liaisons C++ à exposer** (~250 lignes ajoutées, ~350 retirées ; le plus lourd des trois) : `FindScriptClass` ; `MakeMultiplayerColumn`, `AddMultiplayerText/Button/Row` ; `USTRUCT(BlueprintType) FHOLLobbySummary` et `FHOLManagerError` (champs lus par le script) ;
  **rappel dynamique** des réponses asynchrones (`DECLARE_DYNAMIC_DELEGATE` : lobbies reçus, erreur) à la place des lambdas C++ ; actions `BlueprintCallable` : `RequestLobbies()`, `CreateLobby(Name)`, `JoinLobby(Index, bSpectator)`, `WatchLobby(Index, bRejoin)`, `TakePlayerName()`, `static bool IsValidPlayerName(FString)` ; `BP_BuildMultiplayer`. Les textes d'état (« 0 salon », erreurs) restent en C++ (`LOCTEXT`).
  Risque : si le transport ne peut être enveloppé proprement, ce lot se coupe en deux (ECR-3a liaisons + build, ECR-3b script), question `Retours\Q-ecran-multijoueur.md` si l'enveloppe change un contrat réseau.
- **Scénario d'essai** : aucun scénario `.txt` n'ouvre le Multijoueur. Existe `Tools\Scenarios\ai_lobby.ps1` (lance `Saved\Manager\HOLManager.exe` de la copie puis le jeu sous le nom « Chef » : « Multijoueur > Créer un salon ») : c'est le modèle pour un `ecran_multijoueur.ps1` (+ `.txt` pour les captures).
  Un essai **sans gestionnaire** couvre déjà le chemin d'erreur (texte rouge « pas de gestionnaire », nom invalide) et peut être un vrai scénario `play.ps1 ecran_multijoueur_sans_gestionnaire`. L'essai **avec gestionnaire** (créer, rejoindre un salon) passe par un `.ps1` comme `ai_lobby.ps1` : le pré-vol `preflight.ps1 <nom>` ne sait pas démarrer un gestionnaire (NON VÉRIFIÉ) : l'essai reste « LOGIQUE SEULEMENT » pour la partie salon tant qu'il n'est pas prouvé.
- **Test de structure** : `Script\UI\HOLMultiplayerScreen_Test.as` : `Test_MultiplayerScreenScriptClass` ; `Test_MultiplayerPlayerName` (`IsValidPlayerName` : 1 à 16 lettres/chiffres/espace/-/_ ; valeurs par la vraie fonction) ; `Test_MultiplayerLobbyRows` (3 `FHOLLobbySummary` -> 3 lignes avec les boutons attendus : Join sur un salon qui rassemble, Watch sur une partie en cours, Rejoin sur celle que j'ai quittée ; effet = boutons présents).
  **Rouge à voir** : limite 16 faussée en 17 ou un bouton Watch attendu à la place de Join, une fois, VU rouge puis vert.
- **L'utilisateur regarde** : (1) la liste (nom du joueur, lignes, boutons Actualiser / Créer / Retour) ressemble à celle d'aujourd'hui, le champ du nom refuse un nom invalide avec la même raison ; (2) sans gestionnaire, une ligne rouge explique pourquoi (pas de liste vide muette) ; (3) avec gestionnaire (`ai_lobby.ps1`) : Créer un salon ouvre l'écran Nouvelle partie en mode salon, Retour revient à la liste, la perte d'un salon est dite au retour.

## 3. Ordre des lots et parallélisme

Règle de D840 reprise : **un écran = un worktree, une branche, un lot** ; jamais deux écrans fusionnés sans l'essai du premier ; la branche n'est fusionnée dans `main` qu'après la réponse OK de l'utilisateur (BUG : retouche dans la même branche, rien dans `main`).
Le C++ de chaque écran se compile **une fois**, dans son worktree (build lancé par la session principale, `Tools\build.ps1 -LiveCoding=0 -Wait`) ; le C++ ancien de l'écran (`BuildLayout`, `Handle*`) est retiré dans le même commit que le basculement (jamais deux sources).

| Étape | Qui / où | Dépend de | En parallèle avec |
|---|---|---|---|
| ECR-0 (fait ici) | ce plan | | |
| ECR-1a : liaisons C++ du Titre + build | worktree `ecran-titre`, 1 agent C++ | ECR-0 | rien d'autre ne doit toucher `HOLTitleScreen.*`, `HOLMenuPlayerController.cpp` |
| ECR-1b : `HOLTitleScreen.as` + test rouge/vert, puis `ecran_titre.txt/.look`, préflight, PNG, `Retours\E-ecran-titre.md` [À ESSAYER] | même worktree, 2 agents script (script puis scénario) | 1a compilé | **ECR-2a et ECR-3a** (leurs C++ et builds, voir ci-dessous) |
| Essai 1 : l'utilisateur joue `ecran_titre` (3 min) : OK / BUG | utilisateur | 1b | |
| ECR-2a : liaisons C++ Réglages + build | worktree `ecran-reglages`, 1 agent C++ | peut être **écrit** dès ECR-0 ; n'est fusionné qu'après l'OK de l'essai 1 | ECR-3a |
| ECR-2b : `HOLSettingsScreen.as`, tests, scénario, `E-ecran-reglages.md` | même worktree | 2a | ECR-3b |
| Essai 2 : Réglages | utilisateur | 2b ; fusion de ECR-1 faite | |
| ECR-3a/3b : Multijoueur idem, avec `ecran_multijoueur.ps1` | worktree `ecran-multijoueur` | 3a ; essai 2 OK avant fusion | |

Ce qui peut se faire en parallèle sans conflit : les trois C++ (fichiers disjoints : Titre = `HOLTitleScreen.*` + `HOLMenuPlayerController.cpp` ; Réglages = `HOLSettingsScreen.*`, `HOLRotator.*`, `HOLGameUserSettings.h` ; Multijoueur = `HOLMultiplayerScreen.*`, `HOLManagerClient.*` selon l'enveloppe), mais **au plus 3 agents C++** et un build par worktree dans la file `Docs\BuildQueue.state`.
Ce qui ne se parallélise pas : les essais (l'utilisateur, un à la fois), la fusion dans `main` (après OK), et le lot Titre doit être **compilé et vert avant** d'écrire le script des deux autres (le patron `FindScriptClass` + `BP_Build` poussé par la racine du menu est neuf ici ; si le Titre révèle un défaut, les deux autres l'héritent sinon).
Par écran, durée visée : ~15 min d'agent C++ + build (main), ~15 min de script/test (rouge VU), ~15 min de scénario/préflight/PNG ; 3 agents Sonnet par écran, pas plus d'un écran en essai.

Dépendances de la suite (hors des 3 premiers) : Stats après L17-11 (bibliothèque MatchStats en lecture) et un wrapper de `UHOLStatsGraph` ; NewGame (1 519 l.) en 5 à 6 lots à découper ; HUD de jeu en dernier par panneaux (D840).

## 4. Lignes à tenir à jour à la clôture de chaque écran

`EtapeDefini.md` (titres seuls), `Auto_Journey_File.md` (ligne D840 : prochain écran), `Retours\E-<nom>.md`, journal `*InProgress.md`, `Placeholders.txt` (la ligne « Écran-titre / menu principal [style] » ne change pas : le script ne remplace aucun provisoire ; relire à la clôture).

## 5. Non vérifié

- Comment un scénario reste sur l'écran titre jusqu'à `waitkey` (README de `Tools\Scenarios`, extraits seulement).
- Que `play.ps1 -Root <worktree>` écrive `GameUserSettings.ini` dans le worktree et pas dans la copie de l'utilisateur.
- Que `preflight.ps1` accepte un scénario sans partie (carte `Plain` par défaut, `check.ps1` attend « ready »).
- Les estimations de lignes sont des ordres de grandeur d'après les en-têtes et des extraits des `.cpp` ; aucune construction n'a été faite.
- `UHOLRotator::OnValueChanged` et `FHOLLobbySummary` : leur déclaration exacte (délégué natif, `USTRUCT` ou non) n'a pas été lue.
