# FUSION-1116a : fusion groupée du 2026-11-16 (worktree Celeborn, branche hero-j8-close)

APPELS_DECLARES: 27
HEURE_DEBUT/HEURE_FIN: 17:20 / 17:43
PREMIER_BUILD: 17:22 (lancé ; en file derrière Haldir, compilation 340 s, appel total 534 s, fini 17:30)   BUILD: PASS   LAUNCHCHECK_DEBUT: binaire en retard sur HOLSignalGroundMarkTests.cpp et HOLGameScreen.cpp (attendu après fusion, le build l'a réglé)
TEST_ROUGE: sans objet (fusion, aucun code écrit) ; assertit l'effet : sans objet
COMMITS: 6 (5 fusions de branches + 1 fusion de main amendée avec EtapeDefini.md), dernier à 17:42, merge_main_fait: oui (au début et à la fin)
TESTS: tests 1326/1328 (17:34 ; les 2 rouges = EXACTEMENT HeroesOfLiberty.Match.StartingHero.NomadFirstForum et NomadReplicated, rouges connus D831 ; quick_check les étiquette « REGRESSION » parce que Find-KnownRed compare au nom de partie, cf. known_reds.txt) ; values PASS ; net PASS (net + netsig3, 377 s, 17:40) ; aiserver PASS (48 s, 17:41) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (fusion)
QUESTIONS: aucune
FAIT / RESTE:
- main 65287d0 -> 2bd5fd3 (avance rapide 17:42, RIEN POUSSÉ). Preflight_merge PASS 17:42 (seul fichier sale de la copie principale : Auto_Journey_File.md, hors branche).
- Fusionnées sans conflit, dans l'ordre : tiers 0dde5ec (f782517), visual 156f243 (2c9be8a), maps 7464394 (ad2bffc), galadriel-tab 430d459 (b249655), net-playtest 25e0f57 (8d98992). Exclue : fog (énoncé, WIP non compilé). Aucune autre exclusion.
- EtapeDefini.md dans le commit de fusion 2bd5fd3 : L28 « tiers PAS dans main » (x2) -> « DANS main f782517 » ; L92 chat net-playtest -> « script DANS main 8d98992 » ; L352 maps « non fusionnee » -> « DANS main ad2bffc » ; L360 tiers et L578 galadriel-tab « branche prête » -> « DANS main <sha> ». Aucune ligne Sauron D829 (visual) trouvée dans EtapeDefini.md : rien à changer.
- RESTE : index de Retours\ et LISEZ-MOI non touchés (pas demandés par l'énoncé) ; le patch D806 C++ (Docs\Patches) reste non appliqué, comme prévu par sa branche.
NON VÉRIFIÉ: pas de scénario lancé (essais E-* des branches non rejoués après fusion) ; Fog non fusionné ; les autres worktrees n'ont pas refusionné main.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: build en file ~3 min derrière Haldir (normal) ; quick_check affiche « FAIL / REGRESSION » pour les 2 rouges connus D831 (limite connue de Find-KnownRed, documentée dans known_reds.txt) ; fichiers non suivis c28_*/c31_* du worktree non touchés.
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md` + Docs/Rapports non suivis (état d'avant, plus ce rapport) ; inchangée hors ce rapport : oui
