GALADRIEL-1126m2 : rejeu ai_tower (DLL 2608d69), SANS C++, aucun commit (scenario DumpUnit annule).

APPELS_DECLARES: ~28 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 14:23 / 14:47 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: sans objet (DLL de 2608d69 compilee, 2329/2329 selon l'enonce)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun neuf (rouge reel #19 vu au journal) ; assertit l'effet : sans objet   TEST_ROUGE_SEPARE: non (aucun code livre)
COMMITS: 0, merge_main_fait: non (fait puis ANNULE par git reset --keep 2608d69 : les scripts de main (HOLControllerArmedMode_Test.as) ne compilent pas contre la DLL 2608d69, la partie reste figee au chargement ; 1er run perdu)
TESTS: aucun quick_check ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: ai_tower rejoue 2x (play.ps1 ai_tower -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel), LOGIQUE SEULEMENT (journal, aucun PNG ouvert ; preflight non lance)
QUESTIONS: aucune
FAIT / RESTE:
- #7 : faux spin DISPARU (aucune ligne Sanity FAIL #7 sur 2 runs ; tour achevee a 13.39.59 / 13.35.00).
- #19 : FAIL identique aux 2 runs (596 deg en 3 s a (15060,-346)). hol.Sanity.DumpUnit 19 : c'est un Lancier du bloc de 5 du scenario (pas un paysan) qui ORBITE ~475 pas (~12 s) autour de sa place de bloc (slot 15190,-278, ordre Phase 3/1/2, vitesse jusqu'a 680 cm/s), sans arriver, jusqu'a l'achevement de la tour (pas 2785, place -> 15200,-278). Vrai bug de marche en bloc, C++ (arrivee sans freinage), PAS corrige.
- RESTE : lot C++ : test rouge Mass (bloc de 5 vers une place voisine d'un chantier, chaque soldat arrive sans path > 3 x net), correctif, rejeu ai_tower. La branche morale-look-fix-1125m n'est donc PAS verte pour ai_tower (#7 ok, #19 rouge), mais 2608d69 est valide pour #7.
- Fiche Retours\BUG-ai_peasant-spin.md et EtapeDefini.md a jour. Journal du run : Docs\Rapports\GALADRIEL-1126m2-ai_tower.log.
NON VERIFIE: la cause exacte de l'orbite (lecture du dump seulement, code de marche en bloc non relu) ; ai_tower_easy non rejoue ; PNG ai_tower_built non ouvert.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: le 1er play a tourne 420 s dans le vide (erreurs Angelscript apres merge main) ; editeurs d'autres worktrees (Arwen, Glorfindel) actifs, non touches ; quick_tests.log du worktree ecrit a 14:22 par une autre session.
GIT_STATUS_PRINCIPALE: aucun fichier de code modifie : oui (docs : Retours, EtapeDefini, Docs\Rapports)