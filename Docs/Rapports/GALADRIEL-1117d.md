# GALADRIEL-1117d : ECR-1, écran TITRE en script (D840/D839)

NON COMPILÉ : syntaxe C++ et Angelscript relue, jamais compilée. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab`, commit `b80da54`, rien dans main, rien poussé.

APPELS_DECLARES: 17 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 09:34:57 / 09:39 (Get-Date)
PREMIER_BUILD: sans build (la session principale compile)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sortie vide (binaire à jour avant mes changements)
TEST_ROUGE: pas encore vu (compile nécessaire) ; assertit l'effet : non pour la structure (classe poussée + libellés calculés par la base) ; l'effet « le menu montre menu=HOLTitleScreenScript » est vérifié par les captures de quick_check (attendu mis à jour)
COMMITS: 1 (b80da54), dernier à 09:39, merge_main_fait: non (DLL périmée, la session principale synchronise)
TESTS: aucun lancé ; 2 tests script écrits (`Test_TitleScreenScriptClass`, `Test_TitleScreenLabels`) ; TESTS_TAUTOLOGIQUES: 0 (les libellés sont calculés par `ButtonLabel`/LOCTEXT de la base, la classe par `FindScriptClass`)
ESSAIS: ecran_titre : preflight NON LANCÉ, aucun PNG vu : LOGIQUE SEULEMENT, pas de `Retours\E-ecran-titre.md` créé
QUESTIONS: aucune (ouverture de la boîte « comment rester sur le titre » résolue : modèle sites_option, `open /Game/Maps/MainMenu` + `waitui`)
BUILDS_DEMANDES: 1   TESTS_CPP_RETIRES: 0   TESTS_SCRIPT_AJOUTES: 2 (Test_TitleScreenScriptClass, Test_TitleScreenLabels)
LIGNES_CPP_SUPPRIMEES: ~35 nettes (BuildLayout, NativeOnInitialized, 4 Handle* remplacés par les liaisons) ; LIGNES_MIGREES_PAR_LOT: ~50 (HOLTitleScreen.as 49 l.)
TYPES_A_EXPOSER: aucun de plus (UHOLButton, UCommonTextBlock, UVerticalBox, FText déjà liés)

## Fait
- `HOLTitleScreen.h/.cpp` : `UCLASS(Abstract, Blueprintable)` ; `FindScriptClass`, `ButtonLabel(FName)` (4 LOCTEXT gardés, namespace `HOLTitleScreen`, mêmes clés Play/Multiplayer/Settings/Quit), `MakeTitleColumn`, `AddTitleText(Column, Name, Text, bTitle)`, `AddTitleButton`, `AddVersionText`, `OpenNewGame/OpenMultiplayer/OpenSettings/AskQuit`, `BP_BuildTitle`. Propriétés `Title`, `Version`, 4 boutons en `BlueprintReadWrite` publiques (le `BindWidgetOptional` est retiré, comme pour la boîte de confirmation). Triple clic de triche, focus, `bIsBackHandler=false` inchangés en C++.
- `HOLMenuPlayerController.cpp` : pousse `FindScriptClass()` (log `Error` « Title screen: no script class » si nul).
- `Script\UI\HOLTitleScreen.as` (`UHOLTitleScreenScript`) : mêmes noms de widgets (Title, PlayButton, MultiplayerButton, SettingsButton, QuitButton, Version), mêmes libellés (via `ButtonLabel`), titre `Visible`, clics via `OnButtonBaseClicked`.
- `Script\UI\HOLTitleScreen_Test.as`, `Tools\Scenarios\ecran_titre.txt/.json/.look` (waitkey, `hol.UI.Open Quit`, `hol.Mouse.Click CancelButton`, `quit` : ~1 min + le temps de l'utilisateur).
- `quick_check.ps1` et `packaged_smoke.ps1` : `menu=HOLTitleScreenScript` ; `modal=HOLConfirmDialogScreenScript` (l'ancien attendu `HOLConfirmDialog` était déjà périmé depuis 47e222a : à noter, c'était rouge ou non vérifié).
- Ancien C++ : `AskToQuit` gardé (appelants : `HOLUIOpen.cpp:81` et le bouton) ; `BuildLayout`, `NativeOnInitialized`, `Handle*` supprimés dans le même commit (jamais deux sources). Autres appelants de `StaticClass()` du titre : aucun.

## Commandes après build (par la session principale)
1. Build : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait`
2. quick_check : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Tests\quick_check.ps1 -Parts tests,script,ui` (adapter les noms de parties à `quick_check.ps1 -List` ; `tests` seul en cas de doute) : doit montrer `Test_TitleScreenScriptClass` et `Test_TitleScreenLabels`, `Localization.Translations` vert, captures title/stackback/quit justes.
3. Rouge à voir une fois : dans `Script\UI\HOLTitleScreen_Test.as` remplacer `"Jouer"` par `"Jouez"` (ligne `PlayButton`), relancer le quick_check (compte N-1/N), remettre `"Jouer"`, relancer (vert). Faire dans le worktree Galadriel, jamais dans la copie principale.
4. Preflight : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\preflight.ps1 ecran_titre` (PNG à ouvrir : `ecran_titre_menu`, `ecran_titre_quitter`).
5. Essai agent : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 ecran_titre -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -TimeoutSec 180`
6. Essai utilisateur (après E-ecran-titre.md [À ESSAYER]) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 ecran_titre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`

## NON VÉRIFIÉ
- Compilation C++ (UHT : `UFUNCTION` statique `BlueprintPure` avec `FName` par valeur ; `TObjectPtr` des 6 UPROPERTY publics) et Angelscript (noms `BuildTitle` / `HandleButtonClicked`, `FText::FromString`, `Title.SetVisibility(ESlateVisibility::Visible)`, `Button == PlayButton` entre `UCommonButtonBase` et `UHOLButton`).
- Que le texte « Réglages » reste lu identiquement par `Localization.Translations` (clé et espace de noms inchangés, fichier écrit en UTF-8 sans BOM comme avant).
- Que le scénario reste sur le titre pendant `waitkey` (modèle `sites_option` : `open /Game/Maps/MainMenu` + `waitui` ; preflight pas lancé) ; que `-Root <worktree>` écrit ses réglages dans le worktree.
- Les captures title/stackback/quit de quick_check : seul l'attendu a été changé, jamais exécuté.
- Pas de test sur un écran instancié (aucun test script ne crée de widget) : l'effet est couvert par les captures du quick_check et par l'essai.

## ANOMALIES
- `eol_check -Fix` a « corrigé » `Tools\Audio\check_audio_soft.py` (aucun diff git).
- Écritures : fins de ligne LF dans les fichiers écrits (git avertit, CRLF au prochain passage).

PROCESSUS_DETACHES: aucun processus détaché pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: `M AngelscriptInProgress.md`, `M EtapeDefini.md` (docs voulus) + `?? Docs/Priorite_Socle_Factions_2026-11-16.md` : inchangé hors docs voulus : oui
