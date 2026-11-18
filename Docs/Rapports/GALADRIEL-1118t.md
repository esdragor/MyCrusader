# GALADRIEL-1118t : HUD H-1, bannières d'attente et d'absence (2026-11-18, reprise 12, étape 4)

Impact joueur : aucun changement visible attendu ; les deux bannières (en attente de Bob, Bob : connexion perdue) passent du C++ au script, même place, mêmes textes, mêmes lignes de journal.
NON COMPILÉ, NON LANCÉ : fichiers script neufs seulement, aucun build, aucun quick_check. Le crochet n'est PAS posé (HOLGameScreenScript.as intact).

## Fait
Commit `e9f1448` sur `galadriel-tab` (Documents\HeroesOfLiberty-Galadriel), 2 fichiers neufs :
- `Script\UI\HOLWaitBanner.as` (202 l.) : `struct FHOLWaitBannerView`, `namespace HOLWaitBannerText` (NSLOCTEXT "HOLGameScreen"/"WaitingFor" : même espace et même clé que le LOCTEXT du C++, les traductions restent), `namespace HOLWaitBanner` (règles pures : `WaitView`, `AbsenceView`, `Change`, `WaitLogLine`, `AbsenceLogLine`, `JoinNames`, `ChangeNone/Shown/Hidden()` en FONCTIONS car une constante de namespace n'est pas vue d'un autre fichier), `class UHOLWaitBannerView : UObject` (`Build(Screen, Frame)`, `Refresh()`, widgets `WaitPanel`, `WaitText`, `AbsencePanel`, `AbsenceText`, mêmes noms que le C++).
- `Script\UI\HOLWaitBanner_Test.as` : 5 tests purs en littéraux (`Test_WaitBannerHiddenUnlessWaitingForNamedPlayers`, `..TextNamesPlayersAndCountsDown`, `..ChangeLogsOncePerSignature`, `Test_AbsenceBannerOneLinePerNoticeAndSignature`, `..JournalLines`).

Règles reprises du C++ (HOLGameScreen.cpp:1144-1211) :
- Attente visible ssi `IsWaitingForPlayers()` ET liste des absents non vide ; texte `En attente de {Players} ({Time})`, Players = noms joints par ", ", Time = m:ss (`HOLHeroBlock::Clock`, déclaration vue dans HOLHeroBlock.as:210), reste = `Max(0, CeilToInt(WaitEnd - ServerTime))`. Journal sur changement de Players seulement (pas à chaque seconde) : `Wait banner: shown, <texte>` puis `Wait banner: hidden` une fois.
- Absence visible ssi `GetAbsenceBannerLines` non vide ; texte = lignes jointes par "\n" ; signature = `N` + pour CHAQUE notice (même sans ligne) `;Nom+` si de retour, `;Nom-` sinon ; journal `Absence banner: shown, <l1> | <l2>` quand la signature change, `Absence banner: hidden` une fois.
- Cas limite repris tel quel : un seul absent au nom vide (signature "" = jamais "montré") ne journalise pas dans le C++ ; idem ici.

## Rouge à voir (mutations, à poser dans le worktree puis remettre)
1. `HOLWaitBanner.as`, `WaitView` : `HOLHeroBlock::Clock(SecondsLeft)` -> `HOLHeroBlock::Clock(SecondsLeft + 1)` : rouge `Test_WaitBannerTextNamesPlayersAndCountsDown` ("2:05" devient "2:06").
2. `HOLWaitBanner.as`, `AbsenceView` : `(bIsBack ? "+" : "-")` -> `(bIsBack ? "-" : "+")` : rouge `Test_AbsenceBannerOneLinePerNoticeAndSignature` ("1;Bob-").
Les tests de la vue (`UHOLWaitBannerView`, widgets) ne sont pas écrits : la vue ne se teste qu'en jeu (scénario ci-dessous). Rien de tautologique : chaque assertion passe par `WaitView`/`AbsenceView`/`Change`.

## (a) Recette : crochet dans `Script\UI\HOLGameScreenScript.as` (3 endroits, 5 lignes)
1. Membre, après `UCommonTextBlock VolcanoText;` (vers la ligne 121) :
```
	// H-1: the wait and absence banners (HOLWaitBanner.as).
	UHOLWaitBannerView WaitBanner;
```
2. `OnInitialized()`, juste après `BuildVolcanoBanner(Frame);` (ligne 245, AVANT `BuildMinimap`, `BuildPauseScreen` reste en dernier, au-dessus) :
```
		WaitBanner = Cast<UHOLWaitBannerView>(NewObject(this, UHOLWaitBannerView));
		WaitBanner.Build(this, Frame);
```
3. `Refresh()` (ligne 1352), juste après `RefreshVolcano();` :
```
		if (WaitBanner != nullptr)
			WaitBanner.Refresh();
```
`Describe()` : ne RIEN ajouter (le rapport hol.HUD.Report du C++ ne parlait pas de ces bannières : grep de WaitPanel / AbsencePanel / WaitText / AbsenceText dans Source\ et Tools\ = HOLGameScreen.cpp et .h seulement, plus les lignes de journal des outils).

## (b) C++ de liaison à AJOUTER (un lot, un build) : `UHOLWaitBannerReadLibrary`, module HeroesOfLiberty (là où est AHOLGameState)
Script : `HOLWaitBannerRead::GetWaitState(Missing, SecondsLeft)` et `HOLWaitBannerRead::GetAbsenceState(Lines, Names, bBack)` (contexte de monde implicite, comme `HOLSiteRules::GetVictoryCount`).
`Source\HeroesOfLiberty\Match\HOLWaitBannerReadLibrary.h` :
```cpp
UCLASS()
class HEROESOFLIBERTY_API UHOLWaitBannerReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** R3.10: the game state's wait for named players. True: waiting. OutMissing: the names still missing; OutSecondsLeft: whole seconds before the wait's end by the server's clock, rounded up, never under 0. False and empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match", meta = (WorldContext = "WorldContextObject"))
	static bool GetWaitState(const UObject* WorldContextObject, TArray<FString>& OutMissing, int32& OutSecondsLeft);

	/** R7.5: the disconnection banner's lines at the server's time (AHOLGameState::GetAbsenceBannerLines, formatted FText::ToString) and ALL the notices (OutNames, OutBack = BackAt > 0), parallel arrays. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match", meta = (WorldContext = "WorldContextObject"))
	static void GetAbsenceState(const UObject* WorldContextObject, TArray<FString>& OutLines, TArray<FString>& OutNames, TArray<bool>& OutBack);
};
```
`.cpp` (includes comme HOLPauseLibrary.cpp : `Engine/Engine.h`, `Engine/World.h`, `Match/HOLGameState.h`) :
```cpp
static const AHOLGameState* WaitBannerState(const UObject* Context)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetGameState<AHOLGameState>() : nullptr;
}
bool UHOLWaitBannerReadLibrary::GetWaitState(const UObject* Ctx, TArray<FString>& OutMissing, int32& OutSecondsLeft)
{
	OutMissing.Reset(); OutSecondsLeft = 0;
	const AHOLGameState* State = WaitBannerState(Ctx);
	if (!State) return false;
	OutMissing = State->GetMissingPlayers();
	OutSecondsLeft = FMath::Max(0, FMath::CeilToInt(State->GetWaitEndSeconds() - State->GetServerWorldTimeSeconds()));
	return State->IsWaitingForPlayers();
}
void UHOLWaitBannerReadLibrary::GetAbsenceState(const UObject* Ctx, TArray<FString>& OutLines, TArray<FString>& OutNames, TArray<bool>& OutBack)
{
	OutLines.Reset(); OutNames.Reset(); OutBack.Reset();
	const AHOLGameState* State = WaitBannerState(Ctx);
	if (!State) return;
	for (const FText& Line : State->GetAbsenceBannerLines(State->GetServerWorldTimeSeconds())) OutLines.Add(Line.ToString());
	for (const FHOLAbsentNotice& Notice : State->GetAbsentNotices()) { OutNames.Add(Notice.Name); OutBack.Add(Notice.BackAt > 0.); }
}
```
Ce qui manque donc dans la base : seulement ces deux lectures (état d'attente de la partie + secondes restantes ; notices d'absence + lignes déjà formatées). `GetAbsenceBannerLines`, `FormatAbsentLine`, `FormatBackLine` (FHOLDisconnectRules, horloge de fenêtre D720) RESTENT en C++ : partagés avec le serveur, textes localisés (LOCTEXT "HOLGameScreen,AbsentLine/BackLine"). `MakePanel`, `MakeText`, `UHOLTextStyleHUDHeading` : déjà exposés (MakePanel/MakeText vus dans HOLScriptScreen.h:87-91 ; la classe de style est un UCLASS, usage `UHOLTextStyleHUDLabel` déjà en script).

## (c) C++ à RETIRER de `HUD\HOLGameScreen.cpp` / `.h` une fois le script en place (même commit que le crochet et la liaison)
- `.cpp` : lignes 567-568 (`RefreshWaitBanner(); RefreshAbsenceBanner();` dans le tick) ; définitions 1144-1211 (`RefreshWaitBanner`, `RefreshAbsenceBanner`) ; `BuildLayout` 3343-3365 (blocs `WaitPanel` et `AbsencePanel`).
- `.h` : membres `WaitPanel`, `WaitText` (286-289), `AbsencePanel`, `AbsenceText` (293-296), `ShownWaitPlayers`, `ShownAbsence` (595-597), déclarations et commentaires 410-420.
- RESTE : `NativeTick` (cadence), `BoundTableState` (lu par d'autres panneaux : tableau des joueurs), `SetShown`/`MakePanel`/`MakeText` du C++ (autres panneaux), `AHOLGameState::GetAbsenceBannerLines`, `FHOLDisconnectRules::Format*`. Retirer le LOCTEXT "WaitingFor" du .cpp (il vit dans le script, même clé). Vérifier après retrait qu'aucun `using`/include ne devient orphelin (warning seulement).

## (d) Outils de test à faire suivre
Rien à porter : ces bannières n'ont ni `Describe*`, ni `ClickWidget`, ni ligne de `hol.HUD.Report` (grep). Le rapport du HUD reste identique SAUF une exception à contrôler (NON VÉRIFIÉ) : les panneaux créés par `MakePanel` entrent dans `DescribeOverlaps` / `DescribeRects` ("HUD rects", lues par `Tools\Scenarios\HudRects.ps1`, `hud_rects_game_test.ps1`, `hud_rects_test.ps1`) ; quand une bannière est visible, `WaitPanel` / `AbsencePanel` peuvent y apparaître et être comparés aux autres panneaux. Lancer `hud_rects_game_test` après le build ; si elle échoue pendant une bannière, c'est cette ligne, pas un bug de bannière.
Ordre d'affichage : le cadre du script est au-dessus du cadre C++ ; les bannières passent donc au-dessus de l'alerte majeure C++ (au même endroit avant le match). Elles laissent passer les clics (HitTestInvisible), rien ne change pour la souris.

## (e) Scénarios et contrôles qui lisent ces bannières (lignes exactes, à conserver)
- `Tools\Tests\quick_check.ps1:2788` (partie net, client 2 = Alice) : `waitlog Wait banner: shown`, `frames 5`, `capture qcnet_2_wait`, signal ; contrôles 6825-6846 : `Wait banner: shown, (.*)$` avec capture du texte attendu `^En attente de Bob \(([01]:[0-5]\d|2:00)\)$`, la capture `Test capture: qcnet_2_wait ` APRÈS le shown, `Wait banner: hidden` APRÈS la capture, fichier `qcnet_2_wait.png`. Journal attendu : `Wait banner: shown, En attente de Bob (1:5x)` puis `Wait banner: hidden` au départ de la partie.
- `Tools\Tests\lobby_match.ps1` (356, 359, 441, 480, 495, 522, 536) : `waitlog Absence banner: shown`, `waitlog Absence banner: shown, Bob`, `waitlog Absence banner: hidden` ; regex `Absence banner: shown, Bob : connexion perdue — 0:(0\d|10)` (tiret cadratin U+2014, vient du C++ par la liaison : même texte) ; `Absence banner: shown, .*$BackRx` où BackRx = `Bob.*de retour` (fr), `Bob: back` (en), japonais (ja) : les lignes viennent du C++ localisé, inchangé. Journal du script : `Log(n"LogHOLUI", ...)` (même motif que HOLNewGameScreen.LogNewGame) : texte identique, catégorie `LogHOLUI` ; les outils ne lisent que le texte. À vérifier une fois : que le tiret cadratin sorte intact dans le journal via `Log` (encodage), sinon la regex de lobby_match:495 ne trouve rien.
- Commandes à lancer par la session principale après build : `quick_check -Parts tests` (les 5 tests neufs, +5 au total), puis la partie `net` (attente) et `lobby_match` (absence, deux scénarios rejoin).

## 4. Ce que l'utilisateur regardera
Le plan cite `disconnect_look.txt` : lu, il montre la TABLE des joueurs (pastille "Connexion perdue"), pas les bannières. Scénarios qui les montrent vraiment :
- Absence : captures `lobby_match_rejoin_away` (Bob : connexion perdue — m:ss) et `lobby_match_rejoin_back` (Bob : de retour) de `lobby_match` ; en solo on peut la provoquer par la commande `hol.Net.FakeAbsent <Nom> <Secondes>` (HeroesOfLibertyGameModeBase.cpp:82, serveur ou solo en partie).
- Attente : seulement par la partie net de `quick_check` (capture `qcnet_2_wait`), il faut un nom attendu du lobby ; pas de scénario solo.
- Proposition : créer `ecran_hud_bannieres.txt` (solo) : `frames 90`, `hol.Net.FakeAbsent Bob 90`, `frames 60`, `capture hud_banniere_absence`, `hol.Net.FakeAbsent Bob -1` (de retour), `frames 60`, `capture hud_banniere_retour`, `hol.Net.FakeAbsent` (efface) ; `.look` : « sous la barre du haut, centré, une bannière sombre "Bob : connexion perdue — 1:2x" puis "Bob : de retour" ; la souris passe à travers ». Syntaxe exacte de la commande à relire (lignes 82-90 du game mode) avant de l'écrire.

## NON VÉRIFIÉ
- Rien n'a été compilé ni lancé : noms Angelscript déclarés vus dans ce code (`MakePanel`, `MakeText`, `SetBrushColor`, `SetJustification`, `Log(n"LogHOLUI", ...)`, `NSLOCTEXT`, `NewObject(this, ...)`) ; pas vus : `UHOLTextStyleHUDHeading` en script (UCLASS C++ du module UI, comme Label), `TArray<bool>` en paramètre de sortie d'une UFUNCTION liée, `HOLWaitBannerRead::` (n'existe pas avant le lot C++), contexte de monde implicite depuis un `UObject` créé par `NewObject(this, ...)`.
- Fond du panneau : C++ = `HUDPanel.CopyWithNewOpacity(0.8)` ; script = `MakePanel` (HUDPanel) teinté `SetBrushColor(1,1,1,0.8)`. Identique si HUDPanel est opaque ; sinon écart d'alpha à voir sur la capture `qcnet_2_wait` (comparer au PNG C++).
- Police : `UHOLTextStyleHUDHeading` comme le C++ ; marges 16 / 88 / 140 recopiées de `ScreenMargin` 16, `QueueItemHeight` 36 (x2 + 52).
- Traductions anglaises et japonaises du texte d'attente : clé "HOLGameScreen,WaitingFor" gardée, mais `Tools\Localization\fill_po.py` et la collecte (`GatherText` des .as) non regardés.
- Anomalie : aucune de blocage. `git status` du worktree ne montre que mes 2 fichiers (commités).
