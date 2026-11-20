# [À RÉPONDRE] Où va la 3e case de spécialisation des Templiers sur la grille du Forum ?

Contexte (ELROND-1120e, morceau 7 du socle) : les Templiers choisissent entre 3 commanderies (D131). Les cases de spécialisation du Forum sont D puis F
(cases 6 et 7, cvar `hol.UI.SpecializationCell`, défaut 6, ligne du milieu) ; la 3e case suit donc d'office la case 8, qui est aussi la PREMIÈRE case de
recherche du Forum (`ResearchFirstCell = 8`, HOLCommandGridRules.as). Tant qu'un palier est ouvert, la spécialisation passe avant la recherche : la 3e case
MASQUE la première ligne de technologies d'un Forum Templier.

Options :
- A (code actuel, aucune valeur choisie) : cases 6, 7, 8 contiguës ; la 3e voie cache la première recherche du Forum tant qu'un palier est ouvert (chez les Templiers seulement).
- B : les trois cases en 5, 6, 7 pour les Templiers (une case plus à gauche) ; rien de masqué, mais la case 5 (V sur les ordres) change de place selon la faction.
- C : la 3e voie sur la ligne du haut (case 3 ou 4), séparée des deux autres ; rien de masqué, mais les trois voies ne sont plus côte à côte.
- Autre : à dire.

Recommandation : B (aucune recherche masquée, les trois voies restent groupées).
Valeur par défaut codée : A (le plus petit changement : `GetSpecializationCell(OptionCount)` ; passer à B = une ligne dans `GetSpecializationCell`, C++, avec la cvar).
Aussi à dire : le libellé de la 3e voie (`GetSpecializationName`, C++, ne connaît que « Voie A / B du palier N » ; la 3e donnerait « Voie B » : lot C++ à écrire).
