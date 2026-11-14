# [À ESSAYER] E-income_tooltip : survole un montant de la barre du haut (bois, or...) : une infobulle donne le revenu par minute de cette ressource, trajets compris (D788).

STATUT : VU À L'ÉCRAN (le contenu de l'infobulle, dans les 3 langues ; le survol à la vraie souris reste à faire par toi)
GESTE: interne      (`hol.HUD.HoverIncome <i>` : test de survol de Slate au centre du montant, puis la même infobulle dessinée dans la vue pour la capture ; pas de vraie souris)
SON: sans objet
branches: aucune (main)
Recette indépendante 19:3x sur main dc67a16 (worktree Faramir, branche fog) : PASS (preflight 26 s ; WARN sans hol.HUD.Report ni hol.Test.Sanity), PNG vus : income_tip_fr (« +545 bois/min » sous Bois 210 (5)), income_tip_gold_fr (« +0 or/min » sous Or 200), income_tip_en (« +545 wood/min » sous Wood 240), income_tip_ja (« +545 木材/分 » sous 木材 240) ; réserves : texte de l'infobulle minuscule (~6 px de haut à 1280x720, lisible seulement en zoomant : REGLAGE probable) ; l'infobulle de l'or recouvre le « S » de « Scénario » du panneau (le scénario n'appelle pas hol.HUD.Report, recouvrement non audité) ; survol à la vraie souris non rejoué.
Recette indépendante 3 23:09 sur main 091df3c : PASS (preflight 27 s ; mêmes WARN sans hol.HUD.Report ni hol.Test.Sanity ; le verbe `quit` joue : « Scenario quit: the game closes (step 43, nobody at the keys) » puis « LogExit: Exiting. », aucun UnrealEditor restant après le pré-vol), PNG vus (agrandis x2) : income_tip_fr (« +545 bois/min » sous Bois 210 (5)), income_tip_gold_fr (« +0 or/min » sous Or 200 (0)), income_tip_en (« +545 wood/min » sous Wood 240 (5)), income_tip_ja (« +545 木材/分 » sous 木材 240（5）) ; réserves (inchangées) : texte de l'infobulle ~6 px ; l'infobulle de l'or recouvre toujours « Sc » de « Scénario » ; en EN et JA « Triches + » et le titre « Scénario … en préparation… » restent en français ; survol à la vraie souris non rejoué.
Recette indépendante 2 21:28 sur main 15bbd25 (Faramir, fog, après net-playtest + tiers) : PASS (preflight 24 s ; mêmes WARN sans hol.HUD.Report ni hol.Test.Sanity), PNG vus : income_tip_fr (« +545 bois/min » sous Bois 210 (5)), income_tip_gold_fr (« +0 or/min » sous Or 200 (0)), income_tip_en (« +545 wood/min » sous Wood 240 (5)), income_tip_ja (« +545 木材/分 » sous 木材 240（5）) : aucune régression d'affichage ; réserves (inchangées) : texte de l'infobulle ~6 px ; l'infobulle de l'or recouvre toujours le « S » de « Scénario » ; en EN et JA le bouton « Triches + » reste en français (panneau de triche) ; survol à la vraie souris non rejoué.

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
