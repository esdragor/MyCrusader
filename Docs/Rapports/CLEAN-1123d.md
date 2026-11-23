# CLEAN-1123d : nettoyage 0a de Auto_Journey.md et Auto_Journey_Checklist.md (2026-11-23)

HEURE_DEBUT/HEURE_FIN: 08:49 / ~08:52 (Get-Date PowerShell). APPELS_DECLARES: ~14 (arret a 25 vus).

## Fait
- Copie integrale des deux fichiers : `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-23.md` (renvoi ajoute au Sommaire, puce « Archives », et paragraphe « Nettoyage du 2026-11-23 » dans la check-list).
- Contradiction D838/D888 resolue : « Seance de migration » (derniere ligne de regles) cite maintenant D838 (tests Mass C++, autres tests en script) et D888 (fusion locale a chaque passe verte, accord permanent, jamais de push) separement. Les autres mentions de la procedure (ligne « Fusion dans main : accord PERMANENT D888 », « Fusions dans main », « Mode interactif ») etaient deja D888.
- Auto_Journey.md : 2 lignes retirees (redites-renvois d'`agent_rules.md` regles 9 ; 7, 8, 11), 4 phrases retirees ou raccourcies : plafond « 80 appels / 45 vus » (contredisait « 15 min / 25 appels » de brief_commun, remplace par un renvoi), renvoi d'archive du piege « Test 2 », phrase « c'est l'heure d'arret qui compte » (dans brief_commun point 1), plus la phrase D838 reecrite.
- Auto_Journey_Checklist.md : 4 lignes de tableau retirees : T2-3, L3 / T2, L9-10, L19-8 (redites d'`agent_rules.md` / `brief_commun.md` / de la procedure). Un paragraphe « Nettoyage du 2026-11-23 » ajoute.
- Total : 6 lignes retirees (2 + 4), 5 phrases retirees ou reecrites, 1 paragraphe + 1 renvoi d'archive ajoutes.
- Intactes : L24 (5 lignes L24-1 a L24-5, la check-list reste a L24 pour `session_gate.ps1` L80-92 qui lit le plus grand `Lxx-y`), tout le modele d'enonce, ligne L12-6.

## Doutes (lignes gardees)
1. Le modele d'enonce de `Auto_Journey.md` (bloc ``` de la section « Modele d'enonce d'un agent ») est presque entierement une redite d'`agent_rules.md` / `brief_commun.md` ET dit « 45 appels », alors que brief_commun dit 25 appels / 15 min. NON retire : `Tools\AutoJourney\gate_test.ps1:284-291` exige que ce bloc cite `CPP: oui`, `LAUNCHCHECK_DEBUT`, `TESTS_TAUTOLOGIQUES` (et le texte `L12-6`). Le raccourcir demande de modifier `gate_test.ps1` (hors perimetre).
2. Rapport final : la procedure dit « 10 lignes au plus » (ligne Rapport final, et « Economie de tokens » 10 lignes + chemin), `brief_commun.md` point 11 dit « 6 lignes max ». Non tranche.
3. L14-6 (check-list) : en partie redite d'`agent_rules.md` regle 7, mais promet un champ `TEST_ROUGE: observe / suppose` refuse par `Report` du gate (« non appliquee » au texte) : je ne l'ai pas verifie dans `session_gate.ps1`. Gardee.
4. L1-1/L3-6/L4-7/L5-3 (check-list, 80 appels / 45 vus) : redite d'`agent_rules.md` 2, mais `gate_test.ps1:279` verifie « 45 appels » dans agent_rules ; texte tronque a la lecture, gardee. Le brief_commun (25 appels) la contredit en partie.
5. Ligne « Le veilleur » du paragraphe de creation du rappel : mon Edit de dedoublonnage de la cadence 30/5 min n'a pas ete applique (balise gras dans la ligne) ; redite de cadence (3 occurrences) laissee.
6. Les paragraphes historiques « Nettoyage du ... » de la tete de la check-list (lignes 14-36, 10 paragraphes) sont des renvois d'archive ; non regroupes (pas de risque de perdre un renvoi sans les relire un par un).

## NON VERIFIE
- `gate_test.ps1` et `session_gate.ps1` NON lances apres edition (seule la presence des 4 chaines `CPP: oui`, `LAUNCHCHECK_DEBUT`, `TESTS_TAUTOLOGIQUES`, `L12-6` dans Auto_Journey.md a ete verifiee par script, et L24 present x5).
- Le texte de plusieurs lignes de la check-list (tronquees a 260 car. a la lecture) n'a pas ete relu en entier hors les 5 lignes examinees.
- Aucun ID Lxx-y du script `session_gate.ps1` ne reference une ligne retiree (grep des identifiants : L11-1, L11-7, L12-1c, L12-2, L12-6, L13-2, L14-13, L14-8, L15-1, L19-1, L20-3, L7-2, L7-4, L7-6, L8-1, L8-3, L8-6, L8-7, L8-9, L9-4, L9-5 : tous dans des commentaires du script, aucun retire ici).

## Anomalies
- `Docs\Rapports\CLEAN-1123a.md` et `CLEAN-1123c.md` apparaissent non suivis dans `git status` (autres agents, non touches).
- Les fins de ligne et l'absence de BOM des deux fichiers ont ete preservees (diff git : 7 insertions, 12 suppressions).
