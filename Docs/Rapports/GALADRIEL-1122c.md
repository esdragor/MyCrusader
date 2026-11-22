# GALADRIEL-1122c : cause du timeout JA d'income_tooltip + infobulles FR/EN/JA + cas sans bandeau

APPELS_DECLARES: 23 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:46 / 16:53 (arret dur 17:02) ; note : les lignes ajoutees aux journaux disent "16:55", lire 16:52
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++, script charge par play/preflight)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun nouveau (cause = scenario, pas le correctif ; rouge d'origine 8376a47 inchange, jamais VU a l'execution) ; assertit l'effet : oui (8376a47). TEST_ROUGE_SEPARE: oui (8376a47 / 45b871c)
COMMITS: 2 (c3487d4 scenario JA, 555af3a scenario sans bandeau + .look), dernier 16:51, merge_main_fait: non
TESTS: 3 passages income_tooltip -Hidden/preflight, tous PASS (1 play 36 s, 2 preflight 34 s) ; HOLTopBar_Test.as non lance. TESTS_TAUTOLOGIQUES: 0
ESSAIS: income_tooltip : preflight PASS (1er preflight, apres commit c3487d4) ; le 2e preflight affiche FAIL (1) uniquement pour "2 uncommitted files" (avant commit de 555af3a), "check income_tooltip : PASS" ; PNG fr, gold_fr, en, ja, gold_nobanner OUVERTS ; LOGIQUE SEULEMENT (GESTE: reel hol.Mouse.Hover, SON: non entendu, et defaut ci-dessous) ; commande : play.ps1 income_tooltip -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -TimeoutSec 300
QUESTIONS: aucune

## Cause du timeout JA du premier passage : (b) le SCENARIO, probable, non mesure
- Journal du passage expire (Saved\Logs\HeroesOfLiberty-backup-2026.11.22-15.41.49.log) : "Language ja" a l'image 360, "Mouse hover 20,300" lu a 366, puis "Mouse hover WoodAmount ... at 192,27: on SBorder, tooltip none" (image 397) ; les 2 passages PASS donnent 208,27 STextBlock. Largeurs (centre de la cellule bois) : FR 222, EN 203, JA 208 ; le 192 n'est ni l'un ni l'autre : la geometrie lue n'etait pas celle de la barre finale en JA.
- Code : HOLUIMouse.cpp:223-228 resout le centre au moment de la commande (GetCentre du widget en cache). income_tooltip.txt (avant) : apres `hol.UI.Language ja` seulement `wait 1`, qui vaut 1 seconde de jeu mais n'a laisse que ~1 image (Language ja 360 -> hover 20,300 logge 366) ; le pas EN avait `frames 10`. Le survol JA lisait donc la cellule ~6 images apres le changement contre ~15 pour EN : course perdue au demarrage a froid (premiere execution apres compilation, polices CJK).
- (a) ECARTE par le code : HOLTopBar.as:487-496, l'hote transparent est le TOOLTIP du widget (SetToolTip), hors de la mise en page de la barre ; il ne change aucune largeur de cellule. 1 passage sans correctif PASS (1122b) est un echantillon trop faible pour le prouver seul.
- (c) non exclu (demarrage a froid declenche la course), mais la course existe dans le scenario : la cause corrigeable est (b). Pas de largeur avant/apres changement journalisee : « non prouve » par mesure, probable par le code et les images.
- Correctif c3487d4 : `frames 10` apres `hol.UI.Language ja` (attente de l'effet de la mise en page, comme le pas EN ; survol non assoupli). Rejeux : play.ps1 PASS (JA 208,27 STextBlock), preflight PASS (JA 208). 3 passages sur 3 PASS, mais aucun a froid : le demarrage a froid n'a pas ete reproduit.

## Ce qu'on voit (PNG ouverts, 1280x720)
- Bandeau "Scenario income_tooltip en preparation... / Restart / x" (y 48-78, x 435-845) DEGAGE dans les 4 PNG.
- gold_fr : "+0 or/min" a x 443-511, y 83-103, 6 px sous le bandeau, lisible.
- fr / en / ja (bois) : "+377 bois/min", "+377 wood/min", "+377 木材/分" lisibles, mais a y ~93, ~55 px sous le bord de la barre (naturel : y ~36-58). Le bandeau n'est pas au-dessus (x 230-330 contre bandeau 435+) : l'infobulle n'avait PAS besoin d'etre abaissee.
- DEFAUT NON CORRIGE : HOLTopBar.as:505, SetBannerRect appelle IncomeTipDrop(Left, NaturalTop, Right - Left, Left, Right, Bottom) : l'infobulle fictive est posee au bord gauche du bandeau avec sa largeur, donc recouvre TOUJOURS le bandeau et le meme abaissement est applique aux 8 hotes, y compris ceux dont la cellule est hors du bandeau. A corriger : un abaissement par cellule (x du survol = centre de la cellule) ; UN test rouge dans HOLTopBar_Test.as (IncomeTipDrop d'une infobulle a x 230 sous un bandeau 435-845 = 0 par SetBannerRect), puis correctif, commits separes. Non fait : budget de 25 appels.
- Sans bandeau (nouveau pas 5 du scenario, `panel hide`, commit 555af3a, PNG income_tip_gold_nobanner.png, lu) : "+0 or/min" a x 443-511, y 36-58, juste sous la barre, pas abaissee : le rectangle vide (SetBannerRect(0,0,0)) garde l'infobulle a sa place naturelle. Vu a l'ecran.

FAIT / RESTE:
- FAIT : cause (b) etablie, c3487d4 ; 4 PNG FR/EN/JA ouverts ; cas sans bandeau ajoute au scenario et vu (555af3a, + ligne du .look) ; IN_USE.txt libre ; stray_processes : aucun.
- RESTE : defaut ci-dessus (abaissement des 8 infobulles hors bandeau) ; rouge d'origine 8376a47 jamais VU a l'execution (quick_check non lance) ; merge main non fait ; une ligne EtapeDefini.md non mise a jour (aucune ligne "GALADRIEL-1122b" dans ce fichier : seule la ligne d'Auto_Journey_File.md l'est) ; HOLTopBar_Test.as non lance.
- branche prete : NON (defaut de placement des infobulles bois/pierre/nourriture, cosmetique ; tout le reste PASS).
NON VERIFIE: demarrage a froid reproduisant le 192 (non reproduit) ; largeur des cellules avant/apres journalisee (aucune ligne) ; HOLTopBar_Test.as ; DIAGNOSTICS_AS: journal des 3 chargements sans ligne "Angelscript: Error" (serveur de langage non consulte).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; preflight "engine smoke tests: 20 Condition failed (UnifiedErrorTests, culture fr)" hors sujet. Git Bash `date` en retard sur PowerShell de ~2 min.
GIT_STATUS_PRINCIPALE: modifie par moi : Auto_Journey_File.md (ma ligne), InterfaceInProgress.md (ligne ajoutee) ; le reste (EtapeDefini.md, Placeholders.txt, rapports, etc.) vient d'autres sessions : oui.
