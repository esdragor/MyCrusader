# GLORFINDEL-1118t : HUD H-2, message court (D313) + population bloquée, en script

Joueur : le texte qui apparaît au-dessus de la file / grille (« Pas assez de bois », « Population maximale — construisez des maisons »...) et s'efface après 3 s ne change pas ; seule sa vue passe en script.
Commit : `a83b594` sur `net-playtest` (2 fichiers neufs). AUCUN build, AUCUN test lancé, rien de câblé. NON COMPILÉ.

Fichiers : `Script\UI\HOLShortMessage.as` (règles + vue), `Script\UI\HOLShortMessage_Test.as` (6 tests, mutations dans les commentaires).
Les .as ne dépendent d'AUCUNE nouvelle fonction de base (seulement `MakePanel`, `MakeText`, `GetGameScreenBox`, `GetViewportSlateSize`, `AddThreadLine`, `Log(n"LogHOLUI", ...)`, `HOLAlertRules::PopulationFullText`, `UHOLTextStyleHUDAlert` (UCLASS comme `HUDLabel`)) : ils compilent sans le lot C++. Mais NE PAS fusionner avant d'avoir compilé les scripts (NON VÉRIFIÉ : struct à membres UObject `FHOLShortMessageParts`, `FVector2D&` en paramètre, `Log` global lu dans HOLNewGameScreen.as:104 et HOLAIBrain.as mais déclaration non vue).

## Ce que couvre déjà le script, ce qui reste en C++
- Script existant : `HOLGameScreenScript.as:1373 RefreshPopulationAlert` + `HOLAlertRules.as` = la LIGNE DU FIL « Population pleine » quand Used >= Cap (muette, 30 s de répétition, D202). Aucun message court, aucune lecture de file : rien en double avec ce lot.
- C++ aujourd'hui (HOLGameScreen.cpp) : `HandleCommandFeedback` 2737 (reçoit `OnCommandFeedback` du contrôleur, 514), `ShowMessage` 2747, `UpdateMessage` 2765 (age, fondu 0,5 s, masque à 3 s, appelé à chaque image, 656), `CheckPopulationBlocked` 2787 (parcourt les bâtiments du joueur : tête de file à 100 % sans place, D829 ; au front montant : message `PopulationFull` + ligne du fil « Population pleine » + log `HUD: production held at 100 % ...`), mise en page 3648-3660 (`MessagePanel` dans `CommandColumn`, au-dessus de `QueuePanel`), membres `.h` 143-144, 279, 282, 406-408, 425, 658-661, `Describe` 3109 (`, message <clé>`).
- Reste C++ après migration : `GetFeedbackKey` / `GetFeedbackText` (clés + LOCTEXT, 162-236 : le script reçoit clé et texte), `HandleCommandFeedback`, `MessageKey` (miroir pour `Describe`), la lecture des bâtiments (`IsProductionHeldForRoom`, itérateur de bâtiments, `HOLProduction::IsWaitingForRoom`), `AddThreadLine`, la note de file « Population pleine » (2680, `IsPopulationBlocked`).
- Doublon déjà existant (non changé) : le script écrit « Population pleine » au fil quand Used >= Cap (à la mise en file du 2e Paysan) ET le C++ (donc bientôt le script, `CheckPopulationHeld`) l'écrit quand la production est bloquée à 100 % : deux lignes possibles. À regarder à l'essai (le fil déduplique-t-il ?) ; pas touché.
- Écart voulu : en C++ un message sans clé (`GetFeedbackKey` défaut) ne finissait jamais (clé vide = pas de minuteur) ; en script `bActive` le fait finir. Priorité : le dernier message gagne et remet l'âge à 0 (comme `ShowMessage`).

## Recette d'intégration (appliquer dans cet ordre, UN build C++)

### (b) C++ à AJOUTER (un seul lot)
`HUD\HOLScriptScreen.h` (public, à côté de `Refresh`) :
```
/** The game screen's short message (D313) for the script to show: Key names it in the logs and Describe, Text is the C++'s (GetFeedbackText). */
UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
void ShortMessage(const FString& Key, const FText& Text);
/** Every frame (the message's fade is smooth, Refresh runs 4 times a second). */
UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
void FrameTick(float DeltaSeconds);
/** True when a building of the shown player holds a finished unit for lack of room (the C++ CheckPopulationBlocked's loop, D829). */
UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
bool IsProductionHeldForRoom() const;
/** The key Describe prints (", message <Key>"; "" none): the script mirrors it, the C++ line of hol.HUD.Report stays as it was. */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
void SetShownMessageKey(const FString& Key);
/** Wraps a text at Width (Slate units): the message wraps at the grid's width less 24 (308). */
UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
static void SetTextWrapWidth(UCommonTextBlock* Text, float Width);
```
`HOLScriptScreen.cpp` : `IsProductionHeldForRoom` = `GetTypedOuter<UHOLGameScreen>()->IsProductionHeldForRoom()` (false sans écran) ; `SetShownMessageKey` -> `GameScreen->SetMessageKeyMirror(Key)` ; `SetTextWrapWidth` = `Text->SetWrapTextAt(Width)` ; dans `NativeTick` (368, après `Super::NativeTick`) : `FrameTick(InDeltaTime);`.
`HOLGameScreen.h/.cpp` : `bool IsProductionHeldForRoom() const;` (corps = lignes 2791-2811 de `CheckPopulationBlocked`, renvoie `bBlocked`, mêmes `FHOLBuildingIterator` / `IsWaitingForRoom` / `IsPopulationBlocked`) ; `void SetMessageKeyMirror(const FString& Key) { MessageKey = Key; }` ; `ShowMessage` devient : `MessageKey = Key; if (ScriptScreen) { ScriptScreen->ShortMessage(Key, Text); }` (garde `MessageKey` : `Describe` 3109 inchangé).

### (a) Crochet dans `Script\UI\HOLGameScreenScript.as` (5 emplacements)
1. Membres, après `VolcanoSlot`/`VolcanoText` (~l.122) :
```
	// H-2 (D313, D829): the short message and the held production note (HOLShortMessage.as).
	FHOLShortMessageParts MessageParts;
	FHOLShortMessageState MessageState;
	FHOLPopulationHeldWatch HeldWatch;
```
2. `OnInitialized`, à la suite de `BuildVolcanoBanner(Frame);` (l.245) :
```
		MessageParts = HOLShortMessage::Build(this, Frame);
		SetTextWrapWidth(MessageParts.Text, 308.0);
```
3. `Refresh()` (l.1352), après `RefreshPopulationAlert();` : `RefreshShortMessage();`
4. Nouvelles méthodes de la classe, après `RefreshPopulationAlert` (l.1383) :
```
	void RefreshShortMessage()
	{
		HOLShortMessage::Place(this, MessageParts);
		if (HOLShortMessage::CheckPopulationHeld(this, MessageParts, MessageState, HeldWatch, IsProductionHeldForRoom()))
			SetShownMessageKey(HOLShortMessage::PopulationKey);
	}

	UFUNCTION(BlueprintOverride)
	void ShortMessage(FString Key, FText Text)
	{
		HOLShortMessage::ShowText(MessageParts, MessageState, Key, Text);
		SetShownMessageKey(Key);
	}

	UFUNCTION(BlueprintOverride)
	void FrameTick(float32 DeltaSeconds)
	{
		if (HOLShortMessage::Tick(MessageParts, MessageState, DeltaSeconds))
			SetShownMessageKey("");
	}
```
5. `Describe()` : RIEN à ajouter (le rapport du HUD garde `, message <clé>` dans la ligne du C++ via le miroir ; une ligne script en plus changerait le rapport).

Notes : `ShowText` écrit `HUD message: <clé> "<texte>"` (identique à l'UE_LOG C++), `CheckPopulationHeld` écrit ensuite la ligne du fil puis `HUD: production held at 100 % (population full), line to the thread`, dans l'ordre du C++. La signature d'override peut demander `const FString& Key` selon la plugin : prendre la forme déjà employée par `BuildingSelected` dans HOLGameScreenScript.as.

### (c) C++ à RETIRER de HOLGameScreen.cpp / .h (après que le script compile et que le lot ci-dessus passe)
- .cpp : `UpdateMessage` (2765-2785) ; `CheckPopulationBlocked` (2787-2826 : après avoir déplacé la boucle dans `IsProductionHeldForRoom`) ; leurs deux appels (655-656) ; mise en page 3648-3660 (`MessagePanel` / `MessageBox` / `MessageText`, jusqu'à `SetVisibility(Collapsed)`) ; corps de `ShowMessage` (remplacé, cf. b).
- .h : `MessageSeconds`, `MessageFadeSeconds` (143-144), `MessagePanel` (279), `MessageText` (282), `UpdateMessage` (408), `CheckPopulationBlocked` (425), `MessageAge` (659), `bPopulationBlocked` (661).
- À GARDER : `MessageKey`, `ShowMessage`, `HandleCommandFeedback`, `GetFeedbackKey` / `GetFeedbackText`, `AddThreadLine`, la note de file « Population pleine » (2680). Avant de supprimer `LOCTEXT("PopulationFull", ...)` de 2822 : grep la clé (la note de file de la file peut s'en servir) et `Tools\Localization\fill_po.py` (les clés `HOLGameScreen,PopulationMax` / `PopulationFull` ont la même source en C++ et en script : pas de conflit si le texte source reste identique).
- Mesure : `FrameTick` = un appel script par image ; vérifier avec `hol.HUD.Report` / le temps d'image que c'est négligeable (sinon n'appeler que si un message est actif : booléen côté C++ `MessageKey` non vide ou `bHeld`).

### (d) Outils de test
- `Describe` de `HOLGameScreen.cpp:3109` : inchangé (miroir `MessageKey`) ; `hol.HUD.Report` : la ligne `HUD report: ... , message <clé>` reste identique (le message dure 3 s : `frames 3` suffit comme aujourd'hui). `ClickWidget` : le panneau ne prend pas les clics (HitTestInvisible), rien à suivre.
- `HUD rects` / `DescribeOverlaps` : `MessagePanel` devient un panneau du script, donc compté dans « HUD rects » pendant qu'il est visible et comparé aux autres panneaux du script et au panneau de triche ; il est posé juste au-dessus de `QueuePanel` (ou `GridPanel`), et ces deux ne sont pas dans la liste comparée. NON VÉRIFIÉ : `Tools\Scenarios\hud_rects_game_test.ps1` / `HUDRects.ps1` doivent tolérer une entrée `MessagePanel=` de plus (grep des noms attendus avant le build).

### (e) Scénarios et quick_check qui lisent ce message (lignes exactes à retrouver)
- `Tools\Scenarios\alert_population.txt` : `waitlog production held at 100 %`, `hol.HUD.Report`, `waitlog Population pleine]` (fil), capture `alert_population`.
- `Tools\Tests\quick_check.ps1` : 1672 et 1675 (`waitlog HUD message: NotEnoughWood`, une fois par langue : 3 lignes `HUD message: NotEnoughWood` en 4263, le texte vient du C++ localisé) ; 1710 (`waitlog Command feedback: population full` puis `waitlog HUD message: PopulationFull`, capture `build_hud_popfull`) ; 1716 / 1719 (`HUD message: NoMilitaryBuilding`, `NoTechnologicalBuilding`) ; contrôles 4261-4263 (`HUD message: NotEnoughWood "Pas assez de bois"`, rapport `, message NotEnoughWood`) ; 4283-4285 (`HUD message: PopulationFull`, `queued Peasant`, rapport `message PopulationFull`) ; 4288-4291 (`HUD message: NoMilitaryBuilding "Aucun b.+timent militaire"`, rapport `selection none, grid hidden, orders none, queue hidden, message NoMilitaryBuilding`).
- Lignes de journal exactes attendues : `HUD message: <Clé> "<texte>"` ; `HUD: production held at 100 % (population full), line to the thread`. Le script les écrit par `Log(n"LogHOLUI", ...)` (le C++ : `UE_LOG(LogHOLUI, Display)`) : NON VÉRIFIÉ que la verbosité du `Log` global passe le même filtre ; la capture `build_hud_popfull` change peu (message posé au-dessus de la file, au lieu d'être dans la colonne) : à comparer.
- Commande à lancer après le build : `quick_check -Parts <les parties qui contiennent ces lignes>` (grep `build hud` dans quick_check.ps1) puis `play.ps1 alert_population -Hidden -Root <worktree>` ; tests script : les 6 `Test_ShortMessage*` / `Test_PopulationHeld*` (rouge à VOIR : appliquer une mutation du commentaire de chaque test, une à la fois).

## 4. Ce que le joueur regarde à l'essai
`alert_population.txt` (Plaine, seul, ~1 min 30) : à 10 / 10 le 2e Paysan reste à 100 %, le message « Population maximale — construisez des maisons » apparaît UNE fois au-dessus de la file (bas droite du HUD), s'efface en fondu après 3 s, la ligne « Population pleine » est au fil en haut à droite ; tuer un Paysan ou bâtir une maison : le Paysan sort. À ajouter à l'essai : une touche sans bois (« Pas assez de bois ») pour voir le message et son effacement (scénario `build_hud` de quick_check, pas de scénario joueur dédié).

## Manque dans la base (liste)
`ShortMessage`, `FrameTick`, `IsProductionHeldForRoom`, `SetShownMessageKey`, `SetTextWrapWidth` (recette b). Aucun n'est nécessaire à la compilation des deux .as.

## Champs
BUILDS_DEMANDES: 1 (lot C++ de la recette, à lancer par la session principale) ; TESTS_SCRIPT_AJOUTES: 6 (Test_ShortMessageOpacityFadesOverTheLastHalfSecond, Test_ShortMessageLastsThreeSecondsThenIsGone, Test_ShortMessageLatestWinsAndRestartsTheClock, Test_PopulationHeldIsSaidOncePerBlock, Test_ShortMessageJournalLinesAreTheCppOnes, Test_ShortMessagePlaceIsAboveTheQueueOrGrid) ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 (~110 à supprimer en recette c) ; TEST_ROUGE: non vu (rien lancé) ; TYPES_A_EXPOSER: aucun nouveau type, 5 fonctions de base ; tests tautologiques : `Test_ShortMessageJournalLinesAreTheCppOnes` compare des littéraux à des fonctions de formatage (assumé : gèle le mot à mot du journal).
