# Raccourcis reconfigurables (B6, GDD § 7.3 et § 15, D30, D340, D245) — chantier ouvert le 2026-11-15 par GALADRIEL-1115c

Worktree `HeroesOfLiberty-Galadriel`, branche `galadriel-tab`. Relevé B6 : `Auto_Journey_Jour.md` (« Chapitres du GDD non commencés »).

## Où en est le chantier (pour un agent neuf)

- **Fait** : voir le journal ci-dessous (dernier trait daté).
- **En cours** : voir « Journal ».
- **Question** : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-rebind.md` ([À RÉPONDRE] ; défaut codé : onglet grisé jusqu'à M4, table couvrant tous les raccourcis de D30, `HOLRebind::Scope = ScopeAll`).

## Ce que dit le GDD (sans rien trancher)

- § 7.3 (D30) : table des raccourcis par défaut, standard AoE4, « tous reconfigurables par le joueur » ; D453 retire `CTRL + chiffre` (créer/ajouter = `MAJ + chiffre`).
- § 15 (D340) : panneau de commandes 3 × 4, une touche = une case (`A Z E R / Q S D F / W X C V`, AZERTY).
- D245 : l'onglet Commandes des réglages reste grisé jusqu'à M4 (`HOLSettingsScreen.cpp` l. 246, bouton « Changer les touches » + note « pas encore dans le jeu »).
- Touches réservées retenues (non inventées) : `Escape` (menu de jeu, `HOLGameMenuRules::GetEscapeAction`), `Enter` (écrire dans la discussion, D385).

## Plan (étapes de 15 min)

1. [x] Journal + question regroupée `Q-rebind.md` (ouvrir l'onglet maintenant ou à M4 ; grille seule ou tous les raccourcis).
2. [x] Test rouge `Script\Tests\HOLRebindRules_Test.as` + bouchon `Script\Input\HOLRebindRules.as` (table D30 en données, règle qui accepte tout) ; commit ; `quick_check -Parts tests` VU ROUGE (conflit, réservée, action inconnue, restauration, périmètre).
3. [x] Règle pure : refus d'une touche tenue par une autre action, d'une touche réservée, d'une action inconnue, d'une touche vide, d'une action hors périmètre ; restaurer les valeurs par défaut ; `quick_check -Parts tests` vert ; commit.
4. [ ] (C++, M4, PAS dans ce chantier sans C++) brancher la table sur Enhanced Input (`HOLUIInputData.h`, D245) et sur la grille (`HOLCommandGrid`, touches en dur), sauvegarde de la table du joueur, onglet Commandes dégrisé selon la réponse à Q-rebind.

Premier test rouge : `Test_RebindSameKeyTwoActionsRefused` (Grid.Cell0 vers la touche de Grid.Cell1 = `Conflict`, table inchangée).

## Journal

- 2026-11-15 10:20 (GALADRIEL-1115c) : chantier ouvert, IN_USE posé (CPP: non), plan écrit.
- 2026-11-15 10:21 : test rouge f9b3e24 (7 tests ; bouchon : table D30 en données, tout changement accepté). Question `Retours\Q-rebind.md` posée (défaut A).
- 2026-11-15 10:2x : `quick_check -Parts tests` VU ROUGE : 1270/1275, KO = les 5 attendus (GridScopeLocksTheRest, ReservedKeyRefused, RestoreDefaults, SameKeyTwoActionsRefused, UnknownActionOrEmptyKeyRefused) ; DefaultsAreD30 et FreeKeyMovesTheAction verts.
- 2026-11-15 10:3x : règle écrite (`Check` : action inconnue, hors périmètre, touche vide, réservée `Escape`/`Enter`, tenue par une autre action = refus, table intacte ; `RestoreDefaults`) ; `quick_check -Parts tests` 1275/1275 PASS (151 s). Valeurs assertées relues dans la table modifiée (`ActionOf`/`KeyOf`), aucune tautologie.
- **Prochaine étape (agent neuf)** : rien de sans C++ ; attendre la réponse à Q-rebind, puis étape 4 (C++, M4). Touches au format texte (`Ctrl+Shift+1`, casse ignorée, ordre des modificateurs NON normalisé : à faire au branchement Enhanced Input).
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D819 (Q-rebind) = A : onglet Commandes grisé jusqu'à M4, puis tous les raccourcis de D30 ; défaut codé (ScopeAll), aucune ligne neuve.
