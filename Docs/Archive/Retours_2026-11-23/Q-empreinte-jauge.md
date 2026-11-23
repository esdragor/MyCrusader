# [À RÉPONDRE] La jauge d'Honneur et les recharges de pouvoirs doivent-elles entrer dans l'empreinte de la simulation ?

Auteur : GALADRIEL-1123c (lecture seule du code de main, 2026-11-23). Reste « à décider » du socle commun des factions (`Auto_Journey_File.md`).

## Contexte (2 lignes)
L'empreinte de la simulation (`HashUnits`) est un résumé chiffré de l'état de toutes les unités, calculé à chaque pas : elle sert à vérifier que deux parties identiques donnent exactement le même déroulement. La jauge d'Honneur de l'Aube (`AHOLGameState::FactionResource`) et les heures de fin de recharge des pouvoirs (`PowerCooldownEndMs`) vivent dans l'état de la partie, hors des unités : un écart sur ces deux valeurs ne serait donc pas vu.

## Ce que le code fait aujourd'hui (relevé, rien n'est modifié)
- `HashUnits` : `Source\HeroesOfLiberty\Simulation\HOLSimulationSubsystem.cpp:62`, FNV-1a sur les unités vivantes (position, orientation, vitesse, force, cible de déplacement), triées par identifiant. Les parties combat / fantôme / pilotage / ordres / poignées sont dans `FPartHashes` (l. 45), journalisées à côté seulement si `hol.Simulation.StepHashes >= 2`.
- Qui la lit : uniquement le journal (`Step hash: step N <hex>`, l. 387 et 403, désactivé par défaut : `hol.Simulation.StepHashes 0`) et la partie `aiduel` de `Tools\Tests\quick_check.ps1` (l. 7683 à 7783 : deux parties IA identiques de 2 min, comparaison pas à pas des empreintes). Je n'ai trouvé AUCUNE détection de désynchronisation réseau ni relecture qui lise l'empreinte : jouer en réseau n'en dépend pas.
- Où la jauge et les recharges changent : `SetFactionResource` / `SetPowerCooldownEndMs` (`HOLGameState.cpp:292` et `:313`), appelés par script par les récompenses de duel (`Script\Duel\HOLDuelRewards.as:161`), le triche `hol.Cheat.Honor` (`Script\Factions\HOLHonorCheat.as:31`) et le lancement d'un pouvoir (`Script\Net\HOLRpcPlayer.as:1077-1091`). Ces écritures ont lieu sur commande d'un joueur ou d'un duel, pas dans un processeur Mass.
- Précédent : une valeur d'unité rare (`RageMs`) n'entre dans l'empreinte que si elle est non nulle (l. 153), pour que les anciennes empreintes de référence ne changent pas.

## Options
**A. Ajouter la jauge et les recharges à l'empreinte principale (`HashUnits`).**
- En jeu : rien de visible. Côté outils : la partie `aiduel` détecterait aussi un écart d'Honneur ou de recharge entre deux parties identiques. Risque : si une écriture de jauge dépend de l'heure réelle ou de l'ordre d'une commande plutôt que du pas de simulation, `aiduel` devient rouge sans bug réel (faux rouge à démêler). Toute empreinte de référence déjà enregistrée change pour les parties où la jauge n'est pas nulle (en ne mélangeant que le non nul, les parties sans Honneur gardent la même empreinte).
- Coût : environ 15 lignes de C++ dans `HOLSimulationSubsystem.cpp` (lire `AHOLGameState` depuis le monde, parcourir les emplacements) + un test + un build ; un seul fichier, dans le lot C++ de la séance.

**B. Empreinte séparée (une partie « faction » dans `FPartHashes`, journalisée à côté à `StepHashes >= 2`).**
- En jeu : rien de visible. L'empreinte principale et `aiduel` ne changent pas : aucun faux rouge possible. Quand deux journaux diffèrent, la colonne `faction` dit si l'écart vient de la jauge ou des recharges. Contrepartie : `aiduel` ne compare pas cette colonne tant qu'on ne l'ajoute pas à son motif de lecture.
- Coût : même ordre que A (environ 15 lignes C++ + un test + un build), un peu plus la ligne du journal ; `quick_check.ps1` à adapter seulement si on veut que `aiduel` la compare.

**C. Ne rien faire.**
- En jeu : rien. Aucun code ne lit l'empreinte pour la jauge ; l'Honneur et les recharges sont répliqués à tous les clients et écrits par un seul endroit (l'autorité), donc l'absence de l'empreinte ne se voit pas tant que la jauge ne change pas d'unité. Risque : un bug de jauge non déterministe (deux parties identiques, Honneur différent) passerait inaperçu des tests automatiques.
- Coût : zéro.

**Autre** : une idée à toi (par exemple : la jauge entre dans l'empreinte seulement quand l'effet d'un pouvoir existe, `bEffectCoded`, voir `HOLAIPowerSpend`).

## Recommandation
B, à faire plus tard dans le lot C++ du socle : on gagne un outil de diagnostic sans toucher à l'empreinte de référence ni risquer un faux rouge sur `aiduel`. À reconsidérer vers A quand un pouvoir de faction aura un effet réel sur les unités (aujourd'hui aucun, la jauge ne change aucune unité).

## Valeur par défaut tant que tu n'as pas répondu
C (ne rien changer) : aucun code, empreinte et références inchangées. La ligne du socle reste `[?]` dans `Auto_Journey_File.md`. Si tu choisis B ou A : un seul réglage de code, pas de cvar nécessaire pour B (la colonne ne sort qu'avec `hol.Simulation.StepHashes >= 2`) ; pour A, la lecture de la jauge peut se placer derrière un cvar (`hol.Simulation.HashFaction 0/1`, défaut 0) afin qu'on retire le mélange d'une ligne en cas de faux rouge.

## Fichiers concernés
- `Source\HeroesOfLiberty\Simulation\HOLSimulationSubsystem.cpp` : `FPartHashes` (l. 45), `HashUnits` (l. 62), journaux (l. 382 à 405).
- `Source\HeroesOfLiberty\Match\HOLGameState.h` / `.cpp` : `FactionResource`, `PowerCooldownEndMs` (accesseurs l. 287 à 319 du .cpp).
- `Tools\Tests\quick_check.ps1` : partie `aiduel` (l. 7660 à 7783), seulement pour l'option « comparer la colonne faction ».
- Écrivains de la jauge : `Script\Duel\HOLDuelRewards.as`, `Script\Factions\HOLHonorCheat.as`, `Script\Net\HOLRpcPlayer.as`.

## Non vérifié
Je n'ai pas lu le moment exact (pas de simulation ou temps réel) où `PowerCooldownEndMs` compare son heure de fin : si c'est l'horloge réelle, A produirait des faux rouges et seule B/C reste sûre. Rien n'a été compilé ni lancé.
