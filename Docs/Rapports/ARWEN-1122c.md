# ARWEN-1122c : furtivité, unités du joueur local à demi transparentes en zone (C++ liaison + script)

NON COMPILÉ. Impact joueur : ses propres unités dans un bosquet de la carte Forest passent à 0.5 d'opacité, à 1.0 en sortant (D824 Q3 = B) ; rien ne change sur une carte sans zone.

APPELS_DECLARES: ~24 (arrêt à 25 vus ; premier Edit tardif : après le 8e appel, lectures de la liaison nécessaires avant)
HEURE_DEBUT/HEURE_FIN: 16:59 / 17:02 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (pas de build dans cette séance)
TEST_ROUGE: 7314bed (stub qui liste 0 unité + test C++ Tests\HOLStealthOwnUnitsTests.cpp) ; rouge NON VU à l'exécution (pas de build) ; assertit l'effet : oui (vraie simulation Mass, ids et positions lus)   TEST_ROUGE_SEPARE: oui (7314bed puis 4ee4116)
COMMITS: 2 (+ la fusion integ-49 925ed88), dernier 17:02, merge_main_fait: non (main n'est pas ancêtre ; fusion de main non faite, DLL du worktree non vérifiée)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (les ids sont comparés à FHOLUnitStateFragment::UnitId lu sur l'entité, les positions à celles posées sur le Transform ; le joueur adverse doit être absent)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : 2 fichiers neufs Presentation\HOLStealthOwnUnits.h/.cpp + Tests\HOLStealthOwnUnitsTests.cpp)
TESTS_SCRIPT_AJOUTES: 0 (le test est en C++ : simulation Mass, D838)
TYPES_A_EXPOSER: UHOLStealthOwnUnitsLibrary (fait ici) ; côté script l'espace de noms est HOLStealthOwnUnits:: (le plugin enlève U et Library, comme HOLUnitOpacity:: pour UHOLUnitOpacityLibrary)

FAIT
- Liaison (Source\HeroesOfLiberty\Presentation\HOLStealthOwnUnits.h/.cpp), UFUNCTION statiques, monde passé en `const UObject*` (pas de meta WorldContext, piège 7) :
  - `GetPlayerUnits(WorldObject, int32 Slot, TArray<int32>& OutUnitIds, TArray<FVector2D>& OutPositions)` : serveur / solo = UHOLUnitSubsystem::ForEachLivingUnitWithType (clé = State.UnitId, assignée HOLUnitSubsystem.cpp:771-773, converti int32) filtré sur Owner ; client (UHOLVisionSubsystem::IsClient) = UHOLUnitReceptionSubsystem::GetDisplayedUnits (View.UnitId, View.Location), comme UpdateClient du brouillard.
  - `GetLocalPlayerSlot(WorldObject)` : AHOLPlayerState::GetPlayerIndex du premier contrôleur, INDEX_NONE sinon.
- Script (Script\Fog\HOLStealthFilter.as) : l'installateur garde `Zones`, puis `Tick` appelle `ApplyOwnUnitOpacity()` toutes les 0.05 s (cadence du brouillard client, UHOLVisionSubsystem::Tick l.~625), sans lire d'unité si `Zones.Num() == 0` ; HOLStealthApply::ApplyOwnUnits (inchangé) pose 0.5 en zone et 1.0 dehors (1.0 oublie l'unité dans la table). Commentaire du point d'appel mis à jour dans HOLStealthOpacityApply.as.
- Coût : une lecture par cadence. Ordre de grandeur pour 2000 unités (toutes au joueur, pire cas) : parcours de la table + 2 lectures de fragment Mass par unité ~ 0.1 à 0.2 ms en C++, puis ~2000 appels script (SetStealthOpacity avec verrou + règle sur 4 zones) ~ 1 à 3 ms ; à 20 Hz soit ~ 2 à 6 % d'un thread de jeu à 60 i/s. ESTIMATION, non mesurée. Un joueur n'a en pratique que sa part des unités : bien moins. Aucune zone : zéro.

RESTE
- Build (lot C++), lancer `HeroesOfLiberty.Stealth.OwnUnits.OnlyThePlayersUnits` : le rouge est attendu sur 7314bed (0 unité listée) et le vert sur 4ee4116.
- Charger les .as (pièges 1 à 9 relus : pas de `Cast` comme nom, pas de conteneur imbriqué, fonctions en namespace, appel qualifié HOLStealthApply::).
- Les unités mortes ou sorties gardent leur entrée d'opacité dans la table (inoffensif : id jamais recyclé ; nettoyée par ClearStealthOpacities en fin de match si appelé).
- Essai en jeu sur la carte Forest (zones), non fait.

NON VÉRIFIÉ
- Rien n'est compilé ni lancé : signatures UHT, noms AS (HOLStealthOwnUnits::), passage de `this` (UScriptWorldSubsystem) en `const UObject*`, `UWorld::GetWorld()` sur l'objet monde du test.
- Que l'id lu côté client (View.UnitId) soit bien le State.UnitId que le dessin lit chez le client (le serveur : confirmé par le code, UnitId du fragment = clé de la table).
- Le rendu : que HOLUnitOpacity (ARWEN-1122b) lise bien l'id converti en int32 pour l'unité dessinée.
- Que le test place bien des Peasant sans UHOLSimulationSubsystem (le test de brouillard le vérifie aussi IsReady avec lui : si Place renvoie 0, ajouter ce sous-système à IsReady).

RELECTURE_CPP (L20-2) : relu après écriture : includes (Engine/World.h, PlayerController, Vision, PlayerState, Reception, UnitSubsystem, UnitView), FHOLUnitSpawnRequest / RequestUnits / ServeBirthRequests / GetAllUnitIds / FindEntity publics (HOLUnitSubsystem.h:41,91,94,135,147), IsClient() public (HOLVisionSubsystem.h:99), `Slot > MAX_uint8` comparé en int32 : ok, lambda sans capture inutile ; le test inclut HOLSimulationFragments.h pour FHOLUnitStateFragment. DIAGNOSTICS_AS: non relus (serveur de langage indisponible dans cette séance).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune. Premier Edit après le 8e appel (consigne non tenue : la liaison C++ à exposer demandait de lire HOLVisionSubsystem, HOLUnitSubsystem et le modèle de test avant d'écrire).
GIT_STATUS_PRINCIPALE: modifs de docs seulement (Auto_Journey_File.md, EtapeDefini.md, StealthInProgress.md = mes 3 lignes ; le reste préexistait) ; oui.
IN_USE.txt : rendu `libre : ARWEN-1122c ...`, CPP: non.

Commits (branche integ-37, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen) : 925ed88 fusion integ-49 ; 7314bed rouge (stub + test) ; 4ee4116 vert (liaison + installateur).
