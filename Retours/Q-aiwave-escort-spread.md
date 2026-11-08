# [À RÉPONDRE] Q-aiwave-escort-spread : l'escorte de la vague s'étire-t-elle trop, et quelle tolérance pour « un engin devant l'escorte » ?

Contexte : `quick_check -Parts aiwave` (Hard AI, 10 Piquiers + 2 Béliers + 1 Mangonneau donnés à l'IA) était rouge sans rapport avec G1 (Faramir-16, branche fog). Cause trouvée côté script : les engins étaient posés en (-13000, 0), pile au point de ralliement de l'IA (-13300, 0, rayon 1000) où elle construit son Écurie (-13400, 200) et sa Caserne (-13000, -1000) : un Bélier restait coincé dans l'entre-deux, 0 cm de progression. Corrigé dans le script (apparition en (-12500, 250), rayon 150, dans le rayon du ralliement mais hors des empreintes). Deux mesures restent à trancher, aucune n'est corrigée dans le jeu.

Observation 1 : une fois l'engin libre, l'escorte (les 10 Piquiers) se retrouve étirée sur 7012 cm (avant : 964 à 1225 cm quand l'engin coincé faisait écran) ; les Piquiers de tête courent à plus de 300 cm/s pendant que les autres et les engins marchent à ~180-220 cm/s ; les 2 de tête meurent près de l'ennemi, les engins arrivent derrière eux.

Observation 2 : le test mesurait la distance « engin - Piquier le plus avancé en vie » avec une tolérance de 100 cm. Avec le même Mangonneau, le décalage mesuré allait de +111 à +247 cm tant que l'escorte était groupée (Faramir-2 avait vu l'inverse : engins à ~3000 cm derrière).

Options (question 1, l'étirement de l'escorte) :
- A. Normal, c'est la marche d'une vague (pas de groupe à la vitesse du plus lent pour l'IA) : on garde le test tel quel.
- B. Défaut : l'IA doit marcher en groupe (D617, vitesse du plus lent) : chantier IA à ouvrir.
- Autre.

Options (question 2, tolérance de « engin devant l'escorte ») :
- A. Garder 100 cm (valeur codée aujourd'hui, `quick_check.ps1`, partie aiwave : `+ 100`). Elle passe maintenant parce que les Piquiers morts comptent à leur dernier emplacement.
- B. 300 cm (couvre les +111 à +247 mesurés avec l'escorte groupée).
- C. Relative : une fraction de l'étirement de l'escorte.
- Autre.

Recommandation : 1 = A tant que le GDD ne dit rien sur la marche de la vague (à vérifier au prochain essai à l'écran) ; 2 = A, le test passe déjà deux fois avec 100.
Valeur par défaut codée : 100 cm (une ligne : `$Front + 100` dans la partie aiwave de `Tools\Tests\quick_check.ps1`).
Conséquence visible : aucune dans le jeu ; seul le verdict du test change.
