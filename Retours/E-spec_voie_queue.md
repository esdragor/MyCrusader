# [À ESSAYER] spec_voie_queue - LOGIQUE SEULEMENT (preflight FAIL, ARWEN-1125i)

GESTE: réel (hol.Mouse.Click GridCell01 = vrai clic sur SCommonButton, vu au journal)
SON: non entendu
branches: spec-voie-1125m
statut: LOGIQUE SEULEMENT - pas VU À L'ÉCRAN, ne pas jouer avant correction

Essai : siège Templiers (slot 0), Chapelle posée par cheat, clic réel sur la case Z (Frère infirmier) sans voie = refus ; après `hol.Cheat.Specialization 1 1` (Foi) le même clic = mise en file.

Rejouer : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 spec_voie_queue -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`

## Vu (2026-11-25 16:55, PNG before et refused ouverts)
- Chapelle sélectionnée, grille : A Moine, W Vœu de pauvreté ; la case Z (cellule 1 = GridCell01) est VIDE : le Frère infirmier n'est pas dans la grille de la Chapelle du siège Templiers.
- Après la voie Foi (log `Specialization: slot 0 tier 1 option 1 applied`, `chosen 1 -1 -1`), `hol.HUD.Report` donne la même grille (A Moine, W Vœu de pauvreté) : toujours pas de Frère infirmier.
- Le 2e clic ne met rien en file : `waitlog Production: slot 0 queued InfirmaryBrother (1 in the queue` expire au step 50 ; capture `queued` jamais prise.

## Défaut (à corriger, pas du scénario)
Le filtre de grille `HOLFactionRules::FilterProduces` (HOLPlayerController.cpp:2303, 2340) retire le Frère infirmier (UnitTypes.csv Faction=Templars) de la grille de la Chapelle (BuildingTypes.csv Faction=Dawn) pour un siège Templiers, voie ou pas. Cause exacte non cherchée (budget d'appels) ; hypothèse : la table des factions lue au runtime n'a pas de ligne Templars (cf. HOLTemplarsBrother_Test.as, 736f246). Colonne Faction de la Chapelle non touchée.
