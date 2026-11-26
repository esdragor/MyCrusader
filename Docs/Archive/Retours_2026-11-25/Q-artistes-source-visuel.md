# Artistes : où vit le visuel d'un type (maillage, animations, icône) ? (CLAUDE-1125ar, AQ-1)

Contexte : des artistes sans Claude vont remplacer les placeholders depuis l'éditeur (chantier `ArtistesInProgress.md`).
Aujourd'hui le visuel est dans les CSV, sur la même ligne que les règles : `Tools\Data\UnitTypes.csv` (`Mesh`,
`MeshYawOffset`, `MeshScale`, `SkinnedMesh`, `AnimationProvider`, les `*Sequence(s)`, `*PlayRate`, `*Prop(s)`,
`Projectile`, `MuzzleSmoke`), `BuildingTypes.csv` (`Mesh`, `Shape`), `SourceTypes.csv` (`Mesh`). Les DataTables sont
régénérées depuis ces CSV (`Tools\Editor\make_unit_types.py` et voisins, `regen_tables.ps1`), qui remplacent toutes
les lignes : un maillage changé par un artiste dans l'éditeur est écrasé au prochain regen d'un agent.

Options :
- A : **une table visuelle séparée, propre aux artistes** (par exemple un DataAsset par type sous `/Game/Art/`,
  modifié seulement dans l'éditeur, jamais régénéré). Le jeu la lit d'abord, puis retombe sur les colonnes du CSV
  tant qu'elle est vide (transition sans casse) ; les colonnes visuelles du CSV sont ensuite gelées puis retirées.
  En jeu : rien ne change tant qu'un artiste n'a rien posé ; ensuite son modèle s'affiche et aucun agent ne peut
  l'écraser. Coût : un lot de lecture (C++ si la lecture est au rendu Mass, liaison exposée au script sinon),
  `check_placeholders.ps1` lit aussi cette table, les tests des tables suivent.
- B : **le panneau de l'éditeur réécrit le CSV** (script Python de l'éditeur) puis régénère la DataTable. En jeu :
  même résultat. Coût plus faible, mais artistes et agents modifient les mêmes lignes de CSV (une ligne = règles +
  visuel d'un type) : conflits de fusion dès qu'un agent règle un coût pendant qu'un artiste change le maillage du
  même type, et l'artiste dépend des scripts de regen.
- Autre : à préciser.

Recommandation : A (sépare les règles et le visuel : chacun son fichier, pas de conflit avec les agents ; le repli sur
le CSV permet de migrer type par type).
Valeur par défaut codée : AUCUNE (rien n'est codé, la question bloque les lots AR1 à AR6 du chantier).

Reponse: A, table visuelle séparée (2026-11-25 18:5x, GDD D924)
