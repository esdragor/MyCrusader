# [À ESSAYER] E-<nom> : <une phrase : ce que l'utilisateur doit voir ou faire>

STATUT : VU À L'ÉCRAN | LOGIQUE SEULEMENT   (LOGIQUE SEULEMENT n'est jamais listé prêt)
GESTE: réel | interne      (réel = vraie touche puis vrai clic, modèle `Tools\Scenarios\neutrals_qground.txt` ; interne = commande `hol.*` ou appel de code)
SON: entendu | non entendu | sans objet   (non entendu = lancé avec -nosound ou rien entendu par l'agent)
branches: <a, b>      (les branches dont le correctif est joué ; `session_gate.ps1 -Phase Essay -Root <worktree> -Essay <nom>` vérifie que le worktree de la commande contient le tip de chacune, FAIL sinon ; `aucune` si seul main compte)

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.
`session_gate.ps1 -Phase Close` signale (WARN) tout E-*.md `[À ESSAYER]` sans les lignes `GESTE:` et `SON:`.

Pré-vol (<date heure>, agent <nom>, worktree <chemin>, main <sha>) : `preflight.ps1 <nom> -Root <worktree>` PASS (<durée>, <n> PNG).
PNG OUVERTS et vus :
- <nom_png> : <ce qui est vu>
NON VÉRIFIÉ : <obligatoire>

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\play.ps1 <nom> -Root <worktree>`
Durée : environ <n> min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
