# [À RÉPONDRE] Suivre : les unités du joueur comptent-elles ? (Q-follow-own-units)

Contexte : D818 dit « clic droit sur un allié = Suivre » sans dire si les unités À SOI comptent. Rejoué (follow_crowd, 20 unités serrées, 1122f) : avec le code de 6494b40, un clic droit « au sol » dans la masse tombe sur le corps d'un Paysan à soi non sélectionné = « FollowBridge: follow started » au lieu d'un déplacement ; même un clic visé sur un Homme d'armes sélectionné a suivi le Paysan #13 qui l'avait remplacé sous le curseur ; le clic au sol à côté de la masse = déplacement normal.

- A : les unités à soi suivent (comportement de 6494b40). Conséquence : un point de ralliement ou un déplacement visé au milieu d'un groupe à soi devient un Suivre (les unités se collent au Paysan / à l'unité, silencieusement).
- B : les unités à soi sont exclues de Suivre (seuls les coéquipiers sont suivis) ; clic sur un corps à soi = déplacement. Conséquence : plus de piège dans la masse ; on ne peut plus faire suivre un de ses propres Paysans / escorter un Héros par clic droit.
- C : Suivre seulement avec une touche (Maj/Alt + clic droit) sur une unité à soi ; clic droit seul = déplacement. Conséquence : aucun piège, escorte possible, un geste de plus à apprendre.
- Autre : ...

Recommandation : B si D818 visait les coéquipiers, C sinon (escorte d'un Héros voulue).

**Défaut codé (GLORFINDEL-1122g, branche ability-columns, e3ee5ad) = B**, derrière le réglage `hol.Follow.OwnUnits` :
- `0` (défaut) : une unité du joueur local n'est jamais une cible de Suivre (le clic droit y reste un déplacement, comme dans main) ; les unités d'un COÉQUIPIER restent des cibles.
- `1` : comportement de 6494b40 (A) : toute unité à soi hors sélection sous le curseur peut être suivie.
- Le réglage ne touche PAS Soigner : une unité à soi ou alliée, même hors sélection, reste soignable (Q-chapelle-moine = A). Ni l'attaque, ni FindTargetAt, ni le clic au sol.

Valeur à changer : la valeur par défaut `0` du cvar `hol.Follow.OwnUnits` (`CVarFollowOwnUnits`, `Source\HeroesOfLiberty\Player\HOLPlayerController.cpp`, espace de noms `UE::HeroesOfLiberty::Player`, juste avant `LoadAction`) : `1` pour A. En jeu : `hol.Follow.OwnUnits 1` dans la console. L'option C demande du code neuf (touche modificatrice) : à ouvrir si choisie.

Note de comportement (par construction, comme la sélection déjà écartée) : avec 0, une unité à soi dessinée DEVANT un coéquipier est sautée, le coéquipier derrière est pris ; sur le seul corps d'une unité à soi, rien n'est pris (déplacement).

Scénarios : `follow_short` pose `hol.Follow.OwnUnits 1` (un seul joueur : son Paysan est à soi, pas de coéquipier) pour prouver le pont de Suivre ; `follow_crowd` pose 0 et vérifie le déplacement (journal « Order: 10 units to », jamais « follow started »). Un scénario à 2 joueurs ciblant un coéquipier reste à écrire.
