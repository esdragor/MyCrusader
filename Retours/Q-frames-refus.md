# [À RÉPONDRE] check.ps1 doit-il refuser les `frames` > 30 dès maintenant ?

Contexte : L14-8 demande que `check.ps1` refuse un `frames N` > 30 qui attend un événement du jeu (fenêtre cachée ~150 images/s). Le nouvel outil (`Tools\Scenarios\FrameWaits.ps1`, branche fog) en trouve **206 dans 116 scénarios** existants (hors encadrement d'une prise de caméra), et 2 `waitlog` juste après un `wait` > 10 s.

- **A** : refuser tout de suite (`-FrameWaits Fail` par défaut) : 116 scénarios passent FAIL au `check`/`preflight` jusqu'à leur réécriture en `wait`/`waitlog`.
- **B** (recommandé) : signaler en WARN par défaut (codé), refuser seulement dans les scénarios neufs ou réécrits (`HOL_CHECK_FRAMES=fail` ou `-FrameWaits Fail`) ; un chantier « sans C++ » réécrit les 116 puis on bascule le défaut sur Fail (une ligne de `check.ps1`).
- **C** : refuser seulement au-delà d'un seuil plus haut (ex. 300) dès maintenant.
- **Autre** : ...

Défaut codé : **B** (`check.ps1`, paramètre `-FrameWaits`, défaut `Warn`).
