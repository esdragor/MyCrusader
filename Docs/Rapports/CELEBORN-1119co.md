# CELEBORN-1119co : jumeaux script de Victory.Forum.Block / Hold.Display / Look

NON COMPILE, NON TESTE (aucun build, aucun quick_check, aucun editeur). Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche `test-world-11` (partie de integ-22 9ed53fd). Pas de merge, pas de push, pas de merge main. Lance 19:03 (Get-Date), fini ~19:07, ~11 appels vus.

## Commits (test-world-11)
- dc1f2c5 Script/Tests/HOLForumRules_Test.as : +9 lignes (assertions de Text exact ajoutees aux jumeaux Block et Hold.Display).
- 21e8966 retrait de Victory.Forum.Block (C++ : -67 lignes).
- 4541f4d Victory.Forum.Hold.Display reduit (-59 +3).
- 0a7428f Victory.Forum.Look reduit (-24 +2).
(Annuler un test = revert de son commit ; dc1f2c5 doit rester si on garde le retrait de Block.)

## Tables assertion C++ -> assertion script (fichier : Script\Tests\HOLForumRules_Test.as ; lignes script avant ajouts, celles de Block/Display ont bouge de +1..+9 apres)
### Victory.Forum.Block -> Test_ForumBlock (l.211) : COUVERT EN ENTIER apres 3 ajouts
Secondaire (3 assert) -> l.222-224 ; Debout x2 (bShown, health kept, Shown ", lives 2", Lives 2, Text vide) -> l.230-234 ; Fall==1 -> l.238 ; RuinSecondsLeft 25 et 241->25 -> l.240, 242 ; Ruine x2 (shown, health hidden, Shown ", in ruin 25 s, lives 1", Lives 1) -> l.246-248, 250 ; Rebuild 40 % et 0 s -> l.257-258 ; Own (health hidden, Shown, Lives) -> l.260-261, 263 ; Other (health hidden, Shown, Lives) -> l.265-266, 268.
SANS equivalent exact (le script n'avait que Contains) : Ruin.Text == "En ruine — 25 s" ; Own.Text == "Reconstruction — 40 %\nBatisseurs : 3" ; Other.Text == "Reconstruction — 40 %". AJOUTES au script (Text est deja lu en FString source par UHOLForumRulesReadLibrary, aucune liaison neuve ; litteraux UTF-8 comme HOLDisconnectDisplay_Test.as l.113). Test C++ retire en entier.
### Victory.Forum.Hold.Display -> Test_ForumHoldDisplay (l.272) : couvert sauf 3 blocs, test C++ REDUIT
Couvert : ShowsHold, ring none / hidden / shown / Shown "fill 0.58 holder slot 1" / grey / faded / "fill 0.00 grey" / "fill 0.50 grey", HolderSlot 1, blocs Shown (ruine, tenue, receding), DescribeHold (4), teintes Holder/Grey/Faded (portees par les chaines Shown "holder slot 1" / "grey" / "faded slot 1", derivees de la teinte). Ajoute au script : les 4 lignes de Text exact du bloc (tenue, contestee, receding, contestee a 0).
RESTE en C++ (pas de lecture script, liaison C++ neuve requise) : (1) couleurs GetRingColor (FLinearColor, 3 assert) ; (2) tableau des joueurs GetPlayersTable(FHOLMatchConfig, TArray<FHOLForumState>) (5 assert, dont 2 accentuees) ; (3) contour de zone GetZoneOutlinePoint (5 assert + 41 de boucle, FBox2D). Le test garde son nom, son Tuning et un etat reconstruit (Fall, Rebuilding 40 %, Holding 300, HolderSlot NONE = l'etat exact de la fin de la partie supprimee).
### Victory.Forum.Look -> Test_ForumLook (l.336) : couvert sauf HOLUnitAnimation::Choose, test C++ REDUIT
Couvert : GetLook Standing/Rubble/Site -> AssertEquals 0/1/2 (l.342, 347, 353) ; Progress 1.f / 0.f / 0.25 -> ForumNear l.343, 348, 354 ; Fall==1 -> l.346 ; BuildersWait x3 -> l.344, 349, 355.
RESTE en C++ : 3 assert HOLUnitAnimation::Choose (marteau au chantier, debout a la ruine, pas tourne) : FHOLUnitTypeConstants / FHOLUnitAnimationInput / Choose non exposes au script (~12 lignes de liaison, hors tache).

## Falsifications (a jouer AVANT de fusionner ; une par ligne : fichier | avant | apres | test qui doit rougir)
```
Script\Tests\HOLForumRules_Test.as | T.AssertTrue(Standing.Shown == ", lives 2", "standing: described"); | T.AssertTrue(Standing.Shown == ", lives 3", "standing: described"); | HeroesOfLiberty.Victory.Forum.Block (script Test_ForumBlock)
Script\Tests\HOLForumRules_Test.as | T.AssertTrue(Ruin.Text == "En ruine — 25 s", "ruin: its lines"); | T.AssertTrue(Ruin.Text == "En ruine — 26 s", "ruin: its lines"); | HeroesOfLiberty.Victory.Forum.Block (assertion Text ajoutee)
Script\Tests\HOLForumRules_Test.as | == "fill 0.58 holder slot 1", "holding: ring"); | == "fill 0.59 holder slot 1", "holding: ring"); | HeroesOfLiberty.Victory.Forum.Hold.Display (script Test_ForumHoldDisplay)
Script\Tests\HOLForumRules_Test.as | T.AssertTrue(Block.Text == "En ruine — 25 s\nTenue — 35 s", "holding: block lines"); | T.AssertTrue(Block.Text == "En ruine — 25 s\nTenue — 36 s", "holding: block lines"); | HeroesOfLiberty.Victory.Forum.Hold.Display (assertion Text ajoutee)
Script\Tests\HOLForumRules_Test.as | T.AssertEquals(2, HOLForumRulesRead::GetLook(State, Tuning), "rebuilding: a site"); | T.AssertEquals(3, HOLForumRulesRead::GetLook(State, Tuning), "rebuilding: a site"); | HeroesOfLiberty.Victory.Forum.Look (script Test_ForumLook)
```
(Les noms des tests script dans quick_check sont ceux du fichier ; Test_ForumBlock / Test_ForumHoldDisplay / Test_ForumLook.) Si la falsification de Block ne rougit pas, annuler 21e8966 seul. Les chaines a em-dash et accents du .as sont en UTF-8 sans BOM, LF (le fichier etait ainsi) ; risque non verifie : que l'encodage des litteraux arrive intact a FString (cf. HOLDisconnectDisplay_Test.as l.113-114, meme usage) : si les 3 assertions Text de Block rougissent a l'execution SANS falsification, annuler 21e8966 et corriger l'encodage.

## Champs
TESTS_CPP_RETIRES: 1 (HeroesOfLiberty.Victory.Forum.Block). Hold.Display et Look : REDUITS, pas retires.
LIGNES_CPP_SUPPRIMEES: 145 net (150 supprimees, 5 ajoutees : 67 Block + 56 Display + 22 Look) ; sur les 229 visees, 84 restent.
TESTS_SCRIPT_AJOUTES: 0 (7 assertions ajoutees dans des tests existants : 3 Block, 4 Hold.Display ; 9 lignes).
Total de tests attendu : 1786 (1787 - Victory.Forum.Block C++ ; les jumeaux script etaient deja comptes ; a confirmer par quick_check, qui ne doit pas baisser autrement que de ce -1 : le total baisse de 1, ce qui est le retrait voulu d'un doublon).
BUILDS_DEMANDES: 1 (le .cpp de test change) : `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` ; plus rejouer les 5 falsifications (script, sans build C++) avant fusion.
TYPES_A_EXPOSER: GetRingColor, GetZoneOutlinePoint, GetPlayersTable (config + FHOLForumState en entree), HOLUnitAnimation::Choose.
Grep : aucun outil (Tools\Tests\*.ps1, known_reds.txt, Source, Script) ne cite FHOLVictoryForumBlockTest ni les noms complets retires hors des commentaires du .as et des 2 tests conserves ; aucune aide partagee retiree (seuls des corps de test ont ete retires).
EDITS_REFUSES: 0. PROCESSUS_DETACHES: aucun lance.
## NON VERIFIE
Compilation du .cpp (variables inutilises : Tuning/State du Display reduit sont utilises par la table ; Look reduit sans Tuning ni State) ; exactitude de l'etat reconstruit du Display ; execution du script (Text exact, parse de "\n", noms de variables AtZero/Block reaffectee) ; total 1786 ; le rouge des 5 falsifications.
