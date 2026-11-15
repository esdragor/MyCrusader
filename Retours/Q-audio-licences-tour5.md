# [À RÉPONDRE] Q-audio-licences-tour5 : les 8 sons de cheval de la Red Library (Écurie, tour 5) : garder, remplacer ou retirer ?

Contexte : les lettres A à H de `E-audio_compare_buildings_stable` (hennissements, ébrouements, pas) sont coupées dans 4 fichiers de la « Red Library » (collection USC Cinema / Sunset Editorial) téléversés sur archive.org (https://archive.org/details/Red_Library_Animals_Horses_1), page marquée CC0 par archive.org.
Ce marquage n'est pas celui de l'auteur : rien ne prouve que la bibliothèque sonore commerciale d'origine a été versée au domaine public ; les autres sons du tour 5 sont CC0 d'OpenGameArt (sûrs) ou CC-BY 3.0 (crédit à prévoir).

Options :
- **A. Garder** les 8 sons Red Library (CC0 d'archive.org pris tel quel). Conséquence : l'écoute se fait sur les 10 lettres ; risque de licence si l'un est élu et publié.
- **B. Remplacer** les 8 sons par du CC0 sûr (OpenGameArt ou Kenney, auteur identifié) avant l'écoute. Conséquence : un nouveau passage de `make_round5_candidates.py` + import + pré-vol (un agent, sans C++) ; l'écoute de l'Écurie attend.
- **C. Retirer** les 8 sons de la liste : l'Écurie n'a plus que I et J (trot, CC0 EZduzziteh). Conséquence : choix très réduit.
- **Autre** : par exemple écouter d'abord, et ne remplacer (B) que si une lettre A-H est élue.

**Recommandation : « Autre » = écouter d'abord (A pour l'écoute), puis B seulement si une lettre A-H est élue** (chercher un enregistrement CC0 sûr qui ressemble au son choisi). Aucune publication n'est prévue avant l'élection.
Valeur par défaut appliquée : A (rien changé ; la liste `Building.Stable` de `Script/Audio/HOLAudioCandidates.as`, ligne 87, est la ligne à changer pour C).

Pour info (pas de question) : les sons CC-BY 3.0 sont H du Champ de tir (s_shot5_volley_a, copyc4t) et J du Champ de tir (s_shot5_bolt_hit, spookymodem) : crédit à prévoir dans le jeu s'ils sont élus.

Réponse attendue : `A`, `B`, `C` ou `Autre: ...`.
