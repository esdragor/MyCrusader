# CELEBORN-1117d : ecran REGLAGES en script (ECR-2, D840 / D839)

**NON COMPILE** : aucun build, aucun quick_check, aucun preflight, aucun scenario joue. Tout ce qui suit est ecrit, relu, jamais execute.
Raison du C++ (D839) : base d'ecran + liaisons a exposer (plan PLAN-ECRANS-1117, 2.2).

```
APPELS_DECLARES: 21 (arret a 45 vus par toi ; le seuil de 80 se juge sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 09:34:58 / 09:41:41
PREMIER_BUILD: non lance (build a la session principale)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build ; binaire du worktree non verifie)
TEST_ROUGE: aucun rouge VU (pas de build) ; tests ecrits 744d27c ; assertit l'effet : oui pour Test_SettingsVolumeEffect (valeur relue sur UHOLGameUserSettings apres SetVolumeAt) et Test_SettingsQualityChoice (ResolveQualityChoice calcule par le C++), non pour les comptes de OptionsOf (structure)
COMMITS: 2 (744d27c lot ; b0eec22 quick_check), dernier 09:41, merge_main_fait: non (merge main non fait, DLL pÃ©rimÃ©e ; branche prete)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 attendu (le test "autre volume intact" tautologique a ete retire)
ESSAIS: ecran_reglages : preflight NON LANCE, PNG non vus, LOGIQUE SEULEMENT
QUESTIONS: aucune
FAIT / RESTE: voir ci-dessous
NON VERIFIE: tout (compilation C++ et AS, noms AS des UFUNCTION, comportement des rotateurs, apparence identique)
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: M AngelscriptInProgress.md, Auto_Journey_Jour.md, EtapeDefini.md, ?? Docs/Priorite..., Docs/Rapports/ARWEN-1117d.md, GALADRIEL-1117d.md : docs seulement (aucun code) : oui
BUILDS_DEMANDES: 1 (lot C++ unique)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 4 (Test_SettingsScreenScriptClass, Test_SettingsOptions, Test_SettingsVolumeEffect, Test_SettingsQualityChoice)
LIGNES_CPP_SUPPRIMEES: ~200 nets dans HOLSettingsScreen.cpp (594 -> ~580 mais BuildLayout, NativeOnInitialized, 11 Handle*, Refresh, ApplyAndRefresh ne sont plus en C++ ; les LOCTEXT et l'affichage r.SetRes restent)
LIGNES_MIGREES_PAR_LOT: ~260 (HOLSettingsScreen.as, 270 l.)
TYPES_A_EXPOSER: UHOLSettingsScreen (UCLASS Abstract Blueprintable + ~30 UFUNCTION), UHOLGameUserSettings (Get, IsEdgeScrollEnabled, SetEdgeScrollEnabled, GetVolumeAt, SetVolumeAt, GetCameraSpeedIndex, SetCameraSpeedIndex)
```

## Fait (branche hero-j8-close, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn)
- `Source\HeroesOfLibertyUI\Screens\HOLSettingsScreen.h/.cpp` : base `UCLASS(Abstract, Blueprintable)`. Gardes : `Open` (pousse `FindScriptClass()`, Warning + nullptr sans classe), `Find`, `SelectTab`, `GetColumns`, `NativeGetDesiredFocusTarget`, LOCTEXT (`TextOf(FName)`, `OptionsOf(FName)`), affichage de la fenetre (`GetResolutionNames`, `GetShownResolutionIndex`, `GetShownWindowMode`, `ChooseResolution`, `ChooseWindowMode`), index cadence / echelle / qualite / groupes / langue (statiques), `ResolveQualityChoice`. Aides : `MakeSettingsColumn`, `AddSettingsText/Row/Button/Tab/PageButton/Pages/Page/Pair/PairColumn/Hint/Rotator`. `BP_BuildSettings`, `BP_RefreshSettings`, `BP_SettingChanged(Name, Value)`.
- Choix : au lieu d'un delegue dynamique sur `UHOLRotator` (plan), `AddSettingsRotator` lie le rotateur natif (`OnRotatedEvent`, lambda faible) et appelle `BP_SettingChanged(Name, Value)` : aucun changement de `HOLRotator.h/.cpp`. Meme comportement que l'ancien lambda (le booleen est ignore, le handler s'execute pour tout changement).
- `Source\HeroesOfLiberty\Settings\HOLGameUserSettings.h` : `UFUNCTION` sur `Get`, `IsEdgeScrollEnabled`, `SetEdgeScrollEnabled` ; ajout de `GetVolumeAt/SetVolumeAt(int32)` et `GetCameraSpeedIndex/SetCameraSpeedIndex(int32)` (evite d'exposer les enums `EHOLVolume`, `EHOLCameraSpeed`).
- `Script\UI\HOLSettingsScreen.as` (`UHOLSettingsScreenScript`) : meme arbre de widgets et memes noms (`Heading`, `VideoTab`..`LanguageTab`, `TabPages`, `ResolutionRotator`..`EdgeScrollRotator`, `Quality_<groupe>`, `Volume_<Master|Music|Effects|Voice>`, `RemapButton`, `SoundNote`, `ControlsNote`, `BackButton`), memes largeurs, meme logique d'application (`ApplyAndRefresh(bResolutionChanged)` : `ApplyResolutionSettings(false)` seulement pour resolution / mode fenetre).
- `Script\UI\HOLSettingsScreen_Test.as` : 4 tests. `Tools\Scenarios\ecran_reglages.txt/.json/.look` : titre -> clic reel `SettingsButton` -> 4 onglets (clics reels) -> `waitkey` -> `BackButton`.
- `Tools\Tests\quick_check.ps1` (4 attentes) : `menu=HOLSettingsScreen` devient `menu=HOLSettingsScreenScript` (le journal imprime le nom brut de la classe). Peut conflictuer avec la branche Galadriel (meme bloc `$Expected`, lignes 3137-3139) : fusion a la main, une ligne.
- Ligne `EtapeDefini.md` ajoutee (643). Pas de journal `*InProgress.md` dedie (aucun pour les ecrans) : le detail est ici.

## Reglages de l'utilisateur pendant l'essai (verifie)
`play.ps1` lance `UnrealEditor.exe <worktree>\HeroesOfLiberty.uproject -game` sans `-ConfigDir` ni dossier utilisateur : `GameUserSettings.ini` s'ecrit dans `<-Root>\Saved\Config\WindowsEditor\` (existe : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Saved\Config\WindowsEditor\GameUserSettings.ini`, 2026-11-16 15:32). La copie de l'utilisateur (`C:\Users\baudo\Documents\MyCrusader\Saved\Config\WindowsEditor\GameUserSettings.ini`, 2026-11-06) n'est pas touchee ; `%LOCALAPPDATA%\HeroesOfLiberty` n'existe pas. Donc `play.ps1 ecran_reglages -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` ne modifie PAS les reglages de la copie principale, a condition de bien passer `-Root` (sans `-Root`, c'est la copie principale qui serait modifiee). La langue (`[Internationalization]`) y va aussi ; `-culture=fr` est force au lancement.

## Commandes apres build (session principale)
1. `powershell -NoProfile -Command ". C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\LaunchCheck.ps1; Get-StaleBinaryProblem C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn"` (vide attendu apres build)
2. `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`
3. Rouge vu : fausser une valeur (ex. `"Volume"` ... `, 11,` -> `, 12,` dans `Test_SettingsOptions`, ou `ResolveQualityChoice(2), 2` -> `3`), `Tools\Tests\quick_check.ps1 -Parts tests` : compte N-1/N, puis remettre. (Le test C++ `HeroesOfLiberty.Settings.Volumes` et `Localization.Translations` doivent rester verts.)
4. `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\preflight.ps1 ecran_reglages` puis ouvrir chaque PNG du `.look`.
5. Essai (apres preflight PASS) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 ecran_reglages -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (agent : ajouter `-Hidden`).
6. Puis `quick_check -Parts video,sound,controls,language` (les captures qui verifient `menu=HOLSettingsScreenScript`) si ces parties existent.

## Reste / risques de compilation a surveiller
- UHT : `UPROPERTY(BlueprintReadWrite, meta=(BindWidgetOptional))` sur `TObjectPtr<UHOLRotator>` / `UHOLButton` (declares en avant) ; fonctions `static` en `BlueprintPure`/`BlueprintCallable` ; `TMap<FName,FText>` initialise par liste dans `TextOf`.
- Angelscript : noms attendus (`BP_X` devient `X` comme pour la banniere), `Math::RoundToInt`, `FName("Quality_" + Name.ToString())`, `UHOLGameMenuScreen::MenuLog`, `UHOLRotator.SetSelectedItem(int)`, `UHOLGameUserSettings.ApplyResolutionSettings(false)` / `ApplyNonResolutionSettings()` / `SaveSettings()` / `IsVSyncEnabled()` / `SetVSyncEnabled()` / `SetOverallScalabilityLevel()` (UFUNCTION du moteur supposees liees) ; si une ne l'est pas, ajouter un enrobage dans la base (une ligne).
- Le test Test_SettingsVolumeEffect n'ouvre pas l'ecran (pas de menu en test) : il verifie la chaine reglage seulement ; l'effet rotateur -> reglage est a voir a l'essai (note 2 du scenario). Un test d'ecran ouvert (SelectTab(2) -> `GetActiveTab()==2`) reste a ecrire quand un ecran peut etre ouvert en test.
- Apparence identique : non verifie (largeurs et paddings recopies, `AddSettingsPageButton` garde le bouton Remap dans la colonne comme avant).
- Placeholders.txt : aucun provisoire ajoute ni retire (relire a la cloture de l'ecran).
- Rien fusionne dans main, rien pousse. Fichiers non suivis `Tools/Scenarios/c28_*`, `c31_*` non ajoutes.
