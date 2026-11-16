# [À ESSAYER] E-alert_population : ta population atteint le plafond (10 / 10) quand tu mets un Paysan en file au forum : la ligne discrète « Population pleine » entre dans le fil de messages, en haut à droite (D202, D804)

STATUT : VU À L'ÉCRAN (ligne « Population pleine » lisible dans le fil, en haut à droite, sur le PNG alert_population)
GESTE: réel      (vrai clic gauche `hol.Mouse.Click 960 540` sur ton forum, journal « Mouse click 960,540 ... on SViewport » puis « Selection: MainCentre of slot 0 (own) » ; puis la vraie touche A par `hol.UI.Key A` (événement clavier Slate), journal « Key pressed: A » puis « Production: slot 0 queued Peasant ») ; seuls les 9 premiers Paysans sont offerts par `hol.Units.SpawnHere`
SON: non entendu      (`-nosound` ; la ligne est muette dans le code actuel, D449 ; le son court par type de D804 B n'est pas encore codé)
branches: galadriel-tab

Ce que tu fais : le scénario pose 9 Paysans près de ton forum (population 9 / 10, le fil est vide), attend Entrée, puis clique ton forum et appuie sur A (Paysan). Le Paysan mis en file réserve sa place : la population passe à 10 / 10 tout de suite et la ligne « Population pleine » entre au fil (elle y reste environ 8 s, D446). Elle ne revient pas avant 30 s, et seulement si la population a de nouveau de la place puis se remplit.

Pré-vol (2026-11-16 15:47, agent GALADRIEL-1116b, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, branche galadriel-tab 15097fc = a11ed2a (ligne au fil, test PopulationFullThread) + merge main 825b841 + scénario) : `preflight.ps1 alert_population -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (71 s, 1 PNG, audit Sanity passé, aucun fichier non commité).
Journal exact (Saved_Tests\Logs\scenario_alert_population.log) : « HUD report: ... population 9/10, selection MainCentre (own), grid A=Peasant enabled » puis, après la touche A, « HUD report: ... population 10/10 » et « HUD messages: [Population pleine] ».
PNG OUVERTS et vus :
- alert_population : barre du haut « Population 10 / 10 » ; en haut à droite, sous « Vous 2 vies », la ligne « Population pleine » lisible ; forum sélectionné (« Centre principal », grille A = Paysan), panneau de production « Paysan » avec sa barre au début ; 9 Paysans bleus au nord du forum.
NON VÉRIFIÉ : le son (aucun, -nosound) ; la répétition après 30 s en partie (seulement par le test HOLAlertRules_Test) ; la ligne dans une autre langue que le français ; le cas d'un Paysan qui meurt puis d'un autre mis en file.

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 alert_population -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
Reponse: BUG : jamais eu le fil, de meme, le check de population se fait a la fin de la prod (on queue des soldats et des paysans a 100%, ils checkent avant de sortir, si non, ben ils reste a 100% mais ne sortent pas, mettent comme quoi on est max pop et des qu un slot se libere, c est parti) ; utilisateur, 2026-11-16 16:06, dans la conversation ; D829 ; ligne [ ] en tête de file (Galadriel), corrigée dans le worktree Sauron (libre).
