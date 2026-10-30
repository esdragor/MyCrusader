# Auto_Journey : ce qui attend l'utilisateur

Dernière mise à jour : 2026-10-30 18:20 (après le test 1).

**À faire par vous :** 3 essais prêts (`tab`, `surplus`, `tiers` à revérifier) · questions : Paysans de l'IA, 3 Cartes, 2 « A trancher » (D426, V-Q1) · 6 fusions à valider · 1 lot de docs à commiter.

**Manière de faire (confirmée le 2026-10-30)**
- Vous répondez **en ligne**, sous chaque item, par `Reponse: ...`. Les sous-agents y ajoutent leurs lignes (essai prêt, question posée), jamais ailleurs.
- Une réponse **n'enclenche pas de travail** : on l'enregistre (journaux, GDD, `EtapeDefini.md`), on repose des questions si elles émergent, et la correction devient une ligne `[ ]` en tête de `Auto_Journey_File.md`, faite au prochain « commence ton Auto_Journey ».
- **`Reponse : A trancher` = vous voulez en discuter plus en profondeur** : rien n'est noté comme décidé, la question reste ouverte.
- Une question est une **vraie question pour vous** : contexte en 2 lignes simples, options A/B/C avec leurs conséquences en jeu, recommandation, fichier. Jamais une remarque pour soi-même.
- **Ce fichier est léger et sans historique.** Un item dont vous avez répondu et dont la réponse est **reportée dans son chantier** (*InProgress.md, GDD, file de travail) est **supprimé d'ici** ; de même un essai fait et reporté. Ce qui reste = ce qui attend encore quelque chose de vous.
- Un essai est listé **seulement s'il est testable** : scénario créé et démarré hors écran sans erreur, commande `play.ps1` en chemin absolu (PowerShell), gestes précis. Sinon : « scénario à créer » dans la file.

## 1. Essais à faire

Format de chaque essai : titre · commande · quoi regarder · état · ligne `Reponse:`.

### Paliers (`tiers`) — revérification du correctif, branche `tiers` 1093e52
- Commande : `& "C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1" tiers -Root "C:\Users\baudo\Documents\HeroesOfLiberty-Elrond"`
- À regarder : panneaux d'âge des bâtisseurs et vies du forum qui suivent le niveau du héros (1, 3, 6) ; le palier ne baisse jamais ; **les médaillons d'âge I à IV ne sont plus coupés en haut** (correctif 1093e52 à revérifier).
- État : à revérifier seulement (correctif des médaillons coupés, 1093e52).
Reponse: Validé

### Interface (Tab) — branche `galadriel-tab` 97bbe4b, scénario `tab` d4fadd0 (prêt)
- Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 tab -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
- Départ : 9 unités déjà sélectionnées (3 Paysans, 3 Hommes d'armes, 3 Archers) ; arbre au sud-ouest, or au nord-est et pierre au sud-est (plus loin), 3 Lanciers et une maison ennemis à l'est, fondation de maison à poser au nord-ouest (**directions approximatives, non vérifiées**).
- Gestes : Tab pour passer sur « Hommes d'armes » → clic droit au sol (seuls eux marchent) → glisser le clic droit (ligne d'ordre : seuls eux) → clic droit sur les Lanciers ou la maison ennemie (eux seuls attaquent) ; Tab sur « Paysans » → clic droit sur l'arbre / l'or / la pierre (3 Paysans seuls), sur la fondation (ils bâtissent), pose d'une maison (menu du Paysan) ; Tab sur « Archers » puis « Tous » : mêmes clics, « Tous » = les 9.
- À regarder : après Tab, clic droit, ligne d'ordre et pose de bâtiment ne concernent que l'onglet actif ; « Tous » = toute la sélection.
Reponse: Tout est bon (le HUD est crop mais Elrond l'a corrigé)

### Héros (Hommes d'armes en surnombre) — branche `hero-j8-close` (correctif dd16203 + scénario `surplus`, prêt)
- Commande : `& "C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1" surplus -Root "C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn"`
- Gestes : le jeu s'ouvre, panneau en haut au centre, « Entrée : l'ennemi encerclé par 6 des tiens... ». Pour **comparer**, avant d'appuyer sur Entrée : console (²) `hol.Combat.SurplusReassign 1` (correctif, défaut). Entrée : un héros ennemi (niveau 4) apparaît à ~46 m de ton héros avec un Homme d'armes ennemi libre 9 m derrière lui ; 6 de tes Hommes d'armes l'encerclent ; 4 s plus tard 8 renforts arrivent par l'ouest et reçoivent un clic droit sur le héros ennemi. Regarde ~7 s. Puis console `hol.Combat.SurplusReassign 0`, `hol.Scenario surplus` (relance, bouton « Relancer » du panneau possible), Entrée, regarde pareil.
- À regarder : **avec 1**, les renforts qui n'ont plus de place autour du héros ennemi le quittent et frappent l'Homme d'armes libre derrière (mesuré hors écran : 4 des 8 sur lui après 7 s) ; **avec 0** (ancien comportement), tous restent plantés derrière l'anneau sur le héros, certains sans frapper. Ton propre héros est loin et ne combat pas. Limite connue : un Paysan (agressivité 0) ou l'absence d'autre ennemi à 16 m laisse l'attente.
- État : `quick_check -Parts hero` PASS (123 s), test `Hero.SurplusAttackers` vert, scénario lancé hors écran sans erreur (« prêt », 5 notes, mesures A/B concordantes) ; jamais lancé par l'utilisateur.
Reponse: c'est mieux mais je pense savoir ce qui va pas, si il y a 2 demi slot pour taper quelqu'un, personne va se pousser, en fait, il faut que les personne derriere ne soit pas des statues attendant leur tour, si ils voit un debut de place, ils vont pousser les lignes avant (sans prendre leur places) pour qu'ils se decalent, le rang va glisser a gauche ou a droite de la cible et les deux demi places devienne la place mais j'insiste sur le fait qu'un soldat ne doit pas pousser un autre soldat hors du combat donc si le decalage de la premiere ligne empeche celle la de taper (quitte a changer de cible), elle doit pas se decaler

### Brouillard (fog) — branche `fog` 84386a7 (worktree Faramir), retours du premier essai traités
- Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 fog_orders -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` (4 Hommes d'armes déjà choisis, deux déplacements en file déjà donnés vers le noir). Le scénario `fog` d'avant reste valable pour les bords.
- À regarder : (1) les bords du brouillard sont ronds (lecture en B-spline cubique de la texture, plus d'escalier de cases de 4 m) ; (2) les lignes et les points d'ordre restent visibles même dans le noir ; la ligne d'une attaque ne suit plus l'ennemi quand il sort de votre vue : elle s'arrête où vous l'aviez vu (le serveur refuse d'attaquer un ennemi déjà hors de vue).
- État : `quick_check -Parts tests,smoke` PASS (384/384, test `Vision.Match.OrderLine` rouge d'abord) ; captures avant / après hors écran (`fogord_before.png`, `fogord_after.png`, dans `Saved_Tests\Screenshots\WindowsEditor\HOL` du worktree) ; **pas mesuré** : le coût GPU (4 lectures de texture au lieu d'une sur tout l'écran, plus la passe de profondeur personnalisée, activée par `r.CustomDepth=3`) ; jamais lancé par l'utilisateur.
Reponse: - les bords ronds validés
- malgre plusieurs relance, des que les hommes d'armes se deplace ou emettent l'ordre de se deplacer, le brouillard disparait et impossible de le ravoir donc je ne peux pas tester le reste

## 2. Questions

### Brouillard
- **Question à vous : les lignes d'ordre sont-elles assez épaisses sous le brouillard ?** Les lignes d'ordre font 6,25 cm de large (réduites deux fois à vos essais) : à la caméra du jeu elles font 1 à 2 pixels. Sous le brouillard elles sortent maintenant du noir, mais restent fines. Options : A) garder 6,25 cm ; B) 12,5 cm seulement quand le brouillard est actif ; C) 12,5 cm partout. Recommandation : A tant que vous ne dites pas que c'est encore trop discret (réglable à chaud : `hol.Orders.LineWidth`). Fichiers : `FogInProgress.md`, `Source/HeroesOfLiberty/Player/HOLSelectionMarkers.cpp`.
Reponse: pas pu tester suite a un bug
### IA (IA4)
- **Question à vous : les Paysans de l'IA frappent-ils un soldat blessé ? (D572, interprétation à confirmer)** J'ai lu « de base non, mais contre un Lancier à 5 PV ça devrait aller » comme « un Paysan ne frappe que s'il est sûr de gagner le duel ». Aujourd'hui les Paysans de l'IA fuient tout soldat ennemi proche, aucun ne frappe. Options : A) duel gagné avec marge seulement (rapport de force du duel Paysan contre cette unité, PV restants compris, >= 1,5) : un Paysan achève un Lancier à 5 PV, jamais un soldat sain ; B) comme A, mais aussi quand un voisin ou un bâtiment est attaqué et que les Paysans sont en surnombre (3 Paysans contre 1 soldat blessé) ; C) jamais, ils fuient toujours (rien ne change). Recommandation : A (simple, lisible, fidèle à « de base non »). Rien n'est codé là-dessus avant votre réponse. Fichier : `AIInProgress.md`, GDD D572.
Reponse: A
- **Arbitrage des vies (D426)** : utile seulement avec des vies d'avance (donc des paliers) ; à quel rapport l'IA sacrifie-t-elle une vie plutôt que de défendre ? `AIInProgress.md`.
Reponse: A trancher

### Cartes
Vos retours sur l'Arrière-pays (« resserre les arbres et double leur quantité ») sont appliqués dans la branche `maps` : arbres x2 (300 par joueur, 360 à Immense) et plus serrés (1,6 m entre troncs au lieu de 2,2 m). Le x3 ne tenait pas ; le x2 tient aux 5 tailles des deux cartes (Immense : 360 et non 450, faute de place). Il reste quatre vraies questions.

- **M-Q10b : que faire des grands espaces vides, maintenant que le décor est retiré ?**
  Contexte : vous avez demandé de retirer tout ce qui n'est pas du jeu (fleurs, cailloux, taches de sol) et gardé « il doit y avoir quelque chose à l'écran ». Sans décor, même avec deux fois plus d'arbres, il reste des zones vides : sur la Steppe 55 à 75 m (plus d'un écran), sur l'Arrière-pays les coins du carré hors des murs (plus de 80 m). La règle d'avant disait au plus 28 m de vide ; elle n'est plus exigée par les tests en attendant votre réponse.
  - A. Accepter ces vides et retirer la règle des 28 m. En jeu : de grandes clairières d'herbe entre les bosquets, un peu de l'aspect « désolé » dont vous parliez.
  - B. Combler avec de petits affleurements de rocher (comme ceux du milieu de l'Arrière-pays). En jeu : plus de rochers que les armées contournent, rien de décoratif, vides sous 30 m.
  - C. Combler avec davantage de petits tas de ressource (petits filons de 500, tas de baies) entre les territoires. En jeu : plus de ressources à se disputer au centre, des parties un peu plus riches ; les bases restent à égalité.
  - D. (Arrière-pays seulement, s'ajoute à B ou C) ne compter que l'intérieur des murs : les coins hors des murs, inatteignables, restent vides.
  Recommandation : C pour la Steppe (c'est du jeu, pas du décor) et D pour l'Arrière-pays. Fichiers : `MapsInProgress.md`, `Source/HeroesOfLiberty/Tests/HOLMapTableTests.cpp` (`EmptyRuleHeld`), `Tools/Editor/maps/steppe.py` et `hinterland.py`.
Reponse: A, on reparlera de cela des que l'on aura tout (gibier, point d'interet...etc)

- **M-Q10c : jusqu'où resserrer les arbres ?** (à juger sur la prochaine capture)
  Contexte : les troncs d'un bosquet sont maintenant à 1,6 m les uns des autres (avant 2,2 m) ; les paquets restent de 12 à 16 arbres (18 à Immense). Une unité ne passe plus entre deux arbres d'un paquet : un paquet est un mur à contourner.
  - A. Garder 1,6 m. En jeu : bosquets denses, bien lisibles.
  - B. Resserrer encore (1,4 m) : bosquets presque pleins, plus de place pour d'autres bosquets, Immense pourrait atteindre 450 arbres.
  - C. Revenir à 1,9 m : plus aéré, mais Immense reste à 360 et Normal / Grande ont moins de bosquets.
  Recommandation : A, à confirmer sur capture. Fichier : `Tools/Editor/maps/common.py` (`GROVE_SPACING`).
Reponse: je peux pas lire la capture donc impossible de trancher

- **M-Q3 : le pont des Gués (carte Les Gués, D496) : que devient-il quand le siège l'a détruit ?**
  Contexte : la rivière ne se traverse qu'aux gués et par un grand pont au milieu, détruit par le siège et refait gratuitement par les villageois (D496). `MapsInProgress.md` note déjà « A » (2026-10-30) ; je vous le repose en clair avant de coder la carte.
  - A. Le pont tombé devient un chantier (comme la ruine du forum, V3) : un Paysan le relève en quelques secondes, sans coût. En jeu : on voit la ruine et le Paysan au travail, l'ennemi peut le frapper pendant ce temps.
  - B. Le pont d'origine revient seul après un délai fixe, sans Paysan. En jeu : rien à faire, mais on ne peut ni le défendre ni s'en servir comme piège.
  - C. Le pont tombé reste tombé jusqu'à ce qu'un Paysan le paie (ressources). En jeu : plus tactique, plus lent, le fleuve devient une vraie frontière.
  Recommandation : A (déjà votre choix du 2026-10-30). À trancher avant la carte Les Gués. Fichier : `MapsInProgress.md`.
Reponse: A

- **Décor d'identité des cartes à eau, et Plaine.**
  Contexte : par votre règle, plus aucun décor sans intérêt de jeu, sauf s'il donne son identité à la carte (sable côté mer, roseaux côté rivière). Cela ne concerne que les cartes à eau (Les Gués, El Dorado, La Couronne...), pas encore faites. Plaine, la carte d'essai, a encore l'ancien budget et le sol d'origine.
  - A. Aucun décor nulle part, même sur les cartes à eau : seules l'eau et les berges marquent la carte. En jeu : cartes sobres et lisibles.
  - B. Un seul décor d'identité par carte à eau, au bord de l'eau (sable ou roseaux), sans collision. En jeu : les rives ont leur aspect, le reste reste de l'herbe.
  - C. B, plus un sol varié (taches de sable ou de boue) là où il aide à lire le terrain.
  Plaine : (1) la mettre au budget standard et au sol uniforme comme les autres ; (2) la laisser telle quelle, c'est la carte d'essai des tests.
  Recommandation : B pour les cartes à eau (décidé carte par carte quand on les fait) et (1) pour Plaine. Fichier : `MapsInProgress.md`.
Reponse: ma remarque parlait des maps d'eau mais il peut y avoir une autre identité, un map de type montagneux aura des rochers, une map de type aride aura du sable ou de la terre brune...etc, je ne veux juste pas de généralités et de remlissage pour du remplissage

### Volcan / Catastrophe
- **V-Q1** (fenêtres de Catastrophe, D208 sans chiffres) : A première fenêtre 6e-9e min, repos 6-8 min, 3-4 éruptions ; B 8e-12e min. Recommandation A, réglable. `VolcanoInProgress.md`.
Reponse : A trancher

## 3. Commits et fusions à valider

- **À commiter dans la copie principale** (`C:\Users\baudo\Documents\MyCrusader`) : `GDD.md` (§ 7.2 aligné, D564-D575, prochain **D576** ; contient aussi D558-D563 d'autres sessions), `EtapeDefini.md`, les `*InProgress.md`, les `Auto_Journey*.md`, `FogInProgress.md`, `MapsInProgress.md`, `VolcanoInProgress.md`, `Tools\AutoJourney\status.ps1` (non suivis). Ils mêlent plusieurs sessions.
- **Fusions dans `main`** (aucune faite), ordre conseillé :
  1. `ai-ia1` (c806fd8, 3f5f58a, b8bfb9e, 2665240) en premier : corrige le `quick_check -Parts victory` rouge de `main`.
  2. `galadriel-tab` (5e4da9f, 97bbe4b, d4fadd0) : Tab + `quick_check -Parts build` vert + scénario `tab`.
  3. `tiers` (48e3244, 1093e52 ; main déjà fusionné dedans).
  4. `hero-j8-close` (b297add, dd16203) : `-Parts hero` à lancer avant.
  5. `fog` (a94304b, 648da21, 84386a7, 30b5a0c).
  6. `maps` (c3579ba, 3594b4f, 533f480, 89ce28f) : **sans M-Q10 non commité** ; ne pas faire `git checkout` des `.umap` du worktree Haldir (ramènerait les cartes avec décor).
- Chaque fusion : `quick_check` des parties touchées vert avant ; `git checkout -- HeroesOfLiberty.uproject` si la fusion refuse ; recompiler après.
