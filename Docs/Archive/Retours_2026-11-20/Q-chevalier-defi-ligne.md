# [À RÉPONDRE] Q-chevalier-defi-ligne : où vit le Défi du Chevalier (D859 n. 10) et sous quelle forme

Contexte : le Défi (8 s, rayon 8 m, recharge 45 s) n'est lu aujourd'hui que dans `HOLDawnValues.as` (`KnightChallengeDurationMs`, `RadiusCm`, `CooldownMs`, ligne `knight.challenge_*` de `ValueSheet.csv`, qui lit ces nombres PAR REGEX dans le `.as`). D859 demande de les porter dans une ligne `KnightChallenge` de `Tools\Data\UnitAbilities.csv` (une seule source, D839). Deux blocages : la colonne `Target` n'a que `Self` (rayon 0 par convention) et `Ground` ; et l'unité `VirtuousKnight` n'existe que sur la branche tool-known-reds (889a1cc), pas sur zone-effect ni main.

- A (recommandé) : ligne `KnightChallenge`, `Target` = `Self` avec `RadiusCm` 800 (zone centrée sur le Chevalier, la règle `IsInZone` convient) ; on assouplit la note du générateur (« 0 pour Self » devient « 0 sauf aura ») ; `HOLDawnValues` lit la ligne et la ligne `ValueSheet.csv` pointe vers le CSV.
- B : nouveau `Target` = `Aura` (une constante de plus dans `make_unit_abilities.py`), même ligne ; plus net, un peu plus de code.
- C : on laisse `HOLDawnValues.as` source unique jusqu'au lot C++ du Chevalier (aucune ligne CSV maintenant).
- Autre : à préciser.

Conséquence visible en jeu : aucune pour A, B, C avant le branchement du Chevalier ; après, le même Défi.

Valeur par défaut codée : C (état actuel, rien à changer). Une ligne à changer : la ligne `KnightChallenge` du CSV + `HOLDawnValues.as`. À faire après l'intégration de tool-known-reds (la ligne d'unité d'abord).

## Réponse de l'utilisateur (2026-11-20 ~16:0x, interface de choix)

**A** : ligne `KnightChallenge` dans `UnitAbilities.csv`, `Target` = `Self` avec `RadiusCm` 800 ; note du générateur « 0 pour Self » devient « 0 sauf aura » ; `HOLDawnValues` lit la ligne ; `ValueSheet.csv` pointe vers le CSV. Enregistré comme **D875** dans `GDD.md`.
