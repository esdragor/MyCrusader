# GALADRIEL-1122g

NON COMPILE (aucun build, aucun editeur, consigne de l'enonce).

APPELS_DECLARES: 15 (arret a 24 ; 1er Edit/Write au 7e appel : Write du test ; dernier appel = ce rapport puis SubagentHandback)
HEURE_DEBUT/HEURE_FIN: 2026-11-22T18:36:30 / 2026-11-22T18:40:48 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (aucun build ; le merge d'integ-52 a rendu la dll perimee, voulu)
TEST_ROUGE: 0577971 (bouchon + test C++, vu NON a l'execution : rouge par construction, le bouchon ComputeNameZone rend NameLines=99 / NameHeight=CellSize, SetStatus vide, GetNameZoneHeight 0) ; assertit l'effet : oui (texte de la case DescribeContent apres SetCommand + SetStatus, hauteur de la zone du nom, calcul de la zone par ComputeNameZone)   TEST_ROUGE_SEPARE: oui (commit 0577971 seul, puis 564db8d ; le rouge n'est PAS vu a l'execution faute de build)
COMMITS: 3 (0577971 rouge, 564db8d vert, add470d InterfaceInProgress), dernier a 18:40, merge_main_fait: non fait (le premier geste etait merge integ-52, fait sans conflit ; `main` est ancetre de HEAD, code 0)
TESTS: aucun lance. Ecrits : Source\HeroesOfLibertyUI\Tests\HOLGridCellStatusTests.cpp, HeroesOfLiberty.UI.GridCell.NameZone (calcul pur : bande haute + zone du nom + ligne d'etat <= 76, nom de 4 lignes borne a < 4, lignes entieres, cellule plus grande = plus de lignes, ligne haute = 1 ligne minimum, sans etat = rien borne) et HeroesOfLiberty.UI.GridCell.StatusLine (vraie cellule creee avec un joueur : nom seul, puis nom + "\n" + etat, zone bornee > 0 et < 76 - 12, nouvelle commande = etat efface). TESTS_TAUTOLOGIQUES: 0 (les valeurs sont calculees par ComputeNameZone / DescribeContent, jamais recopiees)
ESSAIS: aucun (pas d'editeur). tech_soon.look mis a jour (texte), non rejoue.
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la session principale) ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: sans objet

## Ce qui change (HOLGridCell.h/.cpp, HOLGameScreen.cpp)
- HOLGameScreen.cpp:1165-1169 : plus de `FText::Format("{0}\nBientot", Name)` : `SetCommand(Name, ...)` puis, si NotImplemented, `GridCell->SetStatus(GetResearchBlockText(NotImplemented))`. Le texte est la cle `ResearchSoon` (deja dans Game.po FR "Bientot", EN "Coming soon", JA "近日公開" : les 3 langues sans po neuf). La cle `ResearchSoonCell` n'est plus utilisee (orpheline dans les 3 Game.po, nettoyee au prochain gather).
- HOLGridCell : `SetStatus(FText)`, `GetNameZoneHeight()`, `static ComputeNameZone(CellSize, TopBand, LineHeight, bHasStatus)` -> `FHOLGridCellNameZone {NameHeight, NameLines, StatusHeight}`, constantes publiques `DefaultCellSize = 76` (miroir de HOLGameScreen.cpp:225 et du script HOLCommandGridPanel.as) et `NameTopBand = 12`.
- Mise en page : le nom (UnitName) est dans un `USizeBox NameBox` ecrete (`ClipToBounds`) dont `MaxDesiredHeight` = lignes entieres (hauteur de ligne mesuree par FSlateFontMeasure::GetMaxCharacterHeight, repli 18 px sans renderer) ; l'etat est un `UCommonTextBlock StatusText` (Tiny, centre, une ligne, VAlign_Bottom, marge basse 2 px), enfant de l'overlay racine hors du nom ; le slot du nom prend en bas la hauteur de la ligne d'etat. Avec etat : 76 -> bande 12 + nom 3 lignes (45) + etat 17 = 74 si ligne = 15 ; avec une ligne de 18 px : 2 lignes de nom. Un nom plus long est coupe a la fin d'une ligne.
- `SetCommand` et `ClearUnit` effacent l'etat (SetStatus vide : ni borne ni marge, la case est comme avant) : aucune case sans etat n'est modifiee. `DescribeContent()` rend `nom + "\n" + etat`, IDENTIQUE a l'ancien texte compose : `hol.HUD.Report` et ShownTradeLabels inchanges. Taille de case (76), libelles, ordre des cases : inchanges.
- quick_check / scenarios : grep « Bient » dans Tools et Script : seuls `tech_soon.txt` / `tech_soon.look` (note + capture, aucune assertion sur le texte compose) ; tech_soon.look reformule (nom borne, Bientot sur sa propre ligne en bas, entier). Aucun autre test ne depend de « {0}\nBientot ».

## RELECTURE_CPP (L20-2)
Relu apres edition : (1) une accolade fermante collee a la fin de la ligne StatusSlot->SetPadding (retour a la ligne perdu par mon remplacement) a ete trouvee et corrigee avant le commit vert ; (2) deux commentaires du .h avaient des guillemets francais en mojibake (corriges, ASCII) ; (3) includes ajoutes : Components/SizeBox.h, Fonts/FontMeasure.h, Framework/Application/SlateApplication.h, Rendering/SlateRenderer.h ; (4) le .h declare `class UOverlaySlot; class USizeBox;`, membres NameBox / UnitSlot / StatusText en UPROPERTY(Transient) prives, NameZoneHeight float prive ; (5) SetStatus protege chaque pointeur (appel possible avant NativeOnInitialized).

NON VERIFIE (a compiler, a mesurer) :
- Rien n'est compile ni execute ; le rouge n'est pas vu, le vert non plus.
- `UnitName->GetFont()` : accesseur suppose public sur UTextBlock (UE 5.x) ; si absent, mesurer avec le style (UHOLTextStyleHUDTiny, 11 pt) ou garder FallbackLineHeight. C'est le point de compilation le plus probable a corriger.
- `USizeBox::ClearMaxDesiredHeight`, `FSlateFontMeasure::GetMaxCharacterHeight` : supposes existants.
- `TObjectPtr<UOverlaySlot>` en UPROPERTY et `CreateWidget<UHOLGridCell>(PlayerController)` dans le test StatusLine : le test copie la mise en place monde + joueur du test NewGameSitesBox, a confirmer a l'execution (si CreateWidget echoue faute de contexte, le test est rouge pour une autre raison que la regle).
- Rendu : que 3 a 4 lignes de nom soient bien coupees proprement et que « Bientot » (FR) / « Coming soon » (EN, 11 chars) / « 近日公開 » (JA) tiennent sur une ligne de 76 px : a voir sur la capture de `tech_soon` (la question de la largeur de « Coming soon » en Tiny n'est pas mesuree).
- La case de recherche W n'est pas la seule a recevoir le statut : le code ne l'applique qu'a Research + NotImplemented (comme avant).

A lancer par la session principale (un build, puis) : `Tools\build.ps1 -LiveCoding=0` dans le worktree Galadriel (ou apres fusion), puis `quick_check -Parts tests` (filtre HeroesOfLiberty.UI.GridCell), puis `Tools\Scenarios\play.ps1 tech_soon -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` et ouvrir tech_soon_barracks.png. Pour le rouge a l'execution : construire 0577971 seul (test sans correctif) avant 564db8d.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; premier fichier ecrit au 7e appel (Write)
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, M FortificationsInProgress.md, M OrdersPatrolInProgress.md, M Retours/E-deploy.md, M Retours/E-order_lines_blocks.md, + non suivis Docs voulus ; inchangee hors docs voulus : oui (mes lignes : EtapeDefini.md, Auto_Journey_File.md, ce rapport)

## Suite (build 1 echoue : 2 erreurs)
(1) la case est remplie par FHOLGridCellFill (HOLGameScreen.h:33, cache applique par ApplyGridCell, HOLGameScreen.cpp:1387) : champ Status + SetStatus ajoutes (SetCommand le vide), ApplyGridCell appelle Target->SetStatus apres SetCommand ; (2) StatusBottomPad qualifie (UE::HeroesOfLiberty::UI::GridCell::) dans NativeOnInitialized. Symboles neufs relus : UTextBlock::GetFont, USizeBox::SetMaxDesiredHeight/ClearMaxDesiredHeight, FSlateFontMeasure::GetMaxCharacterHeight verifies dans UE_5.4 (la version du projet UE_5.8_HOL non relue). Test FillStatus ajoute. Commit de correction : voir git log.
