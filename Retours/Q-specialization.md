# [À RÉPONDRE] Choix de spécialisation de palier : deux points laissés ouverts (HALDIR-1112f, 2026-11-12)

Chantier : `SpecializationInProgress.md` (worktree Haldir, branche `maps`). Règles pures en script, rien de visible en jeu pour l'instant.
Défauts codés (une ligne à changer chacun dans `Script/Specialization/HOLSpecializationRules.as`).

## Q1 : deux paliers à choisir en même temps

Contexte : le choix se fait « quand on veut » (D332) et prend 30 / 45 / 60 s (D684, une barre à part dans le HUD de faction).
Un joueur qui atteint le palier 2 sans avoir choisi le palier 1 a deux choix ouverts : le GDD ne dit pas s'ils peuvent courir ensemble.

- **A (recommandé, codé)** : **un déblocage à la fois**. Le bouton du palier 2 reste allumé, mais il faut attendre la fin (ou annuler)
  celui du palier 1. En jeu : une seule barre ; attendre pour choisir coûte un peu plus de temps (cohérent avec « attendre a de la valeur »).
- **B** : **les deux en parallèle**, une barre par palier. En jeu : deux barres dans le HUD ; rattraper un choix en retard ne coûte rien.
- **C** : **le palier 1 d'abord** : le bouton du palier 2 ne s'allume qu'une fois le palier 1 appliqué. En jeu : l'ordre est imposé.
- **Autre** : à préciser.

Réglage : `OneUnlockAtATime` (A = true, B = false ; C demanderait une ligne de plus).

## Q2 : le compte à rebours quand le Forum est à terre

Contexte : « le déblocage se lance au Forum » (D684), qui laisse au chantier le cas du Forum tombé (forum à vies, ruine puis
reconstruction).

- **A (recommandé, codé)** : **en pause** : la barre s'arrête et reprend où elle en était quand le Forum est relevé. En jeu : raser le
  Forum retarde la spécialisation adverse, sans la perdre.
- **B** : **le compte continue** : le Forum n'est que le lieu affiché. En jeu : raser le Forum ne change rien au choix.
- **C** : **annulé** : le choix se rouvre et il faudra relancer un délai entier. En jeu : punition plus forte, le joueur doit y penser.
- **Autre** : à préciser.

Réglage : `ForumDown` (`ForumDownPauses` = A, `ForumDownRuns` = B, `ForumDownCancels` = C).
