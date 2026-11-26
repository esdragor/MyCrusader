# FARAMIR-1126m3 : lot C++ n3 (gate-refund-1126m2), NON COMPILE (build FAIL, correctif commite non recompile)

APPELS_DECLARES: ~42 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 14:15:30 / 15:3x (la file de build a bloque 14:22 -> 15:28 : arret dur 14:44 depasse par l'attente du build, seule exception admise)
PREMIER_BUILD: 14:22   BUILD: FAIL (231 s de compilation, file d'attente de ~1 h ; HOLUnitAbilityReadLibrary.cpp:67 `bool` perdu par mon remplacement, corrige en b2d2fb2, NON RECOMPILE)   LAUNCHCHECK_DEBUT: binaire perime (dll 11:23 plus vieille que HOLTestWorldLibrary.h et HOLGameScreen.cpp) -> build lance
TEST_ROUGE: c378cb3 (Test_CGAKnightChallenge + plumbing enum / bKnightUnit / AbilityId + stub de la regle de cellule) ; rouge JAMAIS VU a l'execution (binaire perime, build FAIL) ; assertit l'effet : oui (cellule via GetCommand, verdict Send + AbilityId de HOLUnitAbilityTable + caster)   TEST_ROUGE_SEPARE: oui (commits separes), mais non execute
COMMITS: 5 sur gate-refund-1126m2 (c378cb3 test, 13ca144 correctif D923, 35b7ae1 retrait D921, b2d2fb2 fix build + journaux ; + merge main c2e13dd), dernier 15:3x, merge_main_fait: oui (c2e13dd, 77b857a)
TESTS: aucun lance (pas de binaire) ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 1 (Test_CGAKnightChallenge dans HOLCommandGridAction_Test.as)   BUILDS_DEMANDES: 1 (a refaire)
TYPES_A_EXPOSER: UHOLUnitAbilityReadLibrary::GetUnitSubsystemOf(AActor) (nouveau, lot C++), FHOLGridActionAnswer.AbilityId, EHOLGridCommand/Kind::KnightChallenge, bKnightUnit
ESSAIS: aucun (scenario knight_challenge_button non ecrit) : LOGIQUE SEULEMENT
QUESTIONS: aucune (cellule 4 / touche T derriere GetKnightChallengeCell, reponse deja donnee : T)
FAIT:
- D923 : enum KnightChallenge (HOLCommandGrid.h, ReadLibrary.h), bKnightUnit (HOLCommandGrid.cpp NoteSelectedUnitType VirtuousKnight, Board.cpp, ReadLibrary.cpp), regle Rules.as GetKnightChallengeCell (4) + branche, Keys.as (cle de grille), Action.as Decide = Send + AbilityId (id de la table, pas de constante C++), pont .as, ExecuteGridCommand : cas KnightChallenge -> RequestCastUnitAbility(Answer.AbilityId, Answer.UnitId), serveur ServerCastUnitAbility -> HOLKnightChallenge::CastChallenge, face « Defi » / infobulle « Defi du Chevalier » (Face.as, GameScreen.cpp).
- D921 (liaison de credit) : COMMENCE puis RETIRE sur ordre du coordinateur (fait par ELROND-1126m3, 7bbb8ee) : commits e2d5bfc / 13c2472 annules par 35b7ae1 (revert squashe) ; diff Net / Buildings / Tests / HOLWallLineRules_Test.as vs c2e13dd = vide ; seul HOLRpcPlayer.as differe (branche D923 du serveur).
RESTE:
- G-5b/c (Produce / Research / Trade / Build vers le script, FHOLGridCostFacts dans le pont) : NON COMMENCE.
- Touche T injectable : NON FAIT (instrumenter hol.UI.Key / reception de la touche par la grille d'abord, L29-3).
- Recharge en secondes de la face du Defi : 0 pour l'instant (la recharge est sur le fragment de combat du Chevalier, pas de lecture client) ; textes .po FR/EN/JA du Defi (« Defi », « Defi du Chevalier » en NSLOCTEXT dans Face.as seulement) ; scenario knight_challenge_button ; lignes Placeholders.txt/EtapeDefini.md non touchees.
- Relancer : `Tools\build.ps1 -LiveCoding=0 -Wait` dans Faramir puis quick_check -Parts tests (quick_check du worktree).
NON VERIFIE: tout (aucune compilation reussie) ; Face.as : accents ecrits en UTF-8 (verifie par Select-String) ; le cas KnightChallenge dans HOLGameScreen.cpp (liste du switch + branche FaceNumbers) et le switch de HOLGameScreen.cpp:1113 (ShownGridTypes) ; la touche T pour la cellule 4 depend du mappage de HOLCommandGridKeys (non lu). Rouge non vu.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: file de build longue (Sauron, Celeborn, Elrond devant, 14:22 -> 15:26) ; `$nbool` dans mon script de remplacement PowerShell a avale `bool` (variable $n collee a un mot) ; filter-branch tente puis abandonne (aucun effet), commits d'annulation recomposes par reset --soft sur MES commits non pousses.
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (aucun fichier de code modifie)
