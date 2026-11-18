# SAURON-1118p : scenarios d'essai Nouvelle partie et Statistiques (NG-7, ST-3 scenario)

Impact joueur : aucun a ce stade (fichiers de scenario seulement). Il pourra comparer l'ecran C++ d'aujourd'hui et l'ecran script de demain avec les memes gestes.

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron` (branche visual), commit unique (voir `git log -1`, sha donne dans la reponse finale ; amende une fois pour un commentaire). Branche prete, rien pousse, rien fusionne.
Fichiers (tous dans `Tools\Scenarios\`) : `ecran_nouvelle_partie.txt/.look/.json`, `ecran_statistiques.txt/.look/.json`.
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; aucun build, quick_check, preflight, lancement d'editeur (consigne).

## Commandes pour la session principale (dans cet ordre, une a la fois)

    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\preflight.ps1 ecran_nouvelle_partie -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\preflight.ps1 ecran_statistiques -TestMap -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 ecran_nouvelle_partie -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 ecran_statistiques -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

Pour un agent : `play.ps1 <nom> -Hidden -Root <worktree> [-TimeoutSec <s>]`. Avant, verifier binaire a jour (LaunchCheck `Get-StaleBinaryProblem`). `-TestMap` du pre-vol Statistiques : le `.json` est celui de `victory_win` (sans `mapSize`, repere des positions prouve sur la carte de test) ; le pre-vol (`preflight.ps1:75-76`) le refuse sinon. Dire dans la fiche E que la carte de test est voulue (positions de `victory_win`). Alternative : ajouter `"mapSize": "Small"` au `.json` et verifier les positions (non essaye).
Avant/apres : jouer les deux scenarios sur le binaire d'aujourd'hui (ecrans C++), garder les PNG (`Saved\Screenshots\HOL\ecran_np_*.png`, `ecran_stats_*.png`), rejouer apres la bascule.

## Syntaxe des etapes (modele : ecran_reglages / ecran_titre / sites_option / victory_win), verifiee
- `frames N`, `wait S`, `waitlog`, `waitunits`, `waitmap`, `waitui`, `capture`, `mark`, `note`, `waitkey`, `quit` : `HOLTestSequence.cpp:300-465` (frames 300, wait 304, waitlog 322, waitmap 326, waitunits 346, waitui 361, capture 366-371, note 399, waitkey 424, quit 465). `open /Game/Maps/MainMenu` : usage dans ecran_titre.txt.
- `hol.Mouse.Click <WidgetName|X Y> [Count]` : `HOLUIMouse.cpp:262-263` ; log `Mouse click %s x%d ...` (`HOLUIMouse.cpp:221`), donc `waitlog Mouse click <Nom>` (meme usage que sites_option.txt, ecran_reglages emploie hol.Mouse.Click). Cherche le widget dans l'ecran de menu ouvert ou le HUD.
- `hol.UI.Choose <Dropdown> <Index>` : `HOLUIOpen.cpp:186-197, 286-287` ; `UHOLDropdown::Choose` (`HOLDropdown.cpp:177-191`) ne prend PAS une valeur grisee ni la valeur deja choisie (il referme seulement la liste) ; log `Dropdown <Nom>: value N chosen` (`HOLDropdown.cpp:190`), d'ou les `waitlog`.
- `hol.UI.Open <NewGame|...|Stats>` : `HOLUIOpen.cpp:271-272` ; `Stats` : `HOLUIOpen.cpp:91-95` appelle `UHOLStatsScreen::Open` qui exige `IsOverMatch` (`HOLStatsScreen.cpp:52-57`).
- `hol.UI.Text`, `hol.UI.Shown` : `HOLUIOpen.cpp:291-297` (journal seulement, aident le diagnostic).
- `hol.Stats.SampleSeconds` : cvar `HOLMatchStatsSubsystem.cpp:20`.
- Le scenario Nouvelle partie ouvre l'ecran par un vrai clic sur `PlayButton` (comme `sites_option.txt`), pas par `hol.UI.Open NewGame`, pour rester le geste de l'utilisateur (les deux atteignent `UHOLNewGameScreen::Open`, `HOLUIOpen.cpp:60-63`).
- Etapes appliquees telles quelles de `victory_win.txt` : `hol.Cheat.*`, `hol.Camera.*`, `hol.Units.SpawnHere`, `hol.Victory.Defeat`, `waitlog Victory: slot 1 collapsed`, `waitlog End banner: Victory` (`HOLGameHUD.cpp:124`, `HOLEndBannerScreen.cpp`).

## Noms de widgets (HOLNewGameScreen.cpp, HOLStatsScreen.cpp, tous presents)
Nouvelle partie : `SizeDropdown` l.274 ; `SlotState<n>`, `SlotColor<n>`, `SlotLevel<n>` construits par `SlotWidgetName(prefix, n)` l.151, 299-307 (le plan dit `SlotState1` : meme forme, `quick_check.ps1:726` l'utilise) ; `BackButton` l.332 ; `PlayButton` du titre (sites_option.txt). Valeurs : etats 0 Ferme / 1 Humain / 2 IA (`StateNames`, l.51-55) ; tailles 0 "-", 1 Petite ... 5 Immense (`SizeNames`, l.157-163 ; `HOLMapSizeRules.cpp`: Small 2 places, Medium 4, Normal 6, Large 8, Huge 8) ; defaut place 0 Humain couleur 0, autres Fermees couleur = index (`HOLMatchConfig.cpp:24,35`).
Statistiques : `SummaryTab` l.69, `MilitaryTab` 70, `EconomyTab` 71, `TimelineTab` 72, `SeriesMilitary` et `SeriesStock` (`"Series" + SeriesNames[]`, l.48 et 98), `BackButton` l.109.
Regle de couleur : `HandleSlotStateChosen` l.1219-1225 (`IsColorTaken` 1317, `FindFreeColor` 1337) : une couleur prise saute a la suivante libre a l'OUVERTURE d'une place ; `Choose` d'une couleur deja prise est refuse (liste grisee, l.893). Le scenario fait donc : place 3 fermee garde la couleur 2, la place 2 la prend (libre), puis on ouvre la place 3 : elle saute a la couleur 3.

## Ce que j'ai ecrit
Nouvelle partie (~1 min 30 de gestes + waitkey) : titre, clic Jouer, capture ouverture ; Taille Moyenne + place 2 en IA (niveau apparait), capture ; couleur dupliquee puis ouverture de la place 3 (saut), capture ; liste Taille ouverte avec 3 joueurs (Petite grisee), capture ; 5 notes ; `waitkey` ; Retour + capture du titre. 5 captures = 5 lignes du `.look`, chaque ligne nomme l'objet attendu.
Statistiques : `victory_win` sans waitkey intermediaire ni attente utilisateur, `hol.Stats.SampleSeconds 2`, `wait 12` pour des echantillons, victoire, capture bandeau, `hol.UI.Open Stats`, onglets Resume / Militaire / Economie / Chronologie, series Armee et Stock, 4 notes, `waitkey`, Retour + capture. 8 captures = 8 lignes du `.look`.

## NON VERIFIE (rien n'a ete lance)
1. Aucun pre-vol ni lancement : les deux scenarios n'ont jamais tourne.
2. Taille par defaut de l'ecran : si c'est deja Moyenne, `Choose SizeDropdown 2` ne fait rien (volontaire, pas de waitlog dessus) ; si `Moyenne` est grisee a 1 joueur (non lu : `IsAllowed`, l.816), la suite (3 joueurs) en depend.
3. Que `hol.Mouse.Click SizeDropdown` ouvre la liste dans l'ecran C++ et que la liste ouverte est visible sur la capture (usage identique a `sites_option.txt` sur `SacredSitesDropdown`) ; que "Petite grisee a 3 joueurs" est bien le rendu (`HOLNewGameScreen.cpp:813-816`, `IsAllowed` non lu).
4. Que `hol.Mouse.Click` atteint les onglets de Statistiques pousses par-dessus le bandeau de fin (le bandeau est script, `HOLEndBannerScreen.as`) : sinon utiliser `hol.UI.Click`, mais sites_option.txt note qu'il ne declenche pas un bouton Common UI.
5. Que `hol.UI.Open Stats` fonctionne tant que le bandeau est ouvert (`IsOverMatch`, `HOLRootLayout` non lu) ; recours : `hol.Mouse.Click ContinueButton` d'abord (`HOLEndBannerScreen.as:26`) ou `hol.Mouse.Click StatsButton` (:29, bouton du bandeau, meme chemin).
6. Que `waitlog Victory: slot 1 collapsed` / `End banner: Victory` arrivent apres `wait 12` comme dans victory_win ; que 12 s donnent au moins 5 points de courbe ; que le noms `SeriesMilitary` / `SeriesStock` existent identiques dans l'ecran script de demain (a imposer a ST-2 : `AddStatsSeriesRow` doit nommer `"Series" + Workers|Military|Population|Buildings|Gathered|Stock`, onglets `SummaryTab|MilitaryTab|EconomyTab|TimelineTab`, retour `BackButton`) ; idem NG : `SizeDropdown`, `SlotState<n>`, `SlotColor<n>`, `SlotLevel<n>`, `BackButton`.
7. Duree : pas mesuree (preflight : <= 180 s ; les `wait`/`frames` des deux scenarios ~ 30-40 s hors partie de Statistiques ~ 40 s).

## Anomalies
- Rien de bloquant. Les `.look` ont des chaines sans accents (comme les modeles).
- `victory_win.json` n'a pas `mapSize` : d'ou `-TestMap` pour le pre-vol Statistiques.
- EtapeDefini.md, Auto_Journey_File.md, `*InProgress.md`, Retours\E-*.md : non touches (consigne). A faire par la session principale apres un pre-vol PASS : fiches `E-ecran-nouvelle-partie.md` et `E-ecran-statistiques.md`.
