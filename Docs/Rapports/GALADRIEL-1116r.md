# GALADRIEL-1116r : bascule EndBanner (MIG-CPP, D830/D835)

Commit eaabc44 sur `galadriel-tab` (worktree HeroesOfLiberty-Galadriel). Branche prete, merge main non fait, NON COMPILE (consigne : pas de build).

## Fait
- Base C++ `UHOLEndBannerScreen` (`Source\HeroesOfLibertyUI\Screens\HOLEndBannerScreen.h/.cpp`, 190 + 120 l.), classe Abstract/Blueprintable :
  - `Open(PC, EHOLEndBanner)` : ferme modale + menu, `PushMenu(FindScriptClass())`, puis `SetResult` ; `SetResult`/`SetDetail`/`OnClosedByPlayer` gardes (le HUD n'a change que le type et l'include).
  - Liaisons nouvelles (strictement necessaires) : `FindScriptClass`, `GetResultValue`, `GetOriginValue` (int, pas d'UENUM : EHOLEndBanner est un enum C++ non UENUM, passe comme EHOLMatchOrigin dans FHOLGameMenuFacts), `MakeBannerColumn`, `AddBannerText`, `AddBannerRow`, `AddBannerButton`, `Continue`, `QuitToMenu`, `QuitToList`, `ReturnToLobby`, `ShowStats`, proprietes `Title`, `Detail`, `FocusTarget`, evenements `BP_BuildBanner` / `BP_RefreshBanner`.
  - Reutilises du menu : `UHOLGameMenuScreen::SetButtonLabel`, `MenuLog`, `SetMatchMenuInput`, `Find`.
  - `LeaveMatch` statique garde (appele par UHOLGameMenuScreen).
- Script `Script\UI\HOLEndBannerScreen.as` (`UHOLEndBannerScreenScript`) : construit titre, detail, 4 boutons, rafraichit par `HOLEndBannerScript::Make`, clics par `HOLEndBannerScript::Click`, relogue les lignes « End banner: ... ».
- Test `Script\UI\HOLEndBannerScreen_Test.as` : `Test_EndBannerScreenScriptClass` (classe trouvee, nom `HOLEndBannerScreenScript`, et la classe du menu reste la sienne).
- Appelants migres : `HOLGameHUD.cpp/.h` (include, type, `Open`), `HOLGameMenuScreen.cpp` (`LeaveMatch`, commentaire). `HOLStatsScreen.cpp` et `HOLUIOpen.cpp` ne referencaient pas la bannière (rien a faire). `Screens\HOLEndBanner.cpp/.h` SUPPRIMES (215 + 80 l.) : tous les appelants sont migres.
- Pourquoi une base distincte et pas `UHOLGameMenuScreen` : son `FindScriptClass` renvoie la premiere classe fille non abstraite ; deux scripts derives de la meme base se voleraient la classe. La bannière a aussi un delegate, un resultat et un detail propres.

## Champs
- BUILDS_DEMANDES: 0 (la session principale compile)
- TESTS_CPP_RETIRES: 0 (les 2 tests C++ EndBanner.* restent)
- TESTS_SCRIPT_AJOUTES: 1 (Test_EndBannerScreenScriptClass)
- LIGNES_CPP_SUPPRIMEES: 295 (HOLEndBanner.cpp 215 + .h 80) ; LIGNES_CPP_AJOUTEES: ~310 (base + liaisons)
- LIGNES_MIGREES_PAR_LOT: ~215 (logique d'ecran vers le script, 100 l. .as)
- TYPES_A_EXPOSER: aucun nouveau type ; UHorizontalBox/UHOLButton/UCommonTextBlock deja exposes
- TEST_ROUGE: PAS VU. A voir apres le build : fausser une fois « HOLEndBannerScreenScript » dans le test, ou lancer avant le build du .as (modele Test_GameMenuScreenScriptClass).
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance

## Commande pour la session principale
`<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests,net,victory,lobby` ; verifier `Test_EndBannerScreenScriptClass` et les 2 tests C++ EndBanner.*.

## NON VERIFIE
- Compilation C++ et chargement du script (rien lance).
- Syntaxe Angelscript : concatenation `"..." + int` (GetOriginValue) dans le log « way out », `Title.SetText(FText)` sur UCommonTextBlock, `Detail.SetVisibility`. Si le script refuse la concatenation int : `FString::Printf` ou `"" + `.
- Comportement des logs : « End banner: <Result> » et « way out » ne sortent qu'une fois par changement de description (avant : a chaque SetResult / activation) ; la ligne « Victory » etait avant loguee meme sans activation.
- Scenarios lisant ces lignes (non cherches dans Tools\Scenarios faute d'appels).
- Etat du popup initial : a l'activation le resultat est None (vue cachee, rien affiche), puis `SetResult` rafraichit ; titre/labels vides un instant avant le premier SetResult (meme sequence qu'avant).
