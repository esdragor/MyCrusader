# HALDIR-1120pd : durcissement de edit_line.ps1 (sans C++, outil partage)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche wall-line, branche prete.

APPELS_DECLARES: 16 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:00:14 / 16:05:07
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: f80bc99 (Tools\Tests\edit_line_test.ps1, section 8 PD-1..PD-6) ; assertit l'effet : oui (code de sortie, octets du fichier, ligne ecrite). Rouge VU a 16:01:53 : "edit_line_test : 9 FAIL".
COMMITS: 4 (f80bc99 test seul, 7365bf7 correctif, edf690f agent_rules L14, dc0e238 merge main), dernier 16:04 ; merge_main_fait: oui (dc0e238)
TESTS: `powershell -File Tools\Tests\edit_line_test.ps1` : avant correctif 9 FAIL, apres "edit_line_test : PASS" (exit 0), re-lance apres le merge main : PASS ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (outil)
QUESTIONS: aucune

## FAIT
1. `-NewLine` vide ou blanc : REFUS (code 1, fichier intact), message qui nomme `-Delete`. Nouvelle option `-Match "<texte>" -Delete` = seul moyen de supprimer une ligne (garde les autres lignes et leurs fins ; derniere ligne sans fin : la precedente n'en recoit pas). `-Delete` + `-NewLine` non vide ou `-InsertAfter` / `-Append` : refuse.
2. `-Match` ou `-After` multi-lignes : refus dont le message dit "multi-lignes" (une ligne a la fois).
3. `-After` / `-Match` : seconde passe (hors -Regex) apres normalisation Unicode NFC et espaces (insecables U+00A0 / U+2007 / U+202F, doubles, tabulations = une espace), seulement si le texte exact ne trouve aucune ligne. Le refus d'un texte absent dit "comparaison essayee aussi apres normalisation Unicode NFC et espaces". Unicite de -Match toujours exigee.
4. `agent_rules.md` (ligne Outils L14, edf690f) mis a jour ; ligne ajoutee dans Auto_Journey_File.md (section Outils, ligne 172, [x]).

## Diagnostic honnete de « -After refuse »
Je n'ai PAS pu reproduire le refus de GLORFINDEL-1120pb : sur le tool d'origine, `-After "E-specialization_cheat = OK"` et `-After "deja"` accentue passent (PowerShell et Git Bash, tests dans ce rapport). Les causes plausibles que le correctif couvre : accent decompose (NFD), espace insecable ou double dans la ligne visee (copie de texte), et message de refus peu clair. Si le texte visé n'etait simplement pas dans le fichier cible, le refus restait normal ; le message l'explique maintenant. Test PD-4/4b/5/5b/6 couvrent ces cas.
Note : `-NewLine ""` passe par `powershell -File` depuis PowerShell 5.1 est perdu par la ligne de commande ("Missing an argument", code 1, fichier intact) ; depuis Git Bash l'argument vide arrivait et effacait la ligne : c'est ce cas qui est desormais refuse par l'outil lui-meme (test PD-1 avec l'argument `""`).

NON VERIFIE: usage par les autres scripts qui appellent edit_line.ps1 (aucun grep fait des appelants : un appel qui comptait sur un -NewLine vide pour supprimer sera refuse, c'est voulu) ; comportement de la seconde passe sur de tres gros fichiers (non mesure) ; la cause reelle du refus de GLORFINDEL-1120pb (non reproduite).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune (un exit 255 vu sur le dernier appel groupe, apres l'ecriture : git status / Get-Date, sans effet sur les fichiers).
GIT_STATUS_PRINCIPALE: modifs docs seulement (Auto_Journey_File.md par ma ligne 172, plus celles d'autres sessions deja presentes) : oui
