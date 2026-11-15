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
