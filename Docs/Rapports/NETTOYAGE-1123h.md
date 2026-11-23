# NETTOYAGE-1123h (étape 0a, 2026-11-23 après-midi) : rapport

Commit 7ae89ba (branche main, non poussé), dernière ligne `Agent: NETTOYAGE-1123h`, 4 fichiers : Auto_Journey.md, Auto_Journey_Checklist.md, Tools/AutoJourney/brief_commun.md, archive `Docs/Archive/Auto_Journey_Procedure_Checklist_nettoyage_2026-11-23_apres-midi.md` (copie intégrale des 3 fichiers avant édition).

## Lignes avant -> après
- Auto_Journey.md : 188 -> 177 lignes (45063 -> 42987 octets). CRLF et sans BOM conservés.
- Auto_Journey_Checklist.md : 248 -> 250 lignes (59889 -> 57570 octets) : lignes réduites à un renvoi, mais paragraphe de nettoyage ajouté.
- brief_commun.md : 35 -> 35 lignes (un ajout au point 9). Fichier en LF (inchangé).

## Identifiants retirés / réduits (aucun renommé, aucune ligne supprimée sans identifiant gardé)
Aucun identifiant retiré du tout : tous restent (le gate Launch lit seulement le plus grand L<N>, L25 reste ; le gate Close lit le titre `### Lecons appliquees`). Ramenés à un renvoi court : L25-1, L25-2, L25-3 (brief points 3b-3d), L22-2 (brief point 4 + agent_rules règle 7), L22-6 (fait : session_gate.ps1:362-363 exempte le trailer `Agent:`), L22-7, L24-1 (RÉGLÉ D888), L24-2 (fait), L15-4 (agents_table.ps1 fait), L15-8, L15-9, L15-10 (brief points 8, 2, 11). L25-6 déplacée après L25-5 et précisée ; L21-1 précisée (« seules questions de lancement : fin et mode ») ; L23-1 : mention de la règle 14 remplacée.
Procédure : le bloc de code complet « Modèle d'énoncé d'un agent » (45 appels, redite de brief_commun.md, contradictoire avec 25 appels) remplacé par un modèle de 5 lignes qui renvoie à brief_commun.md / agent_rules.md ; clause « tout sous Sonnet 5.5 (session principale...) » retirée de « Modèles et limites » (contredisait la phrase Opus) ; sommaire de la section et « Rapport final de 10 lignes » alignés sur brief point 11 (6 lignes).

## Phrases alignées sur L25-6
1. « Au démarrage » : paragraphe « Premier geste (L25-6) » ajouté : fin et mode demandés par l'interface de choix, seules questions permises en autonome, fin dans Docs\SessionEnd.txt, ligne `fin et mode demandés à <HH:MM>` dans le tableau du jour.
2. Règle 14 de « Qualité des essais » : « mode interactif seulement » -> « séance interactive » + exception des questions de lancement.
3. Étape 0a : « mode interactif seulement » -> « séance interactive », doute en autonome dans Retours\ et le rapport.
4. « Mode interactif » : « Lancé seulement par... » -> aussi choisi à la question de lancement.
Phrase « bilan fin choisie par la session principale » : INTROUVABLE dans mes fichiers (Auto_Journey.md, Checklist, brief_commun.md) ; elle est sans doute dans Auto_Journey_Jour.md (hors périmètre), à corriger par l'agent du Jour.

## Doutes laissés en place
- L12-1 (« PROPOSE... ») : en partie faite (Launch -Root FAIL retard dll) mais je n'ai pas vérifié (b), (c), (d) : ligne gardée.
- L15-6 (`build_wait.ps1`) : existence de l'outil non vérifiée : gardée.
- L9-1 (minuteur/cron/Monitor) : proche de brief 3d mais plus large : gardée.
- L19-1 : le gate la lit (« check-list lue ») : gardée en entier.
- L24-5 (refus du classifieur) : agent_rules règle 7 ne couvre que le rouge : gardée.
- « Builds : ne pas saturer la file » ligne « File de build : règles du 2026-11-03 » (long, redites partielles d'agent_rules règle 10) : non touchée.
- Étape 5 « Qualité des essais » règle 5, L5-7 / L22-7 (question du commit de GDD.md au lancement) : en autonome, elle n'est pas dans les questions de lancement permises par L25-6 ; laissée telle quelle (L23-1 prime).

## NON VÉRIFIÉ
- gate_test.ps1 non lancé (consigne). Le gate Close n'a pas été lancé : seule la lecture du code montre que les IDs de leçon ne sont pas lus individuellement.
- Pas de relecture intégrale des fichiers après édition (contrôle par `git diff --stat`, longueurs, sections `##`).
- brief_commun.md : git signale « LF will be replaced by CRLF » (.gitattributes), comportement habituel du fichier.

## Gate
`=> aucun FAIL` (session_gate.ps1 -Phase Launch -End 19:00 -NoCpp -AgentName NETTOYAGE-1123h) ; WARN seulement : L19-1 (exempté NETTOYAGE), no_window_test, 8 worktrees en retard de 7 commits, fichiers suivis modifiés dans la copie principale.

## Anomalies
- commit_docs.ps1 -Only refuse brief_commun.md (hors liste des docs) : commit fait par `git commit -- <4 chemins>` direct (pathspec explicite, pas de `git add -A`).
- Le status initial montrait Docs/SessionEnd.txt modifié et FARAMIR-1123b-fix.ps1 non suivi : non touchés, non commités.
