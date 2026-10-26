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
