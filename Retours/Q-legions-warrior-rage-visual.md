# [À RÉPONDRE] Q-legions-warrior-rage-visual : comment voit-on la rage nécrotique à l'écran ?

Contexte : la rage du Guerrier damné (10 s, +40 % dégâts, +25 % cadence, -3 PV par coup) est codée et lancée par la case « Rage » (E), mais RIEN n'est dessiné pendant qu'elle court (ni aura, ni teinte, ni texte, ni icône d'état) ; seule la barre de vie baisse plus vite. Le scénario `legions_warrior` ne peut donc prouver l'effet qu'au journal (« casts the necrotic rage ») : point noté LOGIQUE SEULEMENT.
Le GDD (§ 11.1 « Rage nécrotique ») ne décrit aucun rendu.

Options :
- A. Teinte rouge sombre du corps du Guerrier tant que la rage court (réglage : une couleur et un drapeau dans le rendu des unités ; 1 ligne de CSV/cvar pour l'éteindre).
- B. Petit anneau ou halo au sol sous le Guerrier (comme les anneaux de sélection), couleur nécrotique.
- C. Seulement une icône d'état dans le bloc de sélection (« Rage 7 s »).
- D. Rien (la barre de vie suffit).
- Autre : ...

Conséquences visibles : A et B se voient en bataille (de loin) ; C seulement unité sélectionnée ; D = le joueur ne sait pas si la rage court encore.

Recommandation : A + C (teinte pour la bataille, compte à rebours pour la sélection).

Constat du scénario (vu à l'écran, PNG `legions_warrior_rage` et `_after`) : la case « Rage » (E) reste identique avant, pendant et après la rage (ni grisée pendant la recharge de 40 s, ni compte à rebours) ; un second clic est accepté par l'ordre mais refusé en silence par la simulation (aucun message). Le même choix A/B/C/D devrait donc dire aussi si la case se grise pendant la recharge (recommandation : oui, comme les cases du héros si elles le font).

Valeur par défaut codée : D (rien n'est dessiné, comportement actuel) ; le choix n'ajoute qu'un drapeau de rendu à lire sur `RageMs > 0`.

Essai pour choisir : Retours\E-legions_warrior_compare.md (5 options à l'écran, Entrée = suivante) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 legions_warrior_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel (cvar hol.Rage.Visual : 0 D rien (défaut), 1 A, 2 B, 3 C, 4 A+C ; B dessinée depuis 2026-11-11).
