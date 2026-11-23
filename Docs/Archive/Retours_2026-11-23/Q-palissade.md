# [À RÉPONDRE] Palissade : PV, coût, temps de pose, empreinte (S3-b, GLORFINDEL-1123b)

Contexte : la Palissade est la première ligne de mur (palier 0, D691, pose en ligne sans plafond D867, chaque case est un chantier). Le GDD ne fixe ni PV, ni coût, ni temps : ligne provisoire dans `Tools\Data\BuildingTypes.csv` (valeurs derrière la table, une ligne à changer + `regen_tables.ps1`).

Valeur codée par défaut (A) : 1 case de 4 m (400 x 400), 400 PV, 5 bois par case, 5 s par case, hauteur 200, vision 800, armure à distance 40 (D692 : le bois reçoit la mêlée et le siège, pas le tir : à brancher quand la colonne de famille d'attaque existe).

Options :
- A (défaut) : 400 PV, 5 bois, 5 s par case. Une file de 20 cases = 100 bois, 100 s de travail réparti sur les Paysans ; la mêlée la perce vite, un Bélier immédiatement.
- B : 800 PV, 10 bois, 8 s par case. Mur plus solide mais file de 20 cases = 200 bois : freine le harcèlement d'ouverture, rend la rush de tour plus chère.
- C : 250 PV, 3 bois, 3 s par case. Barrière de retardement presque gratuite, détruite par trois soldats : sert à canaliser plus qu'à défendre.
- Autre : à préciser (y compris une empreinte plus fine que la case de 4 m).

Conséquences visibles : le coût total d'une ligne de N cases, le temps avant qu'une ligne soit fermée, et la durée pendant laquelle une garnison de tour garde un passage.

Recommandation : A (proche d'AoE4 : palissade bon marché, sacrifiable ; la solidité vient du Mur de pierre du palier 1).
