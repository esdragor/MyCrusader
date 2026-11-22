# CELEBORN-1122m : F8 T4, BuildingShotCadence « after a pause ». NON COMPILE

Joueur : apres une longue pause sans ennemi, la 2e fleche de la tour partait 50 ms trop tot (1,600 s au lieu de 1,625 s apres la 1re) ; les tirs suivants gardent la moyenne exacte de 1,625 s.
Technique : HOLCombatProcessor.cpp ReleaseBuildingArrows, branche « pas de cible » : RemainingMs = StepMs au lieu de 0. Commit cf65174 (integ-41). Aucun changement de test.

HEURE_DEBUT/HEURE_FIN: 18:47:03 / voir Get-Date de fin dans le message final (arret 18:59)
BUILD: NON COMPILE (consigne : NE COMPILE PAS). TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (le calendrier attendu est calcule dans le test depuis le pas et l'intervalle).

## Calcul (pas = 50 ms, intervalle = 1625 ms = 32,5 pas, p0 = pas de la 1re fleche)

Regle D884 : la k-ieme fleche est due k x 1625 ms apres la premiere ; elle part au premier pas ou le temps ecoule couvre k intervalles : pas = p0 + CEIL(k x 32,5) (le report de la regle, RemainingMs += intervalle, donne ceil dans le cas continu, voir ci-dessous).

| k | k x 32,5 | regle p0 + ceil | code, cas continu | code, apres pause (avant correctif) | code, apres pause (apres correctif) | test attend |
|---|---|---|---|---|---|---|
| 0 | 0 | p0 | p0 | p0 | p0 | First |
| 1 | 32,5 | p0+33 | p0+33 | p0+32 (FAUX) | p0+33 | First+33 |
| 2 | 65 | p0+65 | p0+65 | p0+64 (FAUX) | p0+65 | First+65 |
| 3 | 97,5 | p0+98 | p0+98 | p0+97 (FAUX) | p0+98 | First+98 |

Le test (HOLBuildingShotTests.cpp:79-82) : ExpectedStep = First + (K x IntervalMs + StepMs - 1) / StepMs = First + ceil(K x 1625 / 50) : 0, 33, 65, 98 (K=3 : 4875+49 = 4924 / 50 = 98). Juste.

### Code, cas continu (entree creee a R = 0, cible presente)
- pas p0 : entree neuve, R = 0 (aucun retrait), 0 <= 0, fleche, R = 0 + 1625 = 1625.
- pas p0+33 : R = 1625 - 33 x 50 = -25 <= 0, fleche, R = -25 + 1625 = 1600.
- pas p0+65 : R = 1600 - 32 x 50 = 0 <= 0, fleche, R = 1625.
- pas p0+98 : R = 1625 - 33 x 50 = -25, fleche. => p0, +33, +65, +98 = regle = test.
(Aux pas intermediaires R > 0 : p0+32 donne 1625 - 1600 = 25 > 0, pas de fleche.)

### Code, apres une pause (avant correctif)
- Pendant la pause : a chaque pas, R -= 50 (-50), pas de cible, branche l.902 : R = 0. Au dernier pas de pause R = 0.
- 1er pas avec cible p0 : l.864 retranche 50 AVANT le test : R = -50 <= 0, cible trouvee, fleche, R = -50 + 1625 = 1575 (report fantome : 50 ms qui ne se sont jamais ecoules).
- pas p0+32 : R = 1575 - 32 x 50 = -25 <= 0, fleche (le test attend 33), R = 1600.
- pas p0+64 : R = 0, fleche, R = 1625. pas p0+97 : R = -25, fleche.
- => p0, +32, +64, +97 = ceil(k x 32,5) - 1 : une avance d'un pas, et 1600 ms (< 1625) entre les fleches 0 et 1. C'est la ligne rouge du quick_tests.log : « arrow 1 left at step 32, the cadence says step 33 ».

## Qui est faux : la REGLE (le code), pas le test
D884 + la remise a zero disent « pret tout de suite » = comportement d'une entree neuve (R = 0 AU pas ou la cible entre, report 0). La branche « sans cible » mettait R = 0 puis la decrementation du pas suivant (faite avant le test) le ramenait a -StepMs : un credit d'un pas jamais vecu. Le meme defaut touche une entree neuve creee sans cible a son 1er pas.

## Correctif (HOLCombatProcessor.cpp, branche Best == INDEX_NONE)
RemainingMs = StepMs : le retrait du pas suivant ramene R a exactement 0 = pret, report 0, comme une entree neuve. Apres correctif, pause : pas p0 : R = 50 - 50 = 0, fleche, R = 1625, puis meme suite que le cas continu : p0, +33, +65, +98 = test. Pas de rafale (R reste borne a StepMs sans cible). Le report conserve apres un tir (R += intervalle) est inchange : l'intervalle moyen exact tient.

## Preuve que le cas continu n'a pas bouge
- Le cas continu n'atteint jamais la branche modifiee (elle n'est lue que sans cible en portee) : le code des pas continus est identique octet pour octet.
- Les tests du meme fichier qui le prouvent : le cas « steady fire » de BuildingShotCadence (CheckSchedule, 10 fleches sur 16 s, 10e a 9 intervalles du 1er) et « the arrows of six seconds follow the cadence » (HOLBuildingShotTests.cpp:649), cible en portee des le 1er pas.
- Autres cas a relire apres build : « two towers » (meme branche non atteinte), BuildingShot (tour sans cible = 1 entree : inchange, seule la valeur de R change).

## NON VERIFIE
- Rien compile ni lance. Hypothese : First (pas de la 1re fleche apres pause) = 0 ou 1 ; le calcul vaut quel que soit p0. Le decompte de FastIdle (5b4cc49) est suppose deja correct (il fait lire ReleaseBuildingArrows a chaque pas).
- Si quick_check montre toujours 32 apres build, la cause serait ailleurs (p. ex. la pause sautant des pas : FastIdle), a mesurer par BSHOT.

PROCESSUS_DETACHES: aucun lance (aucun editeur ni build). ANOMALIES_PROCESSUS: aucune. APPELS vus: ~15.
