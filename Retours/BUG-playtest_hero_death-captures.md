# BUG-playtest_hero_death-captures : les captures « fallen » et « back » sont prises avant que le HUD ne montre la mort et le retour

Découvert par : agent d'intégration n°2, 2026-11-05 21:22, main eb7b5fe (binaire 21:07), scénario `playtest_hero_death` (défaut du SCÉNARIO, pas forcément du jeu).

SCÉNARIO : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 playtest_hero_death -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
GESTE EXACT (playtest_hero_death.txt) :
1. lignes 19-23 : `hol.Hero.Kill 0`, `waitlog hol.Hero.Kill: slot 0`, `hol.Units.Spawn 6 -14500 300 1 Spearman 150`, `hol.Hero.Report`, `capture playtest_death_fallen` : aucun `frames` entre l'ordre et la capture.
2. lignes 31-33 : `hol.Hero.Return`, `waitlog Hero return: slot 0 bought for`, `capture playtest_death_back` : capture 2 images après l'achat.

VU (C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- playtest_death_fallen.png : héros VIVANT, PV 522/522, pas de délai ni de prix, pas d'ennemis ; journal : capture à 20:21:44.116, 0,2 s après le Kill.
- playtest_death_back.png : Or 5950 (achat fait), mais bloc héros « PV 0/522 », bouton « Ramener le héros — 250 or » et alerte « prêt à revenir » encore affichés, aucun héros près du forum ; journal : achat au step 74, capture au step 76.
ATTENDU (playtest_hero_death.look) : fallen = « bloc du bas affiche le délai de résurrection et le prix, 6 Lanciers ennemis arrivent » ; back = « le héros est revenu près du forum, l'or a baissé du prix ».
Piste (scénario, non appliquée : fichier de scénario non modifié par cet agent) : un `frames` (20 à 30 images) avant chacune des deux captures, comme les autres scénarios (`frames 90` dans forge.txt avant forge_tier1). Ne tranche aucun design.
NON VÉRIFIÉ : si, avec quelques images de plus, le HUD montre bien le délai, le prix et le héros revenu (le journal dit que le héros est ré-apparu à (-15935, 300) au step 74).

## Corrige (agent scenarios, 2026-11-05 21:34, commit 3cea937 sur tiers) : DEFAUT DU SCENARIO. frames 40 avant fallen (PV 0, Pret dans 0:54 vu ; le PRIX n'apparait qu'a la fin du delai : .look et note 2 corriges), frames 60 apres l'achat avant back (PV 522/522, bouton disparu, heros pres du forum vus). Preflight PASS (73 s). Reste vu : l'alerte « Votre heros est pret a revenir » (haut droite) est encore affichee apres l'achat sur playtest_death_back.png (a juger : defaut de jeu possible, non tranche).
