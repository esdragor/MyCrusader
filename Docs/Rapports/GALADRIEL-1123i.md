# GALADRIEL-1123i : tests et PNG de D889 (worktree Galadriel, specialization-trial)

APPELS_DECLARES: 17 (arret a 20)
HEURE_DEBUT/HEURE_FIN: 10:36 / 10:45
PREMIER_BUILD: sans C++   BUILD: sans objet (binaire du 10:35, preflight : "binary not older than any C++ source")   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code touche)   TEST_ROUGE_SEPARE: non (pas de correctif)
COMMITS: 1 (0fb4320, docs : InterfaceInProgress.md), dernier a 10:45, merge_main_fait: non (rien a fusionner, branche = 2d9452c + c1ba30b + 0fb4320)
TESTS: quick_check.ps1 -Parts tests (script du worktree) : tests 2118/2118 PASS en 268 s, UI.GridCell.StatusFit compris (vert, pas de modification du test) ; TESTS_TAUTOLOGIQUES: 0 (StatusFit mesure avec la police de la case, ComputeStatusFit / BreakStatusLines calcules par le code)
ESSAIS: tech_soon : preflight PASS (62 s), 4 PNG ouverts (Saved_Tests\Screenshots\WindowsEditor\HOL\tech_soon_barracks{,_en,_ja,_720}.png). LOGIQUE SEULEMENT pour la partie D889 (voir ci-dessous). Les .look listent tech_soon_barracks_en / _ja / _720 dans le MEME scenario tech_soon (pas de tech_soon_en / _ja separes).
  - FR 1920 : « Hallebarde trempees » sur 3 lignes, « Bientot » sur sa ligne, grise, entier : inchange.
  - JA 1920 : « 近日公開 » sur sa ligne, entier, nom sur 2 lignes : inchange.
  - EN 1920 : « Coming soon » reste sur UNE ligne, en petit (reduit), dernier « n » tout contre le bord droit de la case ; PAS deux lignes.
  - EN 1280x720 (case ~51 px) : « Coming soon » sur UNE ligne en tres petit, contenu dans la case mais pas lisible ; PAS deux lignes.
  => D889 n'est pas effectif a l'ecran.
QUESTIONS: aucune
FAIT / RESTE:
  FAIT : tests verts 2118/2118 ; preflight tech_soon PASS ; PNG vus ; docs (InterfaceInProgress.md commite 0fb4320, Auto_Journey_File.md ligne D889, EtapeDefini.md ligne 18) ; MoraleInProgress.md NON commite (voir anomalies).
  RESTE : la coupure en deux lignes ne se produit pas dans le jeu. Cause probable, NON verifiee : UHOLGridCell::SetStatus (HOLGridCell.cpp:404) coupe si MeasureStatusWidth() > DefaultCellSize - 2*StatusSidePad (= 72, constante 76 et non la largeur reelle de la case ; a 1280 la case fait 51) ; en jeu la mesure y est <= 72 (police du style peut-etre pas encore appliquee a l'appel de SetStatus, ou 76 px a 1920) puis le Paint (:504-514) mesure plus et ne fait que reduire (ComputeStatusFit). Le test passe car il appelle SetStatus avec sa propre mesure sans passer par une case posee a 51 px. Correctif propose (un build) : decider la coupure au Paint avec la largeur reelle (LocalSize.X - 2*StatusSidePad) : si la ligne seule est plus large, BreakStatusLines + StatusLineCount=2 + recalcul de ComputeNameZone ; ajouter un test qui pose la case a 51 px / 76 px et mesure le nombre de lignes affichees (pas SetStatus seul) ; re-jouer tech_soon (_en, _720). Question de design a ne pas trancher ici : a 1920 (case 76) « Coming soon » tient-il sur une ligne (D889 = deux lignes partout ou seulement quand ca ne tient pas) ?
NON VERIFIE: la cause ci-dessus (aucune trace ajoutee, aucun build, arret dur 10:51) ; tech_soon_720 n'a pas ete compare a une version anterieure ; l'automation test tourne-t-il avec un vrai renderer (non regarde).
BRANCHE PRETE: NON (tests verts mais PNG EN = une ligne). Tip : 0fb4320 (code c1ba30b).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune. MoraleInProgress.md du worktree = diff d'UNE ligne (« 2026-11-23 10:01 GALADRIEL-1123e, socle-ai, S2-d partie script... ») : vient de GALADRIEL-1123e, non commite ici ; a recuperer/commiter sur socle-ai (ou le garder tel quel : il est encore modifie dans l'arbre de specialization-trial).
GIT_STATUS_PRINCIPALE: docs seulement (AubeInProgress, Auto_Journey_File, Auto_Journey_Jour, EtapeDefini, Placeholders, Docs/Rapports/*) : oui
