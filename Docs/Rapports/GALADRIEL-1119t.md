# GALADRIEL-1119t : port en script de HOLDuelOrderTests.cpp et HOLHeroTalentOrderTests.cpp (reprise 14, 2026-11-19)

Worktree : C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel (branche port-tests). Aucun C++ ecrit, aucun build, aucun test lance : NON VERIFIE (le script n'a jamais ete compile ni execute, ni vu rouge).
Aucun test C++ supprime.

## 1. HOLHeroTalentOrderTests.cpp -> Script\Tests\HOLHeroTalentOrder_Test.as (commit fd31374, 4 tests script)

| Test C++ | Test script | Statut |
|---|---|---|
| HeroesOfLiberty.HeroTalent.Orders (chaine : heros ne, XP, ordres, feuille, ordres hors slot) | Test_HeroTalentOrdersRefusalByLevel (niveau 1 sans point, niveau 3 : famille hors plage -1 et 3, famille 1 deux fois, troisieme refuse) | PARTIEL : la regle de refus serveur (HOLHero::CanPickTalent, C++) seule |
| idem, partie haut niveau (5 points, sixieme refuse) | Test_HeroTalentOrdersTopLevelFivePoints | PARTIEL, meme limite |
| idem, sceaux (bit 1) et cle apres 3 et 5 choix de la famille 1 | Test_HeroTalentOrdersSealAndKey | PARTIEL : seuils HOLTalent (script), pas le bit de la feuille C++ |
| HeroesOfLiberty.HeroTalent.EffectTable (3 effets, pas de ligne 1, pas de famille 3, table = code ligne par ligne) | Test_HeroTalentEffectTable (valeurs attendues ligne par ligne, 3 effets, ligne 1, famille 3) | PARTIEL : la comparaison avec GrantsAuraArmor / GrantsHealBonus / GrantsDawnStep reste en C++ |

Reste en C++ (a ne PAS supprimer) : la chaine de HeroTalent.Orders (ordre -> file -> sous-systeme de heros -> feuille, ordres hors slot ignores) et la moitie « table = code » d'EffectTable.
Liaisons manquantes pour tout porter :
- `UHOLUnitSubsystem::RequestUnits(const FHOLUnitSpawnRequest&)` + `ServeBirthRequests()` (+ struct `FHOLUnitSpawnRequest` : TypeName, Count, OwnerPlayer, Center) : naissance d'un heros (passe par Mass : D838 pourrait le laisser en C++).
- `UHOLSimulationSubsystem::GetOnStepEnded()` / une aide `BroadcastStepEnded(int64)` pour avancer un pas sans Mass.
- `UHOLHeroSubsystem::BindSteps()`, `GetSheets(int32) const` (FHOLHeroSheets : Public.State/Level/TalentSeals/TalentKeys, Private.TalentPicks), `SetXP(int32, int32)`.
- `UHOLCommandSubsystem::IssueHeroTalentOrder(int32 PlayerIndex, int32 Family)` et `ConsumeHeroTalentOrders(TArray<FHOLHeroTalentOrder>&)`.
- `HOLHeroTalent::GrantsAuraArmor / GrantsHealBonus / GrantsDawnStep(const TArray<uint8>& Picks)` (fonctions inline de HOLHeroSheet.h) : p. ex. trois statiques dans UHOLHeroLibrary.

## 2. HOLDuelOrderTests.cpp -> RIEN PORTE (aucun fichier script, aucun commit)

| Test C++ | Decision |
|---|---|
| HeroesOfLiberty.Duel.OrdersInMatch (6 heros nes, ordres par la file, feuilles, copies GameState, cercle, livre) | LAISSE EN C++ : test de chaine qui exige des heros reels (Mass) et des fonctions non exposees. La moitie « livre de duels » est deja couverte en script (voir ci-dessous) ; la ré-ecrire serait un doublon, pas un port de la chaine. |
| HeroesOfLiberty.Duel.ClearCircleRule (HOLDuelClear::IsInside, GetTarget, MarginCm) | LAISSE EN C++ : regle inline C++ (Simulation\HOLDuelClearRules.h), utilisee seulement par UHOLHeroSubsystem.cpp:423-436 (code par pas) ; aucun jumeau script, la porter = ecrire un jumeau (interdit D839). |

Couverture script deja presente des cas de OrdersInMatch (livre) : trop loin = Test_DuelCommandConditions ; defi envoye / recu / tiers rien = Test_DuelCommandView ; acceptation du defieur sans effet = Test_DuelBookAnswer ; acceptation = Test_DuelBookAnswer + Test_DuelBookProtectedHits ; refus + cooldown du defi = Test_DuelBookRefusesWhatItKnows, Test_DuelRefusalCosts ; silence = Test_DuelBookSilenceInTick, Test_DuelSilenceIsARefusal.

Liaisons manquantes pour porter la chaine :
- `UHOLCommandSubsystem::IssueDuelOrder(int32 PlayerIndex, EHOLDuelOrderKind Kind, int32 OtherSlot)` (et `EHOLDuelOrderKind` en UENUM), `ConsumeDuelOrders(TArray<FHOLDuelOrder>&)`.
- `UHOLDuelSubsystem::Find(UWorld*)`, `CanHitNow(int32, int32)`, `GetAdversaryNow(int32)`, `GetNumEnded() const`.
- `AHOLGameState::GetHeroSheet(int32 Slot) const` renvoyant la struct FHOLHeroSheet (DuelCode, DuelOpponent, DuelEndStep, DuelRadius, DuelCenter) : les lecteurs `HOLHero::GetHeroDuelCode / Opponent / EndStep / Arena` existent deja.
- Les memes que pour les talents : naissance de heros, pas de simulation, BindSteps, GetSheets.
- ClearCircle : `HOLDuelClear::IsInside(FVector2D, FVector2D, double)`, `GetTarget(...)`, `MarginCm` (p. ex. une bibliotheque `UHOLDuelClearLibrary`) ; seulement utile si la regle doit aussi vivre cote script.

## NON VERIFIE
- Le fichier HOLHeroTalentOrder_Test.as n'a pas ete compile ni lance ; rouge non vu. Commande a lancer par la session principale : quick_check sur les tests script (partie Angelscript) ; puis fausser une valeur attendue (p. ex. `6` du niveau max) pour voir le rouge.
- Les noms des fonctions utilisees (HOLHero::CanPickTalent, GetLevelXP, GetMaxLevel, TalentHasEffect ; HOLTalent::HasSeal, HasKey, FirstRow, LastRow, FamilyCount) sont lus dans les sources, pas executes.
- Test_HeroTalentOrdersSealAndKey verifie les seuils du script (HOLTalent), pas le bit ecrit par le C++ dans la feuille.

## Anomalies
- Un test « de monde sans Mass » comme HeroTalent.Orders et Duel.OrdersInMatch fait naitre ses heros par UHOLUnitSubsystem (Mass) : ils ne sont pas portables sans les liaisons ci-dessus, et D838 pourrait les ranger avec la simulation Mass.
- Le texte du CSV du test C++ EffectTable contient des accents mal encodes (lu en CP1252) : ASCII utilise cote script.
