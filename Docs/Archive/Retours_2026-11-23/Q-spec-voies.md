# [À RÉPONDRE] Q-spec-voies : premier effet des voies de commanderie des Templiers

Contexte : choisir Fer / Foi / Pierre (cases 5, 6, 7 du Forum, D868) ne change rien en partie : `ChosenOption` n'est lu que par le tableau. Le GDD (D133, D683) veut : unité de commanderie débloquée dans son bâtiment ET contingent de croisade ; il ne dit pas par quoi commencer. Relevé : `Docs\Rapports\SPEC-VOIES-1123.md`.

Options :
- A : d'abord le déblocage de l'unité seul (l'unité du palier devient recrutable dans son bâtiment quand sa voie est choisie ; pas de croisade). Visible en jeu : le bouton de l'unité apparaît après le choix.
- B : unité + contingent de croisade ensemble (effet complet D133, un gros lot, dépend de la croisade codée).
- C : d'abord le palier 1 des trois voies seulement (3 unités), le reste ensuite.
- Autre.

Recommandation : A (petit, testable par script, ne dépend pas de la croisade ; le contingent suit).
Valeur par défaut codée : A, derrière un réglage (une ligne dans la table de disponibilité des unités de commanderie, vide tant que rien n'est branché).

Constat GALADRIEL-1123b (2026-11-23 09:14) : NON CODÉ. Aucune des 9 unités de commanderie (Sergent du Temple ... Beffroi) n'existe dans `Tools\Data\UnitTypes.csv` (18 unités, aucune Templiers ; faction Dawn : Peasant, Monk, Arquebusier, Halberdier, VirtuousKnight) : A et C exigent d'abord de créer ces unités (données, mesh, coût), ce que la tâche interdit ; `hol.Spec.UnlockUnit` non créé. À trancher : créer les unités du palier 1 d'abord (C), ou autre.
