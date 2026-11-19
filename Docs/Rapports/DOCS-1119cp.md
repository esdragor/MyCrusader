# DOCS-1119cp : archivage des lignes de migration de Auto_Journey_File.md (PARTIEL : archive complète, retrait fait pour 17 lignes sur 141)

Agent DOCS-1119cp, 2026-11-19 19:35 à 19:41 (Get-Date), documents seulement, aucun code, aucun build, aucun commit. 20 appels d'outils vus par moi.

## Résultat en un coup d'oeil

| Quoi | Nombre |
|---|---|
| Lignes copiées mot pour mot dans l'archive | 141 |
| Lignes retirées de `Auto_Journey_File.md` à ce jour | 17 (git : 17 deletions, 1 insertion = la ligne « Section archivée » de « Lignes périmées vues à l'étape 0 ») |
| Lignes encore à retirer | 124 (toutes déjà copiées dans l'archive ; le fichier de la file n'a perdu AUCUNE ligne qui ne soit pas dans l'archive) |

Les deux nombres ne sont donc PAS égaux : 17 retirées, 141 archivées. L'archive est un sur-ensemble, rien n'est perdu.

## Pourquoi le retrait est incomplet

1. J'ai d'abord écrit un script qui relit le fichier, retrouve chaque ligne par égalité exacte avec sa copie de l'archive (refus si 0 ou plusieurs correspondances), les retire d'un coup et ajoute les lignes de pointeur par `edit_line.ps1 -Append`. Le classifieur du mode auto l'a refusé (« réécriture du fichier entier », ce que ton énoncé interdit aussi). Je ne l'ai pas contourné.
2. Repli sur l'outil Edit : 3 blocs passés (ligne 21 ; lignes 23 à 36 ; lignes 142-143), un 4e bloc de 11 lignes refusé pour une faute de recopie dans mon `old_string`. Recopier ~125 lignes de 300 à 2000 caractères par Edit dépasse mes 25 appels et l'heure d'arrêt (19:50) : arrêt volontaire plutôt que de laisser le fichier à moitié édité sans le dire.
3. Constat utile : `Auto_Journey_File.md` a des fins de ligne LF (vérifié sur la zone de « Lignes périmées »), pas CRLF comme annoncé ; l'Edit multi-lignes marche.

## Pour finir (décision à toi)

Script prêt, déjà testé en lecture sur l'archive : `C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\6d746474-c824-4eaa-8a39-659d57b193b3\scratchpad\remove_lines.ps1` (+ `inserts.txt` et `header.txt` dans le même dossier). Il saute les 17 lignes déjà retirées (`ALREADY_REMOVED_BY_EDIT=17`), retire les 124 autres en vérifiant chacune contre l'archive, garde le BOM et la fin de chaque ligne, puis insère par `edit_line.ps1 -Append` : la ligne de pointeur sous chaque titre vidé, et sous « ## Migration D851 » les 2 lignes `[ ]` (integ-23 ; sync_worktrees puis gate Close). Si tu l'autorises : `powershell -ExecutionPolicy Bypass -File <ce chemin>`, puis `git diff --stat -- Auto_Journey_File.md` doit afficher 141 deletions (17 + 124) et les insertions de pointeurs. Sinon, finir par Edit par blocs de 3 à 5 lignes.

## Archive

`C:\Users\baudo\Documents\MyCrusader\Docs\Archive\Auto_Journey_File_migration_archive_2026-11-19.md` : 3 lignes d'en-tête, puis par titre d'origine, `STATUT | <ligne telle quelle>`. 141 lignes à statut (vérifié par comptage). Texte original = tout ce qui suit le premier « | ».

Par section (nombre de lignes archivées) :
- « ## Migration D851 » : 37 (FAITE 22 : 21, 24-31, 33, 35, 36, 39, 40, 42, 46-50, 57, 58 ; PÉRIMÉE 14 : 23 note « Plans », 32 NG-6a, 34 ST-3, 37 H-1, 38 H-2, 41, 43, 44, 45, 51, 52, 54, 55, 56 ; ABANDONNÉE D857 : 1, la ligne 53 C3 / C4 / C5). Numéros = numéros du fichier avant retrait.
- « ## Migration D830/D835 » intro : 2 (61, 62, PÉRIMÉE).
- Glorfindel Pilote 1 : 4. Galadriel Pilote 2 : 5 (73 P2c PÉRIMÉE : la bascule GameMenu est faite, ligne 138). Arwen : 3. Celeborn : 6. Elrond : 4 (92-95). Faramir : 2. Haldir : 2. Sauron : 3.
- Lot C++ de fin de séance : 29 (115 gabarit « les agents ajoutent ici... » PÉRIMÉE, le reste FAITE).
- Lignes périmées vues à l'étape 0 : 2 (PÉRIMÉE). Décisions du 2026-11-17 : 4. Prochaine séance de migration : 26 + 12 = 38 (154 note PÉRIMÉE).
(Le total par sous-section est celui de la liste des plages ; le chiffre qui fait foi est 141 compté dans le fichier d'archive.)

Rangées FAITE sur preuve et non sur la coche (lignes `[~]` / `[>]` / non cochées) : 74 et 88 (Formation), 87 (CommandGrid), 172 (E3-10 RelationIcons), 173 (E3-11 SignalInput), 183 (L17-05 Minimap). Preuve : en `b26be1a`, les fichiers C++ de test correspondants (HOLFormationTests, HOLCommandGridTests, HOLRelationIconsTests, HOLSignalInputTests, HOLMinimapAlertRingTests, HOLMinimapTerrainTests) n'existent plus et les `Script\Tests\HOL*_Test.as` existent. Contrôle de présence seulement, pas de lecture des commits. A reclasser si tu veux « douteuse ».
Ligne 21 (tool-known-reds, « PAS ENCORE dans main ») : `git merge-base --is-ancestor 9ed53fd HEAD` = 0, donc dans main : FAITE.

## Ce qui reste ouvert (à écrire sous « ## Migration D851 » par le script ci-dessus)
- `[ ]` integ-23 (worktree Glorfindel) : rouge vu, retrait des 4 tests C++ de déconnexion (Rules, Gains, Technologies, WindowEnd), puis main.
- `[ ]` fin de migration : `sync_worktrees.ps1` sans `-NoBuild`, puis `session_gate.ps1 -Phase Close`.

## Lignes douteuses (laissées telles quelles dans la file, NON archivées)
1. Ligne 22 `[~]` Forge D856 (« FAIT ET PROUVÉ dans integ-21... RESTE chaîne réseau, main, fiche E-forge-lignes, essai de 3 min ») : bug du jeu, pas de la migration ; main contient integ-22 et l'essai est « donné », mais je ne sais pas si l'utilisateur l'a joué.
2. Ligne 86 `[~]` MIG-C-MatchConfig : `HOLMatchConfigTests.cpp` existe toujours dans main, la ligne dit « restent en C++ jusqu'au rouge vu puis vert » ; je ne sais pas si ce retrait est abandonné par D857.
3. Ligne 178 (suite de E3-FIN, « RESTE à retirer : StartingPeasants.Count, FactionChoice.Bodies, MatchLossText, Map.Rules, GameNames x3, Market.Table ») : orpheline après retrait de E3-FIN ; reste-t-il à faire ou abandonné, non décidé.
4. Ligne 191 `[~]` L17-12 Production : `HOLProductionTests.cpp` est absent de main (fichier renommé ou retiré ?) mais la ligne liste un RESTE (Queue, Advance, Tables) ; non tranché.
Aucune ligne de migration dans « ## Prochaine séance » (lignes 10 à 18) : rien touché.

## NON VÉRIFIÉ
- Que les 124 lignes restantes seront retirées sans collision : le script aborte sur 2 correspondances, mais je ne l'ai pas exécuté (refus).
- Les statuts FAITE sur preuve (ci-dessus) : présence de fichiers seulement.
- Que d'autres sessions n'ont pas édité la file entre mes lectures et mes Edit (le diff git ne montre que mes 18 lignes).
- Les numéros de lignes du § B de ETAPE0-1119ce sont décalés de +2 par rapport au fichier actuel (L30 = ligne 32, etc.) ; j'ai suivi le contenu, pas les numéros.

## Anomalies du processus
- Script de retrait refusé par le classifieur auto (voir plus haut) ; Edit de 11 lignes refusé pour faute de recopie ; budget de 25 appels et arrêt dur 19:50 insuffisants pour un retrait par Edit de ~125 longues lignes. Un outil `edit_line.ps1 -Delete` (une ligne, correspondance exacte) réglerait ce cas.
- `git status` : `Auto_Journey_Jour.md`, `GDD.md` et `Docs\Rapports\GALADRIEL-1119cq.md` sont modifiés / neufs par d'autres sessions, pas par moi. Mes fichiers : `Auto_Journey_File.md` (17 lignes retirées, 1 ajoutée), l'archive (neuve), ce rapport.
- Pas d'`IN_USE.txt` ni de `stray_processes.ps1` : copie principale, aucun processus lancé.

## Suite DOCS-1119cq (2026-11-19, 19:42 a 19:48, Edit uniquement, aucun script de reecriture, aucun commit)

- Retire par Edit : 53 lignes de plus (git : 70 deletions = 17 + 53, 11 insertions : pointeurs + 2 lignes ouvertes). D851 : lignes 22 a 43 de l'ancien fichier (hors Forge D856). D830/D835 : intro (2), Glorfindel (4), Galadriel (5), Arwen (3), Celeborn (6 sur 7, MatchConfig gardee), Elrond (4 sur 6 : les 2 lignes Construction a 2 espaces aussi), Faramir (2), Haldir (2), Sauron (3). Chaque sous-section videe porte la ligne « Section archivee le 2026-11-19 (D857, etape 5 close) ».
- Ecrit sous « ## Migration D851 » : les 2 lignes ouvertes (integ-23 `[~]`, fin de migration `[ ]`), placees juste sous le titre, avant la ligne Forge.
- Encore dans la file et presentes dans l'archive : 71 lignes (controle Select-String / HashSet, egalite exacte). Ce sont : le reste de D830/D835 « Lot C++ de fin de seance » (29), « Decisions du 2026-11-17 » (4), « Prochaine seance de migration » (25 des 26 + 12 = hors la ligne douteuse E3-FIN), « Lignes perimees » (les 2 lignes ETAPE0 de l'etape 0 si presentes). Les 4 lignes douteuses du point 5 sont intactes. Les titres `###` ne sont pas retires : Celeborn (MatchConfig) et Prochaine seance restent non vides.
- Cause du reste : arret volontaire sur budget d'appels (40) ; 6 Edit refuses pour fautes de recopie de lignes tres longues (aucun dommage, l'Edit est refuse en entier).
- Pour finir : meme methode, blocs de 3 a 5 lignes, sur « Lot C++ de fin de seance », « Decisions du 2026-11-17 » et « Prochaine seance de migration » ; la liste exacte des 71 lignes restantes se relit par la boucle de lecture (Select-String -SimpleMatch de chaque ligne de l'archive sur la file).
- NON VERIFIE : que le fichier n'a pas ete modifie par une autre session pendant l'operation (diff stat seulement) ; fins de ligne : le fichier de travail est LF, git avertit « LF will be replaced by CRLF » (avertissement habituel, pas de changement de ma part).

## Suite DOCS-1119cr (2026-11-19, fin 19:52, Edit uniquement, aucun script de reecriture, aucun commit)

- Retirees par Edit par DOCS-1119cr : 62 des 71 lignes ; encore dans la file et presentes dans l'archive : 9. Numstat git de Auto_Journey_File.md (ajouts, retraits) : 12	132	Auto_Journey_File.md.
- Faites : Prochaine seance de migration (hors lignes douteuses), Decisions du 2026-11-17 (titre + ligne de pointeur), Lot C++ de fin de seance en grande partie.
- Restantes (lignes de la file, debut de texte) : L53:- [x] (BASCULE MIG-J-Site, ARWEN-1116t,  || L54:- [x] (MIG-CPP-Site pont serveur, ARWEN- || L55:- (les agents ajoutent ici une ligne `[  || L56:- [x] (REPRISE Glorfindel, net-playtest  || L57:- [x] (REPRISE Celeborn, sans C++ ; cons || L58:- [x] (ETAPE0-1118m, 2026-11-18 : FAITE  || L59:- [x] 885de4f D836 (SAURON-1116o) : copi || L60:- [x] 885de4f D836 (SAURON-1116o) : copi || L61:- [x] (sans C++, Sonnet, suite SAURON-11
- Cause : budget de 40 appels ; 4 Edit refuses pour fautes de recopie (aucun dommage). Les 4 lignes douteuses du point 5 sont intactes, ainsi que les 2 lignes ouvertes de Migration D851.
- NON VERIFIE : que le fichier n'a pas ete modifie par une autre session pendant l'operation ; les 4 sous-sections vides ne sont pas toutes pourvues de la ligne de pointeur (a controler a la main).
