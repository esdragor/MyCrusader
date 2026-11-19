# HALDIR-1119a : HUD H-4, tableau des joueurs (2026-11-19, étape 4, migration D851)

Impact joueur : aucun changement visible attendu ; les lignes du tableau (couleur, nom, équipe/relation, vies, phase du forum) passent du C++ au script, mêmes widgets, même place, mêmes lignes de journal.
NON COMPILÉ, NON LANCÉ : 2 fichiers script neufs, aucun build, aucun quick_check. Les .as NE COMPILENT PAS avant le lot suivant (liaison C++ + hook) : ils appellent `HOLPlayersTableRead::`, `FHOLPlayersTableRowRead`, `UHOLScriptScreen::GetPlayersPanel()` et `Screen.ConstructWidget(...)` qui n'existent pas ou ne sont pas vérifiés. Le crochet n'est PAS posé.
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: FHOLPlayersTableRowRead (USTRUCT), UHOLPlayersTableReadLibrary, UHOLScriptScreen::GetPlayersPanel.

## Fait
Commit sur `maps` (worktree HeroesOfLiberty-Haldir), 2 fichiers neufs dans `Script\UI\` :
- `HOLPlayerTable.as` : structs `FHOLPlayerRow` (ligne vue par les règles), `FHOLPlayerRowView` (ligne dessinée), `FHOLPlayerTableMemory`, `FHOLPlayerTablePlan` ; `namespace HOLPlayerTable` (règles pures : `RowView`, `NoRowView`, `ViewAt`, `RelationKind`, `RelationRoom`, `Signature`, `NamesLine`, `StatesLine`, `NamesLogLine`, `StatesLogLine`, `Plan`, `FromRead`, `NewGroupGap/GreyedOpacity/Kind*` en FONCTIONS) ; `class UHOLPlayerTablePanel : UObject` (`Build(Screen, PlayersPanel)`, `Refresh()`, `DrawRow`).
- `HOLPlayerTable_Test.as` : 5 tests purs en littéraux : `Test_PlayersTableRowFadesStrikesAndGapsLikeCpp`, `Test_PlayersTableRelationIconAndRoom`, `Test_PlayersTableSignatureJoinsReportsWithBars`, `Test_PlayersTableNamesAndPastillesLines`, `Test_PlayersTablePlanRedrawsAndLogsOnlyOnChange`.

Décision de découpage (un seul source) : les LIGNES (qui, ordre, groupes d'équipe, textes « 2 vies » / « Ruine » / « Reconstruction 40 % », relation, masques absent/IA, texte `Shown`) restent calculées par `HOLForumRules::GetPlayersTable` (C++ partagé avec ses tests C++ et le HUD report). Le script possède le DESSIN d'une ligne (opacité 0,45, trou 6 px, barre de nom, icône ou point de relation, pièce 16 px) et le JOURNAL (signature, `Players table: ...`, `Players table state: ...`). DOUTE à trancher par la session principale : si l'utilisateur veut AUSSI les règles de ligne en script, c'est un lot de plus (réécrire GetPlayersTable + ses tests C++, D838 dit que les tests de simulation Mass restent C++ mais GetPlayersTable n'est pas Mass).

Règles C++ reprises (HOLGameScreen.cpp:1044-1142, 3419-3482) :
- Rien n'est redessiné ni journalisé tant que `Shown` (rapports de lignes joints par " | ") ne change pas ; au changement : journal `Players table: <noms joints par ", ">` si les noms ont changé (mémoire initiale vide : une table vide au départ ne journalise pas) ; puis `Players table state: <Nom=IA | Nom=Connexion perdue, ...>` ou `none`, si la liste des pastilles a changé (l'IA l'emporte sur la connexion perdue) ; panneau visible ssi au moins une ligne (HitTestInvisible) ; ligne visible SelfHitTestInvisible, au-delà de la dernière : Collapsed.
- Par ligne : trou de 6 px au-dessus du premier rang d'une nouvelle équipe, opacité 0,45 si vaincu, nom dans la couleur du slot, barre de 2 px dans la même couleur si vaincu, relation (allié = bouclier, ennemi = épées, neutre = point gris 10 px rayon 5 dans la pièce 16 px, soi = rien mais pièce gardée, spectateur = pas de pièce), vies et phase.

## Rouge à voir (deux mutations, à poser dans le worktree puis remettre)
1. `HOLPlayerTable.as`, `GreyedOpacity()` : `0.45` -> `0.5` : rouge `Test_PlayersTableRowFadesStrikesAndGapsLikeCpp` ("faded to 45 %").
2. `HOLPlayerTable.as`, `StatesLine` : `(Rows[Index].bAIHeld ? "IA" : "Connexion perdue")` -> `(Rows[Index].bAIHeld ? "Connexion perdue" : "IA")` : rouge `Test_PlayersTableNamesAndPastillesLines` et `...PlanRedrawsAndLogsOnlyOnChange` (ligne `Bob=Connexion perdue`).
Tests de la vue (`UHOLPlayerTablePanel`) non écrits : la vue ne se teste qu'en jeu. Un test d'accord n'a pas de sens (pas de jumeau : le C++ de la vue disparaît au même commit).

## (a) Liaison C++ à écrire (un lot, un build)
Module HeroesOfLiberty (là où sont AHOLGameState et HOLForumRules) : `Source\HeroesOfLiberty\Match\HOLPlayersTableReadLibrary.h/.cpp`, namespace script `HOLPlayersTableRead` (contexte de monde implicite, comme `HOLDiplomacyRead`, `HOLWaitBannerRead`).
```cpp
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLPlayersTableRowRead
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) int32 Slot = -1;
	UPROPERTY(BlueprintReadWrite) int32 ColorIndex = 0;
	UPROPERTY(BlueprintReadWrite) FString Name;     // Row.Name.ToString()
	UPROPERTY(BlueprintReadWrite) FString Lives;    // Row.Lives.ToString()
	UPROPERTY(BlueprintReadWrite) FString State;    // Row.State.ToString()
	UPROPERTY(BlueprintReadWrite) FLinearColor Color; // FLinearColor(HOLPlayerColors::GetColor(Row.ColorIndex)) : la MÊME conversion sRGB que le C++
	UPROPERTY(BlueprintReadWrite) bool bGreyed = false, bStruck = false, bAway = false, bAIHeld = false, bNewGroup = false;
	UPROPERTY(BlueprintReadWrite) int32 Relation = 0; // (int32)Row.Relation : None 0, Self 1, Ally 2, Enemy 3, Neutral 4
	UPROPERTY(BlueprintReadWrite) FString Shown;    // Row.Shown
};
UCLASS() class HEROESOFLIBERTY_API UHOLPlayersTableReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** FHOLMatchConfig::MaxSlots (the seats the table builds). */
	UFUNCTION(BlueprintPure, Category="HeroesOfLiberty|Match") static int32 GetMaxSlots();
	/** The table's rows (HOLForumRules::GetPlayersTable) as RefreshPlayersTable builds them; empty without a game state. */
	UFUNCTION(BlueprintPure, Category="HeroesOfLiberty|Match", meta=(WorldContext="WorldContextObject"))
	static void GetRows(const UObject* WorldContextObject, TArray<FHOLPlayersTableRowRead>& OutRows);
};
```
`GetRows` recopie EXACTEMENT HOLGameScreen.cpp:1049-1066 : `GameState = World->GetGameState<AHOLGameState>()` ; pour `Seat` de 0 à `MaxSlots-1` : `Forums.Add(GameState->GetForumState(Seat))`, `Tiers.Add(GameState->GetForumTuning(Seat).Tier)` ; `Book = IsDiplomacyMode() ? GetDiplomacyBook() : FBook()` ; `HOLForumRules::GetPlayersTable(GetLaunchConfig(), Forums, GetDefeatedMask(), LocalSlot, FHOLForumTuning::FromSettings(), Tiers, IsDiplomacyMode() ? &Book : nullptr, GetAbsentMask(), GetAIHeldMask())`, LocalSlot = place du joueur local = le player state du contrôleur 0 (`PlayerState->GetPlayerIndex()` ou INDEX_NONE ; le script a déjà `UHOLScriptScreen::GetLocalSlot()` : soit le lire côté C++ via le contrôleur local, soit ajouter le paramètre `int32 LocalSlot` et l'appeler avec `Screen.GetLocalSlot()` : RECOMMANDÉ, une ligne de plus dans `HOLPlayerTable::...` (le .as appelle aujourd'hui `GetRows(Read)` sans paramètre : à ajouter). Ce que lit chaque appel : places et formes des forums (`GetForumState`), niveaux (`GetForumTuning`), masque des vaincus, masque absents, masque IA, mode Diplomatie + livre, configuration de lancement.
Ajout dans `HOLScriptScreen.h/.cpp` (module UI) : `UFUNCTION(BlueprintCallable) UBorder* GetPlayersPanel() const;` retourne `PlayersPanel` du C++. Vérifier que `ConstructWidget(UClass, FName)` est appelable par `Screen.ConstructWidget` depuis une classe script externe (sinon l'exposer ou passer le `UWidgetTree`).
Fonctions du .as qui appellent la liaison : `UHOLPlayerTablePanel::Build` (`GetMaxSlots`, `Screen.GetPlayersPanel` via le hook), `Refresh` (`GetRows`), `HOLPlayerTable::FromRead` (champs de la struct). `HOLRelationIconsRead::GetAllyPath/GetEnemyPath/GetRelationX` : déjà exposées (HOLRelationIconsReadLibrary.h).
`Describe` : le C++ `HOLGameScreen.cpp:3130` imprime `players [%s]` avec `ShownPlayersTable` (lu par `quick_check.ps1:3226` : `, players \[([^\]]*)\]`, puis `Row0 -match "^slot 0 you colour \d+ lives ..."` lignes 3235-3298). Choix : ce `Describe` appelle la même `GetRows` C++ et joint les `Shown` par " | " (une source, pas de membre `ShownPlayersTable`).

## (b) Crochet dans `Script\UI\HOLGameScreenScript.as` (3 endroits)
1. Membre, après `UCommonTextBlock VolcanoText;` (ligne 121) : `UHOLPlayerTablePanel PlayerTable;`
2. `OnInitialized()` (ligne 236), après `BuildVolcanoBanner(Frame);` (245) et avant `BuildMinimap(Frame);` : `PlayerTable = Cast<UHOLPlayerTablePanel>(NewObject(this, UHOLPlayerTablePanel)); PlayerTable.Build(this, GetPlayersPanel());`
3. `Refresh()` (1352), après `RefreshVolcano();` (1360) : `if (PlayerTable != nullptr) PlayerTable.Refresh();`
`Refresh()` tourne à chaque trame (NativeTick du script) ; pas de plafond 0,25 s (le C++ avait `GroupRefreshInterval = 0.25` + 3 abonnements OnForumsChanged / OnVictoryChanged / OnDiplomacyChanged) : le script lit chaque trame, la signature coupe le travail ; si le coût se voit, ajouter un cumul de temps (non fait). Ordre : `PlayersPanel` reste un enfant de `RightColumn` du C++ (au-dessus du fil de messages, MessagesList, qui reste C++) : AUCUN changement de place ni de z-order. Raison de garder `PlayersPanel`/`RightColumn` en C++ : le script lit `GetGameScreenBox(n"PlayersPanel")` (HOLGameScreenScript.as:699, place du panneau Diplomatie) et `HOLScriptScreen.cpp:222` (DescribeOverlaps) ; ils continuent de marcher.

## (c) C++ à RETIRER (même commit que le crochet et la liaison)
- `HOLGameScreen.cpp` : `RefreshPlayersTable` 1044-1142 ; appels 524 et 563-566 (cumul `SinceTableRefresh` + appel) ; abonnements 739-741 (`TableForumsHandle`, `TableVictoryHandle`, `TableDiplomacyHandle`, et leur désabonnement vers 718-735) ; dans `BuildLayout` 3419-3482 (boucle des places : `PlayersList` et tout ce qui suit jusqu'à la fin de la boucle `Seat`) ; `LoadObject` de `AllyIcon`/`EnemyIcon` (3424-3425) ; ligne 3130 (`ShownPlayersTable` dans Describe : remplacer par l'appel à `GetRows`) ; `#include "HUD/HOLRelationIcons.h"` (40) si plus utilisé.
- `HOLGameScreen.h` : `PlayerRows`, `PlayerNames`, `PlayerRelations`, `AllyIcon`, `EnemyIcon`, `PlayerStrikes`, `PlayerLives`, `PlayerStates` (505-525), `SinceTableRefresh` (530), `ShownPlayersTable`, `ShownPlayerNames`, `ShownPlayerStates` (589-593), déclaration `RefreshPlayersTable` (503), handles d'abonnement.
- RESTE C++ : `PlayersPanel` (MakePanel, brush `HUDPanel` à 80 %, marge 8/4, Collapsed, enfant de `RightColumn`) ; `RightColumn`, `MessagesList` et le fil de messages ; `BoundTableState` (lu par ~15 autres endroits : paliers, marché, grille) ; `HOLForumRules::GetPlayersTable` et ses tests C++ ; `SetShown`, `MakePanel`, `MakeText`, `AddToRow`.

## (d) Outils -> ce qu'ils lisent -> où le script l'écrit
| Outil (fichier:ligne) | Lit | Le script l'écrit |
|---|---|---|
| `Tools\Tests\lobby_match.ps1:242` | `Players table: Vous, IA Normal 2$` (journal Alice) | `Refresh`, `Log(n"LogHOLUI", NamesLog)` ; texte `Players table: <noms>` mot pour mot (catégorie `LogHOLUI` comme le C++) |
| `lobby_match.ps1:652`, `:682` | `Players table: Vous, Bob` / `Alice, Vous` / `Alice, Bob` (fin de ligne `$`) | idem |
| `lobby_match.ps1:524-526` | `Players table state: .*Bob=Connexion perdue`, `...Bob=IA`, dernière ligne `Players table state: none` | idem, `StatesLogLine` |
| `lobby_match.ps1:233, 633` | filtre `Players table` et `LogHOLUI: Display: (...)` du journal du jeu | `Log` d'Angelscript doit sortir en `LogHOLUI: Display:` : même point à contrôler que GALADRIEL-1118t (niveau `Display` par défaut de `Log` ; sinon `LogDisplay`/équivalent) |
| `Tools\Tests\quick_check.ps1:3226-3298` | `HUD report: ..., players [<Shown> | <Shown>]` puis `Row0` = `slot 0 you colour N lives 1 ruin/rebuilding N%/standing`, `... defeated greyed struck` | `Describe` C++ (HOLGameScreen.cpp:3130) via `GetRows` (voir (a)) ; texte `Shown` inchangé car calculé par `GetPlayersTable` |
| `Tools\Scenarios\spectator_teams.txt`, `forum_lives.txt` | `hol.HUD.Report 1` (même rapport) | idem |
| `Tools\Scenarios\preflight.ps1:138`, `HOLScriptScreen.cpp:222`, `HOLGameScreenScript.as:699` | boîte du widget `PlayersPanel` | inchangé : `PlayersPanel` reste le widget C++ ; le script remplit son contenu |
| Noms de widgets (aucun outil ne les vise : grep `PlayerRow|PlayerName|PlayerLives|PlayerRelation|PlayerStrike` dans Tools\ = rien) | `PlayersList`, `PlayerRow<N>`, `PlayerRelation[Box]<N>`, `PlayerName[Overlay|Box]<N>`, `PlayerNameStrike[Box]<N>`, `PlayerLives[Box]<N>`, `PlayerState[Box]<N>` | gardés mot pour mot dans `Build` (`DescribeRects` du HUD les voit si visibles : lancer `hud_rects_game_test` après build) |
Note : `HudRects.ps1` / `hud_rects*` ne nomment pas ces widgets (grep « Player » : rien ou non relu : NON VÉRIFIÉ pour `hud_rects_game_test.ps1` qui lit tous les rectangles visibles).

## (e) Suspects de compilation (ordre de probabilité)
1. `FSlateBrush Dot; Dot.DrawAs / TintColor / ImageSize / OutlineSettings.RoundingType / CornerRadii` : accès aux champs de FSlateBrush et de FSlateBrushOutlineSettings en script. Repli sans risque : une fonction C++ `UHOLScriptScreen::SetImageDot(UImage*, FLinearColor, float Size, float Radius)` (le lot C++ y est déjà).
2. `Screen.ConstructWidget(UVerticalBox, FName)` depuis une autre classe que l'écran (le script de l'écran l'appelle sans préfixe) ; sinon exposer ou donner le `WidgetTree`.
3. `NameOverlay.Slot` et `Cast<USizeBoxSlot>` : `UWidget::Slot` visible en script ? Sinon retirer l'alignement Left du slot du contenu du SizeBox (effet : la barre de rayure aurait la largeur de la boîte de 64 px mini au lieu de celle du nom).
4. `TArray<FHOLPlayersTableRowRead>` en paramètre de sortie d'une UFUNCTION liée (comme `TArray<bool>` de GALADRIEL, déjà un doute) ; `FHOLPlayersTableRowRead` en USTRUCT : champs `bool` multiples sur une ligne `UPROPERTY` (écrire un champ par ligne si le générateur refuse).
5. `RowSlots.Add(List.AddChildToVerticalBox(Row))` (la fonction renvoie `UVerticalBoxSlot`, vu au script de l'écran, ligne 417) ; `TSubclassOf<UCommonTextStyle>` en paramètre de `AddCell` avec `UHOLTextStyleHUDLabel` : le script de l'écran passe la classe directe à `MakeText` (ligne 449), conversion implicite attendue.
6. `UHOLTextStyleHUDPlayerName` : UCLASS C++ (HOLUIStyles.h:134) mais jamais utilisée en script : vérifier qu'il est exporté.
7. `AddToRow` du C++ non relu : j'ai supposé centré en vertical + marge à droite comme `DipAdd`. Comparer la capture au PNG du C++ (alignement vertical des vies et des phases).
8. `Row.SetRenderOpacity` : usage vu (HOLGameScreenScript.as:735) ; `SetColorAndOpacity(FLinearColor)` sur `UCommonTextBlock` et sur `UImage` : vus en script.
9. Tests : appel de méthode comme argument d'`AssertEquals` évité (variables locales) ; `FUnitTest& T` comme dans les tests d'hier ; `PT_Row` global à un fichier (utilisé seulement dans ce fichier).

## (f) Deux rouges (voir plus haut) ; commande : `quick_check -Parts tests`, +5 tests au total attendus.

## (g) Scénario d'essai de 3 min
`spectator_teams.txt` et `forum_lives.txt` ne suffisent pas pour VOIR : ils lisent le rapport texte (`hol.HUD.Report`), seul `forum_lives` a un `note` qui parle du tableau (notes 1, 5, 6 : « 2 vies » chacun, « 1 vie » + « Ruine », « Vaincu ») ; `spectator_teams` n'a aucune note sur le tableau. Proposition : `ecran_hud_joueurs` (partie solo à 2 joueurs + IA, équipes, mode Diplomatie non requis).
`ecran_hud_joueurs.txt` : `frames 90` ; `hol.HUD.Report 1` ; `capture hud_joueurs_depart` ; `note 1. Haut droite sous le bouton du menu : un tableau sombre, une ligne par joueur, le nom de chacun dans sa couleur, « Vous » en premier, « N vies » et la phase à droite ; un bouclier vert devant les alliés, des épées rouges devant les ennemis, rien devant « Vous » (mais les noms alignés)` ; `waitkey Entrée : forum de l'IA détruit` ; commande de triche de chute de forum (celle de `forum_lives.txt`, lignes 5 à 7 : à relire) ; `frames 60` ; `capture hud_joueurs_vaincu` ; `note 2. La ligne de l'IA : nom barré dans sa couleur, ligne pâle (45 %), « Vaincu »` ; `hol.Net.FakeAbsent Bob 90` si un nom « Bob » existe (sinon omettre) ; `frames 60` ; `capture hud_joueurs_absent` ; `note 3. Pastille « Connexion perdue » à droite de Bob`.
`.look` : « haut droite : tableau sombre à bords arrondis, lignes alignées (pièce de 16 px, nom, vies 44 px mini, phase), un petit trou de 6 px entre deux équipes ; la souris passe à travers ; la capture doit être identique pixel pour pixel au C++ d'avant (comparer avec une capture prise sur la branche main sans le lot) ». Tout PNG à ouvrir et voir avant d'écrire l'essai (L15-8).

## NON VÉRIFIÉ
- Rien compilé, rien lancé. Syntaxes Angelscript vues dans le dépôt : `ConstructWidget`, `MakeText`, `AddChildToVerticalBox`, `SetPadding(FMargin(...))`, `SetBrushFromTexture(tex, false)`, `LoadObject(nullptr, path)`, `Log(n"LogHOLUI", ...)`, `FText::FromString`, `f"..."`. Pas vus : tout ce qui est listé en (e).
- `SetShown` du C++ (ligne 339) non relu : j'ai supposé `Collapsed` quand caché.
- Latence : le C++ réagissait par événement (OnForumsChanged...) ; le script lit à chaque trame (au plus une trame de retard).
- Le fichier non suivi `Content\Maps\Crown.umap` du worktree n'est pas touché.
