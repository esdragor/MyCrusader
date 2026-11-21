# FARAMIR-1121sg
APPELS_DECLARES: 24 (arret a 25 vus ; heure 20:17 -> 20:22)
HEURE_DEBUT/HEURE_FIN: 20:17:39 / ~20:22
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet : la regle C++ est correcte (HOLHealProcessor.cpp:203-321 : bBoosted = talent pris (2e pick Capitaine) ET Moine a <= 1200 d'un heros/banniere du meme joueur). Le defaut vient du SCENARIO (cas b/c) ; le rouge VU est la recette 1121sb (or=0), le vert est le preflight apres correctif : pixels or 4 -> 76 (base -> boosted), vert boosted 0.
COMMITS: 1 (scenario talents_heal.txt), dernier a 20:21, merge_main_fait: non (aucun code)
TESTS: preflight talents_heal PASS (38 s) ; TESTS_TAUTOLOGIQUES: 0 (aucun test ajoute)
CAUSE: heros slot 0 en (-17535, 300) (log 'Starting hero of slot 0'), Moine et Hommes d'armes spawnes autour de (-15900, 900) : distance ~1741 > AuraRadius 1200 (HOLHeroAbilityRules.h:33). Donc bBoosted=false, HealRate sans bonus, trait vert. Le GDD/regle n'est pas en cause.
FIX: Tools\Scenarios\talents_heal.txt : -15900 900 -> -17000 450 (4 occurrences : Spawn x2, Camera.Focus x2), distance ~550.
ESSAIS: talents_heal : preflight PASS, PNG talents_heal_boosted ouvert : '++' dore visible a droite du groupe (le trait lui-meme est peu visible, camera oblique), jauge SacredLight au-dessus du bloc heros OK ; LOGIQUE SEULEMENT (GESTE interne). Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir -Hidden (utilisateur sans -Hidden)
QUESTIONS: aucune
FAIT / RESTE: fait : cause trouvee, scenario corrige, E-talents_heal.md note. Reste : EtapeDefini.md / *InProgress.md non mis a jour (budget d'appels), basculer E-talents_heal en VU A L'ECRAN a la recette suivante ; baseline 'base' a 4 px or (HUD, normal).
NON VERIFIE: le trait dore lui-meme a l'oeil (seul '++' vu) ; jeu reel ; son ; si le heros bouge, le Moine reste dans 1200 (non verifie sur toute la duree).
PROCESSUS_DETACHES: voir sortie stray
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non lance ; ecrits : ce rapport, Retours\E-talents_heal.md
