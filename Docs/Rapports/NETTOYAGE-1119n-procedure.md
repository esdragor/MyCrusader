# NETTOYAGE-1119n-procedure (étape 0a, 2026-11-19 soir)

Archive : `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-19_soir.md` (99145 octets, copie intégrale des 3 fichiers avant toute modification, un titre par fichier). Rien supprimé.

## Auto_Journey.md (48487 -> 43855 octets, CRLF conservé)
- Section « Séance de migration » : corps (intro, points 1 à 7, textes des reprises 12 à 17) remplacé par 3 lignes : TERMINÉE (D857), bilans dans `Auto_Journey_Jour.md`, plan `ANALYSE-1116a.md`, archive nommée, « continue migration auto journey » = proposer une séance de fonctions, règles D839 / D838 / consigne Sonnet 15 min. Titre de section inchangé.
- Sommaire : puce « Qualité des essais » corrigée (tout sous Sonnet 5.5, agents de 15 min au plus, consigne du 2026-11-16) ; ligne d'archive de ce nettoyage ajoutée à la puce « Archives ».
- « Modèle d'énoncé d'un agent » : phrase d'en-tête ajoutée avant le bloc de code (bloc non réécrit).

## Auto_Journey_Checklist.md (40861 -> 41624 octets, CRLF conservé)
- Aucune ligne archivée (doute : rien n'était sûr, les lignes ne disent pas elles-mêmes « fait »).
- Section « ## Lecons de la reprise 17 du 2026-11-19 » ajoutée à la fin : L17-a, L17-b, L17-c (format ID | Constat | Règle | Où).

## Tools\AutoJourney\brief_commun.md (9403 -> 9209 octets, LF conservé)
- Lignes 3 et 4 réécrites pour une séance de fonctions (arrêt 15 min / 25 appels, pas de build ni quick_check dans un agent, C++ neuf signalé pour le lot groupé, tests neufs en script sauf Mass D838, rouge VU en deux commits, ligne `[ ]` suivante). Champs retirés : `TESTS_CPP_RETIRES`, `LIGNES_CPP_SUPPRIMEES`, `LIGNES_MIGREES_PAR_LOT` ; gardés : `BUILDS_DEMANDES`, `TESTS_SCRIPT_AJOUTES`, `TYPES_A_EXPOSER`.
- Règle 9 : parenthèse datée remplacée par la règle générale (DLL plus vieille que le `Source\` de main : ne fusionne pas main).
- Intacts : paragraphe « Dossiers de `Script\` interdits... », règles 0 à 11, « Pièges Angelscript ».

## Doutes
- Checklist : non nettoyée (consigne : seulement ce qui est sûr).
- `brief_commun.md` règle 3 parle encore de « Tâche C++ » (build `-Wait` par l'agent) et la règle 2 de « UN test rouge + UN correctif » : laissées (hors périmètre demandé), en légère tension avec « pas de build dans un agent » ; à trancher par la session principale.
- Les règles `agent_rules.md` (45 appels vus) et le bloc du modèle d'énoncé (45 appels) divergent de 25 appels : signalé par la phrase d'en-tête, non réécrit.

## NON VÉRIFIÉ
- Aucun contrôle `session_gate.ps1` lancé après les modifs ; aucun test d'outil lu par le brief (références au texte de la section migration ailleurs, ex. `session_gate.ps1` ou la check-list cherchant « Séance de migration », non recherchées).
