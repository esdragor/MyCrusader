# GALADRIEL-1118u : HUD H-1, bascule des bannières d'attente et d'absence (2026-11-18, reprise 12, étape 4)

NON COMPILÉ, NON LANCÉ : aucun build, aucun quick_check, aucun lancement.
Impact joueur : aucun changement visible attendu (mêmes places, mêmes textes, mêmes lignes de journal) ; les deux bannières sont dessinées par le script au lieu du C++.

## Les 3 commits (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab`, au-dessus de e9f1448)
1. `730ce95` C++ de liaison (D839 : liaison à exposer) : `Source\HeroesOfLiberty\Match\HOLWaitBannerReadLibrary.h/.cpp`, `UHOLWaitBannerReadLibrary::GetWaitState(Missing, SecondsLeft)` -> bool, `GetAbsenceState(Lines, Names, bBack)`. Patron : `HOLPauseLibrary` (même dossier, même module que `AHOLGameState`). Le HUD est inchangé après ce seul commit (le C++ dessine encore).
2. `0cdbe4b` BASCULE : `Script\UI\HOLGameScreenScript.as` (+6 lignes : membre `UHOLWaitBannerView WaitBanner;` après `VolcanoTexts`, construction `NewObject` + `Build(this, Frame)` dans `OnInitialized` après `BuildVolcanoBanner`, `WaitBanner.Refresh()` dans `Refresh()` après `RefreshVolcano()`) ET dans `HOLGameScreen.cpp` : retrait de l'appel `RefreshWaitBanner(); RefreshAbsenceBanner();` du tick et des deux blocs de `BuildLayout` (WaitPanel, AbsencePanel) : jamais dessinées deux fois.
3. `67ca269` RETRAIT : `HOLGameScreen.cpp` (-69 : définitions `RefreshWaitBanner`/`RefreshAbsenceBanner`, dont le LOCTEXT "WaitingFor", désormais NSLOCTEXT du script, même espace "HOLGameScreen", même clé) et `HOLGameScreen.h` (-30 : membres `WaitPanel/WaitText/AbsencePanel/AbsenceText`, déclarations, `ShownWaitPlayers`, `ShownAbsence`). Grep de chaque symbole dans `Source\` et `Script\` avant retrait : uniquement ces deux fichiers (le script a ses propres `WaitPanel` etc. dans `UHOLWaitBannerView`).
Restent en C++ (partagés) : `BoundTableState`, `SetShown`, `MakePanel`, `MakeText`, `AHOLGameState::GetAbsenceBannerLines`, `FHOLDisconnectRules::Format*`.

## Correction de la recette
Aucune erreur trouvée : lignes, noms et plages exacts (le C++ à retirer était bien aux lignes 567-568, 1144-1211, 3343-3366). Une précision : les sources du HUD C++ sont dans `Source\HeroesOfLibertyUI\HUD\` (module UI), la liaison dans le module `HeroesOfLiberty` (`Match\`) ; le module UI voit donc la liaison déjà, et le C++ UI retiré ne perd aucun include utilisé ailleurs.

## Table : ligne de journal attendue par un outil -> où le script l'écrit
Tous les outils ne lisent que le texte (la catégorie passe de `LogHOLUI: Display` à `Log(n"LogHOLUI", ...)`).
| Outil (fichier:ligne) | Texte lu | Écrit par |
|---|---|---|
| quick_check.ps1:2788 (`waitlog`), 6834 | `Wait banner: shown, <texte>` (texte `^En attente de Bob \(([01]:[0-5]\d\|2:00)\)$`) | `HOLWaitBanner::WaitLogLine(ChangeShown)` : `"Wait banner: shown, " + View.Text`, appelé par `UHOLWaitBannerView.Refresh` (Log ligne 183) |
| quick_check.ps1:6836 | `Wait banner: hidden` (après la capture) | `WaitLogLine(ChangeHidden)` : `"Wait banner: hidden"`, une fois |
| lobby_match.ps1:356, 522 | `Absence banner: shown`, `Absence banner: shown, Bob` | `AbsenceLogLine(ChangeShown)` : `"Absence banner: shown, " + lignes jointes par " \| "` (Refresh, Log ligne 199) |
| lobby_match.ps1:495 | `Absence banner: shown, Bob : connexion perdue — 0:(0\d\|10)` | idem ; la ligne vient de `GetAbsenceBannerLines` C++ (`Line.ToString()`), tiret cadratin inclus |
| lobby_match.ps1:480, 536 | `Absence banner: shown, .*$BackRx` (de retour / back / ja) | idem |
| lobby_match.ps1:356, 441 | `Absence banner: hidden` | `AbsenceLogLine(ChangeHidden)` |
Règles identiques au C++ (relues cette séance contre HOLGameScreen.cpp avant retrait) : attente = journal quand la liste des absents change seulement ; absence = signature `N;Nom+/-` ; état caché journalisé une fois ; cas limite signature vide inchangé. Aucun `Describe*`, `ClickWidget` ni `hol.HUD.Report` ne parlait de ces bannières (grep Tools\ : seulement quick_check et lobby_match) : rien à porter.

## Suspects de compilation (à regarder en premier)
1. `UHOLWaitBannerReadLibrary` : `TArray<bool>&` en paramètre de sortie lié à Angelscript (jamais vu ici) ; sinon changer en `TArray<int32>` (OutBack 0/1) et adapter `HOLWaitBanner.as:188-190`.
2. Contexte de monde implicite depuis un UObject créé par `NewObject(this, UHOLWaitBannerView)` : `HOLWaitBannerRead::GetWaitState` suppose un `GetWorld()` valide via l'outer (le script screen). Si « no world context », passer `Screen` à Refresh et appeler via un helper de l'écran.
3. `UHOLTextStyleHUDHeading` en script (UCLASS C++ du module UI, comme Label) ; `Screen.MakePanel/MakeText` appelés depuis un autre objet : doivent être publics côté `UHOLScriptScreen` (HOLScriptScreen.h:87-91).
4. Build unity : nom `WaitBannerGameState` dans un namespace anonyme, unique dans le module.
5. `HOLGameScreen.cpp` : un `using`/include devenu orphelin (warning seulement).

## À regarder en jeu (NON VÉRIFIÉ)
- Fond : C++ = `HUDPanel` à 0.8 ; script = `MakePanel` teinté `SetBrushColor(1,1,1,0.8)` (écart d'alpha possible sur `qcnet_2_wait`).
- Tiret cadratin intact dans le journal via `Log` (lobby_match:495).
- Ordre d'affichage : le cadre script est au-dessus du cadre C++ (les bannières passent au-dessus de l'alerte majeure) ; `hud_rects_game_test` : `WaitPanel`/`AbsencePanel` peuvent apparaître dans `DescribeRects` pendant une bannière.
- Cadence : le C++ rafraîchissait à 4 Hz ; le script suit le `Refresh()` du écran (cadence non relue) : sans effet sur les textes, seulement sur la fraîcheur du compte à rebours.

## Commandes de validation (session principale, dans le worktree)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` (UN build : liaison + retrait C++).
2. `quick_check -Parts tests,net` : les 5 tests purs `Test_WaitBanner*` / `Test_AbsenceBanner*` (+5 au total) ; la partie net lit `Wait banner: shown/hidden` + capture `qcnet_2_wait`.
3. `Tools\Tests\lobby_match.ps1` (absence away/back, deux scénarios rejoin).
4. `hud_rects_game_test` (+ `hud_rects_test`).
5. Essai utilisateur (D840) : proposition `ecran_hud_bannieres.txt` du rapport 1118t (section 4), `hol.Net.FakeAbsent <Nom> <Secondes>` ; le panneau n'entre dans main qu'après cet essai.
Rouges à poser une fois (rapport 1118t) : `Clock(SecondsLeft + 1)` dans `WaitView` ; inversion `+`/`-` dans `AbsenceView`.

## Anomalies
Aucune de blocage. Deux appels Bash/Edit ont été renvoyés par le classifieur (hook) puis refaits à l'identique ; sans effet sur le résultat. Docs de suivi (EtapeDefini, *InProgress) non touchés, comme demandé ; Placeholders.txt non touché (aucun provisoire ajouté ou retiré).
