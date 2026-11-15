# Alertes mineures (D202, C1 du relevé 2026-11-15) — journal de GALADRIEL-1115f

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab` (avance rapide sur `main` a082990 à 13:18).
Question : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-alerts-minor.md` [À RÉPONDRE] (défauts codés ci-dessous).

## 1. Inventaire PAR LECTURE du code (2026-11-15 13:20)

GDD § 15, D202 : « Mineures (unité ou bâtiment attaqué ailleurs, recherche terminée, population pleine…) : fil discret sur le côté, ping et son
court propre à chaque type. » D446 : le fil (5 lignes, ~8 s, sans son). D449 : les lignes du fil restent muettes. D397 : héros prêt = alerte mineure.

Le fil = `FHOLForumThread MessageThread` (HOLGameScreen.h:550) ; toutes les lignes passent par `MessageThread.Add` dans `HOLGameScreen.cpp` (5 appels,
l. 760, 772, 781, 798, 825) ; textes dans la table unique `HOLForumMessages.cpp`.

### Présentes dans le fil
| Alerte | Texte | Où | Son | Ping mini-carte |
|---|---|---|---|---|
| Tribut reçu (D372) | « Joueur 1 vous a offert 70 or (taxe 30) » | `GetTributeMessage`, `HandleTributeMessage` l. 754 | non | non |
| Signal reçu (SG3, HALDIR-1115e) | « Joueur 2 : Attaque ici » | `GetSignalMessage`, `HandleSignalMessage` l. 765 | non | (le signal a son marqueur, hors fil) |
| Héros prêt à revenir (D397) | « Votre héros est prêt à revenir » | `GetHeroReadyMessage`, `HandleHeroReadyMessage` l. 777 | non | non |
| Niveau du héros, palier atteint (J8b-Q4) | « Paladin-Commandant : niveau N », « Palier 1 atteint » | `GetHeroLevelMessages`, `HandleHeroLevelUp` l. 786 | oui (fanfare) | non |
| (Majeures, aussi au fil) forum tombé, ruine tenue, fondations rasées, Roi mort, vaincu, a quitté | `GetMessage`, `HandleForumEvent` l. 813 | bandeau + bip (`FHOLForumAlert`) | non |

### Manquantes (exemples de D202)
- **Population pleine** : PAS dans le fil. Existe seulement (a) la note « Population pleine » de la file de production (`QueueNote`,
  HOLGameScreen.cpp:3649, montrée quand la tête de file attend) et (b) le message de commande « Population maximale — construisez des maisons »
  (`EHOLCommandFeedback::PopulationFull`, zone des messages de commande, au clic seulement). → **codée ici en règle pure** (étape 3).
- **Recherche terminée** : rien (aucun événement de fin de recherche relayé au HUD ; Grep `Research` vide hors « Déjà recherchée »).
- **Bâtiment achevé** : rien dans le fil.
- **Unité / bâtiment attaqué ailleurs** : PAS de ligne au fil ni de son ; MAIS un **ping clignotant existe sur la mini-carte**
  (`Script\UI\HOLMinimapRules.as` `Detect` : PV en baisse d'un bien du joueur → point qui clignote 6 s, fusion à 25 m ; tests
  `Script\Tests\HOLMinimapRules_Test.as`). Le relevé (« aucun ping sur la mini-carte dans le code ») est donc **à corriger** pour ce cas.
- **Unité perdue**, **production terminée**, **ressource épuisée**, **villageois inactif** : rien (exemples AoE, pas dans D202 : à trancher).
- Aucun son propre aux mineures (D202 « son court propre à chaque type ») sauf la fanfare du héros ; aucun journal (historique) du fil.

## 2. Plan (étapes de 15 min)
1. [fait] Inventaire ci-dessus.
2. [fait] Question regroupée `Retours\Q-alerts-minor.md`.
3. [en cours] `Script\UI\HOLAlertRules.as` : `ShouldAlertPopulationFull(Watch, Used, Cap, Now, RepeatSeconds)` (front montant
   pas plein → plein ; anti-répétition : pas de nouvelle ligne avant `PopulationRepeatSeconds` = 30 s, défaut de la question) +
   texte (clé existante `HOLGameScreen,PopulationFull` « Population pleine », déjà traduite). Premier test rouge :
   `Test_AlertPopulationFullOnReaching` (le stub renvoie false : vu rouge à l'exécution), puis vert.
4. [suivant, C++] Branchement (voir § 3), puis recherche terminée et bâtiment achevé sur le même modèle.

## 3. Branchement C++ qui RESTE (non fait : sans C++)
- `AHOLPlayerState::GetPopulationUsed/GetPopulationCap` (HOLPlayerState.h:83-85) ne sont pas des `UFUNCTION` : les exposer
  (`UFUNCTION(BlueprintPure)`) pour que `HOLGameScreenScript.as` les lise à chaque rafraîchissement.
- Aucun moyen pour le script d'ajouter une ligne au fil : ajouter à `UHOLGameScreen` un `UFUNCTION(BlueprintCallable) AddThreadLine(FText)`
  qui fait `MessageThread.Add(Message, RealTime); RebuildMessages();` (modèle : `HandleSignalMessage` l. 765, machine du joueur seul,
  rien pour un spectateur).
- Puis dans `HOLGameScreenScript.as` : un `FHOLPopulationAlertWatch` membre, appel de la règle au tick du HUD, `AddThreadLine` si vrai.
- Son court / ping : selon la réponse à Q-alerts-minor (défaut : ni son ni ping, D449).

## 4. Appels d'outils (un trait par appel)
||||| ||||| ||||| ||||| |||||  (25 à l'écriture de ce journal)

## 5. Resultats (13:32)
- Avant : quick_check tests en echec de compilation de script (binaire perime apres merge main : FHOLReceivedSignal inconnu) -> build du C++ de main (PASS 103 s, 13:25, aucun C++ ecrit).
- Rouge VU a l'execution 13:28 : tests 1312/1314, seuls Test_AlertPopulationFullOnReaching et Test_AlertPopulationFullRepeat (stub false).
- Vert 13:32 : regle IsPopulationFull / ShouldAlertPopulationFull / PopulationFullText ecrite, quick_check tests PASS (test + correctif dans le meme commit).
- Etape 3 FAITE. Suivant : branchement C++ du section 3 (agent C++), puis recherche terminee et batiment acheve selon la reponse a Q-alerts-minor.
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D804 (Q-alerts-minor) = B : trois lignes + son court par type + ping mini-carte (pas pour population pleine), « attaqué ailleurs » = ligne + son, 1 fois / 30 s ; question archivée ; ligne [ ] Galadriel (C++) dans Auto_Journey_File.md. Le défaut codé (A pour population pleine seulement) est donc élargi.
