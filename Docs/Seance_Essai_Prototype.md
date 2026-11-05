# Séance d'essai du prototype (PT7 de PlaytestInProgress.md)

Une page : comment juger les dix questions de GDD § 17.1 sans attendre une partie entière par question. Les impressions et les réglages vont dans `Auto_Journey_Retours.md` (`REGLAGE:` alimente la fiche de valeurs, PT0).

## 1. Les scénarios (5 essais, environ 3 minutes chacun, Entrée = étape suivante)

Chaque commande se lance depuis un PowerShell ouvert n'importe où (chemin absolu), sur le build de `main`.

| Question de § 17.1 | Scénario | Ce qu'on regarde |
|---|---|---|
| 1 et 7 : héros important sans être obligatoire ; combats intéressants sans lui | `playtest_hero_army` | le même combat (6 Hommes d'armes contre 10 Lanciers) avec puis sans le héros : compare les pertes |
| 2 et 3 : mort punitive ; fenêtre de résurrection | `playtest_hero_death` | le héros tombe, l'ennemi arrive, le délai vrai (54 s au niveau 3), l'alerte « prêt », le bouton « Ramener le héros » et son prix |
| 4 : duel, vraie décision | `duel_challenge` puis `duel_challenge_accept` | défi refusé par l'IA à forces égales, puis accepté : fenêtre de défi, cercle, issue |
| 5 : progression sans âges | `hero_xp` | l'XP par source, les niveaux, les paliers, la mort et le retour plus cher |
| 8 : volcan, opportunité ou frustration | `volcano` | l'alerte, l'autel, les soldats ; (`volcano_ai` pour l'IA qui conteste) |
| 6, 9, 10 : économie, snowball, factions | aucune : se jugent en **partie réelle** (les 3 parties ci-dessous, puis le lot IA contre IA de PT6) | |

Commande type : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Scenarios\play.ps1 playtest_hero_army`

## 2. Les 3 parties

3 parties 1v1 contre une IA Normal sur trois cartes parmi les sept (25 à 35 min, D02), l'option Catastrophe cochée sur l'une des trois. Un questionnaire de fin de partie par partie (PT4, PT5), lecture par `match_report.py` (PT1, PT3).

## 3. Le lot IA contre IA

20 miroirs Aube contre Aube par `match_batch.ps1` (PT6) pour le snowball (question 9) ; tableau lu avec l'utilisateur.

## 4. Après la séance

Les chiffres de `match_report.py` et le questionnaire sont lus **avec** l'attendu du GDD quand il existe (D728) ; le verdict reste celui de l'utilisateur. Toute valeur changée à l'œil : `REGLAGE:` dans le retour, puis une ligne de la fiche (`make_value_sheet.py --accept`).

*Les scénarios `playtest_*` de PT7 ne comptent pas encore sur les rapports de partie (PT1 à PT3 : C++ de match) : leurs chiffres se lisent dans le journal (`hol.Combat.Report`, `hol.Hero.Report`).*
