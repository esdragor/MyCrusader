# [À RÉPONDRE] Artistes : passer le visuel dans les DA_Art d'un coup ou type par type ? (CLAUDE-1125ar, AQ-2)

Contexte : D924 met le visuel dans une table propre aux artistes (un `DA_Art_<Type>` par type sous `/Game/Art`), lue
avant les colonnes visuelles du CSV, avec repli sur le CSV quand elle est vide. Conception : `ArtistesInProgress.md`,
section « Conception AR0 ». Reste à dire quand les colonnes visuelles du CSV (`UnitTypes.csv` de `Mesh` à `FallSequence`,
`Projectile`, `MuzzleSmoke` ; `Mesh` de `BuildingTypes.csv` et `SourceTypes.csv`) cessent d'être une source.

Options :
- A : **d'un coup**. Dès que le lecteur des `DA_Art` est dans `main`, une commande d'éditeur crée un `DA_Art` pour
  chaque type à partir du CSV actuel (mannequins, `TP_*`, flèches compris), puis les colonnes visuelles sont retirées
  du CSV et des scripts de regen. En jeu : rien ne change. Ensuite le visuel n'a qu'une source (règle de `CLAUDE.md`),
  et un agent qui fabrique un modèle par script (comme l'Arquebusier) écrit dans le `DA_Art` par Python d'éditeur.
- B : **type par type**. Le CSV reste la source tant qu'un artiste n'a pas posé le `DA_Art` d'un type ; ses colonnes
  sont vidées pour ce type à ce moment, et retirées quand tous les types ont leur asset. En jeu : rien ne change non
  plus. Moins de travail au départ, mais deux sources possibles pour le visuel pendant des semaines (savoir laquelle
  gagne demande de regarder les deux).
- Autre : à préciser.

Recommandation : A (une seule source tout de suite ; le panneau des artistes voit tous les types de la même façon ;
le repli sur le CSV reste en filet de sécurité jusqu'au retrait des colonnes).
Valeur par défaut codée : AUCUNE (ne bloque pas le premier lot, AR0-a : le lecteur et son repli servent aux deux options).

Reponse:
