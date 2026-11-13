# Q-talents-icons [À RÉPONDRE]

Contexte : tu as demandé de vraies icônes de talents, du même style que celles des capacités (Aura, Serrez les rangs, Bannière). J'ai dessiné les 24 (une par talent de l'arbre du Paladin, or et blanc cerné de sombre, comme les capacités), hors jeu pour l'instant.
Planche : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-talents-icons.png` (chaque talent dans les 3 cadres, à 64 px puis à 32 px ; colonnes Gardien bleu, Capitaine or, Champion rouge).

Question : quel cadre autour de l'icône, dans le bloc du héros et dans la fenêtre Arbre de talents ?

- **A. Bordure de la couleur de la famille** (fond sombre comme les capacités, liseré bleu / or / rouge) : la famille se lit d'un coup d'œil, l'icône garde le même fond que les capacités.
- **B. Disque de la couleur de la famille** : la famille se voit davantage, mais l'or des icônes du Capitaine se perd un peu sur le disque or (voir colonne du milieu).
- **C. Coin de la couleur de la famille** (fond sombre, petit triangle coloré en haut à gauche) : le plus proche des capacités, la famille se lit moins, surtout à 32 px.
- **Autre** : un autre cadre, ou retoucher des dessins précis (dis lesquels : ex. « Pas de l'Aube » ne montre que des chevrons, le bouclier est caché ; « Escorte du champion » : les petits boucliers sont cachés par le casque).

Recommandation : **A** (lisible à 32 px, même fond que les capacités, la famille reste visible).

Ce qui est fait : `Tools/Editor/make_talent_icons.py` (branche `galadriel-tab`) dessine les 24 PNG et cette planche. Ce qui reste après ta réponse : importer les textures (éditeur), afficher l'icône dans le bloc du héros et dans l'Arbre de talents (script seul, à la place de la lettre actuelle), rejouer `talents_icons_compare`. Valeur par défaut si rien n'est répondu : **A**.

Réponse:
