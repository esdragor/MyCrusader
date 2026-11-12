# ESSAIS À JOUER (nettoyage du 2026-11-12, début d'Auto_Journey)

**Prêts à donner : 0.** Aucun essai n'est rejoué sur le main du jour ; l'Auto_Journey du 2026-11-12 en produira de nouveaux (rejoués par un intégrateur indépendant, session_gate -Phase Essay).

Version précédente (essais du 2026-11-11 et du 2026-11-09, tous joués ou périmés) : `Docs\Archive\Retours_2026-11-12\ESSAIS-A-JOUER_2026-11-11.md`. Réponses du 2026-11-11 soir : `Auto_Journey_File.md`, section « Corrections et suites décidées par l'utilisateur ».

## Fiches encore dans Retours\ mais NON données (6 essais, 7 fiches E-* avec l'index audio)
- **Jamais démarrés chez toi le 2026-11-11, tous `[NE PAS JOUER]` (état au 2026-11-12 12:30, CLOTURE-1112a)** : diagnostic fait (commit 3b94faf) ; causes : joueur non nommé (`-PlayerName`, corrigé), sons candidats non commités d'Arwen, binaire périmé ; reste : recompiler la copie jouée puis `preflight.ps1 <nom> -Root <cette copie>` complet PASS. **Prêts à donner : 0.**
  - `E-legions_roster_gesture.md` (erreur étape 41 « jamais venu »).
  - `E-disconnect_1v1.md` (erreur étape 5 « jamais venu »).
  - `E-audio_compare_buildings_stone.md` et `_metal.md` (les sons ne se lancent jamais ; index : `E-audio_compare_buildings.md`).
- **SON, à écouter par toi seul** : `E-audio_panel.md` (worktree Arwen, même chaîne de sons que stone/metal : à ne donner qu'après le diagnostic des sons qui ne se lancent pas).
- **LOGIQUE SEULEMENT** : `E-playtest_hero_army.md` (preflight FAIL Sanity, spin de mêlée D641, non corrigé).
