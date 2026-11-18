# ELROND-1118t : lot NG-4 (comportement SOLO de l'ecran Nouvelle partie, script)

**NON COMPILE, NON LANCE.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers, commit `0efad66` (1 fichier : `Script\UI\HOLNewGameScreen.as`). Aucun C++, aucun build, aucun quick_check, pas de push, pas de fusion. Pas de nouveau test (consigne : les tests de widgets ne peuvent pas encore creer l'ecran). `HOLNewGameRules2.as` : non cree (aucune regle manquante).
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; PROCESSUS_DETACHES: aucun. Placeholders.txt : rien d'ajoute ni de retire.

## Table fonction C++ -> methode script (`UHOLNewGameScreenScript`)
| C++ (HOLNewGameScreen.cpp) | Script | Regle pure / acces de la base |
|---|---|---|
| abonnements 405-467 (coupes par `bScriptOwnsGestures`) | `default bScriptOwnsGestures = true;` + `Subscribe(liste, n"OnX")` en fin de `BuildNewGame` (listes gardees par les valeurs de retour de `AddNewGameDropdown`) | `UHOLDropdown::OnChosen.AddUFunction(this, FName)` |
| SetConfig / ChooseSize / Refresh | restent C++ (base) | `ChooseNewGameSize`, `Refresh()` |
| HandleMapChosen 971 | `OnMapChosen` | `SetConfigMapIndex`, `MapChoiceNeedsDefaultSize`, `DefaultSize`, `NumOccupied`, `ChooseNewGameSize`, `Refresh` |
| HandleSizeChosen 989 (solo) | `OnSizeChosen` | `ChooseNewGameSize`, `Refresh` |
| HandleTeamsTogetherChosen 1005 | `OnTeamsTogetherChosen` | `SetConfigGlobals(Index == 1, fog, mode)`, `Refresh` |
| HandleFogChosen 1015 | `OnFogChosen` | `SetConfigGlobals(tt, 2 - Index borne 0..2, mode)`, `Refresh` |
| HandleModeChosen 1026 | `OnModeChosen` | `SetConfigGlobals(tt, fog, Index == 1 ? 1 : 0)`, `Refresh` |
| HandleOptionChosen 1037 (solo) | `OnOptionChosen` | `SetConfigOptions(nomad, regicide, catastrophe actuels, SacredSitesList.GetSelectedIndex() == 1)`, pas de Refresh (comme le C++) |
| HandleStateChosen 1220 (solo) | `OnSlotStateChosen` | `NumPlacesOfConfig`, `ApplyStateChosen` (IsColorTaken / FindFreeColor dedans), `SetConfigSlot(i, etat, -1, couleur, -1)`, `Refresh` |
| HandleFactionChosen 1289 (solo) | `OnSlotFactionChosen` | `IsFactionChoosable(Index, HOLFactionChoiceRead::IsCheatOn())`, `SetConfigSlot(i, -1, f, -1, -1)`, `Refresh` |
| HandleColorChosen 1327 (solo) | `OnSlotColorChosen` | `SetConfigSlot(i, -1, -1, Index, -1)`, `Refresh` |
| HandleTeamChosen 1097 (solo) | `OnSlotTeamChosen` | `SetConfigSlot(i, -1, -1, -1, Index)`, pas de Refresh (comme le C++) |
| lecture de la configuration | `ReadConfig()` (membres `Cfg*`) | `GetConfigJson`, `HOLMatchConfigRead::Describe`, `HOLNewGameRead::DescribeGlobals` |
| table des places | `PlacesBySize()` | `HOLMapSizeRead::GetNumSizes / GetSizeAt / GetNumPlaces` (index = numero de taille, case 0 = 0) |
| Launch, Retour, ShowError, focus | NON reecrits : boutons cables par la base | `Launch()` etc. |
| lobby (`IsLobby()` vrai) | chaque geste sort au premier test, rien de plus | lot suivant (SendSettings, SendSlotAIChoice, SendMyChoice, SendSlotOpen) |

Les niveaux d'IA (`SlotLevel`) n'ont aucun abonnement en C++ (grises dans `NativeOnInitialized`) : rien en script non plus. Les journaux `UE_LOG` des gestes C++ (taille, brouillard, mode, faction) ne sont pas repris.

## Manques de la base (A AJOUTER A LA BASE C++)
Aucun bloquant pour le solo. Ecarts a connaitre :
- Journaux perdus : le C++ ecrivait `New game: fog ...`, `mode ...`, `slot N faction ...`, `slot N stays closed ...` ; si un scenario les cherche dans le journal, ajouter `UFUNCTION(BlueprintCallable) static void LogNewGame(const FString&)` (non fait, aucun scenario connu).
- Le nom de faction (`HOLFactionChoiceRead::GetName`, enum) n'est employe qu'au salon : rien a ajouter ici.

## Suspects de compilation (du plus probable au moins probable)
1. `HOLMatchConfigRead::Describe(...)` : 15 arguments dont des `int&`, `bool&`, `TArray<int>&`, `TArray<FString>&` passes comme MEMBRES de la classe (`CfgVersion`...). Si le compilateur refuse un membre en parametre de sortie : passer des locaux puis les copier. Meme patron que `HOLMatchConfig_Test.as` (a verifier au premier build).
2. `HOLNewGameRead::DescribeGlobals(Json, CfgFogMode, bCfgTeamsTogether, Biome, Peasants, Difficulties, AIBrains)` : nom d'espace `HOLNewGameRead` donne par les commentaires de `HOLNewGameReadLibrary.h`, jamais vu employe dans un `.as` ; si inconnu, c'est le nom de classe sans `UHOL`...`Library`.
3. `List.OnChosen.AddUFunction(this, Handler)` avec `Handler` un `FName` variable (le plugin accepte `n"..."`, une variable devrait passer) ; et les `UFUNCTION()` a deux parametres `(UHOLDropdown, int)` doivent correspondre exactement au delegue `FHOLDropdownChosen(UHOLDropdown*, int32)`.
4. `default bScriptOwnsGestures = true;` sur une `UPROPERTY` de la base C++ (suspect 6 d'ARWEN-1118q) ; repli : constructeur ou `UFUNCTION SetScriptOwnsGestures`. Symptome a l'ecran : chaque geste joue DEUX fois (C++ puis script), invisible pour la plupart des listes.
5. `Lists[SlotIndex] == List` et `!= nullptr` sur `UHOLDropdown` dans un `TArray<UHOLDropdown>` a elements nuls (`SlotStateLists[0]`, la ligne 0 n'a pas de liste) : ajout de `nullptr` via une variable `UHOLDropdown StateList = nullptr;`.
6. `const TArray<UHOLDropdown>& Lists` en parametre : ref constante sur un tableau d'objets, devrait passer.
7. `HOLNewGameRules::` appelees depuis un autre fichier : elles sont dans un namespace (piege 2 du brief respecte) ; `HOLMapSizeRead::` idem.
8. Un membre nomme `Subscribe`, `ReadConfig`, `PlacesBySize`, `SlotOfList` ou `OnMapChosen`... en collision avec un nom de la base : rien vu dans le `.h` (verifie par lecture des lignes 40-205), mais non greppe sur toute la hierarchie `UUserWidget`.
9. `int(EHOLAIDifficulty::Normal)` : ligne deja compilee au lot NG-3, inchangee.

## Gestes a verifier a l'ecran (scenario `ecran_nouvelle_partie`, `.look`)
1. Choisir une autre carte : la liste montre la carte, la taille reste (ou prend la taille par defaut si "—"), les tailles trop petites restent grisees.
2. Choisir une taille plus petite que le nombre de joueurs : refus (la liste grise deja ces tailles) ; plus petite avec des places libres : les joueurs se tassent dans les places (slots hors places fermes et listes grisees).
3. Equipes ensemble Non / Oui, Brouillard (Normal / Exploree / Desactive : l'indice 2 - i), Mode (Standard / Diplomatie) : la liste affiche la valeur choisie apres Refresh.
4. Sites sacres Non / Oui : la valeur reste (pas de Refresh) ; Nomade, Regicide, Catastrophe restent grises.
5. Etat d'un slot Ferme / Humain / IA : IA montre le niveau (Normal) et le nom d'IA, Ferme cache faction / pastille / couleur / equipe ; un slot au-dela des places reste ferme ; une couleur prise par un autre slot est remplacee par la premiere libre.
6. Faction : Aube / Legions prises ; Dragon / Ombre / Feu grises et refusees ; Templiers seulement avec le cheat.
7. Couleur : une couleur prise est grisee dans la liste ; la pastille prend la couleur ; "Aleatoire" accepte pour plusieurs slots.
8. Equipe d'un slot : la valeur reste.
9. Lancer : inchange (base), "Trop de joueurs pour cette taille de carte." si les joueurs ne tiennent pas ; Retour ferme l'ecran.
10. Aucun double jeu : un geste ne doit s'appliquer qu'une fois (suspect 4).

## Commande
Compiler le script (voie habituelle de la session principale), puis `Tools\Tests\quick_check.ps1 -Parts tests` puis le scenario `ecran_nouvelle_partie` en `play.ps1 ... -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`. Suite : NG-5 (salon, meme fichier) apres compilation verte.
