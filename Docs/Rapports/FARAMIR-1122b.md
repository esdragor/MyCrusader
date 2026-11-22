# FARAMIR-1122b (zone-effect, sans C++, lecture seule) : D881 cause de la marque « ++ » par-dessus l'Arbre de talents

Impact joueur : le « ++ » doré d'un soin boosté se voit net par-dessus l'Arbre de talents ; un fond opaque ne le cacherait pas, la cause est la couche (calque de debug de Slate), pas la translucidité.

APPELS_DECLARES: 20 (arrêt à 25 vus pour le brief)
HEURE_DEBUT/HEURE_FIN: 16:38:16 / ~16:42 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (lecture seule demandée) + « assertit l'effet : non »   TEST_ROUGE_SEPARE: non (aucun test écrit)
COMMITS: 1 (TalentsInProgress.md, branche zone-effect, voir git log), merge_main_fait: non (rien à fusionner, lecture seule)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: sans objet (aucun .as)
ESSAIS: aucun nouveau ; PNG relu : Saved_Tests\Screenshots\WindowsEditor\HOL\talents_heal_nocheat_cell.png (Read, « ++ » vu en x 649-673, y 404, sur « Charge disciplinée »)
QUESTIONS: Retours\Q-heal-mark-couche-2.md [À RÉPONDRE] ; valeur par défaut codée = rien de changé (option C, DrawDebugString en place, fond alpha 0,88)

FAIT / RESTE:

(1) Où la marque est dessinée et ordre par rapport à la fenêtre
- `Source\HeroesOfLiberty\Presentation\HOLHealEffectSubsystem.cpp:36-41` `DrawHeal` : ligne `DrawDebugLine(..., SDPG_Foreground, 6.f)` (ligne 40, scène) + `DrawDebugString(World, To + (0,0,PlusHeight), "++" ou "+", ..., Colour, ...)` (ligne 41). `BoostedHealColour(255, 215, 60)` ligne 18. Aucune instance 3D ni widget : c'est du texte de debug.
- Chaîne (moteur 5.4 installé lu ; le moteur « UE 5.8 HOL » du .uproject n'a pas été localisé, hypothèse : identique) : `DrawDebugHelpers.cpp:625-627` `PlayerController->MyHUD->AddDebugText(...)` -> `HUD.cpp:162` `AHUD::PostRender` appelle `DrawDebugTextList()` -> `HUD.cpp:741` `DebugCanvas->DrawItem(TextItem, ...)` -> le `DebugCanvas` est présenté par `SDebugCanvas` (`Slate\DebugCanvas.cpp:331-367`) monté dans `Slate\SGameLayerManager.cpp:159-161` au slot `EGameLayerOrder::Debug`, le DERNIER du `WindowOverlay` (après Player, Viewport, TitleBar, Tooltip). Notre interface : `HOLGameHUD.cpp:79` `Screen->AddToViewport()` (ZOrder 0) et `:103` `MenuLayout->AddToViewport(10)` vont dans la couche Player, donc SOUS le calque de debug. Aucun ZOrder de notre côté ne passe au-dessus.
- Contre-exemple de ce qui est sous l'interface : les popups « +N » de dépôt sont tracés par `AHOLGameHUD::DrawHUD` (`HOLGameHUD.cpp:215-261`, `Canvas->DrawItem`), c.-à-d. sur le canevas de scène.
- `DrawDebugString` : aucun autre usage dans `Source\` (grep).

(2) Mesure dans le PNG (System.Drawing, 1280x720)
- Pixels les plus lumineux de la marque : (255, 215, 60) en x 650 et 671, y 404 (253-255, 213-215, 60 sur le reste du trait) = EXACTEMENT `BoostedHealColour`. Atténuation nulle.
- Fond de la fenêtre : (43..53, 44..54, 50..60) ; sur le château (49,54,60) contre (166,152,136) hors fenêtre ; sur l'herbe (59,68,64) contre (139,174,96) hors fenêtre. Donc la fenêtre est bien translucide (le décor passe à ~12 %), mais la couleur du fond lue est ~ (48,52,58).
- Attendu d'une marque DERRIÈRE le fond : 0,12 x (255,215,60) + 0,88 x (48,52,58) = (73, 72, 58), gris terne. Mesuré : jaune franc. Pas de marque hors fenêtre dans ce PNG (aucun soin visible ailleurs), la comparaison se fait donc à la constante de couleur, ce qui est plus fort.

(3) Conclusion : COUCHE (preuve : couleur exacte + calque `Debug` de Slate au sommet de la pile). Translucidité : écartée comme cause de ce « ++ ». Un fond opaque (D881) ne corrigerait pas la marque ; il reste décidable pour la lisibilité seule. Q-heal-mark-couche-2 écrite (options A : tracer la marque dans `AHOLGameHUD::DrawHUD`, B : ne pas la tracer quand une fenêtre est ouverte, C : ne rien changer, recommandation A).

Si la cause avait été la translucidité (non applicable), le lot C++ de FARAMIR-1122a (variante `MakeOpaquePanel`, HOLScriptScreen.cpp:589-596, `HUDPanel.CopyWithNewOpacity(1.f)`) aurait été le plus petit ; changer `HUDPanel` (HOLUIStyles.h:23) toucherait TOUS les panneaux de script, d'où la variante dédiée. Sans objet maintenant.

Lot C++ éventuel (si réponse A, non écrit) : `HOLHealEffectSubsystem.cpp:41` remplacer `DrawDebugString` par une file de marques lue par `AHOLGameHUD::DrawHUD` (HOLGameHUD.cpp:215+, projection `ProjectWorldLocationToScreen`, `FCanvasTextItem` comme l. 257) ; test rouge à prévoir : un scénario/test qui assertit que la marque n'est pas sur `DebugCanvas` (par exemple compter les entrées `AHUD::DebugTextList` = 0 après un soin) ; lot à regrouper avec les autres besoins C++ de la séance.

Placeholders.txt : relu hors scope (aucun provisoire ajouté ni retiré).
Fichiers touchés : TalentsInProgress.md (worktree Faramir, commité), EtapeDefini.md (ligne 9), Auto_Journey_File.md (ligne 97), Retours\Q-heal-mark-couche-2.md, ce rapport. IN_USE.txt du worktree Faramir remis à « libre ».

NON VÉRIFIÉ: moteur 5.8 HOL non lu (seul le 5.4 installé : même chemin supposé) ; aucun PNG avec une marque hors fenêtre ni test avec un fond opaque (on ne prouve pas à l'exécution que le « ++ » resterait net sous un fond opaque, on le déduit de la couche) ; la marque n'a pas été relancée en jeu.
PROCESSUS_DETACHES: voir sortie en fin de séance (ligne ci-dessous)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifs de docs (EtapeDefini.md, Auto_Journey_File.md) + Retours\Q-heal-mark-couche-2.md et ce rapport non suivis, en plus des docs des autres sessions ; hors docs voulus : non
