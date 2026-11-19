# HALDIR-1119b : HUD H-4, tableau des joueurs (2026-11-19, etape 4, migration D851)
Impact joueur : aucun changement visible attendu ; le tableau (haut droite) est dessine par le script, memes widgets, memes noms, memes lignes de journal.
NON COMPILE, NON LANCE : aucun build, aucun quick_check. Worktree HeroesOfLiberty-Haldir, branche maps, jamais pousse, merge main non fait.

## Commits (maps)
1. 763373e LIAISON : Source\HeroesOfLiberty\Match\HOLPlayersTableReadLibrary.h/.cpp (FHOLPlayersTableRowRead, UHOLPlayersTableReadLibrary : GetMaxSlots, GetRows(WorldContext, LocalSlot, OutRows), et BuildRows(GameState, LocalSlot) statique C++ non script) ; HOLScriptScreen.h/.cpp : GetPlayersPanel() et GetShownSlot() ; HOLGameScreen.h : GetPlayersPanel() inline ; HOLPlayerTable.as : membre Screen, GetRows(Screen.GetShownSlot(), Read).
2. bdc8857 BASCULE : HOLGameScreenScript.as (membre PlayerTable, Build apres BuildVolcanoBanner, Refresh apres RefreshVolcano) ET HOLGameScreen.cpp ne construit/remplit/rafraichit plus le tableau.
3. e3be25f RETRAIT : HOLGameScreen.h (membres du tableau, handles, Shown*).
4. (dernier commit) scenario Tools\Scenarios\ecran_hud_joueurs.txt/.look/.json.

## Choix
- LocalSlot : le C++ lisait BoundPlayerState (AHOLPlayerState lie par l'ecran, y compris la place suivie) ; la recette recommandait Screen.GetLocalSlot() = GetOwningPlayerState, qui peut differer. J'ai ajoute UHOLScriptScreen::GetShownSlot() = GetTypedOuter<UHOLGameScreen>()->GetBoundPlayerState()->GetPlayerIndex() (comme GetPopulationNow) : exactement la valeur d'avant. GetRows prend ce LocalSlot, le .as l'appelle avec Screen.GetShownSlot().
- Lignes : toujours HOLForumRules::GetPlayersTable (une seule source) via BuildRows, partagee par GetRows (script) et Describe (C++).
- Describe (DescribeForum) : recalcule par BuildRows(BoundTableState, index de BoundPlayerState) et joint les Shown par " | " : meme texte "selection..., players [..]" qu'avant (lu par quick_check.ps1:3226). Difference : calcule a la demande, plus le dernier etat rafraichi.
- SinceTableRefresh reste (le bloc de 0,25 s continue : BindTableState, RefreshWaitBanner, ...) : il est remis a 0 dans le bloc a la place de RefreshPlayersTable.
- BoundTableState, PlayersPanel, RightColumn, MessagesList gardes en C++. Les abonnements OnForumsChanged/OnVictoryChanged/OnDiplomacyChanged du tableau sont supprimes (le script lit a chaque trame, la signature coupe le travail).
- Le script cree PlayersList et le met dans PlayersPanel (SetContent) ; le C++ ne le cree plus.

## Table outils -> lignes
| Outil | Ligne lue | Ecrite par |
|---|---|---|
| lobby_match.ps1:242, 652, 682 | `Players table: <noms>` | script (UHOLPlayerTablePanel.Refresh, Log(n"LogHOLUI", ...)) |
| lobby_match.ps1:524-526 | `Players table state: ...Bob=Connexion perdue / Bob=IA / none` | script (StatesLogLine) |
| quick_check.ps1:3226-3298 | `HUD report: ..., players [<Shown> | <Shown>]` | C++ Describe (DescribeForum) via BuildRows |
| spectator_teams / forum_lives (hol.HUD.Report 1) | idem | C++ Describe |
| preflight.ps1:138, HOLScriptScreen.cpp:222, HOLGameScreenScript.as:699 | boite de PlayersPanel | inchange (widget C++ garde) |

## Suspects de compilation
1. Script : `Screen.ConstructWidget(...)` depuis UHOLPlayerTablePanel (grep ConstructWidget absent de HOLScriptScreen.h : voir ce que le script de l'ecran appelle) ; FSlateBrush Dot.* en script ; `Cast<USizeBoxSlot>(NameOverlay.Slot)` ; TArray<struct> en sortie de UFUNCTION ; `Log(n"LogHOLUI", ...)` doit sortir en `LogHOLUI: Display:` (lobby_match filtre ce prefixe) ; liste complete (e) de HALDIR-1119a.md.
2. C++ : USTRUCT FHOLPlayersTableRowRead dans HOLPlayersTableReadLibrary.h qui inclut Match/HOLForumRules.h (fournit FHOLPlayersTableRow) ; BuildRows est statique non UFUNCTION dans un UCLASS (OK) ; HOLGameScreen.cpp inclut desormais Match/HOLPlayersTableReadLibrary.h a la place de HUD/HOLRelationIcons.h (plus aucun usage de HOLRelationIcons dans ce fichier) ; includes devenus inutiles (SizeBoxSlot, Overlay...) laisses ; `HOLDiplomacy::FBook` dans la bibliotheque (Match/HOLDiplomacyRules.h inclus).
3. UBorder dans HOLGameScreen.h pour l'inline GetPlayersPanel : deja complet (PlayersPanel est un TObjectPtr<UBorder>).

## Commandes de validation
- Build : Tools\build.ps1 -LiveCoding=0 -Wait (worktree Haldir) ; un seul build.
- quick_check -Parts tests (5 tests script HOLPlayerTable_Test.as, voir HALDIR-1119a.md pour les 2 rouges) puis -Parts hud (Describe, lignes `players [`) ; hud_rects_game_test ; lobby_match (lignes Players table / Players table state) ; scenarios a rejouer : forum_lives, spectator_teams, ecran_hud_joueurs (play.ps1 ecran_hud_joueurs -Hidden -Root <worktree>), preflight.ps1 ecran_hud_joueurs (NON FAIT).
- Comparer hud_joueurs_depart a une capture du C++ d'avant (branche main).

## Lignes C++ supprimees
HOLGameScreen.cpp : -176 (+5) ; HOLGameScreen.h : -26 (+3) ; soit environ 200 lignes retirees. BUILDS_DEMANDES: 0. TESTS_CPP_RETIRES: 0. TYPES_A_EXPOSER: faits (FHOLPlayersTableRowRead, UHOLPlayersTableReadLibrary, GetPlayersPanel, GetShownSlot).

## NON VERIFIE
Tout (rien compile, rien lance). Scenario : commandes reprises de forum_lives.txt, preflight non joue, PNG non vus ; pastille "Connexion perdue" non couverte (hol.Net.FakeAbsent ne pose qu'une banniere). Le commentaire d'en-tete de HOLGameScreen.h sur le tableau (V2.4) est perime (non retouche). Le fichier non suivi Content\Maps\Crown.umap n'est pas touche.