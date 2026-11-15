# Zones de furtivité (GDD § 8.2, D383) : chantier en cours

Ouvert le 2026-11-15 10:20 par ARWEN-1115c (worktree `HeroesOfLiberty-Arwen`, branche `d751-root`), relevé B3 de
`Auto_Journey_Jour.md` (« chapitres du GDD non commencés »). SANS C++ à ce stade : règle pure Angelscript et ses tests.

## Ce que dit le GDD (D383, § 8.2)

La furtivité vient, pour l'instant, de **zones dédiées posées par la carte** (ni forêt ni hautes herbes) : les unités qui
s'y tiennent sont **cachées, sauf pour un ennemi tout proche ou entré dans la zone** (comme les bosquets d'AoE4). Placement
symétrique (level design). Le Spectre Assassin n'a pas de furtivité propre, il profite des zones comme tout le monde (D396).

## Règle pure (`Script\Fog\HOLStealthRules.as`, tests `Script\Tests\HOLStealthRules_Test.as`)

Une unité **dans une zone** est cachée pour un camp qui la regarde, **sauf si** une unité de ce camp est dans la même zone ou à
moins de N (distance « tout proche », `hol.Stealth.RevealDistance`, cm, défaut 800 = 2 cases, réglage en attente de
`Retours\Q-stealth.md`). Hors zone : la règle ne cache rien (la vue normale du brouillard s'applique). La règle ne remplace
pas la vue : visible = case en vue ET pas cachée par la furtivité.
Zone provisoire = rectangle aligné (centre, demi-étendue), comme une case de bloqueur ; la forme vraie dépend de la carte.

## Étapes (15 min chacune)

1. [fait] Journal, question `Retours\Q-stealth.md` (N, cartes, aspect).
2. [en cours] Test rouge : vu à N - 1, caché à N + 1 (ennemi hors zone), vu si ennemi dans la zone loin de l'unité, jamais
   caché hors zone. Bouchon qui ne cache rien = rouge vu à l'exécution.
3. Vert : la règle.
4. (Plus tard, C++, autre agent) Branchement dans la vue par camp, voir ci-dessous.

## Ce qu'il faudra lire côté C++ ensuite (vision par camp)

- `UHOLVisionSubsystem::IsVisible(Slot, Location)` (`Source\...\HOLVisionSubsystem.h`) dit si la **case** est en vue pour un
  camp ; il faudra en plus, pour une unité en zone, les **positions des unités du camp qui regarde** (ou la distance au plus
  proche observateur de ce camp) pour appeler `HOLStealth::IsHiddenFrom`.
- `UHOLFogNetSubsystem` (réplication filtrée par camp, D518) : une unité cachée par la furtivité ne doit pas être envoyée au
  camp qui ne la voit pas (sinon un client modifié la lit), avec le délai de grâce D526.
- La texture du brouillard côté client (D525) n'a pas à changer : la case reste en vue, seule l'unité est cachée.
- L'auto-acquisition et l'IA (D519) passent par la même requête « caché pour ce camp ».
- Point d'accroche prévu (`FogInProgress.md` § 11) : champ `StealthZone` du bloqueur ; aucune carte n'en pose encore
  (`MapsInProgress.md`, après C6).

## Décompte des appels (un trait par appel)

||||||||||||
