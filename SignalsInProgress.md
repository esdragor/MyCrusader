# SignalsInProgress.md : les signaux sur la carte (GDD § 15, D386 ; avec D369, D370, D375, D376, D357, D367)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112e, worktree Haldir, branche `maps`, chapitre A1 du relevé du jour). Méthode des
dossiers par feature : règles tirées du GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions
(`C:\Users\baudo\Documents\MyCrusader\Retours\Q-signals.md`). Règle D483 : règles pures en Angelscript ; le transport (RPC, serveur) en C++.

## Règles tirées du GDD (rien à trancher)

1. **Roue de signaux typés** (D386, § 15) : Alt maintenu + clic, **sur le monde ou la mini-carte**. Cinq types : **Attaque ici**,
   **Défends ici**, **Besoin d'aide**, **Regroupement**, **Éclaire ici**. Un **simple Alt + clic** pose un **signal neutre** (6e type).
2. Chacun a **son icône, son son et une ligne dans le fil** (traduite d'office, fr / en / ja ; fil de messages D445).
3. **Vus des alliés seulement** ; en *Diplomatie*, **des alliés mutuels** (D369 : alliés seulement si les deux se disent Allié ; la
   relation est la plus hostile des deux postures, D375 ; une rupture est immédiate, D370, donc les signaux disparaissent tout de suite).
   Ni un Neutre ni un Ennemi ne les voient. En mode à équipes : les coéquipiers (IA comprise).
4. **Nombre limité contre le spam** : indicatif **3 toutes les 5 s** (par joueur).
5. **Une IA alliée pourra plus tard y réagir** (pas dans ce chantier : seulement les recevoir).
6. Un éliminé en chacun pour soi et en *Diplomatie* ne parle qu'aux spectateurs (D376) : **aucun signal** vers ses anciens alliés.

Lectures faites sans décision (pas des questions) : l'émetteur voit son propre signal (comme AoE4) ; la limite compte par joueur et un
signal refusé ne compte pas ; fenêtre glissante (le 4e est refusé tant que le 1er a moins de 5 s).

Pas dans le GDD (questions, défauts codés) : la **durée de vie** d'un signal sur la carte (Q1) ; le **coéquipier éliminé** en mode à
équipes (Q2 ; D357 lui laisse la discussion avec ses coéquipiers).

## Ce qui existe (relevé du 2026-11-12)

- **Aucun code de signal** : Grep `Signal|Ping|Beacon|Marker` dans `Script/` et `Source/` ne trouve que `MassSignals` (moteur), le
  `PingSeconds` du cercle de clic (`Source/HeroesOfLiberty/Player/HOLSelectionMarkers.cpp:67`, retour de commande, pas un signal),
  le son `1kSineTonePing` du forum (`HOLForumMessages.h:256`) et `AHOLSelectionMarkers` (anneaux de sélection).
- **Alliés mutuels** : `HOLDiplomacy::IsAlly(Book, A, B)` de `Script/Diplomacy/HOLDiplomacyRules.as` (relation = la plus hostile, D375 ;
  IA = Ennemie en *Diplomatie*, D367 ; `NewTeamBook` pour les modes à équipes : coéquipiers alliés, IA comprise). **Réutilisée, pas
  recopiée** par `HOLSignal::Sees`.
- À réutiliser plus tard : le fil de messages (D445) pour la ligne du signal ; le cercle de la mini-carte (D669, `hol.Minimap.Alert*`)
  comme modèle d'icône animée ; `AHOLSelectionMarkers` pour un anneau au sol.

## Plan (étapes de 15 min)

- **SG0 relevé** : FAIT (ce fichier, 2026-11-12).
- **SG1 règles pures en script** : `Script/Signals/HOLSignalRules.as` (types, `Sees`, `CanSend`, `Post` avec la limite, `Expire`,
  `SeenBy` avec la durée de vie) et `Script/Tests/HOLSignalRules_Test.as` (6 tests, rouge d'abord 76f66b8). Valeurs au registre
  `Tools/Data/ValueSheet.csv` (`comm.signal.*`).
- **SG2 transport réseau (C++, agent C++)** : RPC serveur `ServerPostSignal(Kind, Location)` sur le contrôleur ; le serveur tient le
  `FHOLSignalBoard` (appelle `HOLSignal::Post` : la limite est appliquée au serveur, jamais au client seul) et envoie le signal aux
  seuls joueurs pour qui `Sees` est vrai (RPC client par contrôleur, pas de réplication à tous : un ennemi ne doit pas le recevoir) ;
  horloge = temps de partie du serveur en ms. Test : `net_match` à 3 joueurs (allié reçoit, ennemi ne reçoit pas, 4e refusé).
- **SG3 saisie et affichage** : le GDD dit « roue (Alt maintenu + clic) » et « un simple Alt + clic pose un signal neutre » ; la frontière
  entre les deux (clic maintenu qui ouvre la roue, clic bref = neutre ?) n'est pas écrite : à relire avec l'utilisateur à cette étape (pas
  posé maintenant, 2 questions au plus) ; icône sur la mini-carte (modèle
  D669) et marque au sol pendant `LifetimeMs` ; ligne dans le fil (D445). Icônes provisoires : ligne à ajouter à `Placeholders.txt`.
- **SG4 sons** : un son court par type (règle des sons non usants : graves, sourds, courts ; packs Kenney d'AudioInProgress) ; provisoire
  noté dans `Placeholders.txt`.
- **SG5 vérification** : scénario `signals` (Alt + clic, roue, allié qui voit, ennemi qui ne voit pas, 4e refusé), essai listé seulement
  VU À L'ÉCRAN, geste réel.
- **SG6 IA alliée** (après IA4) : lire `SeenBy` (Attaque ici / Défends ici) ; note seulement.

## Premier test rouge (SG1)

`Script/Tests/HOLSignalRules_Test.as` : 6 tests sur la vraie règle de *Diplomatie* (`HOLDiplomacy::SetStance`, `NewTeamBook`) : l'allié
mutuel voit le signal posé, l'allié d'un seul côté, le Neutre et l'Ennemi non ; la rupture le cache tout de suite ; le 4e signal en 5 s est
refusé et le plateau n'en montre que 3 ; un autre joueur garde son compte ; le signal s'efface après sa durée ; un éliminé de *Diplomatie*
ne signale pas. Commit du rouge 76f66b8 (namespace `HOLSignal` absent).

## Questions (`Retours\Q-signals.md`, défauts codés)

- Q1 : durée de vie d'un signal. Défaut A = 5 s (`LifetimeMs`), égale à la fenêtre de la limite.
- Q2 : le coéquipier éliminé signale-t-il ? Défaut B = oui en mode à équipes (`DefeatedTeammateSignals`), jamais en chacun pour soi ni
  en *Diplomatie* (D376).
- **Réponses de l'utilisateur (2026-11-12, D784)** : Q1 = **A, 5 s** (déjà codé) ; Q2 = **A, NON : un coéquipier éliminé voit les signaux de son équipe mais n'en pose plus** (À CHANGER : `HOLSignal::DefeatedTeammateSignals = false`, test à adapter).

## Suivi

- 2026-11-12 11:33-11:4x (HALDIR-1112e, `maps`, sans C++) : SG0 et test rouge SG1 (76f66b8 ; quick_check `tests` 11:36 : « Namespace
  'HOLSignal' doesn't exist », 0 test lancé). **SG1 VERTE** : quick_check `tests` 1186/1186 PASS (215 s), `values` PASS (3 lignes au
  registre `comm.signal.max_per_window`, `window_ms`, `lifetime_ms` ; Valeurs.md/.csv régénérés). Commit vert 9d30e2c, puis `git merge main` (arbre propre). **Prochaine étape pour un agent neuf : SG2** (agent C++ : RPC serveur, plateau tenu au
  serveur, envoi aux seuls `Sees`), puis SG3. Aucun test rouge en cours. Rien de visible en jeu : pas de ligne à `Placeholders.txt`.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D784 applique : DefeatedTeammateSignals = false (il voit, ne pose plus) ; rouge dbc9699 (Post -> Silenced, le vivant ne voit rien de lui, le vaincu voit le signal de son equipe ; + Test_SignalStaysFiveSeconds a 4999/5000 ms), vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.
