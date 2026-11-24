# ARWEN-1124f : F7, outil replace_text.ps1 (integ-37, sans C++)

APPELS_DECLARES: 12 (arret a 25 ; seuil 80 juge sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 11:38:05 / 11:41 (arret dur 11:53)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 5fd822d (Tools\AutoJourney\replace_text_test.ps1, outil absent) vu a 11:39 : 4 ok / 9 rouge sur 13 verifications ; assertit l'effet : oui (octets du fichier apres l'appel de l'outil, codes de sortie)   TEST_ROUGE_SEPARE: oui
COMMITS: 3 (5fd822d test rouge, 8bb61f9 replace_text.ps1, 5a349b7 brief_commun item 5), dernier a 11:40, merge_main_fait: non (sans C++, binaire non concerne ; la session principale synchronise)
TESTS: replace_text_test.ps1 apres le correctif : 13 ok, 0 rouge, code 0 (11:39) ; 6 cas : remplacement unique (CRLF + accent gardes a l'octet pres), 0 occurrence -> code 1 fichier intact, 2 occurrences -> code 1 fichier intact, BOM EF BB BF garde (un seul), chemin relatif -> code 2 fichier intact, texte litteral (regex et `$1` non interpretes). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (outil sans effet joueur)
QUESTIONS: aucune

FAIT :
- Tools\AutoJourney\replace_text_test.ps1 (rouge d'abord, commit separe).
- Tools\AutoJourney\replace_text.ps1 `-File <abs> -Old <txt> -New <txt>` : texte litteral, exactement 1 occurrence, BOM garde, reste conserve a l'octet pres, relecture et restauration si differente. Codes : 0 ok, 1 refus (fichier absent, -Old vide, 0 ou N occurrences), 2 chemin relatif ou erreur inattendue, 3 relecture differente. Analyse Parser::ParseFile : 0 erreur.
- brief_commun.md item 5 (copie du worktree) : cite replace_text.ps1 avant la commande inline, modifie AVEC l'outil (1 ligne de diff).
- EtapeDefini.md (ligne 1033) et Auto_Journey_File.md (ligne 217, sous la ligne de file L15-3) mis a jour.
RESTE : build_wait.ps1 (tache suivante de la file) ; recopier le changement de brief_commun.md item 5 dans la copie principale a la fusion (le fichier y est aussi suivi par git ; je n'y ai pas touche).

NON VERIFIE : fusion de la modification de brief_commun.md avec la copie principale (non comparee) ; l'outil n'a pas ete essaye sur un fichier de plus de quelques Ko ni avec des fins de ligne LF seules (le test couvre CRLF) ; les fichiers de test ne sont pas passes par session_gate.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune. Note : le journal de worktree (IN_USE.txt) etait deja « libre » a l'entree, remis a « libre : ARWEN-1124f termine ».
GIT_STATUS_PRINCIPALE: sorties de `git status --short` de la copie principale : fichiers modifies par moi = EtapeDefini.md et Auto_Journey_File.md (docs voulus) ; inchange hors docs voulus : oui (JourneyInProgress.md, ELROND-1124e.md, E-deploy_far.md apparus pendant la seance viennent d'autres agents).
