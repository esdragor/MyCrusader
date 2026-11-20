# GLORFINDEL-1120f : cas limites de HOLFactionPowerCast::Decide, 2026-11-20

APPELS_DECLARES: 11 (arret a 25 vus par moi)
HEURE_DEBUT/HEURE_FIN: 10:37 / 10:39 (Get-Date PowerShell ; arret dur 10:58)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun quick_check)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: dee5279 (tests seuls, premier commit) ; assertit l'effet : oui. ROUGE NON VU A L'EXECUTION (aucun build ni quick_check permis) : le rouge attendu est Test_FactionPowerCastHugeCoordinates (voir ci-dessous) ; les 6 autres tests sont attendus verts des le commit 1.
COMMITS: 2 (dee5279 tests, 87608f5 correctif), dernier 10:38, merge_main_fait: oui (premier geste, `git merge main` = 37bf476, sans conflit)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (chaque valeur assertee vient de la decision calculee par Decide ; les attendus sont les codes Refusal*(), la jauge ou l'echeance donnees en entree, CooldownEndMs de la regle partagee)
ESSAIS: aucun
QUESTIONS: aucune neuve (Q-sceau-cout.md deja posee par 1120e ; defaut aucun)

FAIT:
- Fichier `Script\Tests\HOLFactionPowerCastEdge_Test.as` (worktree Glorfindel), 7 tests script (TESTS_SCRIPT_AJOUTES: 7) :
  Test_FactionPowerCastNegativeIndex (cas 1), Test_FactionPowerCastRefusalPriority (cas 2 : pas l'Aube > pouvoir absent > recharge > portee > jauge, 6 combinaisons), Test_FactionPowerCastRefusalWritesNothing (cas 3 : NewCooldownEndMs et jauge inchanges sur pas l'Aube, pouvoir absent, recharge, portee), Test_FactionPowerCastCooldownLongEnded (cas 4), Test_FactionPowerCastDistanceRounding (cas 5 : diagonale 480/640 acceptee, (800,1) refuse, (799,1) accepte), Test_FactionPowerCastNegativeDeltas (cas 6 : gauche, dessous, diagonale negative, origine non nulle), Test_FactionPowerCastHugeCoordinates (cas 7).
- BUG TROUVE (cas 7) : `Decide` calculait `DeltaX*DeltaX + DeltaY*DeltaY` en int64 ; un ecart de 2^32 - 1 cm (point choisi par le client dans le RPC) donne ~1,8e19 > 9,2e18, le carre boucle en negatif, `Math::Sqrt` d'un negatif = NaN, `CeilToInt(NaN)` ne vaut pas plus que la portee : le lancer etait ACCEPTE de n'importe ou. Correctif minimal et evident 87608f5 (HOLFactionPowerCast.as:83-86) : carres en double, distance plafonnee a 2e9 avant `CeilToInt`. Aucun comportement attendu discutable.
- Journal AubeInProgress.md, ligne EtapeDefini.md, ligne Auto_Journey_File.md mises a jour.

RESTE / NON ECRIT (depend de Q-sceau-cout ou du monde) :
- Cas 8 (jauge plafonnee a une autre valeur : depend du cout), cas 9 (recharge ecrite = CooldownMs du livre : rouge aujourd'hui, depend de la reponse), cas 10 (chaine du monde : StartLightSeal refuse, Slot != emetteur, Moine mort ; demande un monde).
- Cas 7, seconde moitie : `NowMs` proche de 2^31 (`CooldownEndMs(Now, 40000)` en int 32 bits deborde, et `CooldownEndMs - NowMs` aussi). Non teste : le comportement attendu (horloge en ms depuis le debut de partie, 24 jours avant debordement) est une decision de format, pas une regle ; a signaler seulement.
- A faire par la session principale : `Tools\quick_check.ps1 -Parts tests` dans le worktree Glorfindel (aucune DLL requise : script seul). Pour voir le rouge, lancer d'abord sur dee5279 (attendu : Test_FactionPowerCastHugeCoordinates rouge, 1 test sur 7), puis sur 87608f5 (tout vert).

NON VERIFIE: rien execute ni compile. Hypotheses : `Math::Min(double, double)` et `Math::CeilToInt(double)` existent en Angelscript (utilises sous cette forme ailleurs : `CeilToInt(Math::Sqrt(double))` etait deja dans la regle) ; `-2147483647 - 1` accepte comme int ; `CeilToInt(NaN)` n'a pas ete observe (la regle etait fausse sans lui : un carre negatif donne soit NaN soit une valeur rouge dans tous les cas).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (AubeInProgress.md, Auto_Journey_File.md, EtapeDefini.md modifies par moi ; FortificationsInProgress.md, Retours\Q-mur-ligne-longueur.md et les Docs\ non suivis viennent d'autres sessions)
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
