# [À RÉPONDRE] Q-specialization-ui : où est le bouton de spécialisation (SP3) ?

Posée par ELROND-1115h le 2026-11-15 (chantier `SpecializationInProgress.md`, SP3, branche `tiers`).

**Contexte.** D332 dit « un bouton s'allume dans le HUD (emplacement de faction, D201) » et D684 « une barre à part montre le compte à
rebours », sans dire où exactement. Codé en attendant : deux cases dans la grille du Forum principal sélectionné.

**Q1 : où le joueur choisit-il sa spécialisation ?**

- **A (défaut codé, le plus simple)** : deux cases de la grille du Forum, **D** (Voie A) et **F** (Voie B), rangée du milieu, libre sur un
  Forum. Pendant le déblocage, la case de l'option en cours affiche « En cours : N s / Annuler » (un clic annule sans coût), l'autre case
  reste cliquable (elle remplace, réponse B de Q-specialization-switch). Conséquence visible : il faut sélectionner le Forum pour voir le
  choix ; aucune barre ailleurs dans le HUD.
- **B** : un bouton permanent à l'emplacement de faction (D201, près du portrait du héros) qui s'allume quand un palier est ouvert et ouvre
  un petit panneau des deux options ; le compte à rebours en barre sous ce bouton. Conséquence : visible sans sélectionner le Forum, plus de
  travail d'interface (un panneau neuf).
- **C** : les cases **Q** et **S** de la grille du Forum (à gauche de la rangée du milieu) au lieu de D F. Conséquence : même chose que A,
  autres touches.
- **Autre** : à préciser.

**Recommandation : B** (c'est la lettre de D332 et de D684 : bouton dans le HUD de faction + barre à part), en gardant A comme raccourci.
Réglage : `hol.UI.SpecializationCell` (case de la Voie A, la Voie B juste après ; 6 = D, 4 = Q).

Réponse :
