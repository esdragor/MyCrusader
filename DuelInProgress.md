# Duel de héros in progress

Chantier **Duel de héros** (GDD § 10, D14, D16, D17, D18, D35, D43, D80, D117-D120, D403, D423). Ouvert le 2026-10-31 par Auto_Journey
(Celeborn, branche `hero-j8-close`). D483 : le neuf en Angelscript sauf Mass / simulation / réseau / rendu.

## Ce que dit le GDD (tranché)

- **Déclenchement** (10.1) : un héros défie un héros adverse ; les deux vivants, ni en récupération ni en duel ; le défié **accepte ou refuse**
  (D80 : définitif, fenêtre ~5-8 s, **le silence vaut refus**) ; celui qui défie ne peut pas annuler, sa recharge de défi (2-3 min) est consommée.
  Refus = *Hésitation* (moral, 20-30 s, D16) sur les unités du refuseur autour de son héros ; l'Aube perd en plus de l'Honneur (D76, D81).
  Fenêtre de défi à l'écran sans pause (D117) ; l'IA estime ses chances (D423).
- **Arène** (10.2, D18) : sur la carte, cercle de duel ; **seuls les deux duellistes peuvent se toucher** (unités, tours, capacités, événements) ;
  la bataille continue autour, les auras restent ; un seul duel par héros, pas de 2 contre 1 ; zone : courte distance, visibles l'un de l'autre,
  **hors du rayon du forum principal de n'importe quel joueur** (D403).
- **Kit** (10.3, D14, D43, D118-D120) : attaque automatique, contact automatique (pas de déplacement libre) ; posture offensive / équilibrée /
  défensive ; parade commune + 3 capacités propres + ultime de duel au niveau 10 ; attaques fortes annoncées 0,5-1 s ; kit RTS remplacé pendant le duel.
- **Durée** (10.4) : visée 30-90 s ; temps écoulé = **pas de vainqueur**, les deux survivent, sans récompense, la recharge de défi s'applique.
- **Issue** (10.5, D35) : victoire = adverse mort (récupération) ; *Triomphe* (vainqueur) / *Démoralisé* (vaincu) 30-45 s autour du duel, bonus d'XP modéré,
  un effet de victoire propre à la faction.

## Pas tranché (voir `Auto_Journey_Retours.md`, section 2, « Duel »)

- **DU-Q1** : les deux héros tombent dans le même pas. Les deux options sont prêtes derrière `bDoubleKnockOutBothFall` (défaut A).
- **DU-Q2** : le défié se met à l'abri (entre dans le rayon d'un forum, sort de la vue) pendant la fenêtre : le défi tient-il quand même ? Non codé.
- Valeurs « indicatives, à régler en test » du GDD (fenêtre, recharge, Hésitation, Triomphe, durée max) : le jeu `HOLDuel::Preset` prend le milieu de
  chaque fourchette ; ce n'est pas une décision, juste un point de départ réglable en un endroit.

## Relevé de l'existant (D0, 2026-10-31)

- Aucun code de duel : seulement des mentions (`HOLHeroTests.cpp`, `HOLVictoryTests.cpp`, `HOLRootLayout.h`, `HOLAIArmy.as`, `HOLAIBrain.as`).
- Règles pures en script : modèle `Script/Economy/HOLMarketRules.as` + `_Test.as` (structure de réglages + espace de noms de fonctions pures,
  tests `Test_*(FUnitTest& T)` lancés par `quick_check -Parts tests`).
- Héros : `Source/HeroesOfLiberty/Hero/` (J8), états `EHOLHeroState` (Dead / ReadyToReturn...), retour et prix en script (`Script/Hero/HOLHeroReturn*.as`).

## Plan (étapes de 15 min)

- **D0** relevé (ci-dessus) : **fait**.
- **D1** état et issue en script (`Script/Duel/HOLDuelRules.as`) : phases Idle / Pending / Active / Over, `Challenge`, `Respond`, `Advance` (silence = refus,
  temps écoulé, chute d'un héros, double chute), `Effects` (Hésitation, Triomphe / Démoralisé, XP, effet de faction, morts, recharge) ; test rouge d'abord.
- **D1 faite** (c88435b), **D2, D3 faites** (bb2f235), **D3b faite** (cb86e48 : livre des duels, plusieurs duels à la fois, un par héros, recharges, fins rendues une fois, protection d'ensemble).
- **D2** conditions du défi en script (`CanChallenge` : vivants, récupération, recharge, duel en cours, ennemis, distance, visibilité, zone du forum, effet bloquant) + recharge.
- **D3** duel protégé en script (`CanHit` : seuls les deux duellistes se touchent, D18) et un seul duel par héros.
- **D4** pont C++ : état du duel dans la partie (serveur autoritaire, répliqué), commande « défier » / « répondre » client vers serveur, test de la chaîne. *Demande DU-Q2.*
- **D5** arène : cercle de duel, ciblage protégé dans le processeur de combat (test rouge sur la vraie chaîne), contact automatique des deux héros.
- **D6** kit de duel : posture, parade commune, attaques fortes annoncées (premier kit : Paladin-Commandant), puis Seigneur Damné.
- **D7** récompenses branchées (Triomphe / Démoralisé, XP, effet de faction, Hésitation sur les troupes) ; **D8** fenêtre de défi à l'écran (D117) ;
  **D9** IA (D423) ; **D10** scénario d'essai `duel`, GDD, clôture.

## Suivi

- 2026-10-31 07:4x : chapitre ouvert, D0 faite, plan D1-D10. D1 : tests écrits d'abord contre une coquille vide.
- 2026-10-31 07:4x : **D1, D2, D3 et D3b faites** : règles pures de l'état du duel (Script/Duel/HOLDuelRules.as : phases, issues, Preset au milieu des fourchettes du GDD, Challenge / Respond / Advance / Effects, CanChallenge, InDuel, CanHit, FHOLDuelBook avec Start / Answer / Tick / BookCanHit). Tests Script/Duel/HOLDuelRules_Test.as (26 cas), **rouge d'abord à chaque étape** (coquille vide : 13 en échec pour D1, 4 pour D2-D3, 6 pour D3b), puis vert ; quick_check -Parts tests 602/602 PASS. Commits c88435b (D1), bb2f235 (D2-D3), cb86e48 (D3b). **Arrêt aux règles pures** (consigne : pas de pont C++ ni d'écran sans question tranchée) ; DU-Q1 et DU-Q2 posées dans Auto_Journey_Retours.md section 2. Suite : D4 (pont C++, état répliqué, commande defier / repondre) après DU-Q2 ; D5 (arène, cible protégée dans le processeur de combat) peut partir de CanHit / BookCanHit.
