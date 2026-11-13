# [À RÉPONDRE] Spécialisation : changer d'option pendant le déblocage

Chantier : spécialisation (`SpecializationInProgress.md`, worktree Haldir, branche `maps`). 1 question. Rien n'est bloqué : le défaut
est le comportement actuel.

## Q1. Le joueur a lancé une option (compte à rebours en cours) et clique sur l'autre option du même palier : que se passe-t-il ?

Contexte : D684 permet d'annuler sans coût et de rouvrir le choix tant que rien n'est appliqué ; la règle codée exige aujourd'hui
d'annuler d'abord (le clic sur l'autre option est refusé).

- **A. Annuler d'abord (actuel)** : un clic sur l'autre option ne fait rien tant que le premier déblocage court ; il faut cliquer
  Annuler puis l'autre option (2 clics), le délai repart de zéro. En jeu : aucun changement par erreur ; un clic « mort » possible.
- **B. Le clic sur l'autre option remplace directement** : l'option en cours est annulée et l'autre se lance, délai entier (1 clic).
  En jeu : plus rapide ; un mauvais clic coûte le temps déjà écoulé.
- **C. Remplacer en gardant le temps écoulé** : l'autre option reprend le compte à rebours là où il en était. En jeu : changer d'avis
  ne coûte rien ; on peut lancer « à blanc » puis choisir au dernier moment.
- **Autre.**

**Recommandation : B** (un clic, et le coût du changement reste le temps perdu, comme une annulation).
Défaut codé : A (comportement actuel de `HOLSpecialization::Start`, refus tant que `IsRunning`) ; passer à B = une ligne dans `Start`.

Reponse: B (le clic sur l'autre option remplace directement : l'option en cours est annulée, l'autre se lance avec le délai entier). (réponse donnée dans la conversation, 2026-11-13 ~17:10)
