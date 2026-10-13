# Journey in progress

**Journal du chantier « parcours du joueur » (D252)**, ouvert le 2026-10-13. Après M2, le jeu avance dans l'ordre où
le joueur découvre une partie, chaque étape étant jouable : **J1** emplacement de départ et caméra RTS ; **J2** centre
principal par joueur, sélection ; **M2.5** réseau ; **J3** panneau du bâtiment et production d'un Paysan ; **J4** ordres
de déplacement et chemins ; **J5** récolte et ressources ; puis construction, armée et combat.

Cohabitation (mêmes règles que le chantier menu) : M2 tourne dans une autre session sur le module `HeroesOfLiberty` ;
décisions numérotées **D252 et suivantes**, tenues ici pendant le plan puis reportées au GDD (§ 21) ; pas de worktree,
commits sur `main` par chemins explicites ; ne compiler que si aucun éditeur ni compilation du projet ne tourne ; jamais
de code qui ne compile pas ; ne pas changer l'environnement du stress test (GameMode global, pion, caméra) sans le
vérifier sur S1 et S2.

---

## Où on en est

| Étape | État |
|---|---|
| D252 : ordre d'avancement par le parcours du joueur | **fait** (2026-10-13, au GDD § 16.6 et § 21) |
| J1 — plan (questions ci-dessous) | **en cours** |

## Prochaine étape exacte

**Plan de J1, Q1** (périmètre), puis les questions suivantes une par une.

**Règle de tenue** (permanente) : mettre à jour ce fichier à **chaque étape** — tableau ci-dessus, prochaine étape
exacte, entrée de journal — avant de passer à la suivante.

## Plan de J1 (à valider question par question)

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J1 : caméra et emplacement de départ seuls, ou avec la partie de l'onglet Commandes qui règle la caméra, ou avec le remappage complet de D30. | à poser |
| Q2 | **Comportement de la caméra** : déplacement, zoom, angle, rotation, limites de la carte. | à poser |
| Q3 | **Fabrication** : pion caméra en C++, actions Enhanced Input de la partie, lien avec le stress test (qui déplace aujourd'hui le pion par défaut pour ses vues de mesure, D236). | à poser |
| Q4 | **Emplacements de départ** sur la carte : posés dans la carte ou calculés (cartes symétriques, § 14.4 ; emplacement = index de l'emplacement, D244). | à poser |
| Q5 | **Vérification** : tests d'automatisation, captures, stress test. | à poser |

## Décisions du chantier (à reporter au § 21 du GDD)

| # | Date | Sujet | Décision |
|---|---|---|---|

## Journal d'avancement

### 2026-10-13 — Chantier ouvert (D252)

- Proposition de l'utilisateur après la clôture du menu : avancer par le parcours du joueur (« je spawn, je bouge ma
  caméra, je clique sur mon centre, son interface apparaît… ») plutôt que par couches techniques. Option A retenue :
  réseau (M2.5) inséré après J2, avant que le joueur change l'état de la partie. **D252** consignée directement au GDD
  (§ 16.6 et § 21), comme D248, car elle concerne tous les chantiers.
- Constats pour J1 : la carte de bataille tourne avec le pion par défaut du moteur ; le stress test le déplace pour ses
  vues de mesure (`HOLStressTestSubsystem`, D236, D237) ; le GDD n'a sur la caméra que les raccourcis de D30 (centrer
  sur un groupe, sur le héros) ; le centre principal s'appelle « centre principal » au GDD (§ 17).
