# Launcher in progress

Journal du chantier **lanceur** : un petit programme que les amis installent une fois, qui **télécharge et met à jour**
le jeu tout seul, puis le lance. Ouvert le 2026-10-23, à la demande de l'utilisateur (réponse à R-Q8 de
`NetPlaytestInProgress.md` : « A, mais avoir un autre chantier en parallèle qui est de faire un launcher »).
Décisions nouvelles : à la suite de la review du GDD (prochain numéro libre au moment du report).

## Pourquoi

Le chantier réseau (`NetPlaytestInProgress.md`) livre le jeu **en zip par version** (R-Q8 = A). Le serveur et les
joueurs doivent avoir la **même version** (M6-Q1), sinon le joueur est refusé. Avec des mises à jour fréquentes, chaque
ami retéléchargerait tout le zip (1 à 3 Go, à mesurer). Le lanceur remplace ce geste.

## Ce dont il dépend

- Le **jeu empaqueté** et son **numéro de version** (R1 et R1.7 du chantier réseau).
- Le **refus d'une version différente** par le serveur (chantier réseau), que le lanceur évite en mettant à jour
  avant de lancer.
- Peut commencer **en parallèle** : la conception et le squelette ne dépendent de rien ; le téléchargement réel attend
  le premier paquet.

## Ce qu'il fait (premier jet, à trancher)

1. Affiche la version installée et la dernière version disponible.
2. Télécharge ce qui manque (de préférence **seulement les fichiers changés**).
3. Lance le jeu ; peut-être aussi « Rejoindre » le serveur connu (adresse, nom).
4. Sur le PC de l'utilisateur : met aussi à jour **le serveur** et le relance (M6-Q2 : serveur démarré avec Windows).

## Questions à trancher (une à la fois, options A / B / C, recommandation)

| Q | Question | Pistes |
|---|---|---|
| L-Q1 | Où sont rangées les versions ? | sur le PC de l'utilisateur, déjà allumé en permanence et joignable par Tailscale (R-Q2) ; un partage (Google Drive, OneDrive) ; GitHub Releases (dépôt privé) ; itch.io + `butler` (le lanceur n'aurait plus lieu d'être) |
| L-Q2 | Mise à jour complète ou par fichiers changés ? | liste des fichiers avec leur empreinte, seuls les changés téléchargés ; paquet `.pak` découpé par chunks pour que peu de fichiers changent ; zip complet |
| L-Q3 | Avec quoi l'écrire ? | piste : C#, comme le gestionnaire de salons (M6-Q6b du chantier réseau) ; C# (.NET, une fenêtre WPF ou WinForms, un seul `.exe`) ; Python empaqueté (PyInstaller) ; application Unreal (trop lourde) |
| L-Q4 | Que montre-t-il ? | version, barre de progression, « Jouer » ; nouveautés de la version ; état du serveur (allumé, joueurs dans le salon) |
| L-Q5 | Met-il à jour le serveur ? | oui sur le PC de l'utilisateur (mode « serveur ») ; non, script à part |
| L-Q6 | Se met-il à jour lui-même ? | oui (télécharge sa nouvelle version au démarrage) ; non (rare, à la main) |

Pistes venues du chantier réseau : le jeu lit l'adresse du PC de l'utilisateur dans un `servers.json` à côté de
l'exe (M6-Q5), que le lanceur peut tenir à jour ; le lanceur pourrait aussi montrer l'état des salons (L-Q4).

## Plan

À écrire après les questions L-Q1 à L-Q6, étapes de moins de 15 min, comme les autres chantiers.

## Suivi

- 2026-10-23 : chantier ouvert, questions listées. Rien tranché, rien codé.
