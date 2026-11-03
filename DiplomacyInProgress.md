# Diplomatie in progress

Chantier **Diplomatie** (mode *Diplomatie* du prototype, entre humains : GDD § 14.3, § 14.1, § 15 ; décisions D366 à D376, D385 à D387, D401, D432, D433).
Ouvert le 2026-11-03 par le test autonome (Galadriel, branche `galadriel-tab`). D483 : le neuf en Angelscript sauf Mass / simulation / réseau / rendu ; style des règles pures : `Script/Economy/HOLMarketRules.as` + `_Test.as`, lancées par `quick_check -Parts tests`.

## Ce que dit le GDD (tranché)

- **Postures (D369)** : chaque joueur règle sa posture **Allié / Neutre / Ennemi envers chaque autre** ; unilatérale, effet immédiat, **annoncée à tous**. Alliés seulement si **les deux** se disent Allié. Neutre : pas de tir automatique, ordre d'attaque possible. Propositions avec conditions : V2 (D432), après le prototype.
- **Rupture (D370)** : immédiate, à la seconde, sans préavis ni passage par Neutre ; la trahison fait partie du mode.
- **Postures qui ne concordent pas (D375)** : la plus hostile s'applique aux deux ; une seule relation par paire (Ennemi gagne sur Neutre, Neutre sur Allié).
- **Neutre dans les autres règles (D401)** : adversaire qui ne tire pas d'office (conteste une ruine, prend un relais, tient un autel, ne bâtit pas près du forum, peut défier) ; seul le tir automatique change.
- **Les IA (D367, D433)** : seules contre tous, toujours Ennemies envers tous, quelle que soit la posture prise envers elles ; la victoire alliée oblige à les éliminer.
- **Partage (D371)** : comme des coéquipiers (portes, soins, auras, remparts) **sauf la vision** : « partager ma vision » se coche envers chaque allié (unilatéral, annoncé) ; tout est coupé à la seconde à la rupture.
- **Garnison et remparts (D374)** : jamais de garnison chez un allié ; les unités sur les remparts d'un ancien allié y restent et ne peuvent plus descendre.
- **Tributs (D372)** : déjà faits (`HOLTributeRules.as`, ordre de tribut dans `main`/`galadriel-tab`, voir `MarketInProgress.md`) ; le panneau les appelle.
- **Victoire alliée (D373)** : tous les survivants alliés entre eux (alliance mutuelle) **et au moins un joueur vaincu** : victoire commune immédiate ; pas de case, pas de compte à rebours.
- **Spectateur (D376)** : en *Diplomatie*, voit toute la carte, ne parle qu'aux spectateurs.
- **Interface (D385 à D387)** : bouton en haut + raccourci, panneau par-dessus la partie, une ligne par joueur (couleur, nom, faction, vies, posture, case vision, tribut, message privé) ; en standard, postures verrouillées et grisées ; canaux Alliés / Tous / privé.

## Pas tranché (questions dans Retours)

- Posture de départ de chaque joueur envers chaque autre au lancement d'une partie *Diplomatie* (DQ-1) : provisoirement **Ennemi**, derrière `HOLDiplomacy::DefaultStance`.

## Relevé de l'existant (D0, 2026-11-03)

- **Aucune règle de diplomatie dans le code** hors tributs : `Script/Economy/HOLTributeRules.as` (Check, Received, Make, ordre dans `UHOLCommandSubsystem`).
- Camp d'un emplacement : `HOLVictoryRules::GetCamp` (`Source/HeroesOfLiberty/Match/HOLVictoryRules.cpp`) : équipe 1 à 4 telle quelle, équipe 0 = seul ; son commentaire dit déjà que c'est « l'endroit unique où les postures de D369 se brancheront ». Équipes : `FHOLMatchSlot::Team` (`HOLMatchConfig.h`, `MaxTeams` 4).
- Relation d'un joueur à un autre dans la liste du forum : `EHOLPlayerRelation` (Ally / Enemy…) dans `HOLForumRules.cpp` (vue, vies).
- Scripts qui supposent « même équipe = alliés » : `Script/Relays/HOLRelayRules.as` (`TeamBySlot`), à reprendre quand la relation devient dynamique (D6).
- Mode de jeu *Diplomatie* : absent de `FHOLMatchConfig` (option de salon à ajouter, avec D5).

## Plan (étapes de 15 min, la plus petite d'abord)

- **D0** relevé de l'existant et des décisions : **fait** (ci-dessus).
- **D1** (**faite**) règle pure en script (`Script/Diplomacy/HOLDiplomacyRules.as` + `_Test.as`) : postures Allié / Neutre / Ennemi, relation d'une paire = la plus hostile (D375), rupture immédiate et compte rendu du changement pour l'annonce (D370, D369), IA toujours Ennemies (D367), postures verrouillées hors mode *Diplomatie* ; Neutre = pas de tir d'office mais adversaire (D401).
- **D2** (**faite**) partage de la vision par allié (D371) : case unilatérale, valable seulement entre alliés, coupée à la seconde à la rupture ; garnison jamais chez un allié (D374) et remparts d'un ancien allié (les unités y restent).
- **D3** (**faite**) victoire alliée (D373) en script : survivants tous alliés entre eux et au moins un vaincu ; IA vivante empêche la victoire alliée (D367) ; le vaincu n'est plus un « survivant ».
- **D4** (**faite**) le même état en C++ (`Source/HeroesOfLiberty/Match/HOLDiplomacyRules.{h,cpp}`, comme `HOLMarketRules` : simulation et réseau en C++), test de parité avec les cas du script.
- **D5** état par partie (serveur autoritaire, répliqué à tous), option de salon *Diplomatie*, ordre « poser une posture / partager la vision » client vers serveur (file d'ordres, validation par la règle), annonce à tous.
- **D6** branchement : tir automatique (Neutre ne tire pas), `GetCamp` / victoire alliée, relais, ruine, autel, zone anti-construction, duel (D401), vision partagée au brouillard.
- **D7** panneau « Diplomatie » (D387) : écran, **pas codé sans décision de présentation** ; les canaux de discussion (D385) sont un autre chantier de réseau.
- **D8** IA : toujours Ennemie (rien à coder, un test de non-régression) ; scénario d'essai `diplomacy`, GDD (numéros par la session principale), clôture.

## Suivi

- 2026-11-03 09:2x : chantier ouvert, D0 faite (relevé), plan D1-D8, question DQ-1 posée dans `Retours/Q-diplomacy.md`.
- 2026-11-03 09:5x (Galadriel, test autonome) : **D1, D2, D3 et D4 faites, rien branché au jeu.** D1 : `Script/Diplomacy/HOLDiplomacyRules.as` (`FHOLDiplomacyBook`, `NewBook` / `NewTeamBook` (modes à équipes : verrouillé, même équipe = alliés, IA coéquipière), `Relation` = la plus hostile (D375), `SetStance` avec refus ordonnés (créneau, soi-même, posture, IA, verrouillé) et compte rendu `FHOLStanceChange` (annonce, rupture `bBroke` / formation `bFormed`, avant / après) ; `IsAlly`, `FiresAtOnce` (Ennemi seul tire d'office), `IsAdversary` (Neutre compte, D401) ; IA toujours Ennemie et sans posture (D367)). D2 : `SetVision` / `SharesVision` (case unilatérale entre alliés seulement, automatique entre coéquipiers en standard, coupée à la rupture), `CanGarrison` (jamais chez un allié, D374), `DoorsOpenFor` / `CanLeaveRampart` (portes et remparts d'un ancien allié fermés à la seconde de la rupture). D3 : `AlliedVictory` (survivants tous alliés deux à deux ET au moins un vaincu ; IA vivante bloque ; sans survivant ce n'est pas une victoire alliée ; les équipes fixes donnent la victoire d'équipe). D4 : la même règle en C++ (`Source/HeroesOfLiberty/Match/HOLDiplomacyRules.{h,cpp}`, `namespace HOLDiplomacy`), test `HeroesOfLiberty.Diplomacy.Rules` sur les mêmes cas. Tests : D1 rouge d'abord (861/870 : 9 échecs avec la coquille), vert 861/861 ; D2+D3 rouge (5 échecs) puis 866/866 ; D4 rouge constaté en neutralisant la coupure de vision à la rupture (866/867, seul `Diplomacy.Rules` en échec) puis vert 867/867 ; `quick_check -Parts tests,aiserver,aiduel` PASS (189 s). Commits sur `galadriel-tab` (pas fusionnés). **Choix provisoires (non tranchés, à confirmer)** : (a) posture de départ = Ennemi (DQ-1, `Retours/Q-diplomacy.md`) ; (b) la coupure de la vision à la rupture est définitive : si l'alliance se reforme, il faut recocher « partager ma vision » ; (c) cocher la vision envers un joueur non allié est refusé (`NotAlly`) ; (d) `AlliedVictory` compte comme « vaincu » seulement les joueurs de la liste `Defeated` (l'appelant retire les emplacements fermés du livre). **Suivant** : D5 (état par partie répliqué, option de salon *Diplomatie*, ordre client vers serveur, annonce) puis D6 (branchement au tir automatique, à `HOLVictoryRules::GetCamp`, relais, ruine, autel, brouillard, duel) ; le panneau (D7) attend une décision de présentation.