# ARWEN-1124c : faction_aim_free joue avec le geste REEL (suite d'ARWEN-1124b), sans C++

APPELS_DECLARES: 14 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:04:02 / 11:08 (arret dur 11:18)
PREMIER_BUILD: sans C++   BUILD: sans C++   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code, un scenario) + assertit l'effet : sans objet   TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (f483c4b, ~11:07, branche integ-37), merge_main_fait: oui (git merge main en premier geste, 1fbf4c0)
TESTS: preflight.ps1 faction_aim_free -Root <worktree> -AllowUncommitted : PASS (63 s, 0 avertissement, 0 erreur Angelscript) ; TESTS_TAUTOLOGIQUES: 0
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun (le geste reel existait deja : hol.Mouse.Hover X Y, hol.Mouse.Click X Y, hol.Mouse.Click FactionBox0). La lecture de hol.Camera.TestCursor ajoutee en c8b0ec8 dans DrawFactionAimPreview n'est plus utilisee par ce scenario (reste utile aux autres scenarios a curseur virtuel).
ESSAIS: faction_aim_free : preflight PASS, 3 PNG rouverts (aim, aim_far, after ; ready deja vu en 1124b), GESTE: reel ; STATUT VU A L'ECRAN. Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 faction_aim_free -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
QUESTIONS: aucune
FAIT / RESTE:
- Scenario Tools\Scenarios\faction_aim_free.txt : le curseur virtuel (hol.Camera.TestCursorGround / TestCursor off / hol.Select.LeftClickGround) est remplace par la vraie souris Slate : hol.Mouse.Click FactionBox0 (deja reel) arme la case ; hol.Mouse.Hover 600 380 12 (souris pres des soldats) puis capture aim ; hol.Mouse.Hover 1030 300 12 (a droite) puis capture aim_far ; retour a 600 380 ; hol.Mouse.Click 600 380 = vrai clic au sol (deplacement, appui, relache), attente du journal "Sacred Light placeholder: golden ring of 600 cm". Viewport cache 1066x600 (modele deploy).
- PNG vus : aim = cercle VERT de 6 m sous la souris, dans l'anneau creme de portee ; aim_far = cercle ORANGE a droite (coupe par le bord de l'ecran), hors anneau ; after = deux anneaux dores au point clique, Honneur 84/100, recharge 18 s, PV heros 246/450. Le rayon de l'apercu passe donc par la VRAIE position de souris (DeprojectMousePositionToWorld), ce qui etait le NON VERIFIE (1) de 1124b : leve pour la souris virtuelle de Slate.
- .look mis a jour (3 descriptions). Fiche Retours\E-faction_aim_free.md : GESTE: reel, ligne de pre-vol ajoutee. EtapeDefini.md ligne 20 mise a jour.
- RESTE : quick_check script (rouge ceacd08 puis vert 0beca84, session principale) ; sur la fiche, le geste est reel pour l'agent mais reste sur le curseur VIRTUEL de Slate (pas le curseur du bureau) : la seule difference avec l'utilisateur est la source physique de l'evenement ; HOLHero::GetHeroLocation sur un client reseau (scenario local).
NON VERIFIE: la souris physique du bureau ; la lecture de l'origine de portee sur un client reseau ; le son (-nosound, SON: non entendu inchange) ; le cercle d'aim_far est a moitie hors ecran (souris a 1030 px sur 1066) : lisible mais un point a 950 px serait plus net ; DIAGNOSTICS_AS: 0 erreur (journal du jeu au preflight, pas le serveur de langage VS Code).
PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Stop : aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen)
ANOMALIES_PROCESSUS: aucune ; mojibake de mes propres edits dans la fiche E (corrige par remplacement exact, 2 fois)
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File.md, EtapeDefini.md, Placeholders.txt, SessionEnd.txt, FortificationsInProgress.md, GATE-*, Docs/Rapports/*, Retours/*) ; aucun fichier de code ; oui
