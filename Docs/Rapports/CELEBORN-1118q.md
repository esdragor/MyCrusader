# CELEBORN-1118q : ecran Statistiques, liaison manquante (commit 1) et bascule ST-3 (commit 2)

**NON COMPILE.** Aucun build, aucun quick_check, aucun editeur. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`, branche `hero-j8-close`, base 696b818. Non fusionne, non pousse. Pas touche : EtapeDefini.md, Auto_Journey_File.md, *InProgress.md, quick_check.ps1, packaged_smoke.ps1.

- Commit 1 `824b975` (5 fichiers, +~80/-6) : liaison C++ + rebranchement du script, comportement de l'ecran inchange.
- Commit 2 `bd3e321` (2 fichiers, +16/-203) : bascule ST-3, ecartable seul (`git revert bd3e321`).

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: ~203 (commit 2).

## Signatures ajoutees (commit 1)
- `UHOLMatchStatsReadLibrary` (.h/.cpp, namespace script `HOLMatchStatsRead::`, contexte de monde implicite) :
  - `FText GetWorldSlotName(const UObject* WorldContextObject, int32 SlotIndex)` : `HOLForumMessages::GetPlayerName(config, SlotIndex, INDEX_NONE)` pour n'importe quel slot (fermes inclus) ; vide sans GameState.
  - `FLinearColor GetWorldSlotColor(const UObject* WorldContextObject, int32 SlotIndex)` : `HOLPlayerColors::GetColor(Slots[i].ColorIndex)` ; blanc hors plage ou sans GameState.
  - `FString DescribePlayer(const FHOLPlayerMatchStatsState& Player)` : `HOLMatchStats::DescribePlayer(ToPlayer(Player))`.
- `UHOLStatsScreen` (.h/.cpp) :
  - `static FText NumberText(int32 Value)` (UFUNCTION BlueprintPure) = `FText::AsNumber`.
  - cle `Dash` dans `TextOf` = `FText::AsCultureInvariant(TEXT("—"))`.
- Point (d) : le C++ n'ecrit `DescribePlayer` que dans le JOURNAL (`UE_LOG "Stats screen: row ..."`, pas a l'affichage). Reproduit quand meme dans le script (ligne de journal identique) car `quick_check.ps1:770` attend `waitlog Stats screen: row`.

## Script `Script\UI\HOLStatsScreen.as` (commit 1)
- `CellText` : tiret par `TextOf(n"Dash")`, nombres par `UHOLStatsScreen::NumberText(Cell.Value)`. Plus aucune chaine en dur (les `FText::FromString("")` restants sont des textes vides).
- Nouvelle methode `BuildSlotMarks(const FHOLMatchStatsState&)` dans la classe ecran : couleur et nom du slot par `GetWorldSlotColor` / `GetWorldSlotName` (slots fermes inclus), etiquette par `MakeMarkLabel`. `DrawTimeline` l'appelle a la place de `HOLStatsRules::BuildMarks` (HOLStatsRules.as non permis : `BuildMarks` y reste, inchange, avec sa limite ; `Test_StatsTimelineSeries` le teste toujours ; il n'a plus d'appelant ecran, a supprimer ou aligner plus tard).
- Journal par ligne : `Log(n"LogHOLUI", "Stats screen: row <nom> (slot N) military M economy E, <DescribePlayer>")`.
- **Journal de la chronologie** : le script ecrivait `timeline X, N curves, M marks, axes` SANS le compte de points ; `quick_check.ps1:3176` parse `(\d+) curves, (\d+) points, (\d+) marks, (.*)$` : il serait reste rouge. Ajout du total de points (somme des `Points.Num()`), meme format que le C++.

## Commit 2 : lignes C++ retirees (`HOLStatsScreen.cpp` -203, `.h` 3 lignes)
- `Open` : pousse `FindScriptClass()` ; sans classe : `UE_LOG Warning "Stats: no script class (Script/UI/HOLStatsScreen.as)"` et `nullptr` (comme Settings:156-162).
- `BuildLayout` (54 lignes de mise en page) -> corps de 1 ligne `BP_BuildStats();`. Le `override` reste : c'est lui qui appelle le script.
- `RefreshTimeline` (64 lignes : courbes, reperes, journal) -> `BP_RefreshTimeline(SeriesIndex);`.
- `Refresh` (84 lignes : statut, grilles, lignes, journal) -> `BP_RefreshStats(bRefused);`. Reste la cible de `GetOnStatsUpdated().AddUObject` et de `OnRefused`.
- `IsScriptScreen()` supprime (definition + declaration, plus aucun appelant : grep fait).
- `UCLASS(Blueprintable, Abstract)` : la classe C++ n'est plus instanciable ; `FindScriptClass` garde son `*It != StaticClass()` (inoffensif).
- GARDE : `AddStats*`, `MakeStatsColumn`, `TextOf`, `HeadersOf`, `MakeMarkLabel`, `NumberText`, `SelectTab`, `SelectSeries`, `Close`, `NativeOn*`, l'abonnement au sous-systeme, la minuterie de 10 s, la peinture de `UHOLStatsGraph` (non touche), le namespace anonyme `StatsScreen` (constantes + `SeriesNames` + `ETab`) entier. `SeriesNames` ne sert plus qu'au `static_assert` (inoffensif, a retirer plus tard). Includes non nettoyes (sans risque).

## Appelants de `UHOLStatsScreen::Open` (tous passent par Open, aucun ne construit la classe C++)
- `Source\HeroesOfLibertyUI\Dev\HOLUIOpen.cpp:93` (`hol.UI.Open Stats`)
- `Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp:2846` (bouton Statistiques)
- `Source\HeroesOfLibertyUI\Screens\HOLEndBannerScreen.cpp:142` (bouton de la banniere de fin)
- Autre usage : `HUD\HOLGameHUD.cpp:171` fait `Cast<UHOLStatsScreen>(Layout->GetActiveMenu())` : la classe script en derive, le Cast reussit.

## Controles d'outils a adapter (NON modifies)
1. `Tools\Tests\quick_check.ps1:3167-3168` : `menu=HOLStatsScreen` (summary, military, economy, workers, army). Le menu actif sera desormais l'instance de la classe script, dont le nom est `HOLStatsScreenScript` (`Test_StatsScriptClass` l'affirme). A VERIFIER : si la comparaison est exacte, a changer en `menu=HOLStatsScreenScript` ; si c'est une sous-chaine, rien. Meme question pour `packaged_smoke.ps1` (grep fait, aucune occurrence `HOLStatsScreen` ni `Stats screen` trouvee dedans).
2. `quick_check.ps1:770, 777, 779, 781, 3176, 3215-3216` lisent `Stats screen: row`, `tab 3`, `timeline Military`, `closed`, `opened` : `row`, `timeline` (avec points) emis par le script, `tab`, `opened`, `closed` restent en C++. Formats conserves.

## Suspects de compilation
1. `UFUNCTION(BlueprintPure) static FText NumberText(int32)` et `FText GetWorldSlotName(...)` : nom `NumberText`/`DescribePlayer` ne doivent pas entrer en conflit avec un autre membre (grep non fait au-dela du fichier).
2. `HOLMatchStatsReadLibrary.cpp` : `ToPlayer` du namespace anonyme est utilise par `DescribePlayer` plus bas dans le meme fichier (ordre correct : il est defini en tete). Build unity : aucun nouveau namespace anonyme.
3. `HOLMatchStats::DescribePlayer(const FHOLPlayerMatchStats&)` suppose, d'apres l'appel du C++ retire (`*HOLMatchStats::DescribePlayer(Player)`) ; retourne `FString`.
4. `UCLASS(Abstract)` + `UCLASS(Blueprintable)` : OK avec Angelscript (comme HOLSettingsScreen ?). A confirmer au premier lancement : si l'Abstract empeche la creation de la classe script derivee, retirer `Abstract` (alors `FindScriptClass` exclut deja la classe C++).
5. Script : `f"...{Rows[Index].Name.ToString()}..."` et `{HOLMatchStatsRead::DescribePlayer(Player)}` dans une f-string, `FLinearColor` renvoye par `GetWorldSlotColor` sans contexte explicite. `FText::ToString()` existe-t-il en script ? (probable, non verifie).
6. `BuildLayout` ne contient plus que `BP_BuildStats();` : sans classe script chargee, `Open` renvoie null (avertissement) : l'ecran n'existe plus en C++ seul (normal, c'est la bascule).
7. Warnings possibles : `SeriesNames` plus utilise que par `static_assert` ; includes devenus inutiles (`Components/SizeBox.h`... encore utilises par `AddStatsGraph`).

## Commande a lancer par la session principale
`powershell -NoProfile -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests` (3 tests HOLStatsScreen_Test), puis la partie `Statistiques` de quick_check (captures `qc_stats_*`).

## A comparer a l'oeil, ancien ecran (main) / nouvel ecran
- Nombres : groupement des milliers (ex. 12 345 / 12,345 selon la culture) dans les 3 tableaux : doit etre identique (`NumberText`).
- Colonne Equipe : « — » quand pas d'equipe, sinon le numero.
- Pastille de couleur avant chaque nom, largeur de colonne du nom, alignement des chiffres a droite, espacement (padding des lignes, double sous l'en-tete).
- Onglets (Resume, Militaire, Economie, Chronologie) : onglet selectionne mis en evidence, retour a l'onglet et a la serie montres en dernier a la reouverture.
- Chronologie : couleurs des courbes, bouton de serie actif, reperes datees avec nom correct meme pour un joueur dont le slot est ferme (« X vaincu », « Forum de X tombe »), graphe 1100 x 440 centre.
- Statut : « Les statistiques seront disponibles a la fin de la partie. » (joueur en match), « Statistiques en attente du serveur… » (copie pas encore recue), tableaux masques dans les deux cas.
- Bouton Retour et touche retour ; ouverture depuis le menu, depuis la banniere de fin et par `hol.UI.Open Stats`.

## NON VERIFIE
Tout (aucune compilation, aucun rendu). Les points 1 et 4 ci-dessus en priorite.

## Anomalies
Horloge Get-Date : ~21:14 a 21:17 sur ~25 appels. L'outil Edit a exige une relecture du .as avant edition (aucun effet). Un `rg` et `tail` absents du shell PowerShell (remplaces par Grep/Select-String). `git commit --amend` utilise une fois sur MON commit 1 non pousse pour y ajouter le correctif du journal de la chronologie.
