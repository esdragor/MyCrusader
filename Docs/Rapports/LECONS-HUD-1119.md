# LECONS-HUD-1119 : ce que H-1 à H-4 ont appris (à lire avant d'écrire un panneau du HUD en script)

Écrit par la session principale le 2026-11-19 (reprise 13), d'après les rapports GALADRIEL-1118t/u/v, GLORFINDEL-1118t/u, ELROND-1119a-d, HALDIR-1119a-c.
Chaque point a coûté un rouge ou un défaut réel ; les fichiers cités sont des modèles qui compilent et tournent.

## Modèles
- Panneau simple avec liaison de lecture : `Script\UI\HOLWaitBanner.as` + `HOLWaitBanner_Test.as`, liaison `Source\HeroesOfLiberty\Match\HOLWaitBannerReadLibrary.h/.cpp`.
- Panneau avec boutons, clics et infobulles : `Script\UI\HOLTopBar.as` + `HOLTopBar_Test.as`, liaisons dans `Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp` (membres de `UHOLScriptScreen`).
- Panneau qui remplit un widget resté C++ : `Script\UI\HOLPlayerTable.as` (le script dessine dans `PlayersPanel`, les lignes viennent d'une règle C++ gardée, une seule source).
- Crochet : `Script\UI\HOLGameScreenScript.as` (membre, `Build` dans `OnInitialized` après `BuildVolcanoBanner`, `Refresh()`, `ClickWidget`).

## Journal (2 rouges)
1. Une ligne de journal lue par un outil s'écrit MOT POUR MOT comme le C++, **niveau compris** : `UHOLGameMenuScreen::MenuLog(Ligne)` écrit `LogHOLUI: Display: <ligne>` comme le faisait `UE_LOG(LogHOLUI, Display, ...)`. `Log(n"LogHOLUI", Ligne)` écrit `LogHOLUI: <ligne>` SANS « Display: » et `lobby_match.ps1:633` ne la voit plus (lobby rouge le 2026-11-19).
2. Avant de bouger un panneau : grep de `Tools\` (quick_check.ps1, lobby_match.ps1, Scenarios\*.txt, hud_rects*) pour chaque ligne de journal et chaque nom de widget du panneau ; table « outil -> texte lu -> où le script l'écrit » dans le rapport.

## Vue (1 défaut vu seulement à la capture)
3. Un test de règle ne voit pas la vue : le bouton Menu de H-3 s'affichait SANS libellé avec 1548/1548 tests verts. La preuve d'un panneau = le même scénario joué AVANT (panneau C++) et APRÈS (script), captures découpées sur la zone du panneau et comparées à l'œil (l'égalité au pixel de l'image entière ne vaut rien en partie : le décor 3D bouge).
4. `UHOLQueueItem::SetLabel` garde maintenant le libellé donné avant l'initialisation du bouton (corrigé 1b30d3e) ; pour tout AUTRE widget C++ construit par `Screen.ConstructWidget(...)`, ne suppose pas que ses sous-widgets existent déjà à la construction : pose les textes dans `Refresh` ou vérifie le C++ du widget.
5. Mêmes noms de widgets que le C++ (les outils `hol.HUD.Click`, `hol.Mouse.Hover`, `DescribeRects` les cherchent par nom) ; `hol.Mouse.Hover` trouve les widgets de l'arbre du script (HOLUIMouse.cpp:105-122) ; `hol.UI.Focus` ne cherche que le modal et le menu actif.
6. `Describe()` du C++ (ligne de `hol.HUD.Report`, lue par quick_check) doit rester mot pour mot : ou bien le C++ garde la copie des champs `Shown*` sans widget, ou bien il recalcule par la règle.

## Compilation Angelscript (erreurs réellement rencontrées)
7. Une bibliothèque C++ `UHOLXxxLibrary` s'appelle `HOLXxx::Fonction(...)` en script (le plugin retire « U » et « Library ») ; une statique d'une classe non-bibliothèque s'appelle `UHOLClasse::Fonction(...)`.
8. N'existent pas en script : `FText::GetEmpty()` (écrire `FText::FromString("")`), `FText::AsNumber`, `GetWidgetFromName`, `CreateWidget(contrôleur)`.
9. Un gestionnaire de `OnButtonBaseClicked` prend un paramètre `UCommonButtonBase` ; `ImageSize` d'un brush veut un `FVector2f` ; un `float` script est un double (`float32` pour un `float&` C++).
10. Une classe `UHOLTrucView` et une struct `FHOLTrucView` se heurtent : nommer la classe `UHOLTrucPanel`.
11. Deux fichiers de test ne définissent pas la même aide globale ; une fonction ou constante d'un fichier n'est visible d'un autre que dans un `namespace`.

## Scénario de preuve
12. Le `.json` du scénario porte `"mapSize": "Small"` (sinon le pré-vol échoue : carte grise de test, D820) ; modèle `Tools\Scenarios\ecran_hud_joueurs.json`.
13. Le scénario appelle `hol.Test.Sanity` (sinon WARN du pré-vol) ; `waitkey` avant chaque étape ; une capture par point du `.look`, l'objet attendu nommé.
14. État fixé avant chaque capture (ressources par triche, Paysans immobiles) pour que la zone du panneau soit comparable entre deux passages.

## Fusion
15. Chaque panneau retire SON C++ de `HOLGameScreen.cpp/.h` et ajoute SON crochet dans `HOLGameScreenScript.as` : deux panneaux se heurtent toujours là. Résolution : les crochets gardés côte à côte, le C++ retiré par chaque côté retiré des deux.
