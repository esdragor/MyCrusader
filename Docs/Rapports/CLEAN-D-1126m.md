# CLEAN-D-1126m : nettoyage 0a de Auto_Journey.md et Auto_Journey_Checklist.md (2026-11-26)

APPELS_DECLARES : ~19. PREMIER_BUILD : sans C++. BUILD : sans objet. Aucun code touché, aucun test lancé.
COMMIT : f3721d5 (branche main, copie principale) ; 4 fichiers par nom : `Auto_Journey.md`, `Auto_Journey_Checklist.md`, `Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-26.md`, `EtapeDefini.md` (une ligne). Ce rapport et `Retours\Q-clean-d-1126m.md` restent non commités (la principale les commite). Message anglais, ligne finale `Agent: CLEAN-D`, pas de push.

## Archive
`Docs\Archive\Auto_Journey_Procedure_Checklist_nettoyage_2026-11-26.md` : copie intégrale des deux fichiers avant nettoyage (486 lignes), ajoutée à la ligne « Archives » du Sommaire de `Auto_Journey.md`. Un paragraphe « Nettoyage du 2026-11-26 » est ajouté à la check-list (même forme que les précédents).

## Lignes de check-list retirées (vérifiées par grep)
- L28-4 : devenue un contrôle de `session_gate.ps1 -Phase Report` (`:313-319`, rapport `Docs\Rapports\<AGENT>.md` et ligne d'EtapeDefini absents = FAIL).
- L28-8 : devenue un contrôle de `session_gate.ps1 -Phase Launch` (`:85-92`, `-MaxWorktrees` 8, FAIL au-delà).
- L27-10 : outil fait (`launch_agent.ps1` existe, cité par la procédure l.45) ; le relevé de `/context` à mi-séance et à la clôture est la RÈGLE IMPÉRATIVE (a) de `Auto_Journey.md`.
- L15-6 : outil fait (`build_wait.ps1` existe, `Auto_Journey_File.md` l.244 « FAIT » ; `build.ps1 -Wait`) ; attente par notification et jamais `Wait-Process` : `brief_commun.md` points 3 et 3d ; sync de fin : procédure étape 8 + `Close`.
- Puce « Un script `launch_agent` » de « Outils encore à construire » (fait).
Le gate ne lit pas les IDs de la check-list (seulement `### Lecons appliquees <date>`) : aucun effet sur `-Phase Close`.

## Procédure : 4 redites retirées (aucune règle changée)
- Ligne « Réponse finale de l'agent (3 lignes au plus...) » : redite de `brief_commun.md` point 11 (la formule « rapport de 13 champs ... `brief_commun.md` » reste dans « Modèle d'énoncé »).
- « Modèles et limites » : « arrêt dur = heure de lancement + 30 min ; il enchaîne 2 tâches de sa file » (dit par `brief_commun.md` paragraphe 1) ; le plafond 300k / 30 min / 45 appels et la consigne datée restent.
- « Contrôles mécaniques » : « (avant chaque lancement, avant la fusion, avant de donner une commande `play.ps1`, avant le bilan) » remplacé par « (quand : « Au démarrage », étape 0) » (le quand est écrit à l'étape 0) ; « rapport à 13 champs vérifié à la réception » (en-tête d'`agent_rules.md`).
- « Séance de migration », règles qui restent : « agent de 30 min / 300k tokens au plus depuis le 2026-11-25 » (dans « Modèles et limites »).

## DOUTES (lignes gardées, aussi dans `Retours\Q-clean-d-1126m.md`)
- L28-3 : Sanity retirée / TESTS 0/0 sont dans le gate Report (`:299-311`), mais « un vert se lit dans le même appel que sa commande » n'est contrôlé nulle part ; gardée.
- L28-1 : le gate Launch compte builds, quick_check et preflight (`:173-193`), mais « un seul build C++ à la fois dès que BUILD LENT » n'est pas dans le gate (il FAIL à plus de 2 builds si le dernier dure plus de 5 min) ; gardée.
- L29-5 : le gate Essay refuse une commande abrégée (`:379-383`) mais ne valide pas la commande exacte ; gardée. L29-6 : `sync_worktrees.ps1` utilise `ForeignSession.ps1` (commit récent absent du tableau), pas le « pris » d'`IN_USE.txt` d'une autre session ; gardée.
- L27-2 : `integrate.ps1` refuse-t-il un vert dont le tip est « test seul » / bouchon ? le grep ne trouve que le cherry-pick des rouges ; gardée.
- L28-5, L29-7 et L20-8 disent la même règle (« premier Edit avant le 8e appel », fichier:ligne donné par l'énoncé) ; redite entre trois lignes, pas avec le brief ni le gate ; gardées, à fusionner par un humain. L19-4 / L22-3 : idem (reporté du 2026-11-25).
- L15-9 : la moitié « UN test rouge + UN correctif » est dans `brief_commun.md` point 2, l'autre (plan de plus de 25 appels de lecture coupé en deux lignes de file) n'y est pas ; gardée.
- L25-4 (« la principale lit `BuildQueue.csv` avant tout arrêt ») et L9-1 (TaskStop à la 2e notification) : tension non levée (reporté du 2026-11-25).
- Sommaire l.9 et « Modèles et limites » l.45 répètent la consigne du 2026-11-25 ; veilleur : cadence répétée aux l.106, 108, 157, 167 ; redites internes à la procédure, gardées.
- L.78 de `Auto_Journey.md` (chemins `\` mangés dans Bash) : consignes dans `brief_commun.md` point 9, raison absente ; gardée (reporté).

## NON VÉRIFIÉ
- `gate_test.ps1` / `session_gate.ps1` non relancés (docs seulement).

PROCESSUS_DETACHES : aucun. ANOMALIES_PROCESSUS : aucune. GIT_STATUS_PRINCIPALE : seuls les fichiers ci-dessus commités ; le reste (`Auto_Journey_Jour.md`, `Docs\SessionEnd.txt`, `Docs\Rapports\`...) non touché par cet agent.
