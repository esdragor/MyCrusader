# [À ESSAYER] E-income_tooltip : chaque infobulle de revenu (bois FR, or FR, bois EN, bois JA) est ouverte par un vrai survol de la souris.

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: sans objet
branches: galadriel-tab

À l'utilisateur : cet essai a déjà été joué OK le 2026-11-14. Seul le geste change : les quatre survols passent par hol.Mouse.Hover (curseur virtuel de Slate immobile 30 images) et le texte est vérifié sur l'infobulle que Slate ouvre lui-même (« tip text "+545 bois/min" », « +0 or/min », « +545 wood/min », « +545 木材/分 ») ; en EN et JA la souris sort du montant puis y revient (nouvelle infobulle dans la nouvelle langue). L'OK du 2026-11-14 reste valable.

Pré-vol (2026-11-15 09:38, agent GALADRIEL-1115a, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, galadriel-tab 57cccd0) : `preflight.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (environ 40 s, 4 PNG ; WARN : pas de hol.HUD.Report ni hol.Test.Sanity dans ce scénario, comme avant).
PNG OUVERTS et vus :
- income_tip_fr : sous Bois, l'infobulle « +545 bois/min ».
- income_tip_gold_fr : sous Or, « +0 or/min ».
- income_tip_en : barre en anglais, sous Wood, « +545 wood/min ».
- income_tip_ja : barre en japonais, sous 木材, « +545 木材/分 ».
Pré-vol 2 (2026-11-15 09:59, agent GALADRIEL-1115b, galadriel-tab 10290ba) : hol.HUD.HoverIncome RETIRÉ du scénario (il dessinait une 2e infobulle par-dessus la vraie) ; preflight.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel PASS (40 s ; WARN sans hol.HUD.Report ni hol.Test.Sanity). Log : « tip text "+545 bois/min" », « "+0 or/min" », « "+545 wood/min" », « "+545 木材/分" ».
PNG OUVERTS et vus (captures de 09:53, même scénario, check PASS) : une SEULE infobulle par image, plus aucun chevauchement ; texte lisible dans les 4 langues mais petit (environ 7 px de haut à 1280x720, police par défaut de Slate, fond blanc) : « +545 bois/min » sous Bois, « +0 or/min » sous Or (recouvre le début du titre du panneau du scénario), « +545 wood/min » sous Wood, « +545 木材/分 » sous 木材. Le bloc du héros est dans la bonne langue (Level 1 / HP, レベル 1) : l'anomalie de langue de retard du 11-14 n'est plus visible.
Question : Retours\Q-income-tooltip-taille.md (taille du texte de l'infobulle ; défaut A = état actuel).
session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -Essay income_tooltip : aucun FAIL (09:58).
NON VÉRIFIÉ : le survol avec le vrai curseur du bureau (curseur factice de Slate) ; les PNG du pré-vol 2 (09:59) non rouverts (ceux de 09:53, même scénario, l'ont été).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Recette 2026-11-15 (build fusionné 753fd00)
RECETTE PASS (RECETTE-1115a, intégrateur indépendant, 2026-11-15 11:22 ; worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog = main 753fd00, binaire de 11:08).
- `preflight.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` : PASS, 0 FAIL (check PASS 28 s, journal sans avertissement, lancement joueur OK) ; 2 WARN : pas de ligne HUD overlap, pas de ligne Sanity. `session_gate.ps1 -Phase Essay -Essay income_tooltip` : aucun FAIL (le worktree contient le tip de galadriel-tab).
- Geste relu dans le scénario de Faramir : 4 survols `hol.Mouse.Hover` (WoodAmount 30, GoldAmount 30, puis sortie `20 300 5` et retour WoodAmount 30 en EN et en JA), aucun `hol.HUD.HoverIncome`. Journal (Saved_Tests\Logs\scenario_income_tooltip.log) : « Mouse hover WoodAmount after 30 frames at 222,27: on STextBlock, tooltip shown, tip text "+545 bois/min" », « GoldAmount ... "+0 or/min" », « "+545 wood/min" », « "+545 木材/分" » ; les 2 sorties donnent « on SViewport, tooltip none ».
- PNG OUVERTS et vus (4, captures de 11:22) : income_tip_fr (une seule infobulle blanche sous Bois, « +545 bois/min ») ; income_tip_gold_fr (« +0 or/min » sous Or) ; income_tip_en (barre en anglais, « +545 wood/min » sous Wood, bloc du héros « Level 1 / HP ») ; income_tip_ja (barre en japonais, « +545 木材/分 » sous 木材, bloc du héros « レベル 1 »).
- Réserves cherchées pour faire échouer, non bloquantes : (1) le texte de l'infobulle est TRÈS petit (environ 7 px de haut à 1280x720, à la limite du lisible sur la capture) : déjà posé dans Q-income-tooltip-taille ; (2) l'infobulle de l'or recouvre le début du titre du panneau du scénario (« Scénario » en partie caché) ; (3) le chiffre +545 bois/min pour 5 Paysans n'est pas recoupé par un autre calcul (non jugé) ; (4) aucune capture en 1920x1080.
- Inchangé : GESTE réel au sens de la fiche (curseur virtuel de Slate, pas la souris du bureau), SON sans objet.
- Commande de la fiche (Galadriel) gardée : binaire de Galadriel non périmé (Get-StaleBinaryProblem vide à 11:18, dll 09:19, HEAD 85c0735). NON REJOUÉ depuis Galadriel : cette recette ne porte que sur le build de Faramir ; Galadriel ne contient pas 753fd00.
