# DOCS-1123ph (2026-11-23, 18:1x) : reponses aux 6 fiches Q-* (interface de choix)

Docs seulement : aucun code, aucun build, aucun worktree, copie principale. Commit 2ae5360 (sur main, local, rien pousse), derniere ligne `Agent: DOCS-1123ph`.

## Faits (reponses de l'utilisateur, 2026-11-23 18:1x, interface de choix)
1. Q-lumiere-sacree-visee : C « Le heros s'approche » (pas la reco A) : D900.
2. Q-ligne-case-bloquee (et question 3 de Q-murs-grille-fine) : B « Sauter l'obstacle » (pas la reco A, comportement code) : D901.
3. Q-commanderie-batiment : A (batiment de l'unite copiee), defaut deja code : D902.
4. Q-moral-attenuation : A (pas d'estompe), defaut deja code : D903.
5. Q-murs-grille-fine : Q1 A (palissade + bois + pierre sur la grille fine), Q2 B (prix actuel par sous-case, ligne 4 fois plus chere, pas la reco A) : D904 (Q3 = D901).
6. Q-sp3-libelles : B (« Specialisation : a choisir » / « Specialisation : Voie A (palier 1) », pas le defaut code A) : D905.

## Fait
- GDD.md : `git diff --stat GDD.md` vide avant mon edit (aucune ligne d'une autre session). D900 a D905 ajoutees (format D896-D899, reponse + effet en jeu + « a preciser » non decides) ; D898 complete par un renvoi (reponses 1, 2 et 6 de ses points « a preciser » : D904, D901) ; « Prochain numero » = D906.
- 6 fiches : ligne `Reponse: <option> (utilisateur, 2026-11-23 18:1x, interface de choix, D<n>)` sous le titre, retitrees [REPONDU], `git mv` vers Docs\Archive\Retours_2026-11-23\. Retours\ = 9 fichiers (5 E, 2 BUG, LISEZ-MOI, ESSAIS-A-JOUER).
- Journaux : une ligne dans AubeInProgress.md, FortificationsInProgress.md, SpecializationInProgress.md, MoraleInProgress.md. EtapeDefini.md : une ligne apres celle de DOCS-1123pf + un suffixe court sur les 4 lignes de la section « Fichiers de developpement » (Aube, Fortifications, Moral, Specialisation).
- Auto_Journey_File.md : 4 lignes `[ ]` en tete du socle (apres la ligne de l'apercu de visee) : D900 (ordre de marche du heros a portee puis lancer ; HOLFactionPowerServer.as TryCastPower / DecideAimed / CastOriginOf, HOLFactionAim.as, HOLPlayerOrders.as), D901 (sauter les cases bloquees ; HOLWallLineRules.as Validate « All or nothing »), D904 (cout par sous-case au prix actuel, a integrer au plan de la grille fine), D905 (libelles SP3 ; HOLSpecializationSlot.as, HOLGameScreenScript.as ~l.2052, table de textes, quick_check a grepper). Ligne de l'apercu de visee mise a jour (hors portee : l'apercu montre que le heros va s'approcher). « Confirme D902 » sur la ligne CELEBORN-1123pc, « confirme D903 » sur la ligne S2-c (moral), reponse D905 sur la ligne SP3 existante, reponses D904 / D901 sur la ligne D898.
- Placeholders.txt : ligne SP3 bouton permanent GARDEE, ajout « reponse B, D905, a coder ».
- Auto_Journey_Retours.md (en-tete et section des questions), Retours\LISEZ-MOI.md, Retours\ESSAIS-A-JOUER.md : 0 question ouverte, 0 essai en attente (« prets mais non donnes » : 2, tower_wood et specialization_slot, inchange).

## NON VERIFIE
- Aucun code lu : les noms de fichiers des lignes `[ ]` viennent de l'enonce et des fiches (HOLGameNames.cpp ou Game.po pour la table de textes : a verifier par l'agent).
- Le moment du paiement de l'Honneur (D900) et la case sautee a l'affichage (D901) ne sont pas decides : ecrits « a preciser ».
- « Si aucune case n'est libre, rien n'est pose » (D901) est une consequence, marquee comme telle.

## Anomalies
- Fichiers a fins de ligne MIXTES (GDD.md, Placeholders.txt, fiches Q en LF seul) : mon premier script a place les lignes `Reponse:` en fin de fiche (split CRLF sur du LF) ; corrige avant le commit (ligne sous le titre), verifie par `head`. Avertissements git « LF will be replaced by CRLF » sans effet.
- Non commites, pas a moi : Docs/Rapports/DOCS-1123pg.md, FARAMIR-1123b-fix.ps1, SYNC-1123pm-fin2.txt. Ce rapport non plus (consigne).
