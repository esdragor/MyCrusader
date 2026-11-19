# [À RÉPONDRE] Forge : 8 technologies pour 4 cases, les deux armures n'ont pas de case

Posée le 2026-11-19 18:2x par la session principale (reprise 16 de la migration). Trouvé par un test neuf, pas par un essai :
**LOGIQUE SEULEMENT, pas encore vu à l'écran.**

## Le constat

La Forge a 4 lignes de technologies, chacune en deux niveaux (table `Tools\Data\Technologies.csv`) :

| Ligne | Niveau I (palier 1) | Niveau II (palier 2) |
|---|---|---|
| Attaque de mêlée | Lames affûtées | Lames affûtées II |
| Attaque à distance | Empennage | Empennage II |
| Armure d'infanterie | Maille | Maille II |
| Armure de cavalerie | Barde | Barde II |

La grille de commandes n'a que 4 cases de recherche (W X C V) et y met les technologies du bâtiment dans l'ordre de la table,
une case par technologie. Elle montre donc : W = Lames affûtées, X = Lames affûtées II, C = Empennage, V = Empennage II.
**Maille, Maille II, Barde et Barde II n'ont aucune case : on ne peut pas les rechercher.**

Ce n'est pas la migration : la règle était la même dans l'ancien C++ (`Script\Player\HOLCommandGridRules.as:177-181`, recopiée
telle quelle). C'est le balayage de toute la table écrit aujourd'hui (`Test_CGSWResearchRowFits`) qui l'a vu. Ce test reste rouge
exprès (rouge connu, `Tools\Tests\known_reds.txt`) : il servira de test rouge à la correction.

## La question : comment la Forge montre-t-elle ses 4 lignes à 2 niveaux (3 au palier 3, GDD § 11.2) ?

- **A (recommandée) : une case par LIGNE.** W = mêlée, X = distance, C = armure d'infanterie, V = armure de cavalerie. Chaque case
  montre le prochain niveau à rechercher (I, puis II quand I est fait, puis III au palier 3) ; ligne finie = case marquée faite.
  En jeu : les 4 cases du bas suffisent pour toujours, même avec le niveau III ; on ne voit qu'un niveau à la fois (le coût et
  l'effet du suivant sont dans l'infobulle). C'est la lecture du GDD (« Forge × 4 lignes ») et la convention d'AoE4.
- **B : une case par TECHNOLOGIE, sur deux rangées.** La Forge ne forme pas d'unité : ses deux rangées du haut sont libres.
  Rangée du milieu = niveaux I, rangée du bas = niveaux II (grisés tant que le I n'est pas fait ou que le palier manque).
  En jeu : tout est visible d'un coup, mais 8 cases dès maintenant et pas de place pour le niveau III (12 technologies) sans
  troisième rangée ; les touches ne sont plus W X C V seules.
- **C : laisser tel quel pour l'instant.** Les deux armures restent impossibles à rechercher jusqu'au chantier Technologies
  (`TechnologiesInProgress.md`). Rien à coder maintenant, le test reste rouge connu.

Valeur codée en attendant : **C de fait** (rien n'est changé, le défaut reste). La correction A ou B est du script seul (règle
de la grille + son test), sans C++ ; elle attendra dans la file après ta réponse.

Reponse:
