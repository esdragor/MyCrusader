# Angelscript in progress

Préparation des deux questions que D483 laisse « à trancher avec le premier fichier en script » :

- **AS-Q1** : tests en Angelscript, ou tests d'automatisation C++ qui appellent le script ?
- **AS-Q2** : scripts précompilés dans le paquet du lanceur ?

Fait sans compiler ni lancer Unreal (le moteur Hazelight 5.8.4 recompile pendant ce temps, 2026-10-26).
Rien n'est tranché ici : les questions sont posées à l'utilisateur une à la fois (options A / B / C, recommandation).

## Suivi

- 2026-10-26 : choix de l'utilisateur parmi les tâches faisables sans Unreal (« préparer Angelscript »). Deux
  recherches en lecture seule lancées : sources Hazelight (`C:/UE/Src/HZ`) et côté projet (tests, paquet, lanceur).
- 2026-10-26 : côté projet revenu (faits ci-dessous, « Côté projet »).
- 2026-10-26 : côté Hazelight revenu (« Côté moteur ») ; plugin `Angelscript.uplugin` `EnabledByDefault: true`, donc
  déjà chargé par le projet. Questions AS-Q1 et AS-Q2 rédigées (options, recommandation) ; **AS-Q1 posée à
  l'utilisateur**, en attente. Rien committé.
- 2026-10-26 : **AS-Q1 = A** (choix de l'utilisateur, conforme à la recommandation). AS-Q2 posée.
- 2026-10-26 : **AS-Q2 = B** (choix de l'utilisateur ; recommandation : A puis B). Les deux reportées au GDD en
  **D484** (compteur à D485 ; la session IA travaillait aussi dans GDD.md et EtapeDefini.md de la copie principale).
  Reste : rien à coder avant le premier fichier en script ; à ce moment, le test rouge exprès et l'étape du cache.
- 2026-10-27 : moteur AS_3 prêt. **AS1 (plomberie des tests en script)** dans le worktree Elrond, branche
  `angelscript-first` (depuis main 85508be) :
  - `Script/Tests/HOLScriptSmoke_Test.as` : `Test_SeesProjectClasses` (le script voit `AHOLBuilding`) ;
  - `quick_check` : filtre `HeroesOfLiberty+Angelscript.UnitTests` ; contrôles « scripts compiled » (3 premières
    lignes d'erreur `Angelscript: Error:` du journal, tests et jeu) et « script tests ran (N) » > 0 ;
  - `-as-exit-on-error` sur tous les lancements automatiques de `Tools/Tests/*.ps1` (pas `play.ps1`) : sans lui, un
    script qui ne compile pas ouvre au démarrage la fenêtre « corrigez les erreurs » et l'éditeur attend (vu : 421 s
    puis délai dépassé, « tests 0/0 ») ; avec lui, il quitte tout de suite.
  - Rouges vus avant le vert : assertion fausse exprès → `tests 187/188 FAIL`, le test en script nommé ; fichier qui
    ne compile pas → `scripts compiled` KO avec fichier, ligne et message, côté tests et côté jeu, `script tests ran
    (0)` KO. Vert : `quick_check` par défaut PASS en 37 s, tests 187/187 (186 C++ + 1 script).
  - Notes : `StaticClass()` est déprécié en script (le type sert de valeur : `AHOLBuilding`) ; `FUnitTest` existe hors
    éditeur, les `*_Test.as` compilent donc aussi dans le jeu empaqueté (assertions inertes) ; l'extension VS Code
    Angelscript se connecte au port de débogage de chaque éditeur lancé (sans effet sur les tests). Deux départs de
    jeu calés > 120 s sur `zen.exe service status` pendant que trois autres copies lançaient leurs tests : charge de
    la machine, rien du script.
  - Attend la validation de l'utilisateur pour committer. Suite : AS2 = cache précompilé au paquet (D484), puis le
    premier vrai fichier en script (question à poser).
- 2026-10-27 : AS1 validé, commité, dans main (806ae43). **AS2 (cache précompilé au paquet)** en cours dans Elrond :
  - `Tools/Package/precompile_scripts.ps1 -PackageDir` : lance l'exe du jeu (`Windows\`) puis celui du serveur
    (`WindowsServer\`) avec `-as-generate-precompiled-data` ; vérifie le `Script\PrecompiledScript.Cache` écrit, sans
    erreur de script ; retire ce que le lancement laisse (`Saved\` s'il n'existait pas, `AS_JITTED_CODE\`).
    `package.ps1` l'appelle après `Version.txt` (étape 5b).
  - `packaged_smoke.ps1` : exige « Using fully precompiled scripts » et aucune `Angelscript: Error:`.
  - `package.ps1` : UAT relancé toutes les 20 s (60 min au plus) quand UnrealBuildTool est déjà pris
    (`Failed (ConflictingInstance)`, exit 10, vu deux fois ce matin : la compilation d'Arwen, lancée en chemins
    relatifs `dotnet ..\..\Engine\...`, échappe à l'attente de l'étape 3). `-ubtargs=-WaitMutex` essayé : sans effet
    (il va dans chaque cible, le verrou est pris avant).
  - Ordre prévu : paquet sans cache → `packaged_smoke` rouge (seul défaut : scripts non précompilés) →
    `precompile_scripts` seul → `packaged_smoke` vert → paquet complet avec l'étape 5b → vert.
  - Vu (paquet 2026-10-27-806ae43, sans cache, 13 min) : `packaged_smoke` rouge, « scripts not precompiled » ;
    le jeu compile alors ses scripts au démarrage en ~0,4 ms (un seul petit fichier). `precompile_scripts` seul :
    jeu et serveur, cache de 0,9 Ko en 7-8 s chacun ; `packaged_smoke` vert (30 s) ; le serveur empaqueté démarre lui
    aussi sur son cache (« Using fully precompiled scripts », lancé 40 s puis arrêté).
  - Trouvé en route : (1) le jeu et le serveur empaquetés écrivent au démarrage 12 erreurs de liaison du moteur
    (`RegisterObjectMethod` sur des délégués de test `FTestDynamicDelegateFloat`… dont le paramètre s'appelle `float` ou
    `int64`, mots réservés du script), sans effet ; `Tools/Package/script_errors.ps1` ne retient donc que les erreurs
    des fichiers `.as` (vérifié sur un faux journal : l'erreur d'un `.as` sortie, celles des liaisons ignorées).
    (2) Le lancement `-as-generate-precompiled-data` sort en code 1 : il quitte pendant le démarrage du moteur et
    l'arrêt du plugin PCG échoue alors sur un `ensure` (« Attempted to unregister a data class that was not
    registered »), cache déjà écrit ; le critère est donc « cache écrit, aucune erreur de `.as` ».
  - main fusionné en avance rapide (e36efe9 : IA1.1, J8a.1) ; paquet complet relancé avec l'étape 5b.
  - Paquet complet 2026-10-27-e36efe9 : **PASS en 34 min** (10 min d'attente d'UnrealBuildTool pris ailleurs, la
    relance a marché ; compilation complète 13,6 min après la fusion ; cuisson ~15 min dont 10 au ralenti, machine
    partagée). Étape 5b : jeu 12 s, serveur 7 s, cache 0,9 Ko chacun. `packaged_smoke` PASS (10 s).
    `quick_check` sur le code fusionné : tests 193/193 PASS, smoke PASS (relancé : deux départs calés sur le
    redémarrage de zenserver, arrêté avec la fin du paquet).
  - **À savoir pour les prochains paquets** : tant que BuildCookRun tourne, il tient le verrou d'AutomationTool, et
    tout éditeur ou jeu lancé dans une autre copie attend au démarrage (Turnkey `-WaitForUATMutex`) : 30 min de
    blocage pour les autres sessions ce midi. Un paquet complet se lance donc machine calme, ou en prévenant les
    autres sessions. Pour vérifier une étape d'après UAT, la lancer seule sur le paquet existant.
  - Validé par l'utilisateur, commité.
- 2026-10-27 : AS2 dans main (a365967 ; avance de main faite à la main dans la copie principale : `EtapeDefini.md` et
  `packaged_smoke.ps1` y portaient des changements non commités d'autres sessions, gardés). **AS-Q3 posée et tranchée :
  A** (ci-dessous). Chantier en veille : rien à coder ici ; Elrond libéré.
- 2026-10-28 (par J8a.5, aa0f11f, `JourneyInProgress.md`) : **premier écran d'interface en script**, recette pour la
  migration du HUD au fil des chantiers (choix de l'utilisateur J8a5-Q3 = C, Q3b = C). (1) **`HeroesOfLibertyUI` en
  Runtime** (était ClientOnly) : le serveur dédié compile tous les scripts au démarrage (`-as-exit-on-error`) et ne
  connaissait pas les classes d'interface ; contrôle « server: scripts compiled » dans la partie `net`. (2) Base C++
  **`UHOLScriptScreen`** (`HUD/HOLScriptScreen.h`, `UUserWidget` abstrait : `Describe`, `ClickWidget`, `Refresh`,
  `MakePanel`, `MakeText`, `MakeButton`, `MakeBar`, couleurs), créée par `UHOLGameScreen` au-dessus de son cadre ; un
  widget en script construit son arbre dans `OnInitialized` (`ConstructWidget`, `SetRootWidget`). (3) Statiques d'une
  classe sans suffixe « Library » : `UHOLScriptScreen::X()` en script. (4) **Textes : `NSLOCTEXT` littéraux en
  script**, GatherText lit `Script/*.as` (`Config/Localization/Game.ini`), puis `fill_po.py` comme pour le C++. (5)
  `LoadObject(nullptr, Chemin)` pour une texture. À vérifier au prochain paquet : cible Server qui lie l'interface,
  cache précompilé WindowsServer.

### AS-Q3 — Que fait le chantier après AS1 et AS2 ? (tranchée le 2026-10-27 : **A**)

- **A — Rien à migrer maintenant** : le premier code neuf en script vient d'IA0.4 (cerveau de l'IA, `AIInProgress.md`)
  et de J8a.4 (mort et retour du héros, `JourneyInProgress.md`) ; un fichier C++ ne passe en script que quand un
  chantier va beaucoup le toucher (option D de D483), et la recette de migration s'écrit à ce premier cas réel.
- B — Migrer un petit écran (`HOLEndBanner`, 215 lignes, 2 tests) comme essai : recette de migration d'avance, sans
  gain pour le jeu.
- C — Migrer `HOLBuilding` (~1 240 lignes) : vrai cas, mais long et en travers d'IA1 et du héros.
- **Choix de l'utilisateur : A**, conforme à la recommandation (pas de travail sans usage).

## Côté projet (relevé du 2026-10-26)

- **Tests** : 176 tests d'automatisation C++, tous `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, préfixe `HeroesOfLiberty.`,
  24 fichiers (`Source/HeroesOfLiberty/Tests/HOL*Tests.cpp`, ~10 400 lignes, + `HOLManagerErrorTextTests.cpp`).
  Surtout des règles pures (`FHOLStartRules`, `HOLConstructionRules`, `HOLForumRules`), parfois un monde jetable
  (`UWorld::CreateWorld`). Ces règles restent en C++ (D483), leurs tests aussi.
- **Lancement** : `quick_check.ps1` partie `tests` = `UnrealEditor-Cmd.exe -ExecCmds="Automation RunTests
  HeroesOfLiberty;SoftQuit" -nullrhi ...` ; verdict par la regex `Test Completed. Result={..} Path={..}` et le compte
  `Test Started` = `Test Completed`. Les autres parties passent par `hol.Test.Run <séquence>` (C++,
  `Dev/HOLTestSequence.cpp`) et la lecture des journaux.
- **Angelscript dans le projet** : aucun `.as`, pas de plugin au `.uproject`, aucune dépendance `AngelscriptCode`
  dans les `Build.cs`, rien dans `Config/`. `Script/` ne contient que `Binds.Cache*` (généré, ignoré par git).
- **Paquet** : `Tools/Package/package.ps1` = `BuildCookRun ... -server -build -cook -stage -pak -archive`,
  **Development** (pas de Shipping). `publish.ps1` publie `Windows\` seulement ; manifeste = tous les fichiers (sauf
  `*.pdb`) hachés, un correctif HDiffPatch **par fichier**. Aujourd'hui : un changement de code = un correctif sur l'exe
  seul (~8 Mo) ; une table = 46 octets sur le `.ucas`. Des fichiers de script à part seraient pris tels quels par
  `HOLPublish`, sans rien y changer.
- **Premiers fichiers possibles** : `HOLPlayerController.cpp` (4 672 lignes, 49 commits depuis le 1er octobre, le
  plus touché) et `HOLGameScreen.cpp` (2 772, 24) sont gros ; `HOLBuilding.cpp/.h` (~1 240 lignes, 20 commits) est le
  premier réaliste ; un petit écran (`HOLGameMenu` 284, `HOLEndBanner` 215, déjà testés : `Victory.GameMenu` 5 tests,
  `Victory.EndBanner` 2) serait l'essai le moins cher.

## Côté moteur (sources Hazelight 5.8.4, relevé du 2026-10-26)

Plugin : `C:/UE/Src/HZ/Engine/Plugins/Angelscript/Source/AngelscriptCode/Private` (noté AC). Pas de documentation
dans les sources ; tout vient du code.

### Tests

- **Tests en script** : découverts par leur nom (`AC/Testing/DiscoverTests.cpp:166-203`) :
  `void Test_<Nom>(FUnitTest& T)`, `ComplexUnitTest_<Nom>` (+ `_GetTests`) pour les tests à paramètres,
  `IntegrationTest_<Nom>(FIntegrationTest& T)` pour les tests sur une carte (PIE, serveur + client par défaut).
  Assertions `AssertTrue`, `AssertEquals`, `AssertAlmostEquals`… ; aides `AdvanceTime`, `SetIsServer`,
  `AddExpectedError`, `CallNativeBeginPlayFor`. Chaque test unitaire a son monde neuf, sans carte.
- **Éditeur seulement** : découverte sous `WITH_EDITOR`, assertions inertes hors éditeur. Ni le jeu empaqueté ni le
  serveur ne lancent ces tests (comme nos tests C++ aujourd'hui, lancés par `UnrealEditor-Cmd`).
- **Même lanceur que nos tests** : deux tests d'automatisation complexes, `Angelscript.UnitTests` et
  `Angelscript.IntegrationTests` (`UnitTest.cpp:635`, `IntegrationTest.cpp:794`), un sous-test par fonction
  (`<Module>.<Fonction>`). Un échec d'assertion écrit une erreur que le cadre d'automatisation compte comme échec.
  `Automation RunTests HeroesOfLiberty+Angelscript.UnitTests` devrait donc passer dans `quick_check` sans autre
  changement que le filtre (à vérifier : la regex `Test Completed` et le compte `Started` = `Completed`).
- **Nom de fichier imposé** sur ce chemin : un test unitaire doit vivre dans un module `*_Test` (fichier
  `Script/.../Bidule_Test.as`), un test d'intégration dans `*_IntegrationTest`, carte sous `/Game/Testing/<fonction>`
  (`Engine/Config/BaseEditor.ini:699-708`, `UAngelscriptTestSettings`).
- **Commandlet** `-run=AngelscriptTest` : tests unitaires seuls, sans filtre ; **risque de faux vert** (découverte
  après le scan du registre d'assets, que ce commandlet ne demande pas : 0 test trouvé, code 0). À ne pas utiliser
  sans un test rouge exprès.
- **Au rechargement à chaud** : les tests unitaires des modules recompilés se relancent seuls dans l'éditeur
  (`bRunUnitTestsOnHotReload`, vrai par défaut).
- **C++ qui appelle le script** : les classes de script sont de vraies `UClass` (`/Script/Angelscript.<Nom sans
  préfixe>`), trouvées par `FindObject` ou `FAngelscriptManager::Get().GetClass(...)` (dépendance `AngelscriptCode`
  au module de tests) ; appel par `ProcessEvent`, ou mieux par un `BlueprintNativeEvent` déclaré en C++ et redéfini en
  script. Au rechargement complet l'ancienne classe est renommée `<Nom>_REPLACED_N` : ne pas garder de `UClass*`.
- **Couverture** : `-as-enable-code-coverage` écrit un rapport HTML dans `Saved/CodeCoverage/` (pas en Shipping).

### Scripts dans le paquet

- **Hors éditeur** (jeu et serveur, même chemin) : le jeu cherche `Script/PrecompiledScript_<Development|Test|
  Shipping>.Cache`, puis `PrecompiledScript.Cache` (`AngelscriptManager.cpp:441-450`). **Sans cache, il compile les
  `.as` au démarrage** : c'est un mode prévu. Avec un cache valide, les `.as` ne sont pas lus.
- **Mise en paquet** : le moteur copie déjà tout `Script/` en fichiers libres, hors `.pak`
  (`Engine/Config/BaseGame.ini:141-143`). Les dossiers `Dev`, `Developers`, `Editor`, `Examples` sont ignorés hors
  éditeur.
- **`Binds.Cache` obligatoire** hors éditeur (erreur fatale s'il manque ou s'il est périmé,
  `AngelscriptBindDatabase.cpp:104-116`) ; écrit dans `Script/` du projet par tout commandlet, la cuisson comprise.
  Il ne change que quand le C++ change. Notre `package.ps1` fait `-build -cook` dans l'ordre : il est donc à jour.
- **Cache précompilé** : lancer l'exe empaqueté (même configuration) avec `-as-generate-precompiled-data` ; il
  écrit `Script/PrecompiledScript.Cache` puis s'arrête. Le cache ne vérifie que la configuration (pas la cible ni
  l'exe) : le refaire à chaque paquet, et pour le serveur avec l'exe du serveur (déduction, non essayé).
- **Script traduit en C++** (« static JIT ») : produit dans le même lancement (`AS_JITTED_CODE/`), mais **rien ne le
  compile tout seul** (à brancher à la main dans un module), et chaque changement de script oblige à recompiler l'exe
  (sinon le code traduit est ignoré, avertissement au démarrage).
- **Correctifs du lanceur** : `.as` bruts = seuls les `.as` changés ; cache sans JIT = ce seul fichier ; JIT = l'exe
  et le cache à chaque changement de script.
- **Serveur de débogage** : en Development sans cache, le jeu écoute le port TCP 27099 (`ConnectionPort`,
  `-asdebugport=`) ; absent en Test et Shipping. À garder en tête pour le serveur dédié et la box.

## Questions

### AS-Q1 — Où vivent les tests du code en script ? (tranchée le 2026-10-26 : **A**)

- **A — Tests en script** pour tout le code en script (`Test_*` dans des fichiers `*_Test.as`), lancés par le même
  `Automation RunTests` que les tests C++ (filtre élargi à `Angelscript.UnitTests`). Les règles restées en C++
  gardent leurs tests C++.
- **B — Tests C++ seulement** : les tests restent dans `Source/.../Tests/`, et appellent le script par les classes
  trouvées par nom ou des `BlueprintNativeEvent`. Un seul langage de test, mais chaque test dépend de
  `AngelscriptCode`, des noms de classes et du rechargement ; on perd les tests relancés seuls au rechargement.
- **C — A, plus tests d'intégration en script** (`IntegrationTest_*`, PIE sur une carte de `/Game/Testing/`) à la
  place des séquences `hol.Test.Run` pour les nouveaux cas de jeu. Deux mécanismes pour le même besoin, cartes de test
  à créer, fonctionnement en `-nullrhi` non vérifié.
- **Recommandation : A.** Le test vit dans le langage du code testé ; le lanceur et la lecture des résultats restent
  ceux de `quick_check` ; les séquences `hol.Test.Run` et les scénarios restent tels quels. Au premier fichier : un
  test en script rouge exprès doit rendre `quick_check` rouge (règle du test rouge d'abord), puis on le corrige.

### AS-Q2 — Quels scripts dans le paquet du lanceur ? (tranchée le 2026-10-26 : **B**)

- **A — `.as` bruts** : rien à ajouter (le moteur les copie déjà, `HOLPublish` les prend tels quels) ; correctifs
  minuscules ; prix : compilation des scripts à chaque démarrage du jeu et du serveur, et sources lisibles par les
  amis (sans enjeu au prototype).
- **B — Cache précompilé sans JIT** : une étape après `package.ps1` lance l'exe du jeu puis celui du serveur avec
  `-as-generate-precompiled-data` (sans fenêtre), et `packaged_smoke.ps1` vérifie le message « Using fully precompiled
  scripts » ; démarrage plus rapide, un seul fichier par cible dans les correctifs.
- **C — B plus script traduit en C++** : script plus rapide, mais l'exe change à chaque script (correctif de ~8 Mo au
  lieu de quelques Ko) et compilation à brancher à la main ; seulement si les mesures S1 à S7 accusent le script.
- **Recommandation : A tout de suite, B dès que le temps de compilation au démarrage se voit** (à mesurer au premier
  paquet avec du script : ligne de temps dans le journal, `packaged_smoke.ps1`) ; C écarté jusqu'à ce qu'une mesure le
  demande.
## Pilote 2 de la migration D830/D835 : HOLGameMenu (GALADRIEL-1116m, 2026-11-16 soir)
- MIG-P2a/P2b (sans C++) : Script/UI/HOLGameMenuRules.as (jumeau temporaire des 5 règles CanSurrender, CanQuitToMenu, OffersReturnToLobby, QuitsToList, ShowsStatsButton ; énumérations EHOLGameMenuBanner/EHOLGameMenuOrigin, copies valeur pour valeur des enum class C++ non UENUM) et Script/UI/HOLGameMenuScript.as (vue pure du menu : boutons montrés, libellés aux clés C++, Click -> EHOLGameMenuAction, ligne Describe) ; 7 tests Test_GameMenu* rouges vus à 20:21 (1326/1335) contre bouchons. Les 7 tests C++ de HOLGameMenuTests.cpp RESTENT (le C++ appelle encore les règles). AssertEquals refuse une énumération de script : comparer int(...).
- Relevé des liaisons (aucune UFUNCTION dans HOLScreen.h, HOLRootLayout.h, HOLButton.h, HOLConfirmDialog.h, HOLEndBanner.h, HOLGameHUD.h, HOLManagerClient.h, HOLUIInputData.h) : le widget du menu en script ne peut ni se construire (BuildLayout, MakeDialogColumn, AddText, AddButton non exposés ; UHOLButton::SetLabel non exposé), ni s ouvrir (UHOLRootLayout::Get/PushMenu/PushModal/GetActiveMenu/HasMenuOtherThan/IsOverMatch), ni agir (AHOLPlayerController::RequestSurrender/SetGameMenuOpen/GetHOLPlayerState, AHOLGameState::IsSlotDefeated/IsMatchOver/IsMatchPaused/GetDefeatedMask/GetWinnerMask, AHOLPlayerState::GetPlayerIndex, UHOLManagerClient::GetMatchOrigin/ForgetLobby/SetAfterMatch + EAfterMatch imbriquée, UHOLEndBanner::LeaveMatch, AHOLGameHUD::ReopenEndBanner, UHOLUIInputData::GetMappingContext), et UHOLConfirmDialog::SetUp prend une TFunction (impossible en script : il faut un délégué dynamique). Donc MIG-P2c (bascule) passe au lot C++ de fin de séance, sans build ce soir.
- MIG-P2d (sans C++) : Script/UI/HOLEndBannerScript.as (jumeaux temporaires GetEndBanner et ShouldOpenEndBanner, vue de la bannière de fin : titre, Continuer, Retour au salon, Quitter liste/menu, Statistiques) ; 3 tests Test_EndBanner* (vecteurs de Victory.EndBanner.Which/.Closed) rouges vus à 20:31 (1333/1338), verts à 20:34 (1336/1338 ; 2 rouges Nomad D831 de main). Même besoin de liaisons que le menu pour le widget ; HOLEndBanner.cpp/.h restent (215 + 80).

## Socle étape 1 de la migration D830/D835 : S1b + S1c réduits à D836 (SOCLE-1116a, 2026-11-16 21:30, worktree Celeborn, branche hero-j8-close)
- Exposé (S1b) : `FHOLResources` (USTRUCT BlueprintType, 4 champs BlueprintReadWrite, `Match\HOLResources.h`) ; `FHOLMarketRow` (BlueprintType, 10 champs BlueprintReadWrite, `Match\HOLMarketTypes.h`). PAS `FHOLTerrainZone` (Terrain reste C++ seul, D836 (1)).
- Exposé (S1c) : `UHOLSiegeCheckLibrary` neuf (`Siege\HOLSiegeCheckLibrary.h/.cpp`, script `HOLSiegeCheck::`) : Check, FindEngine, DescribeBlock (+ GetTier, InPrototype, GetHealthStat, GetDamageStat : tout `HOLSiegeRules.h`, pour que la copie script disparaisse) ; numéros du script (moteur = `EHOLSiegeEngine`, blocage = `HOLSiege::Available`...).
- `UHOLDiplomacyReadLibrary` étendue (script `HOLDiplomacyRead::`) : struct `FHOLDiplomacyStances` (Slots, bLocked, Stances, bAI, Vision ; nom distinct du `FHOLDiplomacyBook` du script) + GetBook(monde, livre répliqué), NewBook, NewTeamBook, SetStance, SetVision (rendent le numéro d'`EResult`), StanceOf, Relation, IsAlly, IsAdversary, FiresAtOnce, SharesVision, CanLeaveRampart. Les fiches de changement (bAnnounce, bFormed, bBroke, Before/After) ne sont PAS exposées.
- `UHOLUnitAbilityReadLibrary` neuf (`Simulation\HOLUnitAbilityReadLibrary.h/.cpp`, script `HOLUnitAbilityRead::`) : GetRageUnitType + les 7 constantes (Duration, Cooldown, HpCostPerHit, MinHp, StopHp, DamagePercent, RatePercent) + CanCastRage, GetRageHitCost, GetRageIntervalMs.
- Lignes C++ : ~+330 (2 headers changés 17 l., 4 fichiers neufs ~190 l., Diplomacy h/cpp ~+150). Aucun changement de comportement (enveloppes).
- Tests script (`Script\Tests\HOLSocleLibraries_Test.as`) : Test_SiegeCheckLibraryVectors (vecteurs Siege.Tiers + accord C++/script), Test_DiplomacyReadLibraryVectors (vecteurs Diplomacy.Rules), Test_UnitAbilityReadLibraryAgreesWithTable (constantes contre UnitAbilities.csv / chaîne de moral Effects.csv + accord), Test_CoreTypesVisibleFromScript (visibilité seule).
- Suivant (lot sans build) : rediriger l'IA et l'interface sur ces bibliothèques, puis `git rm` des copies script (HOLSiegeRules.as, HOLDiplomacyRules.as, HOLUnitAbilityRules.as côté règles) ; tests C++ gardés.
- État : build PASS 21:48 (632 s) ; rouge VU 21:50 tests 1352/1358 (4 valeurs faussées, une par test) ; vert 21:56 tests 1356/1358 (seuls NomadFirstForum/NomadReplicated, D831) + net et netsig3 PASS. S1d non préparé (consigne 15 min).
- S1d (S1D-1116b, 2026-11-16 22:1x, commit dcf1702, branche hero-j8-close, PAS DE BUILD, C++ oui) : 5 bibliotheques de lecture ecrites, enveloppes seules : `UHOLStartRulesReadLibrary` (script `HOLStartRulesRead::`, Match\HOLStartRulesReadLibrary.h/.cpp : GetMissing, ShouldStart) ; `UHOLHUDLayoutReadLibrary` (`HOLHUDLayoutRead::`, dans le module HeroesOfLibertyUI comme son header HUD\HOLHUDLayoutRules.h : Overlaps en 2x2 FVector2D + Tolerance sans defaut, GetLowWindowHeight, IsLowWindow, CheatsStartOpen, ShouldFoldCheats ; les deux `bool&` entree-sortie deviennent la struct `FHOLHUDFoldState` {bWasLow, bWasShown, bFold} rendue) ; `UHOLVersionReadLibrary` (`HOLVersionRead::`, HOLVersionReadLibrary.h/.cpp : Get, Parse, Display, IsAccepted avec `FString& OutError`, GetDev/GetUnknown pour les constantes) ; `UHOLAccessKeyReadLibrary` (`HOLAccessKeyRead::`, Net\ : ParseServerKey, IsAccepted, MakeLoginOption) ; `UHOLStartingPeasantsReadLibrary` (`HOLStartingPeasantsRead::`, Match\ : GetKingLocation, GetStartingPeasantLocations ; la part « Count » reste C++).
- S1d : lignes C++ ajoutees 277 (5 x .h + .cpp, aucun fichier existant change) ; script `Script\Tests\HOLS1dLibraries_Test.as` neuf (88 l., 5 tests de fumee, un vecteur de chaque test C++). Il NE COMPILE qu'apres le build des bibliotheques : le chargement des scripts est rouge d'ici la, attendu. Reste de S1d : porter les tests (E3-03, E3-04, E3-07, E3-08), puis rediriger/supprimer ce qui doit l'etre ; E3-06 (Tables, ~180 l.), E3-04 « Count » et les 4 autres du releve S1D-1116a ne sont pas ecrits.
- E3-08 (GALADRIEL-1116p, c484e65, galadriel-tab) : HOLHUDLayoutTests.cpp porte en Script\UI\HOLHUDLayout_Test.as (Test_HUDLayoutOverlap, Test_HUDLayoutCheatsLowWindow), rouge vu 1346/1350 puis vert 1348/1350 ; test C++ a retirer en E3-FIN.
- E3-09 (GALADRIEL-1116q, d647a91, galadriel-tab) : HOLGameNamesTests.cpp porte en Script\UI\HOLGameNames_Test.as (Test_GameNamesLegionsNames, Test_GameNamesForgeName), rouge vu 1361/1364 puis vert 1362/1364 ; FactionNames non porte (E3-06) ; registre : LegionsLanguages -> LegionsNames (moitie fr/Dawn), ForgeKey -> ForgeName (nom seul).
- Migration E3-06 (ELROND-1117f, tiers 2c24229, sans C++, 10:26-10:38) : Test_FactionTechnologyFilterGrid + Test_FactionTechnologyOpenTo (HOLFactionTechnology_Test.as) via HOLFactionTechnologyRead ; rouge vu 1391/1394 (Legions 'contient' faussé) puis vert 1392/1394 (restent 2 rouges connus Nomad) ; HOLFactionTechnologyTests.cpp couvert en entier, test C++ NON supprimé ; branche prête.
- 2026-11-16 21:36 (HALDIR-1116n, D836, BASCULE MIG-J-Terrain) : lignes script supprimees : HOLTerrainRules.as -58/+9, HOLTerrainRules_Test.as -171/+19 (6 tests de la copie retires), commit c377240.
- 2026-11-16 21:29-21:4x ELROND-1116n (tiers, sans C++) : D836 point 6 FAIT bd64095 : HOLUnitTier::MinTierOf donne 0 au Hero, King, LightInfantry, LightInfantryNanite (comme la colonne MinTier) ; test Test_UnitTierAgreesOnEveryRowOfTheTable change (16 listees, 0 laissee de cote, 5 assertions neuves) VU ROUGE 21:32 (1351/1354), vert 21:35 (1352/1354, seuls rouges NomadFirstForum/NomadReplicated D831). D836 point 2 (Technologies par HOLTechnologyCheck::Check) NON COMMENCE (consigne 15 min) : ligne MIG-CPP-Technology de la file decrit le geste exact.
- Mesures du pilote 1, le chat (MIG-P1a-d, GLORFINDEL-1116m/n, net-playtest, 2026-11-16) : regle du chat cote serveur entierement en script (HOLChatRules.as + HOLChatBoardScript.as), le C++ garde un pont mince (UHOLChatSubsystem : lecture des sieges repliques, RPC du controleur). LIGNES C++ : 293 supprimees (HOLChatRules.cpp 133 + .h 69, HOLPlayerController.cpp 16, HOLChatTransportTests.cpp 75) contre 259 ajoutees (pont 195, controleur 22, test 42) : -34 nettes ; la regle elle-meme (202 lignes) n'existe plus qu'en script. CHANGER UNE REGLE : SANS BUILD (P1c : MaxTextLength 200 -> 1 dans le .as, le test C++ Chat.ServerBoard passe au rouge au relancement suivant de l'editeur, 27 s, aucune compilation). Tests : 2 tests C++ retires (Chat.Route, Chat.Gate), 4 tests script (P1a) + 1 test C++ du pont (Chat.ServerBoard). RPC en script : voir P1d.
- Pilote 1, RPC en script (MIG-P1d) : **PROUVE le 2026-11-17** (ELROND-1117g diagnostic, ELROND-1117h, tiers 438aa39, rpc_probe_run.ps1 -Root Elrond) : la sonde `Script/Net/HOLScriptRpcProbe.as` (UFUNCTION Server et Client ecrites en script, Iris) nait sur le serveur pour chaque client, « RpcProbe: received by the server: 'hello from the client' from HOLPlayerController_1 », le client 2 voit send=1 (rouge send=0, vert send=1). CAUSE de l'echec du 2026-11-16 21:31 et des 2 premieres passes du 11-17 : la sonde etait dans `Script/Dev/`, que le plugin saute hors editeur interactif (AngelscriptManager.cpp:825-835) ; ce n'etait ni Iris ni le sous-systeme de monde. Pont C++ conserve (HOLScriptRpcProbeBridge : la sonde est un acteur possede par le controleur, cree par PostLogin). Suite : plan `Docs/Rapports/PLAN-MIG2-S3-1117.md`.
- REGLE PERMANENTE (P1d) : tout code serveur ou de jeu en script ne va JAMAIS dans un dossier `Script/Dev`, `Script/Developers`, `Script/Examples` ou `Script/Editor` (sous-dossier a n'importe quelle profondeur) : le plugin Angelscript les saute hors editeur interactif (`-server` et `-game` de UnrealEditor.exe n'ont pas GIsEditor), sans erreur ni ligne de journal ; le code y est invisible au serveur dedie, aux clients et aux tests serveur. Meme regle dans le brief commun des agents (`Tools/AutoJourney/brief_commun.md`).
- MIG-CPP bascule GameMenu (GALADRIEL-1116n, C++, FAIT ET VERT 21:30, 444ee83) : base C++ `UHOLGameMenuScreen : UHOLScreen` (Screens/HOLGameMenuScreen.h/.cpp, 155 + 281) = ce que le script ne peut pas faire : Open/Find/AskToSurrender (dialogue à TFunction gardé en C++), SetMatchMenuInput (IMC_UI) ; liaisons UFUNCTION : FindScriptClass (modèle UHOLScriptScreen), ReadFacts (USTRUCT FHOLGameMenuFacts : slot, vaincu, fin, pause, pause libre, pauses restantes, CanPauseNow, masques, origine en int), SetButtonLabel, MenuLog, MakeMenuColumn/AddMenuTitle/AddMenuButton (enveloppes des aides protégées de UHOLScreen), AskSurrender/PauseMatch/QuitMatch/ReturnToLobby/ShowResult, événements BP_BuildMenu/BP_RefreshMenu (surchargés en script sous BuildMenu/RefreshMenu : Angelscript retire le préfixe BP_), FocusTarget. Classe de script `UHOLGameMenuScreenScript` (Script/UI/HOLGameMenuScreen.as, 110 lignes) : colonne, 6 boutons, état par HOLGameMenuScript::Make + HOLEndBannerScript::GetEndBanner, clic par HOLGameMenuScript::Click (OnButtonBaseClicked), lignes de journal gardées + « Game menu: view <Describe> ». 5 appelants basculés ; HOLGameMenu.cpp/.h supprimés (333 + 88). Choix : enveloppes et faits en int plutôt que UENUM EHOLEndBanner/EHOLMatchOrigin et UFUNCTION sur HOLScreen.h/HOLButton.h/HOLGameState.h/HOLManagerClient.h (un seul build ; contrôleur verrouillé non touché). Mesures : build PASS 333 s ; rouge vu 21:21 tests 1353/1356 (Test_GameMenuScreenScriptClass, pas de classe de script) ; vert 21:28 tests 1354/1356 (seuls Nomad D831) + net 19 parties PASS ; victory PASS 21:29 (menu ouvert, Abandonner, menu sur la bannière). Reste : ligne MIG-CPP bascule EndBanner (nouveau build) ; tests C++ Victory.GameMenu.* tant que HOLGameMenuRules a des appelants C++ ; lobby_match non rejoué (ligne « ways out lobby, list, result »).
- D836 point 1 (SAURON-1116o, 2026-11-16 21:3x, sans C++) : copies script supprimees, le C++ reste seule source : Deploy (Script/Siege/HOLDeployRules.as 252 l. + Script/Tests/HOLDeployRules_Test.as 288 l., 9 tests ; Tools/Tests/deploy_accord_test.py 117 l.) et ExplorationXP (Script/Heroes/HOLExplorationXPRules.as 126 l. + HOLExplorationXPRules_Test.as 91 l., 4 tests ; Tools/Tests/exploration_xp_accord_test.py 125 l.) ; 6 lignes values de quick_check.ps1 retirees. 757 lignes de script et 242 de python supprimees ; regles couvertes par HOLDeployRulesTests.cpp (9) et HOLExplorationXPTests.cpp (FogPaysTheHero).
- 2026-11-16 21:37-21:50 ELROND-1116o (tiers, sans C++) : D836 point 2 Technologies FAIT 8d4180c (IA et tests lisent HOLTechnologyCheck::Check, copie script supprimee, test d'accord retire) ; rouge vu 1350/1353, vert 1351/1353 ; branche prete
- 2026-11-16 22:21-22:3x ELROND-1116p (tiers, sans C++) : MIG-S1c-UnitAbility FAIT 03084e1 (HOLAIRage.as lit HOLUnitAbilityRead, HOLUnitAbilityRules.as supprime, Test_UnitAbilityCanCast/HitCostNeverKills sur la bibliotheque, Test_RageAIReadsTheLibrary neuf) ; rouge vu 1346/1349, vert 1347/1349 (2 Nomad D831) ; branche prete
- 2026-11-16 22:3x Celeborn-1116p : E3-03 fait (Test_StartRulesVectors, rouge 1347/1350 vu, vert 1348/1350) + E3-04 part Locations (Test_StartingPeasantsLocations) dans Script\Tests\HOLStartRules_Test.as ; test C++ gardes jusqu'a E3-FIN.

- 2026-11-16 22:3x S1D-1116c (Elrond, tiers, C++ oui, 7be47d9, NON COMPILE) : socle etape 1 partie 2 : 4 bibliotheques de lecture (D836) : HOLFactionChoiceReadLibrary (Match, 10 fonctions + USTRUCT FHOLClickCounter), HOLMapRulesReadLibrary (Map, 5 + USTRUCT FHOLMapInfoValue ; chemins d'asset en texte), HOLManagerErrorTextReadLibrary (UI/Net, 6 + USTRUCT FHOLManagerErrorValue + UENUM EHOLManagerErrorKind), HOLGameNamesReadLibrary (UI/HUD, 3, Row toujours nul) ; 369 lignes C++ (.h+.cpp) ; Script\Tests\HOLS1dLibraries2_Test.as (4 tests de fumee, ne compile qu'apres le build). Reste : GetMissing hors lot (ReadFailure, MakeMeBody non exposes), controles FTextInspector/.locres/CSV de MapRules et GameNames restent C++. Rapport Docs\Rapports\S1D-1116c.md.
- S1f (SAURON-1117a, visual 823943c, C++ NON COMPILE, 2026-11-17 00:2x) : 3 bibliotheques de lecture D836 (HOLRelayReadLibrary + USTRUCT FHOLRelayReadView, HOLHeroAbilityReadLibrary 14 fonctions, HOLDeployReadLibrary 14 fonctions + 3 UENUM + 3 USTRUCT) ; Script\Tests\HOLS1fLibraries_Test.as (3 tests de fumee, ne compile qu'apres le build) ; reste : build, puis port des tests L17-07/08/10. Rapport Docs\Rapports\SAURON-1117a.md.
- 2026-11-16 22:3x Celeborn-1116q : E3-07 fait (HOLVersion_Test.as : Test_Version, Test_VersionDisplay, Test_VersionCheck, Test_AccessKey ; 4 valeurs faussees = 1348/1354 rouge vu, vert 1352/1354, seuls rouges Nomad D831) ; test C++ garde jusqu'a E3-FIN.
- 2026-11-16 22:5x GLORFINDEL-1116p : E3-12 fait (HOLManagerErrorText_Test.as : Test_ManagerErrorTextCodes, Test_MatchLossText, Test_ManagerErrorTextReturnToLobby ; 3 valeurs faussees = 1360/1365 rouge vu, vert 1363/1365, seuls rouges Nomad D831) ; non porte : ReadFailure 401 (bad_key fait a la main) ; test C++ garde jusqu'a MIG-C++-FINAL.
- MIG-S1c-Diplomacy (ARWEN-1116q, 2026-11-16 22:3x, sans C++, d751-root 0240d17) : PARTIEL : HOLSignalRules.as, HOLSignalBoardScript.as, HOLSignalRules_Test.as, HOLDiplomacyPanel_Test.as lisent HOLDiplomacyRead ; HOLDiplomacyRules.as et son test RESTENT (non exposes : constantes de posture, fiches de changement, InMatch, DoorsOpenFor, CanGarrison, AlliedVictory) ; rouge vu 1345/1348, vert 1346/1348 (Nomad D831).
- E3-14 (2026-11-16 22:44, Sauron-1116q) : Test_MarketTableRows (HOLMarketRules_Test.as) lit Tools/Data/Market.csv en script vers FHOLMarketRow ; rouge vu 1346/1349 puis vert 1347/1349 ; CreateTableFromCSVString non expose, test C++ conserve (E3-FIN).
- E3-02 (2026-11-16 23:0x, Celeborn-1116r, sans C++) : HOLFactionChoice_Test.as (Test_FactionChoiceList, Test_FactionChoiceNames, Test_FactionChoiceTripleClick) contre HOLFactionChoiceRead ; rouge vu 1360/1365 puis vert 1363/1365 ; Net.FactionChoice.Bodies non porte (fonctions du manager) ; test C++ conserve (MIG-C++-FINAL).

- MIG-S1c-Siege (ARWEN-1116p, 2026-11-16 22:2x, sans C++, d751-root) : HOLAISiege.as, HOLSiegeRules_Test.as, HOLUnitTierRules_Test.as lisent HOLSiegeCheck:: ; copie script retiree de HOLSiegeRules.as : TierOf, InPrototype, Check, FindEngine, DescribeBlock, HealthStatOf, DamageStatOf ; RESTENT (non exposees) : enum EHOLSiegeEngine, constantes/EngineAt (numerotation), PrototypeEngines/OpenedEngines/IsAvailable (redirigees sur la bibliotheque), HealthWithTechnology, AttackWithTechnology, TakesUnitsOn, IsInArea ; rouge vu 1345/1348 (Test_SiegeEngineTiers, valeur 9 au lieu de 1), vert 1346/1348 (Nomad D831 seuls).
- Haldir (maps, sans C++, 2026-11-16 23:0x, HALDIR-1116q) : E3-13 part regle FAITE, commit 088dc49 (HOLMapRules_Test.as, 3 tests script, rouge vu puis vert, 1363/1365) ; test C++ supprime en MIG-C++-FINAL
- MIG-CPP-UnitTier pont serveur (ARWEN-1116r, 2026-11-16 23:1x, C++ : pont serveur vers le script, d751-root 8d36950, NON compile) : UHOLUnitTierSubsystem (Simulation, abstrait, BlueprintNativeEvent CheckUnitTier, defaut = regle C++) + HOLUnitTierBoardScript.as ; HOLProductionSubsystem.cpp l. 446-448 appelle le pont (repli C++ sans board) ; test HeroesOfLiberty.UnitTier.ServerBoard (rouge : bouchon 'return HOLUnitTier::Available;' dans le .as) ; C++ de la regle et appel Slate gardes.
- MIG-CPP-Site pont serveur (ARWEN-1116s, 2026-11-16 23:3x, C++ : pont serveur vers le script, d751-root ee1cf53, NON compile) : UHOLSiteBoardSubsystem (Simulation, abstrait, BlueprintNativeEvent StepSiteRule / VictoryStepRule, defaut = regle C++) + HOLSiteBoardTypes.h (FHOLSiteState et FHOLSiteVictoryState en USTRUCT, deplaces de HOLSiteRules.h) + HOLSiteBoardScript.as ; HOLSiteRules.as et HOLSiteVictory.as : surcharge Step avec les pas donnes ; HOLSiteSubsystem.cpp OnStepEnded appelle StepSite une fois par site et par pas (comptes agreges), VictoryStep une fois par pas ; test Sites.ServerBoard.
- MIG-J-Site bascule faite (ARWEN-1116t, 2026-11-16 23:5x, d751-root 505aeac, NON compile) : script seule source du pas et de la victoire des sites ; pont sans board = etat inchange + erreur ; supprime HOLSite::Step/VictoryStep C++, 3 fonctions de HOLSiteRulesLibrary, test C++ Sites.VictoryRule, 2 tests script d'accord ; GoldStep, ResetCounts, AddUnit gardes C++.
- MIG-Garrison-pont (GLORFINDEL-1116q, 2026-11-16 23:1x, C++, net-playtest 75fe739, NON COMPILE) : UHOLGarrisonBoard + HOLGarrisonBoardScript.as + HOLGarrisonSubsystem.cpp via AdmittedForOrder (repli C++ sans script) + test Garrison.ServerBoard ; rouge par bouchon et vert A FAIRE a la compilation ; copie C++ non supprimee.
- MIG-Garrison-source-unique (GLORFINDEL-1116r, 2026-11-16 23:3x, C++, net-playtest 0196e7b, NON COMPILE) : copie C++ de la regle Admitted supprimee (HOLGarrisonRuleLibrary.h/.cpp, HOLGarrisonParity_Test.as, HOLGarrisonRules.h reduit a EKind+KindOf, pont sans repli : refus sans board). REGISTRE : 2 tests retires (Test_GarrisonScriptAndCppAdmitTheSame, Test_GarrisonScriptAndCppDefaultsAgree) -> ce qui couvre : HOLGarrisonRules_Test.as + Garrison.ServerBoard + les 8 tests C++ Garrison.* ; le total quick_check baisse de 2 (retrait voulu) ; a compiler.
- S1e (CELEBORN-1116s, 2026-11-16 23:0x, C++ : exposer des regles qui restent en C++ D836/D839, hero-j8-close 94966ee, NON COMPILE) : HOLDiplomacyRead GetStanceEnemy/Neutral/Ally/Default + GetResult* + FHOLDiplomacyStanceChange/VisionChange (ChangeStance/ChangeVision) ; HOLUnitAbilityRead GetRageCooldownEndMs/GetRageEffectEndMs/IsRageEffectEnded ; 3 tests de fumee HOLS1eLibraries_Test.as (compilent apres build) ; redirection de l interface = lot suivant
- S1e redirection (CELEBORN-1116t, hero-j8-close 0b54124, sans C++, 23:4x) : HOLDiplomacyPanel/GameScreen/Panel_Test/SignalRules_Test/Rules_Test lisent les constantes et fiches de HOLDiplomacyRead ; rouge vu 1361/1364, vert 1362/1364 ; HOLDiplomacyRules.as reste (CanGarrison, DoorsOpenFor, AlliedVictory, InMatch non exposés).
- E3-FIN (2026-11-16 23:0x, SAURON-1116r, visual 16613ce, sans build) : 16 tests C++ retires (registre dans Docs\Rapports\SAURON-1116r.md), 7 fichiers supprimes, HOLCampTable.h/.cpp retires ; gardes : Market.Table, StartingPeasants.Count, FactionChoice.Bodies, MatchLossText, Map.Rules, GameNames ; a compiler.
- 2026-11-16 23:2x ELROND-1116q (tiers, sans C++) : pont UnitTier, 8 cas Trebuchet/Cannon : la table CSV n'a pas leurs lignes, le C++ repond NotListed, le script repondait TierTooLow/Available ; HOLUnitTier::Check script = NotListed pour eux (HasRowInUnitTypes, MinTierOf intact pour l'IA), test Test_UnitTierSiegeEnginesLikeCpp (rouge vu 1359/1363, vert 1361/1363, ServerBoard vert), 5108aa3 ; branche prete.
- MIG-Relay-pont (ELROND-1116r, 2026-11-16 23:3x, C++, tiers, NON COMPILE) : UHOLRelayBoard (HOLRelayBoard.h/.cpp) + HOLRelayBoardScript.as (HOLRelay::StepWith / TerritoryXPAt ajoutes a HOLRelayRules.as, Step/TerritoryXP les appellent avec les constantes) + HOLRelaySubsystem.cpp via StepForRelay / TerritoryXPFor (repli C++ sans script) + test Relay.ServerBoard ; MakeView reste C++ (replique) ; rouge par bouchon et vert A FAIRE a la compilation ; copie C++ non supprimee.
- MIG-Relay-suppression (ELROND-1116s, 2026-11-16 23:5x, C++, tiers 6bed6a3, NON COMPILE) : copie C++ de Relay Step/TerritoryXP supprimee (152 l. C++ retirees, 94 l. de tests script), pont sans repli (inerte + erreur loggee), 0 test C++ retire (Ring garde : FillPercent/MakeView), 1 test script d'accord retire + 1 reduit ; a faire par la session principale : build + quick_check -Parts Relay,Script + bouchon ServerBoard.
- MIG-Market-pont (SAURON-1116s, 2026-11-16 23:3x, C++, visual, NON COMPILE) : UHOLMarketSubsystem + HOLMarketBoardScript.as (Recover, Received) via HOLProductionSubsystem.cpp (repli C++ sans script) + test Market.ServerBoard ; rouge par bouchon et vert A FAIRE à la compilation ; NewBook, devis Slate, HOLPlayerController, HOLAIOrders et Apply restent C++ ; copie C++ non supprimée.
- MIG-Market-copie (SAURON-1116t, 2026-11-16 23:5x, C++, visual f1dc2dc, NON COMPILE) : copie C++ de Recover et Received supprimee (pont sans repli : livre inchange / montant envoye + erreur loggee) ; 8 assertions C++ retirees (Market.Rules 4, Market.Tribute 4, equivalents Test_MarketRulesVectors / Test_MarketTributeVectors), aucun test entier ; Market.ServerBoard sur chiffres a la main ; a compiler + quick_check, puis Apply/NewBook. Registre : Docs\Rapports\SAURON-1116t.md.
- MIG-CPP bascule EndBanner (GALADRIEL-1116r, 2026-11-16 23:4x, C++, galadriel-tab eaabc44, NON COMPILE) : UHOLEndBannerScreen (HOLEndBannerScreen.h/.cpp : Open par PushMenu(FindScriptClass), SetResult/SetDetail/OnClosedByPlayer, Continue/QuitToMenu/QuitToList/ReturnToLobby/ShowStats, LeaveMatch, MakeBannerColumn/AddBannerText/AddBannerRow/AddBannerButton, GetResultValue/GetOriginValue, SetButtonLabel+MenuLog reutilises du menu) + Script\UI\HOLEndBannerScreen.as (vue HOLEndBannerScript::Make) + test Test_EndBannerScreenScriptClass ; HOLEndBanner.cpp/.h (295 l.) supprimes ; les 2 tests C++ EndBanner.* restent ; base distincte de UHOLGameMenuScreen (son FindScriptClass prend la 1re classe fille).
- MIG-Camp-pont (HALDIR-1116r, 2026-11-16 23:4x, C++, maps, NON COMPILE) : UHOLCampBoard (HOLCampBoard.h/.cpp, structures et enums du pont, defaut = regle C++) + Script\Neutrals\HOLCampBoardScript.as + HOLCampSubsystem.cpp via StepForCamp / RewardsForCamp + test Camp.ServerBoard (5949e51 sans script = rouge, 70ebae7 script = vert A VOIR) ; rien du C++ retire.
- MIG-Camp-source-unique (HALDIR-1116s, 2026-11-17 00:0x, C++, maps f9a43b5, NON COMPILE) : repli C++ du pont retire (camp inerte + erreur sans board), HOLCampRules.cpp supprime, HOLCampRules.h = types + GetStateName, Neutrals.CampRules retire (-> Test_CampRulesVectors), Camp.ServerBoardInert ajoute ; REGISTRE dans Docs\Rapports\HALDIR-1116s.md ; reste : build + quick_check.
- S1e part 2 (CELEBORN-1116u, hero-j8-close 0516664, C++ NON COMPILE) : HOLDiplomacyRead expose CanGarrison, DoorsOpenFor, AlliedVictory (fiche FHOLDiplomacyAlliedVictory), InMatch ; 3 tests de fumee dans HOLS1eLibraries_Test.as ; redirection script (HOLDiplomacyRules_Test.as, HOLSignalRules.as) dans Docs\Rapports\CELEBORN-1116u_script_apres_build.patch a appliquer apres build, puis git rm HOLDiplomacyRules.as.
- L17-01 (ELROND-1117a, tiers, sans C++, 2026-11-17 00:19) : Diplomacy.Rules etait DEJA porte (Script\Diplomacy\HOLDiplomacyRules_Test.as, Test_DiplomacyRulesVectors + ...GarrisonVictory, CELEBORN-1116t/u) ; rouge vu 1362/1366 (2 valeurs faussees), vert 1364/1366 ; test C++ NON supprime (lot C++ suivant) ; registre dans Docs\Rapports\ELROND-1117a.md ; branche prete.
- 2026-11-17 ARWEN-1117a L17-02 : Siege Tiers/Technologies/Health deja en script ; + HOLSiege_Test.as (Test_SiegeTiers, Test_SiegeTechnologyTiersNotBeforeTheWorkshop, Test_SiegeHealthRounding), e57e270 sur d751-root ; HOLSiegeRules.as reste (HOLAISiege, tests) ; Docs\Rapports\ARWEN-1117a.md
- L17-03 (CELEBORN-1117a, 2026-11-17 00:2x, sans C++) : HOLTechnologyTableTests.cpp Table + Playable -> Tools\Tests\technology_table_test.py (grille GDD 11.1, 26 lignes, Implemented/CodedStats, chaine I->II, Playable = 17, file de 5), branche dans quick_check values ; rouge vu (ligne en moins, valeur fausse : FAIL 1) puis PASS ; Rules reste C++ ; Docs\Rapports\CELEBORN-1117a.md
- L17-06 (HALDIR-1117a, 2026-11-17 00:2x, maps, C++ NON COMPILE) : UHOLConfirmDialog -> UHOLConfirmDialogScreen (base Abstract, Ask(PC, question, label, TFunction), SetUp TFunction gardee cote C++ : les 2 appelants sont des lambdas C++) + Script\UI\HOLConfirmDialog.as + Test_ConfirmDialogScreenScriptClass ; HOLConfirmDialog.cpp/.h supprimes ; a compiler puis voir le rouge.
- S1g (ELROND-1117b, 2026-11-17, tiers 9a31252, PAS DE BUILD, C++ oui) : 3 bibliotheques de lecture (OrderChain, MatchStats 9 fonctions, Production 3 fonctions), enveloppes seules, 346 lignes C++ neuves ; HOLS1gLibraries_Test.as (3 tests de fumee) ne compile qu'apres le build ; non exposes : MakeChunk/MergeChunk/Record*, TryEnqueue/Cancel/Advance, EndBuildingTarget.
- L17-09 / L17-11 (ELROND-1117c, 2026-11-17, tiers 88685fe, sans build, CPP non) : HOLOrderChain_Test.as (12 tests) et HOLMatchStats_Test.as (5) ; rouge vu 00:44 (6 tests faussés, 1387/1395), vert 1393/1395 ; registre dans Docs\Rapports\ELROND-1117c.md.
- L17-12 partiel (ELROND-1117d, 2026-11-17, tiers, sans C++ ni build) : HOLProduction_Test.as (Test_Production_PopulationCap, _Progress, _WaitingForRoom) ; rouge vu 00:59 (1386/1389), vert 1387/1389 ; reste C++ : Queue, Advance, Tables.
- L17-12 reste (GLORFINDEL-1117g, 2026-11-17, net-playtest 2c06b4f) : Script/Tests/HOLProductionQueue_Test.as (Test_Production_Queue, _AdvanceQueue, _Tables) contre HOLProductionRead::TryEnqueue/Cancel/Advance (FHOLProductionQueueState) et HOLProductionTables (FindUnitType, FindBuildingType, GetStartingResources) ; C++ neuf (enveloppes) ; binaire perime : rouge a voir apres build, puis supprimer les 3 tests C++ Production.Queue/Advance/Tables.
- L17-10 (ARWEN-1117b, 2026-11-17 00:5x, d751-root 4e5991a, sans C++, sans build) : HOLDeployRulesTests.cpp (9 tests) -> Script\Tests\HOLDeployRules_Test.as (9 tests Test_Deploy*), rien non porte ; rouge vu 9/9 (1376/1387), vert 1385/1387 (2 rouges connus) ; les 9 tests C++ sont retirables, supprimes a la main par la session principale.
- L17-04 (GALADRIEL-1117a, 2026-11-17, galadriel-tab eb5652e, sans C++) : couverture de HOLGameMenuTests.cpp par le script : 5 des 7 tests deja couverts (Surrender, QuitToMenu, StatsButton, EndBanner.Which, EndBanner.Closed) ; Escape et Input ajoutes (GetEscapeAction, KeepsMatchMenuInput dans HOLGameMenuRules.as + 2 tests), rouge vu 1365/1368 puis vert 1366/1368 ; test C++ NON supprime (les 7 tests C++ sont retirables au lot C++ suivant), rapport Docs\Rapports\GALADRIEL-1117a.md.
- L17-03 (CELEBORN-1117a, 2026-11-17 00:2x, sans C++) : HOLTechnologyTableTests.cpp Table + Playable -> Tools\Tests\technology_table_test.py (grille GDD 11.1, 26 lignes, Implemented/CodedStats, chaine I->II, Playable = 17, file de 5), branche dans quick_check values ; rouge vu (ligne en moins, valeur fausse : FAIL 1) puis PASS ; Rules reste C++ ; Docs\Rapports\CELEBORN-1117a.md
- L17-05 (GLORFINDEL-1117a, 2026-11-17 00:2x, sans C++) : AlertRing + TerrainSources non portes (AlertDiameter et GetSourcesSignature vivent dans HOLMinimapView.cpp, aucune copie .as) ; liste a exposer dans Docs\Rapports\GLORFINDEL-1117a.md ; ligne passee en C++ bibliotheque requise.
- L17-01/03/04 (CELEBORN-1117b, 2026-11-17, hero-j8-close 75f375a, C++ NON COMPILE) : 9 tests C++ retires (Diplomacy.Rules 1, GameMenu/EndBanner 7, Technologies.Table 1) ; le total quick_check doit baisser de 9 ; Playable, Rules, Siege gardes ; Docs\Rapports\CELEBORN-1117b.md
- S1h (GLORFINDEL-1117b, 2026-11-17, net-playtest 4efe1f6, C++ sans build) : UHOLMinimapReadLibrary (AlertDiameter, GetSourcesSignature, GetConsoleFloat ; HOLMinimapView.cpp inchange) + Script\Tests\HOLS1hLibraries_Test.as (2 tests de fumee, a compiler apres build) ; FHOLSource non USTRUCT -> tableaux paralleles ; port L17-05 apres build ; rapport Docs\Rapports\GLORFINDEL-1117b.md.
- L17-05 (GLORFINDEL-1117c, 2026-11-17, net-playtest, sans C++, sans build) : HOLMinimapAlertRingTests + HOLMinimapTerrainTests -> Script\Tests\HOLMinimapRules_Test.as (Test_MinimapAlertRing, Test_MinimapTerrainSources) contre UHOLMinimapReadLibrary ; rouge vu (16 -> 17) puis vert, tests 1386/1388 (D831) ; .cpp non supprimes.
- E3-11 (GLORFINDEL-1117f, 2026-11-17, net-playtest, sans C++, sans build) : HeroesOfLiberty.Signals.AltClick (HOLSignalInputTests.cpp) -> Script\Tests\HOLSignalInput_Test.as Test_SignalAltClick (KindOfAltClick, Expire, ColorOf via HOLSignalInputRead, MakeSignal), rouge vu 10:30 (Test_SignalAltClick seul rouge), vert 10:34 tests 1391/1393 (2 rouges connus Nomad), f7ec5f8 ; git rm HOLSignalInputTests.cpp encore a faire (C++, un build) ; RelationIcons + FactionTechnology (E3-10, filtre) pas encore portes
- L17-07/L17-08 (SAURON-1117b, 2026-11-17, visual fe6189c, sans C++) : HOLRelayRing_Test.as (1 test) + HOLHeroAbility_Test.as (3 tests) contre UHOLRelayReadLibrary / UHOLHeroAbilityReadLibrary ; rouge vu (4/4 echouent), vert 1380/1382 ; rien de non porte cote HeroAbility ; cote Relay l'API FHOLRelayView (replique) lue par son jumeau FHOLRelayReadView ; jumeau .as garde (appelants).
- L17-07..11 (HALDIR-1117b, 2026-11-17, maps 7a7a541, C++ NON COMPILE) : 15 tests C++ retires (Orders.Chains 1, Match.Stats.Scores 1, Relays.Ring 1, HeroAbility 3, Siege.Deploy 9 ; fichiers RelayRing/HeroAbility/DeployRules supprimes) ; le total quick_check doit baisser de 15 ; Docs\Rapports\HALDIR-1117b.md
- ECR-1 écran TITRE (GALADRIEL-1117d, 2026-11-17 09:4x, galadriel-tab b80da54, C++ oui, NON COMPILÉ) : UHOLTitleScreen Abstract/Blueprintable (FindScriptClass, ButtonLabel, MakeTitleColumn, AddTitleText/Button, AddVersionText, OpenNewGame/Multiplayer/Settings, AskQuit, BP_BuildTitle ; triple clic, focus et LOCTEXT gardés en C++) ; HOLMenuPlayerController pousse FindScriptClass ; Script\UI\HOLTitleScreen.as + HOLTitleScreen_Test.as (2 tests) ; quick_check/packaged_smoke : menu=HOLTitleScreenScript, modal=HOLConfirmDialogScreenScript (l'attendu de la boîte était périmé depuis 47e222a) ; Tools\Scenarios\ecran_titre.txt/.json/.look. Ancien C++ (BuildLayout, Handle*) retiré dans le même commit ; AskToQuit gardé (HOLUIOpen, bouton Quitter). Suite : build, rouge vu, preflight, essai.
- ECR-3 écran MULTIJOUEUR (ARWEN-1117e, 2026-11-17 10:35, d751-root 80e743b) : compilé ; correctifs : ScreenText qualifié UHOLMultiplayerScreen::ScreenText (AS ne trouve pas le static non qualifié), nom de test trop long (18 car. > 16). quick_check tests 1393/1395 (2 rouges connus Nomad). Preflight PASS. Reste : essai à la main, bouton Créer non grisé, fond d écran.
- ECR-3 suite (ARWEN-1117f, 10:40) : les 2 écarts du PNG ne sont pas des bugs de script. (1) Créer actif : CanCreateLobbyAfter (HOLManagerErrorText.cpp, testé) ne grise que bad_key ; log « HTTP 0 » = injoignable = actif ; le .look disait grisé à tort, à corriger en « actif » (non fait, édition refusée) ou changer la règle (C++, build). (2) pas de fond : l'ancien écran C++ n'en avait pas (MakeCenteredColumn), le scénario l'ouvre au-dessus d'une partie ; un voile exige d'exposer la racine du WidgetTree (C++). Questions : Retours\Q-ecran-multijoueur-fond.md. Aucun commit de code.
- ECR-2 Réglages (CELEBORN-1117e, 2026-11-17 10:3x, hero-j8-close, merge main 172dafd, NON COMPILÉ) : relu à sec (HOLSettingsScreen.as/_Test.as/.h/.cpp) sans erreur trouvée ; DLL périmée (01:09) : build puis rouge (Test_SettingsOptions 11 -> 12) puis vert, preflight ecran_reglages, fiche E-ecran-reglages.md (LOGIQUE SEULEMENT).
- ECR-2 Réglages fin (CELEBORN-1117h, 2026-11-17 11:2x, hero-j8-close 8c10fe6, sans C++) : écart 1117g (Défilement au bord = Non) = Tools\Tests\TestSettings.ps1 / check.ps1 forçaient bEdgeScroll=False dans Saved_Tests pour tout test ; ecran_reglages.txt porte maintenant « # default-settings » et check.ps1 retire alors la section HOLGameUserSettings (Reset-TestHolSettings) ; .look inchangé ; ini Saved et Saved_Tests remis à zéro ; preflight PASS, 5 PNG conformes ; E-ecran-reglages.md [À ESSAYER] VU À L'ÉCRAN.
- Migration E3-10 (GALADRIEL-1117e, galadriel-tab e96dd0b, sans C++) : Test_RelationIconsChoice porte en script (HOLRelationIcons_Test.as, via HOLRelationIconsRead), rouge vu 10:30 (1390/1393) puis vert 10:33 (1391/1393, 2 Nomad connus) ; test C++ HOLRelationIconsTests.cpp CONSERVE (moitie textures 64 px / groupe UI reste en C++).

- MIG-C++-Market Apply (GALADRIEL-1117f, galadriel-tab 5730795 rouge + 719b49e, C++ NON COMPILE) : HOLMarket::Apply script + UHOLMarketSubsystem::Apply/TradeNow/TradeOnce + FHOLMarketBridgeTrade ; HOLProductionSubsystem et HOLPlayerController redirigés ; reste devis Slate, NewBook, suppression C++ après build. Docs\Rapports\GALADRIEL-1117f.md.
- MIG-Market-source-unique (GALADRIEL-1117g, galadriel-tab, C++ NON COMPILE) : HOLMarket::Apply et Buy supprimes du C++ (pont sans repli) ; Sell/BuyWithGold/NewBook gardes (devis Slate, HOLGameState) ; test C++ Market.Apply retire (equivalents Test_MarketApply* verts vus 10:11), assertions Buy de Market.Rules retirees (= Test_MarketRulesVectors) ; registre Docs\Rapports\GALADRIEL-1117g.md ; a compiler.
- SAURON-1117f (2026-11-17 11:3x, sans C++, branche visual) : port des tests Production.UnitTier / PopulationCheckedAtEnd : monde complet non expose au script (aucun test script ne cree de monde) ; PARTIEL : Script/Tests/HOLUnitTierProduction_Test.as (3 tests : comptes de files 1/1/0/0/0, 2/2/1/1/1, 3/3/1/1/1, Mangonel palier 2) rouge vu 11:30 (1393/1396, valeur 1/1/1/0/0), vert 11:33 (1394/1396, Nomad connus) ; PopulationCheckedAtEnd = Mass (D838) reste C++, parts pures deja en script ; aucun test C++ retire ; registre et bibliotheque a exposer : Docs/Rapports/SAURON-1117f.md.
- L17-12 reste suite (GLORFINDEL-1117h, 2026-11-17, net-playtest ef67576, sans build) : rouge vu sur Test_Production_Queue / _AdvanceQueue / _Tables (une valeur fausse chacun, 1391/1396, Get-StaleBinaryProblem vide), vert revu 1394/1396 (rouges connus Nomad) ; FHOLProductionQueueTest, FHOLProductionAdvanceTest, FHOLProductionTablesTest retires (HOLProductionTests.cpp supprime, 207 lignes) ; registre Docs\Rapports\GLORFINDEL-1117g.md ; a compiler, total quick_check attendu -3.
- MIG-B-SWEEP (GLORFINDEL-1117j, 2026-11-17, net-playtest, sans C++, sans build) : 15 tests C++ de la classe B (Siege 4, MatchStats 3, FactionChoice.Bodies, Market.Table, Map.Rules, Neutrals.Owner, Technologies.Rules, Market.Rules, Hero.ReturnQuote/ReturnOrderPrice) relus contre leurs jumeaux script : 0 retire (chacun a des assertions sans equivalent : import CSV reel, ChooseTarget/GetMaxHealth, Record*/MakeChunk/MakeCurve, FTextInspector, JSON du manager, monde), aucun faussement lance, total quick_check inchange ; registre (nom C++ -> script, raison du gardé, bibliotheques a exposer) : Docs\Rapports\GLORFINDEL-1117j.md.
