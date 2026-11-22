# GALADRIEL-1122a : infobulle or FR sur le bandeau Scenario (RECETTE-1121sd)

APPELS_DECLARES: 25 (arret par budget d'appels, pas par l'heure)
HEURE_DEBUT/HEURE_FIN: 16:31 / 16:34
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 8376a47 (test Test_TopBarGoldTipDoesNotCoverTheScenarioBanner + bouchon IncomeTipDrop qui renvoie 0), assertit l'effet : oui (rectangle de l'infobulle contre rectangle du bandeau) ; rouge NON VU a l'execution (aucun quick_check, aucun chargement du jeu : budget d'appels). Rouge par construction : avec le bouchon, la 2e assertion (pas de chevauchement) est fausse.
TEST_ROUGE_SEPARE: oui (8376a47 test + bouchon, puis 45b871c correctif)
COMMITS: 2, dernier a 16:34, merge_main_fait: non (branche = main, rien a fusionner)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (le haut du bandeau vient de HOLScenarioPanelRules::PanelTop, la largeur de l'infobulle de IncomeTipView(0, Gold) FR, le decalage de IncomeTipDrop ; seuls la largeur du bandeau 860 et la hauteur 70 sont des entrees de test)
ESSAIS: aucun (income_tooltip non charge)
QUESTIONS: aucune
FAIT / RESTE:
- Fait : IncomeTipDrop (decalage vertical = bas du bandeau + 6 - haut naturel, 0 sans bandeau ou si l'infobulle est hors de la largeur du bandeau), IncomeTipNaturalTop, IncomeTipWidth ; les 8 infobulles sont enveloppees dans un UBorder transparent (IncomeTipHost) dont le padding haut est le decalage (0 par defaut : rendu inchange) ; UHOLTopBarPanel.SetBannerRect(Left, Right, Bottom).
- RESTE (autre fichier, non touche) : Script\UI\HOLGameScreenScript.as doit appeler TopBar.SetBannerRect avec le rectangle du bandeau (ScenarioPanel : WidthSlot / ScenarioWidth, haut = HOLScenarioPanelRules::PanelTop), chaque Refresh ; sans cet appel le correctif est inerte en jeu. Sinon rien ne decale (le bandeau n'est pas accessible depuis HOLTopBar.as).
- Limite du moteur : l'infobulle moteur est posee au curseur ; seul un vide transparent au-dessus du panneau la deplace (vers le bas). Hypothese non verifiee : la fenetre d'infobulle accepte ce vide transparent.
NON VERIFIE: compilation du script (SetToolTip(Host), UBorder.SetBrushColor/SetContent sur un UBorder construit par ConstructWidget, FName(f"...") copies de motifs existants ; Text.Len(), float(int)) ; 0 erreur Angelscript au journal (L22-1) NON VU ; PNG FR non ouvert : le bandeau n'a pas ete vu degage ; rouge non vu a l'execution ; largeur du texte estimee (8 px par caractere) sans moteur de rendu ; position reelle du curseur (haut naturel 56 px) estimee.
DIAGNOSTICS_AS: non releves (serveur de langage non consulte)
PROCESSUS_DETACHES: aucun lance par moi (aucun editeur demarre)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non releve ; seules EtapeDefini.md (ligne ajoutee) et ce rapport touches dans la copie principale : oui
Notes : premier Edit au 16e appel et non avant le 8e (lecture de 7 fichiers dont le bandeau necessaire pour fixer la regle) ; ligne Auto_Journey_File.md non mise a jour (budget). Journal : InterfaceInProgress.md du worktree.
