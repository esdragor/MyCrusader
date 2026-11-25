# [À RÉPONDRE] Artistes : Git LFS et verrous pour les fichiers d'art ? (CLAUDE-1125ar, AQ-3)

Contexte : les artistes (sans Claude) vont pousser des `.uasset`, des FBX et des textures. Le dépôt est sur GitHub
(`origin https://github.com/esdragor/MyCrusader.git`) **sans LFS** : `.gitattributes` déclare seulement `*.uasset binary`,
etc. (aucun `filter=lfs`, aucun `lockable`). Conséquences : (1) deux personnes peuvent modifier le même `.uasset` et le
conflit ne se résout pas (on garde l'un ou l'autre) ; (2) GitHub refuse tout fichier de plus de 100 Mo hors LFS (FBX,
textures 4K). Un `DA_Art` par type (D924) limite déjà les collisions, mais pas sur les maillages et textures partagés.

Options :
- A : **LFS + verrous pour les NOUVEAUX fichiers d'art seulement** : `.gitattributes` ajoute
  `Content/Art/** filter=lfs diff=lfs merge=lfs -text lockable` (et `*.fbx`, `*.psd`, textures sources) ; l'historique
  existant n'est pas réécrit. Un artiste fait `git lfs lock <fichier>` avant de modifier, `unlock` après ; l'éditeur
  Unreal sait afficher les verrous (Source Control Git LFS 2). Coût : quota LFS GitHub (1 Go gratuit, puis packs
  payants), chaque poste installe Git LFS (déjà installé sur cette machine).
- B : **pas de LFS, règle d'équipe** : un dossier par artiste ou par type, annonce dans un canal avant de toucher un
  fichier partagé, fichiers sources (FBX, PSD) hors dépôt (dossier `MyCrusader_Art\` comme aujourd'hui). Coût nul,
  risque de conflit et de fichier trop gros reporté sur les humains.
- C : **LFS pour tout le dépôt avec migration de l'historique** (`git lfs migrate`) : propre mais réécrit l'historique,
  casse tous les clones et worktrees existants (7 worktrees d'agents) : à éviter maintenant.
- Autre : à préciser (autre hébergeur, Perforce...).

Recommandation : A (pas de réécriture, protège exactement ce que les artistes vont toucher ; à faire AVANT que le
premier artiste pousse un fichier).
Valeur par défaut codée : AUCUNE (rien n'est changé dans `.gitattributes` tant que la question n'est pas tranchée).

Reponse:
