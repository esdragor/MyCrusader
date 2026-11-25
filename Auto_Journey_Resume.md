# Auto_Journey : résumé pour la session principale (40 lignes au plus ; la procédure complète est `Auto_Journey.md`, lue par les agents, pas par la principale)

Créé le 2026-11-25 (DOCS-1125b, demande de l'utilisateur : réduire le budget de la principale). Les renvois « § » sont des titres de `Auto_Journey.md`.

## Premier geste (§ Au démarrage)
- Poser par `AskUserQuestion` la **fin de séance** et le **mode** (autonome / interactif), jamais choisis seule (L25-6) ; fin dans `Docs\SessionEnd.txt`, ligne `fin et mode demandés à <HH:MM>` dans `## Agents du <date>` de `Auto_Journey_Jour.md`. Seules questions permises en autonome (L21-1, L23-1).
- Ne lis NI `Auto_Journey_Checklist.md` NI le Jour NI Retours en entier : l'agent de l'étape 0 les lit et rend 30 lignes. La preuve au Jour est `check-list lue par ETAPE0 (L<N>)` (gate L19-1).

## Étapes (§ Au démarrage, numéros d'origine)
- **0a** nettoyage des docs : 4 agents en parallèle archivent dans `Docs\Archive\` (jamais de suppression) ; aucun agent de travail avant leur commit.
- **0** `sync_worktrees.ps1` (aucun agent ne tourne), puis `session_gate.ps1 -Phase Launch -End <heure>` avant CHAQUE lancement (FAIL = pas de lancement) ; tableau `## Agents du <date>` ; alarme fin - 15 min ; `-Phase Essay` avant un `play.ps1` donné ; `-Phase Merge` avant la fusion ; `-Phase Close` avant le bilan.
- **1** lire ce résumé (pas `Auto_Journey.md`) ; créer le veilleur (30 min autonome, 5 min interactif, § Le veilleur).
- **2** lancer l'étape 0 (sous-agent lecture seule : `status.ps1`, plan de lancement d'une ligne par worktree, § Plan de lancement).
- **3** lire les `Reponse:` de `Auto_Journey_Retours.md` ; elles deviennent des lignes `[ ]` en tête de file.
- **4** lancer d'un coup le plan (3 agents C++ au plus en même temps ; `Tools\AutoJourney\launch_agent.ps1` écrit l'énoncé dans `Docs\Rapports\ENONCE-<AGENT>.txt`, l'appel Agent = « Lis ce fichier et exécute-le »).
- **5** à chaque fin d'agent : rapport lu (3 lignes), docs commitées par nom de fichier, agent suivant = première `[ ]` de la file du worktree.
- **6** file vide : ligne suivante d'`EtapeDefini.md` « À faire » sans essai ni décision, sinon chapitre du GDD non commencé ; jamais de nouveau worktree.
- **7** avant de donner des essais : fusionner, rejouer le pré-vol sur `main`, recette indépendante, ~6 essais de 3 min au plus (§ Qualité des essais).
- **8** fin : `sync_worktrees.ps1` SANS `-NoBuild`, `session_gate.ps1 -Phase Close`, verdict « nouvelle version » (L27-12), bilan dans le Jour, Retours à jour.

## RÈGLE IMPÉRATIVE de budget (utilisateur, 2026-11-23, § Règles de conduite)
- 300k tokens de contexte au plus (`/context` à mi-séance et à la clôture, écrit au bilan) ; à 250k plus aucun chantier neuf.
- Énoncé d'agent = 8 lignes (tâche, worktree, heure d'arrêt, fichier d'énoncé) ; énoncés détaillés écrits par un agent préparateur dans `Docs\Rapports\`.
- Pas de tableau des agents tenu à la main (agent de docs, `agents_table.ps1`) ; un message d'UNE ligne à l'utilisateur (avance de main, essai prêt, question, blocage) ; toute sortie d'outil lue est filtrée à 5 lignes.
- Build + tests de la principale : UNE commande, `Tools\AutoJourney\after_commit.ps1` (TOOLS-1125a, en cours d'écriture).

## Fusion et limites
- **D888** (2026-11-22) : fusion locale dans `main` à chaque passe verte (build PASS, tests verts sur le résultat, `-Phase Merge` sans FAIL), séances autonomes comprises, JAMAIS de push (§ Pièges techniques).
- **Plafond C++** : 3 agents en même temps (`agent_rules.md` règle 10) ; **L25-5** (proposée) : 2 seulement quand un build dépasse 5 min, la principale lance le build du lot dès le commit de l'agent.
- Sous-agents : Sonnet 5.5, **30 min / 45 appels au plus**, 2 tâches de leur file à la suite (consigne du 2026-11-25 17:4x) ; réponse finale de 3 lignes, détail dans `Docs\Rapports\<AGENT>.md` (`brief_commun.md` points 1 et 11).

## Règles 14 et L21-1 (§ Qualité des essais, règle 14)
- Séance interactive : toute question passe par `AskUserQuestion`. Séance autonome : 0 `AskUserQuestion` pour une question de travail (elle va dans `Retours\Q-*.md` avec sa valeur par défaut) ; seules exceptions, fin et mode au lancement (L25-6).

## Clôture (§ Au démarrage point 8, § Règle de durée)
- Mieux vaut finir 15 min plus tard que plus tôt ; le gate Launch refuse sous 5 min ; arrêt dur des agents = fin annoncée.
- Fin : worktrees synchronisés et sans DLL périmée, `-Phase Close` sans FAIL, tableau « Leçons appliquées », docs de la copie principale commitées par nom de fichier (jamais `-A`), `Placeholders.txt` relu.
