# [À RÉPONDRE] Q-income-tooltip-taille : taille du texte de l'infobulle de revenu (barre du haut)

Contexte : depuis que l'infobulle de revenu est ouverte par un vrai survol (GALADRIEL-1115b, galadriel-tab), la capture montre l'infobulle de Slate elle-même (le dessin de remplacement `hol.HUD.HoverIncome` est retiré). Elle est lisible mais très petite : environ 7 px de haut à 1280x720 (« +545 bois/min », « +545 木材/分 »), contre environ 14 px pour les montants de la barre.
PNG : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Saved_Tests\Screenshots\WindowsEditor\HOL\income_tip_fr.png` (et `income_tip_gold_fr`, `income_tip_en`, `income_tip_ja`). L'infobulle de l'or recouvre aussi le début du titre du panneau du scénario (« Sc… »).

Options :
- A. Garder l'infobulle par défaut de Slate (petite, fond blanc) : rien à faire.
- B. Donner à l'infobulle de revenu le style du HUD (police de la barre, environ 14 px, fond du panneau sombre, bord doré) : C++ dans le widget de la barre du haut (ToolTipWidget au lieu du texte simple).
- C. B pour toutes les infobulles du jeu (style commun dans HOLUIStyle).
- Autre : à préciser.

Conséquences visibles : A = texte à plisser les yeux en 1080p ; B = infobulle lisible, cohérente avec la barre ; C = même effet partout, chantier plus large.
Recommandation : B (puis C si d'autres infobulles ont le même défaut).
Valeur par défaut : A (état actuel) ; aucun réglage possible sans C++ (agent sans C++).
