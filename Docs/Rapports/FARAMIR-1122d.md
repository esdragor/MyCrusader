# FARAMIR-1122d : agents_table.ps1 (L15-4), sans C++

APPELS_DECLARES: 18 (arret a 25 vus ; seuil de 80 juge sur tool_uses de la session principale)
HEURE_DEBUT/HEURE_FIN: 17:07:03 / 17:12:01
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet, outil PowerShell)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 81eba67 (bouchon + test, 15 rouges sur 26 verifications, vu a 17:08, code de sortie 1) ; assertit l'effet : oui (contenu du fichier, octets, codes de sortie, ligne relue)   TEST_ROUGE_SEPARE: oui (81eba67 rouge, 1540e94 vert). Le test a recu 3 corrections au commit vert (decoupage des cellules, stderr de powershell.exe sous $ErrorActionPreference Stop, retrait de la comparaison de hash du vrai Jour qui bouge sous les autres sessions) : le commit rouge garde ses 15 rouges reels.
COMMITS: 2 (81eba67 rouge, 1540e94 vert), dernier a 17:11, merge_main_fait: non (branche zone-effect, outil seul ; la session principale synchronise)
TESTS: agents_table_test.ps1 = 37 ok / 0 rouge (VERT, code 0) ; essai de bout en bout sur une copie du vrai Jour (-Add, doublon refuse code 1, -Update, absent refuse code 1) : copie = original + 1 ligne, 0 LF seul ; TESTS_TAUTOLOGIQUES: 0 (les valeurs attendues viennent du fichier relu apres l'execution de l'outil, l'heure de Get-Date)
ESSAIS: aucun (outil)
QUESTIONS: aucune

## Syntaxe exacte
```
powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\agents_table.ps1 -Add <agent> -Id <id> -Worktree <worktree> -Task "<tache>" -Cpp oui|non [-Jour <fichier>]
powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\agents_table.ps1 -Update <agent> -Summary "<texte>" [-Jour <fichier>]
```
- Section `## Agents du <yyyy-MM-dd du jour>` ; Lance = Get-Date (HH:mm) a l'ajout (Fini = `en cours`, resume vide), Fini = Get-Date a la mise a jour ; aucun parametre d'heure (un `-Lance` est refuse par CmdletBinding).
- `-Add` refuse (code 1) si l'agent existe deja dans la section du jour (un agent d'un autre jour ne compte pas) ; `-Update` refuse (code 1) si absent ou present sur plusieurs lignes ; section / tableau absent, texte avec fin de ligne, moins de 8 colonnes : refus. Fichier intact a l'octet dans tous les refus.
- `|` du texte echappes en `\|` ; CRLF, UTF-8 sans BOM (BOM deja present garde), derniere ligne sans fin de ligne : rien de colle ; ligne neuve = juste apres la derniere ligne du tableau.
- Sortie : `agents_table : <fichier> ligne N` puis la ligne relue ; codes 0 / 1 refus / 2 erreur / 3 relecture differente (original restaure).

FAIT / RESTE: FAIT : outil + test (2 commits). RESTE : la session principale peut l'utiliser pour le tableau du jour ; les autres outils de L15-3 a L15-6 (replace_text.ps1, build_wait.ps1, agent_rules.md) ne sont pas touches.
NON VERIFIE: usage sur le vrai Auto_Journey_Jour.md (jamais ecrit : seulement une copie) ; pas de sortie quand deux sessions ecrivent le Jour en meme temps (l'outil relit puis ecrit en une fois, sans verrou) ; une ligne du tableau avec `\|` deja echappe par un autre outil est comptee comme une seule cellule (voulu).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune ; les fichiers Write sont en LF dans l'arbre de travail (git les passera en CRLF), sans effet sur l'outil
GIT_STATUS_PRINCIPALE: aucun fichier hors docs voulus modifie par moi : oui (EtapeDefini.md l.11 et Auto_Journey_File.md l.185 seulement ; IN_USE.txt du worktree, non suivi, remis a libre)
