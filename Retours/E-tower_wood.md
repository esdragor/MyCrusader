# [À ESSAYER] E-tower_wood (branche zone-effect, PAS ENCORE DANS MAIN) : la Tour de guet en bois lâche une flèche en vol sur 3 Lanciers rouges à 12 m (flèche grossie x3 : fût beige, pointe grise, empennage blanc)

STATUT : VU À L'ÉCRAN   (flèche EN VOL vue sur 2 PNG, FARAMIR-1123pe 2026-11-23 ; geste interne, son non entendu)
GESTE: interne
SON: non entendu
branches: zone-effect (e1528ac)

AIDES D'ESSAI, PAS LE RENDU DU JEU : le scénario règle `hol.Arrow.Scale 3` (flèche 3 fois plus grosse) et `hol.Arrow.SlowMotion 4` (vol DESSINÉ 4 fois plus long ; les dégâts tombent à l'heure), plus `hol.Combat.LogBuildingShots 1` (une ligne au lancer, pour que le scénario capture à ce moment). Par défaut (1 / 1 / 0) rien ne change : en jeu la flèche d'une tour est 3 fois plus petite et vit ~0,3 s.

Pré-vol FARAMIR-1123pe (2026-11-23 16:55, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, commit e1528ac ; build PASS 38 s, quick_check tests 2163/2163) : preflight.ps1 tower_wood PASS (37 s, 9 PNG). PNG OUVERTS : tower_wood_before (la tour seule, aucune flèche), tower_wood_during, tower_wood_during2, tower_wood_during3 ; les 5 autres (during4..6, after) NON ouverts par cet agent.
- tower_wood_during : UNE FLÈCHE EN VOL au-dessus du toit de la tour : fût beige ~190 px de long x ~13 px, pointe grise à gauche, empennage blanc à droite, inclinée vers les Lanciers rouges à gauche (absente de before) ; capture prise ~50 ms (3 images) après la ligne « Combat: building shot step 248 ... flight 171 ms » ; ArrowReport juste avant : en vol 1, lancées 2, atterries 1.
- tower_wood_during2 (+4 images) : la même flèche, ~90 px, plus près du Lancier de gauche.
- tower_wood_during3 (+4 images) : plus de flèche nette (un petit objet blanc près du Lancier de gauche, non identifié) ; les petits traits orange près des Lanciers sont leurs barres de vie, les traits sombres leurs lances (pas des flèches).
NON VÉRIFIÉ : geste réel (le scénario est interne) ; son ; barres de vie lisibles ; la flèche à sa taille réelle (x1, ~20 px, 0,3 s) ; during4..6 et after non ouverts ; le worktree zone-effect n'est pas dans main.
Commande complète : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 tower_wood -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
Durée : environ 1 min. Réponse attendue : OK, BUG: ... ou REGLAGE: ....

Historique (avant FARAMIR-1123pe, flèche en vol alors NON vue) :

Pré-vol (2026-11-23 10:50, agent CELEBORN-1123h, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, commit 4d61f5d) : preflight.ps1 tower_wood check PASS (40 s, 6 PNG) ; le FAIL global n'était que « 2 fichiers non commités » (corrigé par le commit 4d61f5d).
PNG OUVERTS et vus :
- tower_wood_during : 3 Lanciers rouges au centre, petit trait orange à gauche de la tête du Lancier de gauche + hampe sombre : la flèche, minuscule à Zoom 2 (~20 px)
- tower_wood_during2 / during3 / during4 : le trait orange suit le Lancier de gauche qui avance vers la tour (la flèche suit sa cible ; ArrowReport : en vol 0, lancées 1, atterries 1)
- tower_wood_after : une flèche (hampe + pointe sombre, trait orange) à gauche d'un Lancier près de la tour ; barres de vie non lisibles
- tower_wood_before : NON ouvert
NON VÉRIFIÉ : flèche EN VOL (elle ne vit que 6 pas = 0,3 s à 12 m, la capture tombe après l'impact) ; PV entamés lisibles ; geste réel ; son ; que le worktree Sauron contient integ-41.

Commande complète : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 tower_wood -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
(Le Root Sauron doit contenir integ-41 : sinon jouer avec -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn.)
Durée : environ 1 min. Réponse attendue : OK, BUG: ... ou REGLAGE: ....

CELEBORN-1123i (10:57) : balayage 6 captures (during..during6, un ArrowReport avant chacune, toutes les 3 images). Preflight : FAIL (1) global (fichiers non commites avant commit), check PASS. PNG ouverts : during (rapport 'in flight 1 launched 1 landed 0' juste avant : AUCUNE fleche visible, 3 Lanciers rouges, hampe sombre de lance seulement), during2 et during3 (rapport 'in flight 0 launched 1 landed 1' : petit trait orange horizontal au-dessus de la tete du Lancier de gauche, plantee/au sol, pas en vol). Fleche EN VOL non vue : la capture part quelques images apres le rapport et la fleche vit ~0,3 s. Reste LOGIQUE SEULEMENT. Cause : ArrowReport est un instantane, pas de waitlog conditionnel possible sur 'in flight 1' ; il faut une capture declenchee par le code (lot C++) ou une fleche plus lente. Commande jouable : play.ps1 tower_wood -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn (Sauron ne contient PAS integ-41, verifie).
NETTOYAGE-1123g 2026-11-23 : le prealable integ-41 est perime : 0d1c150 (T7/T8) et af8ff83 (balayage du scenario) sont DANS main et dans le worktree Sauron ; la commande Sauron/Celeborn ci-dessus est donc jouable ; manque pour devenir pret : une capture de la fleche EN VOL (lot C++ de la file : journal au lancer + ArrowScale, Auto_Journey_File.md l. 46), puis pre-vol ; geste reel et son restent non verifies.
