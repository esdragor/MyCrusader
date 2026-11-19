# SAURON-1119f : H-7 file de production, retrait C++, build, tests (2026-11-19)

Branche envelope, worktree HeroesOfLiberty-Sauron. Pas de push, pas de merge.

## Commits
- bc80867 : retrait du C++ sans appelant (HOLGameScreen.cpp -98, HOLGameScreen.h -33 = 131 lignes supprimees).
- (suivant) : correction script HOLQueuePanel.as ligne 260.

## Retire
.cpp : RefreshQueue, UpdateHead, HandleQueueItemClicked, QueueItemName, constexpr QueueItemWidth.
.h : NumQueueItems, EHeadState, RefreshQueue, UpdateHead, HandleQueueItemClicked, QueueItems, QueueProgress, QueueNote, bQueueShown, ShownQueueTypes, HeadState, HeadProgress.
GARDE : QueueItemHeight (lu par GroupsHeight, HOLGameScreen.cpp:2564), QueuePanel (BindWidgetOptional, le script le remplit), ReadQueue (la liaison, copie propre de la lecture, ne depend d'aucun symbole retire). Les grep restants ne sont que des commentaires et les noms de parametres de ReadQueue.

## Erreurs rencontrees
1. Build : aucune erreur (PASS 61 s, premier essai).
2. Scripts : `HOLQueuePanel.as (260:48): No matching signatures to 'UHOLQueueItem::GetFName()'` -> `GetName()` (renvoie un FName en script, comme ailleurs dans Script\).
3. Tests : apres correction 1588/1588 (1580 + 8 attendus, total reel 1588).

## Rouge vu (SAURON-1119d (f), les deux ensemble)
- ItemsShown renvoie QueueCount (sans plafond) + HeadState sans le test `Progress >= 1.0` : quick_check 1586/1588 a 01:41, rouges `Test_QueuePanelItemNamesAndCountAreTheCppOnes` et `Test_QueuePanelHeadStateIsUpdateHeadsRule` (exactement les deux attendus).
- Remis par edition inverse, vert 1588/1588 a 01:44 ; git diff = la seule correction GetName.

## Non verifie
Rendu en jeu (libelles des boutons de file, barre de tete, clic annuler, scenario avant/apres), quick_check parties building_queue, hud, lobby : non lances (consigne : `-Parts tests` seulement).

BUILDS_DEMANDES: 1 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (les 8 de 799be7c tournent) ; LIGNES_CPP_SUPPRIMEES: 131 ; TEST_ROUGE: vu a 01:41 : 1586/1588 ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: voir stray_processes.
