# [À ESSAYER] E-income_tooltip : survole un montant de la barre du haut (bois, or...) : une infobulle donne le revenu par minute de cette ressource, trajets compris (D788).

STATUT : VU À L'ÉCRAN (le contenu de l'infobulle, dans les 3 langues ; le survol à la vraie souris reste à faire par toi)
GESTE: interne      (`hol.HUD.HoverIncome <i>` : test de survol de Slate au centre du montant, puis la même infobulle dessinée dans la vue pour la capture ; pas de vraie souris)
SON: sans objet
branches: net-playtest

Pré-vol (2026-11-13 18:30, agent Glorfindel L54, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e4f81eb fusionné) : `preflight.ps1 income_tooltip -Root <worktree>` PASS (104 s, 4 PNG).
Textes relus dans les .po (a70bc24) : `IncomeTip` fr "+{0} {1}/min", en "+{0} {1}/min", ja "+{0} {1}/分" ; `TributeWood/Gold` = bois / or, wood, 木材.
Corrigé en route : les montants et les compteurs de cueilleurs de la barre du haut n'étaient pas survolables (textes SelfHitTestInvisible : la souris tombait sur le fond de la barre, AUCUNE infobulle ne s'ouvrait en jeu) : 0688481.
PNG OUVERTS et vus :
- income_tip_fr : sous « Bois 310 (5) », une petite infobulle « +545 bois/min ».
- income_tip_gold_fr : sous « Or 200 (0) », « +0 or/min ».
- income_tip_en : barre en anglais, sous « Wood 350 (5) », « +549 wood/min ».
- income_tip_ja : barre en japonais, sous « 木材 400 (5) », « +549 木材/分 ».
NON VÉRIFIÉ : le survol à la vraie souris (délai d'ouverture de l'infobulle du moteur, sa fenêtre à part) ; la taille du texte de l'infobulle (petite, style par défaut du moteur, à juger à l'œil) ; les chiffres sont gonflés par `hol.Gather.SpeedScale 5` du scénario.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`
Durée : environ 2 min. Pose la souris sur le montant de bois puis d'or. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
