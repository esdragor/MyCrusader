# RECETTE-1121sd (recette independante, main a4f3531, worktree Sauron)

HEURE_DEBUT/HEURE_FIN: 20:48 / 20:51
APPELS_DECLARES: 9 (arret a 25)
BUILD: aucun (sans C++, aucun code ecrit)

## talents_heal : PASS
- preflight.ps1 talents_heal -Root Sauron : PASS (40 s, no warning, sanity OK, 3 points).
- talents_heal_base.png : bloc heros (1 talent pris), jauge « 0 / 100 SacredLight » AU-DESSUS du bloc, sans recouvrement ; trait de soin vert, « + ».
- talents_heal_cell.png : Arbre de talents ouvert, Boucliers leves et Mains du guerisseur dores (pris), AUCUN « ++ » ni trait dessine sur la fenetre ; bloc heros plus haut (2 talents), jauge remonte avec lui sans le couvrir.
- talents_heal_boosted.png : « ++ » dore visible a cote du trait de soin du Moine (vers x 545-565, y 342), jauge au-dessus du bloc. Le defaut de la recette sb (trait vert) est corrige (scenario deplace, FARAMIR-1121sg).
- Statut de la fiche : reste LOGIQUE SEULEMENT (GESTE interne, SON non entendu).
- Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

## income_tooltip : PASS
- preflight.ps1 income_tooltip -Root Sauron : PASS (37 s) ; WARN seulement : pas de ligne « HUD overlap » ni « Sanity » (le scenario n'appelle pas hol.HUD.Report / hol.Test.Sanity).
- income_tip_fr : « +373 bois/min » sous le montant de bois, fond sombre bord dore.
- income_tip_gold_fr : « +0 or/min » sous le montant d'or.
- income_tip_en : « +377 wood/min » ; haut d'ecran « Idle: 0 », panneau joueurs « You » / « AI Normal 2 », « 2 lives ».
- income_tip_ja : « +377 木材/分 » ; haut d'ecran « 待機中 : 0 », panneau joueurs « あなた » / « AI ふつう 2 », « ライフ2 ».
- Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

## Anomalies (hors verdict, cosmetiques)
- income_tip_gold_fr : l'infobulle « +0 or/min » recouvre le debut du bandeau « Scenario income_tooltip » (la tooltip est posee sous le montant, le bandeau du scenario est juste en dessous).
- « SacredLight cost 25 soon » (jauge de faction) reste en anglais en FR, EN et JA.
- Bruit du preflight : 20 « Condition failed » UnifiedErrorTests.cpp sous -culture=fr (messages anglais compares), non compte.

NON VERIFIE : geste reel clavier/souris pour talents_heal ; son ; bruit des Paysans (+373 vs +377 selon l'instant).
