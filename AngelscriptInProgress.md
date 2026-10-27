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
