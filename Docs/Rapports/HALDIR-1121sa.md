# HALDIR-1121sa (D879 centre secondaire A : "tout centre" en script)

Branche wall-line, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir. NON COMPILE, non lance (brief : aucun build ni quick_check).

APPELS_DECLARES: 16 (arret a 25 vus ; premier Write a mon 8e appel, pas avant : 7 appels de lecture, dont 1 perdu sur une erreur de guillemets PowerShell)
HEURE_DEBUT/HEURE_FIN: 19:12:28 / ~19:17 (arret dur 19:26)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 0516682 (test seul, avant la regle) ; assertit l'effet : oui. Rouge NON VU a l'execution (aucun quick_check lance) : stub = code existant (les appels compilent, le resultat est faux pour SecondaryCentre). Les deux commits sont separes comme demande ; la regle 7393200 est le correctif.
COMMITS: 2 (0516682 test, 7393200 regle), dernier a ~19:14 (seuil 30 min), merge_main_fait: non (non demande ; ne fusionne pas main ni integ-25)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculees par la grille et la table audio, le forum sert de reference)
ESSAIS: aucun (script de regle)
QUESTIONS: aucune
BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun
TESTS_SCRIPT_AJOUTES: 5, dans Script\Tests\HOLSecondaryCentreAll_Test.as

## Rouges attendus au commit 0516682, nom par nom (tous verts au commit 7393200)
1. Test_SecondaryCentreIsAloneInGroupLikeTheForum : IsAloneInGroup(SecondaryCentre) faux avant.
2. Test_SecondaryCentresSplitAloneSubgroups : 4 sous-groupes au lieu de 5 (les deux centres secondaires se joignent).
3. Test_SecondaryCentreCommandPriorityLikeTheForum : GetCommandPriority place le MainCentre (index 2) en premier, pas le secondaire (index 1).
4. Test_SecondaryCentreRingsTheForumSelectionSound : SelectEventFor("SecondaryCentre") = -1 et FamilyOf = "" avant.
5. Test_SecondaryCentreHasAnAudioRow : HOLAudioBuildings::Has("SecondaryCentre") faux avant.
Tests existants a surveiller au vert (non lances) : HOLCommandGridBoard_Test (Test_CGBIsAloneInGroup), HOLAudioBuildings_Test / HOLAudioBuildingFamilies_Test (boucles sur la table : la ligne SecondaryCentre doit etre un vrai type, famille Stone) ; HOLAudioElected_Test (SelectEventFor).

## FAIT
- Regle (7393200) : namespace HOLCentreKind { IsCentre(FName), IsCentreName(FString) } place dans Script\Player\HOLCommandGridGroups.as (piege AS n°2 : namespace) ; une seule source, appelee par :
  - HOLCommandGridGroups.as : IsAloneInGroup (ex-ligne 126) ;
  - HOLAudioBuildings.as : ligne de table SecondaryCentre (AlertTrainingDone comme le MainCentre, ex-ligne 55), FamilyOf (ex-189, Stone), SelectEventFor (ex-211, SelectBuildingForum).
- Journal : SecondaryCentersInProgress.md (worktree, ligne 104) ; EtapeDefini.md : une ligne ajoutee apres celle de HALDIR-1120pb.

## RESTE : lot C++ (NON codes ici)
- Source\...\HOLConstructionRules.cpp:466
- Source\...\HOLBuildProcessor.cpp:978
- Source\...\HOLGameNames.cpp:123
(les trois emplois C++ de MainCentre relevés par f85b3c0 ; chemins complets a retrouver par `git grep` dans Source\.)
Commande a lancer par la session principale apres le lot C++ : Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts tests (un seul).

## NON VERIFIE
Rien n'a ete execute : compilation des scripts, rouge puis vert, diagnostics AS (DIAGNOSTICS_AS non releves : serveur de langage non consulte). Points de syntaxe a surveiller : namespace HOLCentreKind declare avant HOLCommandGridGroups dans le meme fichier mais utilise depuis HOLAudioBuildings.as ; champs FHOLGroupMemberView.TypeName / bAlone et FHOLSubgroupView.Members supposes d'apres les tests existants ; commentaire d'en-tete du test dit « avec l'alerte de production du MainCentre » alors que le test ne verifie que Has (la valeur AlertTrainingDone est dans la regle, non testee).
PROCESSUS_DETACHES: voir la sortie du dernier geste ci-dessous
ANOMALIES_PROCESSUS: aucune ; 1 appel perdu (erreur de guillemets PowerShell) ; premier Edit a mon 8e appel.
GIT_STATUS_PRINCIPALE: a ete modifie par EtapeDefini.md seulement (doc voulu) ; les autres fichiers deja listes au lancement (GDD.md, Docs\...) inchanges par moi : oui
