# DOCS-1120t : lecons de la seance du 2026-11-20

APPELS_DECLARES: 12 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 13:15 / 13:17 (arret dur 13:45)
PREMIER_BUILD: sans C++   BUILD: sans objet (docs seulement)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs) ; assertit l'effet : sans objet
COMMITS: 1 (1b299a5, 13:17), merge_main_fait: sans objet (copie principale, branche main)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0

FAIT:
- (A) `Auto_Journey_Checklist.md` : section `## L19 : lecons de la seance du 2026-11-20` en tableau `| ID | Constat | Regle / controle | Ou |` (format des sections L17/L18), lignes L19-1 a L19-9 ; L19-1 marquee « LA PLUS IMPORTANTE ». CRLF et absence de BOM gardes.
- (B) `Auto_Journey_File.md`, section « Outils et processus » seulement : 5 lignes `[ ]` inserees apres la derniere ligne `[ ]` de la section (L19-2, L19-3, L19-8, L19-9, L19-5). Section « Socle commun des factions » non touchee (le fichier etait relu juste avant l'ecriture ; diff = 5 lignes ajoutees, 0 retirees).
- (C) `Tools\AutoJourney\brief_commun.md` : pieges 5 a 9 ajoutes a « Pieges Angelscript » (Cast reserve, evenement C++ protege, WorldContext des wrappers, delegue Slate `const FText&in`, struct de namespace a qualifier) + ligne « Avant le rapport (L19-2) : DIAGNOSTICS_AS » ; regle des chemins absolus (L19-3) ajoutee a la fin du point 5. LF et absence de BOM gardes (le fichier n'avait ni BOM ni CRLF).
- (D) `Auto_Journey_Jour.md` : `### Lecons appliquees 2026-11-20` (titre du gate, inchange) apres la ligne « Cloture incomplete » du bilan du 2026-11-20 ; tableau `| Regle | Etat | Preuve |` : L19-1 `non appliquee`, L19-2 a L19-9 `a appliquer`.
- Commit 1b299a5 (anglais, sans Co-Authored-By, fichiers par nom, pas de push).

RESTE: ecrire les outils des lignes `[ ]` (integrate.ps1, controle du gate L19-3, champs DIAGNOSTICS_AS / TEST_ROUGE_SEPARE dans `agent_rules.md`, modele de `.look`). `EtapeDefini.md` non touche (aucune avancee de chantier, seulement des lecons enregistrees) ; `Placeholders.txt` non concerne.

NON VERIFIE:
- Les pieges 5 a 9 sont ecrits d'apres l'enonce (une ligne chacun) ; le piege 1 (`Cast`) dit seulement « ne pas l'employer comme nom », la cause exacte n'a pas ete reproduite. Le gate `session_gate.ps1 -Phase Close` n'a pas ete relance (le titre y est present, exactement `### Lecons appliquees 2026-11-20`).
- Le brief_commun.md et agent_rules.md n'ont pas encore de champ `DIAGNOSTICS_AS` dans la liste des 13 champs (a faire, ligne `[ ]` L19-2).

PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune. Un avertissement git « LF will be replaced by CRLF » pour `brief_commun.md` (fichier en LF dans l'arbre, comportement deja existant).
GIT_STATUS_PRINCIPALE: apres commit, seuls restent les 4 fichiers non suivis de DOCS-1120s / autres sessions (`Docs\Concept_Faction_Theatre_2026-11-18.md`, `Docs\HSR_Aha_Allegresse_Nihilux.md`, `Docs\Priorite_Socle_Factions_2026-11-16.md`, `Docs\Rapports\DOCS-1120s.md`) : inchange hors docs voulus : oui
