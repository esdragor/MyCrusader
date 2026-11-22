# CELEBORN-1122j : DIAG du rouge HeroesOfLiberty.Combat.BuildingShotCadence (integ-41, 5d3b09f). AUCUN CODE MODIFIE, CAUSE NON ETABLIE

Joueur : la Tour de guet tirerait environ 3 fleches en 16 s au lieu de 10 (cadence reelle 3 a 5 fois trop lente), et son compte a rebours fuit si le dernier ennemi disparait.
Technique : le rouge est reel et vient de la REGLE (ou de son appel), pas de la valeur attendue du test ; la cause exacte n'est pas prouvable par lecture seule.

APPELS_DECLARES: 20 (arret 22)
HEURE_DEBUT/HEURE_FIN: 18:25:37 / ~18:37 (Get-Date)
PREMIER_BUILD: sans build (consigne)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance
TEST_ROUGE: aucun commit (diagnostic seul) ; TEST_ROUGE_SEPARE: sans objet
COMMITS: 0 ; merge_main_fait: non
TESTS: aucun lance. Journal lu : Saved_Tests\Logs\quick_tests.log (ligne 27593-27604, run local 18:23, binaire 18:12 = tip 5d3b09f). TESTS_TAUTOLOGIQUES: 0

## Valeurs obtenues (journal, tests.cpp lignes 286, 288, 294, 431)
- pas = 50 ms (321 pas pour 16 s), intervalle = ToMilliseconds(1.625) = 1625 ms, attendu = 33 pas par fleche (ceil(1625/50)).
- steady fire : premiere fleche au pas 3 (test : `First <= 1`, ligne 286 FAUX) ; 3 fleches au lieu de 10 ; fleche 1 au pas 111, attendue au pas 36 (3 + 33).
- after a pause : premiere fleche pas >= 2 ; 1 fleche au lieu de 3.
- two towers : 4 fleches au lieu de 10 ; fleche 2 au pas 99 attendue 36.
- cleanup : 2 compteurs encore vivants 2 pas apres la destruction des tours (attendu 0).
- Les comptes (`EntriesAlive` 1 / 2, 0 sans tour armee, le TMap) PASSENT : la structure du compte a rebours est bonne.

## Verdict
1. Le test n'est PAS faux sur ToMilliseconds : le calendrier attendu (33 pas) vient de StepMs = 50 et de 1625, conforme a D884 (1,625 s par fleche) ; la valeur attendue ne se change pas. Les ecarts de 108 et 96 pas (5400 et 4800 ms) ne sont pas une erreur d'arrondi (au plus 1 pas = 50 ms).
2. Le test est faux sur UN point seulement : `First <= 1` (hypothese NON VERIFIEE de 1122h). La premiere fleche part au pas 3 (3 pas sans cible : l'unite nee par ServeBirthRequests n'est pas candidate avant, ou Execute sort avant ReleaseBuildingArrows). Ce n'est qu'un echec sur 8 ; il ne change pas le calendrier (calcule depuis First). A corriger seulement apres avoir su POURQUOI 3 (voir ci-dessous), pas en elargissant la tolerance.
3. La regle (ou son appel) est en cause pour le reste : 3 fleches au lieu de 10 et periode 108 pas (steady) differente de 96 pas (2 tours) pour la meme tour : le decrement de `RemainingMs -= StepMs` n'a pas lieu a chaque pas de RunStep, ou le compte est remis a 0 sans cible. Un decrement a chaque pas donnerait une periode constante de 33 pas. Interaction T5b (5d3b09f) : `BuildingCooldownMs.Reset()` n'est dans `!bFighting` (l.1166) que ; le test tourne avec Enabled 1, donc T5b n'explique pas ce rouge (verifie : l.1162-1166, le cas Enabled 1 ne passe pas par la).
4. Trou reel lu dans le code, a corriger au meme lot : `if (Fighters.Num() == 0) { ... return; }` (HOLCombatProcessor.cpp l.1113-1118) sort AVANT `ReleaseBuildingArrows` (l.1505). Sans aucune unite vivante, aucun batiment ne tire (normal) mais les comptes ne sont jamais purges : fuite (meme defaut que T5b, autre sortie). Correctif propose, non applique faute de preuve que c'est la cause du rouge `cleanup` : `BuildingCooldownMs.Reset();` dans ce bloc, plus test (tours detruites ET victime morte : 0 entree).
5. Explication la plus probable du 3, 108, 96, (non prouvee) : `Fighters.Num() == 0` ou un autre retour precoce de `Execute` saute les pas ou la victime n'est pas encore/plus dans la requete Mass (naissance echelonnee, entite creee via file de commandes), donc ni decrement ni purge ; periode variable selon le monde.

## Le test aurait-il echoue sur le bouchon e76adf9 ?
Oui, par lecture : le bouchon garde le TSet BuildingsShot (une seule fleche en tout, aucun compte) ; `Steady.ReleaseSteps.Num()` = 1 != 10 (l.288), `EntriesAlive` 0 != 1 (l.400), `Two.EntriesAlive` 0 != 2. Le test discrimine donc bien le bouchon ET la regle actuelle.

## Pour la session principale (une mesure, pas un build de plus que le lot)
Dans le test (lot du build suivant) ou en temporaire : journaliser par pas, dans RunStep du test, `Combat->GetBuildingCooldownCount()`, `UHOLCombatProcessor::GetTotalArrowsReleased()` et le nombre d'entites vivantes (Units->GetAllUnitIds) pour les 120 premiers pas du scenario steady ; si les entites valent 0 aux pas 0-2 et a intervalles, la cause est le retour `Fighters.Num() == 0` ; sinon logger `StepMs` et `Cooldown->RemainingMs` dans ReleaseBuildingArrows (UE_LOG sous `hol.Combat.Debug`).

NON VERIFIE: tout (aucun test ni build lances ; la cause 5 est une hypothese ; la periode 108 / 96 est deduite du journal, pas rejouee ; le rapport entre 3 pas de retard initial et l'echelonnement des naissances).
PROCESSUS_DETACHES: aucun lance par moi (stray_processes non necessaire, aucun processus demarre)
ANOMALIES_PROCESSUS: aucune. 1 chemin inattendu : les journaux de test sont dans Saved_Tests\Logs, pas Saved\Logs.
GIT_STATUS_PRINCIPALE: docs seulement, inchange hors ce rapport et les deux lignes de suivi : oui
