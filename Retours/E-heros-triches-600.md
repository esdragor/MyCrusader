# [À ESSAYER] E-heros-triches-600

**VU À L'ÉCRAN** (pré-vol PASS, 1 PNG ouvert et vu : « Triches + » replié en haut à gauche, bloc du héros entier en bas à gauche, fenêtre de 1280 x 600). Écrit et joué par le même agent (pas de recette indépendante). Worktree Galadriel, branche `galadriel-tab`, commit ab2384a, pas dans `main`.

**Constat à lire d'abord** : remesuré aujourd'hui, le chevauchement de ta question n'existe pas à 600 px : l'échelle du HUD suit la hauteur de la fenêtre (à 1280 x 600 : échelle 0,555, la mise en page vaut celle de 1080 px), et le journal dit `HUD overlap: none` avec le panneau Triches OUVERT et le bloc du héros affiché (partie jouée avec la règle coupée, `hol.HUD.CheatsLowFold 0`). Ton choix A est quand même appliqué comme demandé (sous 700 px : replié au départ, se replie quand le bloc du héros s'ouvre, un clic le rouvre et il reste ouvert), sans effet sur ce que tu vois d'autre ; coupe-le avec `hol.HUD.CheatsLowFold 0` ou dis-moi de le retirer. Défaut trouvé et corrigé en route : le pré-vol lisait « HeroPanel x HeroPortraitFill » (un cadre dans un autre) comme un chevauchement dans tout scénario avec héros ; seuls les cadres extérieurs sont comparés maintenant.

Commande (PowerShell) :

    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 heros_triches_600 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel

Gestes (durée ~1 min) :
1. Le scénario ouvre « Triches » dans une fenêtre haute (1280 x 1080), puis abaisse la fenêtre à 1280 x 600 : le panneau se replie tout seul. Regarde en haut à gauche (« Triches + ») et en bas à gauche (bloc du héros : portrait, niveau, PV) : rien ne se cache.
2. Clique sur « Triches » : il s'ouvre et reste ouvert ; reclique : il se replie.

Non vu à l'écran : le démarrage replié d'une fenêtre déjà basse avec « Triches » gardé ouvert d'une partie à l'autre (règle testée en pur, pas rejouée en lancement réel).

À regarder : à 600 px, le panneau replié et le bloc du héros sont-ils tous deux lisibles et entiers ? Le repli automatique te gêne-t-il ?

À répondre dans Retours (une ligne) : `E-heros-triches-600 OK`, `E-heros-triches-600 BUG: ...` ou `E-heros-triches-600 REGLAGE: ...`.

Reponse:
