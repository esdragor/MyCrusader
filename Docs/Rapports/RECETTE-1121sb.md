# RECETTE-1121sb (recette indépendante, main a7833ea, worktree Sauron, integ-43)

APPELS_DECLARES: 14 (aucun build, aucun code)
HEURE_DEBUT/HEURE_FIN: 20:06:59 / ~20:10 (arrêt dur 20:22)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (recette)
COMMITS: 0, merge_main_fait: non (recette)
TESTS: preflight income_tooltip PASS (40 s), preflight talents_heal PASS (43 s) ; TESTS_TAUTOLOGIQUES: 0

ESSAIS:
- income_tooltip : PASS, VU À L'ÉCRAN (GESTE réel Hover). 4 PNG ouverts : income_tip_fr (« +377 bois/min » sous le Bois), income_tip_gold_fr (« +0 or/min » sous l'Or), income_tip_en (« +377 wood/min »), income_tip_ja (« +377 木材/分 », pas « /min »). Infobulle fond sombre, bord doré, police lisible (~14 px). Le preflight n'a que 2 WARN (pas de ligne HUD overlap ni Sanity : scénario sans hol.HUD.Report/hol.Test.Sanity). Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 income_tooltip -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
- talents_heal : FAIL (trait doré). 3 PNG ouverts. OK : jauge « 0 / 100 SacredLight » au-dessus du bloc héros sans recouvrement (base avec 1 talent, boosted avec 2 talents, cell) ; fenêtre « Arbre de talents » : Boucliers levés et Mains du guérisseur dorées, aucun « ++ » ni trait dessiné par-dessus. ÉCHEC : talents_heal_boosted attend un trait de soin DORÉ (255, 215, 60) avec « ++ » après le talent Mains du guérisseur ; la capture montre un trait VERT et aucun « ++ » (comptage de pixels dans la zone 480-620 x 170-300 : or = 0 en boosted comme en base, vert 414 contre 431). Le talent pris ne change donc pas le trait à l'écran. Cause non cherchée (hors budget) : soit l'effet du talent n'atteint pas le tracé du trait de soin, soit le « ++ » est transitoire et manqué par la capture ; à trancher côté code (HOL trait de soin du Moine, lecture du talent). Statut gardé LOGIQUE SEULEMENT (GESTE: interne). Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

QUESTIONS: aucune
FAIT / RESTE: fiches E-income_tooltip.md et E-talents_heal.md mises à jour (ligne recette, commande Sauron). Reste : corriger le trait doré du Moine (talents_heal), puis rejouer ; pas de ligne écrite dans Auto_Journey_Retours.md.
NON VÉRIFIÉ: le doré au vrai jeu (geste interne seulement) ; son ; geste clavier/souris réel pour talents_heal ; taille exacte de police de l'infobulle (estimée à l'œil sur capture 1066x600). Remarque hors essai : « Inactifs : 0 » n'est pas traduit en EN ni en JA, et « Vous / IA Normal / vies » restent en français dans les captures EN/JA.
PROCESSUS_DETACHES: voir ci-dessous (sortie de stray_processes.ps1 -Stop)
ANOMALIES_PROCESSUS: aucune vue
GIT_STATUS_PRINCIPALE: non lancé ; seuls Retours\E-income_tooltip.md, Retours\E-talents_heal.md et ce rapport écrits dans la copie principale
