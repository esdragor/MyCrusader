# GALADRIEL-1123pb (2026-11-23, worktree Galadriel, socle-ai) : S2-b, effets de moral du duel

Joueur : gagner un duel galvanise les troupes proches (Triomphe), le perdre les affaiblit (Demoralise) ; effet pose une fois, quelques secondes.

- Commit 1 rouge : 96bcc61 (test C++ HeroesOfLiberty.Morale.ApplyEffectAroundLaysDuelOrders + bouchon ApplyEffectAround vide). TEST_ROUGE: NON VU (build en file derriere Faramir a 15:41, jamais lance).
- Commit 2 vert : e0dc642. UHOLMoraleLibrary::ApplyEffectAround(WorldContext, EffectId, CentreX, CentreY, Slot, DurationMs, RadiusCm) met l'ordre en file ; UHOLMoraleProcessor::QueueEffectAround + drain dans Execute : l'ordre devient une source du camp au centre, dans le meme passage que le cri et la banniere (une seule regle, jamais un Paysan D705). HOLDuelLedger.as : ApplyMoraleOrders vide PendingMorale apres Tick.
- Raison C++ (D839) : processeur Mass + pont a exposer. Videur de PendingMorale en script.
- Valeurs : durees et rayon des lignes Triumph / Demoralised / Hesitation de la table (rayon 12 m) ; aucune valeur nouvelle, aucune question, aucune cvar.
- NON COMPILE : build lance 15:41, en file derriere HeroesOfLiberty-Faramir ; commande : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check.ps1 -Parts tests` du worktree. A surveiller : HOLMoraleLibrary.h ApplyEffectAround (const UObject*, nom du namespace script HOLMorale:: pour l'appel du ledger), FScopeLock (Misc/ScopeLock.h), Sources.Add({...}) aggregate.
- RELECTURE_CPP : .generated.h dernier include conserve ; file statique protegee par FCriticalSection ; commits : bouchon et correctif separes. Pas de HOLCombatProcessor / HOLHealProcessor / HOLAIBrain touches.
- RESTE : Hesitation (le ledger n'a pas la position du heros refuseur : l'ordre sans lieu est abandonne, bHasCenter = false) ; test script du videur ; mesure ; Placeholders : aucun nouveau provisoire.
- BUILDS_DEMANDES: 1 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun de plus ; EDITS_REFUSES: 0 ; DIAGNOSTICS_AS: non verifie ; PROCESSUS_DETACHES: non lance.
- socle-ai : PAS prete (e0dc642 non compile).
