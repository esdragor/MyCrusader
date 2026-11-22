# GALADRIEL-1122d : abaissement des infobulles de revenu par cellule (HOLTopBar.as)

APPELS_DECLARES: 24 (arret a 25 vus ; le premier Edit est venu au 9e appel, pas avant le 8e : un Write d'IN_USE refuse "not read" et deux Read de contexte ont consomme le budget)
HEURE_DEBUT/HEURE_FIN: 16:54 / 16:59 (arret dur 17:08)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++, script charge par le preflight)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: a1b7e0b (HOLTopBarTip_Test.as + bouchon IncomeTipDropForCell qui garde l'ancienne regle « bord gauche du bandeau »), assertit l'effet : oui (le decalage vertical de l'infobulle, calcule par la vraie regle) ; rouge NON VU a l'execution (aucun quick_check demande), attendu : Test_TopBarTipOutsideTheBannerKeepsItsNaturalPlace (drop > 0 pour une cellule a x 222 / 900) rouge, les deux autres tests passent avec le bouchon. TEST_ROUGE_SEPARE: oui (a1b7e0b rouge / a8e3e24 correctif)
COMMITS: 2 (a1b7e0b, a8e3e24), dernier 16:57, merge_main_fait: non (DLL non verifiee, la session principale synchronise)
TESTS: preflight income_tooltip PASS (39 s, check PASS, « no warning or error in the log », 0 ligne `Angelscript.*rror`) ; HOLTopBarTip_Test.as et HOLTopBar_Test.as NON lances. TESTS_TAUTOLOGIQUES: 0 (largeur via IncomeTipView + IncomeTipWidth, haut via IncomeTipNaturalTop, decalage par IncomeTipDropForCell ; seules entrees : x de cellule et rectangle de bandeau)
ESSAIS: income_tooltip : preflight PASS, PNG fr, gold_fr, en, ja OUVERTS (gold_nobanner non rouvert : inchange par ce correctif) ; LOGIQUE SEULEMENT (geste hol.Mouse.Hover interne, SON: non entendu) ; commande : play.ps1 income_tooltip -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -TimeoutSec 300
QUESTIONS: aucune
FAIT / RESTE:
- FAIT : HOLTopBarTip_Test.as (3 tests : hors bandeau a gauche et a droite = 0 ; croisant le bandeau (gold x 443, et debut a gauche atteignant le bandeau) = haut sous le bord bas + 6 ; sans bandeau = 0).
- FAIT : HOLTopBar.as : IncomeTipDropForCell(CellCentre, TipWidth, BannerLeft, BannerRight, BannerBottom) = IncomeTipDrop avec le rectangle propre de l'infobulle ; SetBannerRect boucle sur les 8 hotes, lit la boite de SA cellule (Screen.GetWidgetBox sur AmountOf/GatherersOf), largeur du texte courant (IncomeTipWidth), cellule inconnue = place naturelle.
- FAIT : preflight, PNG : bois FR/EN/JA a y ~36-58 (pres de la barre, x 230-330, bandeau 435+), or FR a y 83-103 sous le bandeau (y 48-78), pas de recouvrement. Journal : cellule bois 222/203/208, or 434.
- Docs : InterfaceInProgress.md l.255, EtapeDefini.md l.8 (ligne neuve), Auto_Journey_File.md l.118, Retours\E-income_tooltip.md l.8 (recette), IN_USE.txt libre.
- RESTE : quick_check des deux fichiers de test (rouge du premier jamais vu) ; le test de 8376a47 (Test_TopBarGoldTipDoesNotCoverTheScenarioBanner, IncomeTipDrop inchange) reste vrai par construction.
- branche prete : oui, sous reserve du quick_check des deux fichiers de test.
NON VERIFIE: rouge a l'execution ; HOLTopBarTip_Test.as et HOLTopBar_Test.as lances ; le x de l'infobulle = centre de la cellule (hypothese tiree du survol, tip left ~ cursor ; PNG : bois x 230 pour un centre 222) ; `IncomeTipTexts[Index].GetText().ToString()` compile (preflight : aucune erreur, donc oui) ; DIAGNOSTICS_AS: 0 erreur (journal du preflight ; serveur de langage VS Code non consulte).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; preflight : « engine smoke tests: 20 Condition failed » (UnifiedErrorTests, culture fr) hors sujet ; edit_line refuse -Match avec -Append (utiliser -After) : 4 appels perdus.
GIT_STATUS_PRINCIPALE: modifie par moi : InterfaceInProgress.md, EtapeDefini.md, Auto_Journey_File.md, Retours\E-income_tooltip.md (lignes ajoutees) ; le reste vient d'autres sessions : oui.
