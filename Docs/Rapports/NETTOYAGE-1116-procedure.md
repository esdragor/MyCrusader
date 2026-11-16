# NETTOYAGE-1116-procedure (étape 0a, bloc Auto_Journey.md + Auto_Journey_Checklist.md)

Archive : `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-16.md` (texte tel quel + preuve de chaque contrôle).

## Check-list : 13 lignes archivées + 1 ligne raccourcie
Paragraphe « Nettoyage du 2026-11-16 » ajouté (« Devenues des contrôles (2026-11-16) : ... ») :
- L1-12(c) (check.ps1, Sanity FAIL) : ligne « Outils encore à construire » raccourcie, (a)(b) restent.
- L8-2 (HOLSanityRules.h check 5), L10-4 (AGENT 6), L12-2 (gate L87), L12-3 (build.ps1 -MaxWaitMin ; reste à l'énoncé), L12-7 (BUILD LENT ; reste PRINCIPALE), L13-2 (launch_check_test.ps1 + AGENT 10), L14-3 (ForeignSession, status -InMain), L14-7 (build.ps1), L14-8 (FrameWaits), L14-12 (commit_docs -Only ; reste SESSION « docs gelées »), L14-13 (extend_session.ps1), L15-1 (Close ; reste étape 8), L15-2 (edit_line.ps1 ; reste à l'énoncé).

## Procédure : redites remplacées par un renvoi
- « Comment écrire l'énoncé » : ligne « Chaque agent met à jour... » -> renvoi `agent_rules.md` règles 4, 5, 10, 14.
- « Pièges techniques » Test 2 : quick_check ne compile pas / IN_USE sans accents / heredocs -> renvoi règles 6 et 8 (le reste de la ligne garde).
- « Builds » : calcul du plafond C++ (L13-2) -> renvoi règle 10.
- Étape 8 : « contrôle DLL à ajouter au gate » était périmé (gate fait, en-tête de session_gate.ps1 ligne 7) -> « fait au gate ».
- Ajout L15-10 (brief_commun.md + Docs\Rapports\<AGENT>.md) dans « Comment écrire l'énoncé ».
- Sommaire : lien vers la nouvelle archive.

## Doutes (lignes laissées en place)
1. L15-5 : `agent_rules.md` règle 7 dit encore « Commit du test AVANT l'implémentation (le SHA va au rapport) » et le champ TEST_ROUGE demande le SHA du commit du test : contraire à L15-5 (rouge prouvé à l'exécution, un seul commit). Règle non appliquée, à trancher.
2. L14-1 (edit_line fait, mais « Read groupé au premier appel » non écrit dans agent_rules), L13-3 (preflight_merge ne signale pas les fichiers modifiés des deux côtés : aucune trace dans le script), L10-3 (gate Report existe, consigne de l'agent de fusion non écrite) : partiellement faites, laissées.
3. L11-4 (Close « rapport fait sans ligne cochée ») : aucune trace dans session_gate.ps1, laissée.
4. L13-5 (dossiers non suivis > 50 fichiers), L14-2 (plafond C++ sur vm libre), L14-5/L14-6 (Essay GESTE, Report « suppose »), L14-10 (« candidats absents » dans play.ps1), L15-3/4/6 (replace_text.ps1, agents_table.ps1, build_wait.ps1 absents) : pas faits, laissés.
5. `Tools\AutoJourney\brief_commun.md` n'existait pas au moment du relevé (ls) ; la ligne L15-10 de la procédure le cite comme demandé.
6. « Qualité des essais » 1-15 recoupe largement `agent_rules.md` règle 18 mais en est la source citée (règle 18 renvoie aux règles 13-15) : non touché.
7. Q-close-gate-l15-1.md (2 cas à trancher sur le gate L15-1) : la question vit dans Retours\, non vérifiée ici.
