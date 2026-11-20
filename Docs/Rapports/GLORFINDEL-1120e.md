# GLORFINDEL-1120e : question du coût du Sceau de lumière + cas limites non testés de HOLFactionPowerCast, 2026-11-20

APPELS_DECLARES: 8 (arrêt à 25 vus par moi)
HEURE_DEBUT/HEURE_FIN: 10:30 / ~10:36 (Get-Date PowerShell ; arrêt dur 10:48)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, aucun code édité)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code, aucune édition de test) ; assertit l'effet : sans objet
COMMITS: 0, merge_main_fait: non (premier geste = `git merge integ-26` fait dans le worktree Glorfindel, branche ability-columns, sans conflit, 12 fichiers dont HOLFactionPowerCast.as et son test)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (aucun test écrit)
ESSAIS: aucun
QUESTIONS: `C:\Users\baudo\Documents\MyCrusader\Retours\Q-sceau-cout.md` [À RÉPONDRE], 1 question (A coût 25 / B coût 40 / C aucun Honneur, capacité de Moine) ; recommandation C ; valeur par défaut codée : aucune.

FAIT / RESTE:
- Question posée. Constat vérifié dans le code : `HOLFactionPowerCast::Decide` paie le coût de `Rows[PowerIndex].Cost` ; la seule ligne du livre (`HOLDawnPowers.as`) est SacredLight (coût 25, recharge 20000 ms). Le Sceau passe donc par l'indice 0 : il coûte 25 par accident et sa recharge est `SealCooldownMs()` = 40000 (HOLFactionPowerCast.as:31), la recharge du livre (20 s) n'est jamais lue pour lui. D859 (GDD.md:3676) fixe 25 / 20 s pour la Lumière sacrée et 40 s de recharge pour le Sceau, sans coût ; D687 (GDD.md:3340) dit « capacité active du Moine, cadre de capacités des Légions ».
- Autre écart vu : l'indice 0 est « Lumière sacrée » ; si la ligne 0 devient un jour le vrai soin de zone (bEffectCoded), le bouton Sceau le lancerait sous son nom. Le correctif dépend de la réponse (A/B : ligne `LightSeal` dans le livre ; C : sortir le Sceau de `Decide`).

Cas limites NON TESTÉS de `HOLFactionPowerCast::Decide` (un seul test suffit par cas ; rien corrigé). Tests existants (`HOLFactionPowerCast_Test.as`) : chaîne acceptée puis recharge (Test_FactionPowerCastChain), portée +1 cm, jauge vide / un Honneur de moins / exact, pas l'Aube, pouvoir hors livre (Test_FactionPowerCastRefusals).
1. Indice négatif (`PowerIndex = -1`) : la garde `PowerIndex < 0` n'est jamais exercée (seul `Rows.Num()` l'est).
2. Priorité des refus, jamais testée quand deux causes coexistent : (a) pas l'Aube + indice invalide -> doit dire « pas l'Aube » ; (b) recharge + hors portée -> recharge ; (c) hors portée + jauge vide -> portée ; (d) recharge + jauge vide -> recharge.
3. Refus qui ne doit rien écrire : `NewCooldownEndMs` inchangé sur chaque refus (seule `NewGaugeValue` est assertée, et seulement pour recharge, portée, jauge) ; jauge inchangée pour « pas l'Aube » et « pouvoir absent ».
4. Recharge déjà écoulée depuis longtemps (`CooldownEndMs < NowMs`, reste négatif ramené à 0 par `Math::Max`) : seuls 0, fin - 1 et fin exacte sont testés.
5. Arrondi de distance (rapport FARAMIR : « distance arrondie au-dessus ») : seul un point sur l'axe X est testé. Manquent : diagonale exacte 3-4-5 à 800 cm (480, 640) acceptée ; (800, 1) = 800,0006 cm arrondi à 801 donc refusé (c'est ce test qui prouve le `CeilToInt`).
6. Point à gauche / en dessous de l'origine (deltas négatifs) et origine non nulle avec point sur le cercle de portée côté négatif.
7. Grandes coordonnées (écart > 2^31 cm si la carte en admet ; calcul en int64 prévu) : refus de portée, jamais d'acceptation par débordement ; et `NowMs` proche de 2^31 : `CooldownEndMs(Now, 40000)` en int peut déborder.
8. Valeur de jauge supérieure au coût (pleine, 100) puis dépense, jamais en dessous de 0 : testé seulement à travers `Before - Cost` ; pas de cas « jauge plafonnée à une autre valeur de plafond ».
9. Recharge écrite : le test lit `SealCooldownMs()` pour calculer l'attendu (même source), il ne détecte donc pas l'écart de 20 s (livre) contre 40 s (règle) ; un test « la recharge écrite égale la `CooldownMs` de la ligne du pouvoir » est rouge aujourd'hui et ne devrait être écrit qu'après la réponse à `Q-sceau-cout`.
10. Chaîne du monde non couverte (signalée par FARAMIR, restée vraie) : `StartLightSeal` refuse (la jauge ne doit alors pas être payée), `ServerCastFactionPower` avec `Slot != émetteur`, Moine mort ou absent (le point d'origine vient de `GetHeroBannerLocation`).

NON VÉRIFIÉ: rien exécuté ni compilé ; lecture seule de `HOLFactionPowerCast.as`, `HOLDawnPowers.as`, du test et des décisions D687, D859 du GDD ; les numéros de ligne GDD viennent de `Select-String` dans la copie principale (peuvent bouger). Le nombre « 4 lancers / 2 lancers » de la question est (100 / 25) et (100 / 40) arrondi par le bas, jauge pleine sans gain.
PROCESSUS_DETACHES: aucun lancé (stray_processes non utile, aucun processus démarré)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchangé hors docs voulus : oui (seulement `Retours\Q-sceau-cout.md`, ce rapport, et mes deux lignes de suivi)
