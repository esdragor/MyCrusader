---
description: "Use when: doing any code or analysis work in this repository, or when the user says 'continue ImplementationInProgress'. Requires keeping the ImplementationInProgress.md journal up to date at every step."
applyTo: "**"
---

# Journal d'avancement obligatoire

Le dépôt suit l'avancement du code dans **`ImplementationInProgress.md`**, à la racine. C'est le point de
reprise : l'utilisateur doit pouvoir interrompre une séance et la reprendre plus tard en disant simplement
**« continue ImplementationInProgress »**.

## À faire, toujours

- **Avant de commencer** : lire `ImplementationInProgress.md` et repartir de sa section « Prochaine étape
  exacte ». Si l'utilisateur dit « continue ImplementationInProgress », le lire d'abord, sans rien supposer.
- **Après chaque étape ou analyse** (implémentation, mesure, diagnostic, échec, changement de plan) : mettre
  le fichier à jour **dans le même tour**, pas « plus tard ». Une étape non consignée est une étape perdue.
- **Y mettre** : ce qui est fait, les résultats **mesurés** (chiffres, pas d'intentions), la prochaine étape
  exacte, les questions ouvertes, et les commandes qui ont réellement servi.
- **Journal d'avancement** : une entrée datée par étape, la plus récente **en haut**, avec le numéro de
  décision du GDD concerné et le commit s'il y en a un.
- **Corriger une mesure passée** : la remplacer en indiquant l'ancienne valeur et pourquoi elle changeait
  (« mesuré avant la correction du pipeline »). Jamais d'écrasement silencieux.
- **Si le travail s'arrête en cours de route** (blocage, question en attente, session interrompue) : l'écrire
  explicitement, avec ce qui reste à faire et pourquoi on s'est arrêté.

## Répartition des deux documents

| Document | Contient |
|---|---|
| `GDD.md` | conception et décisions (§ 21, journal `D…`), point de reprise de la review (§ 22) |
| `ImplementationInProgress.md` | avancement du **code** : état des jalons, mesure, prochaine étape, questions ouvertes |

Une décision de conception va dans le GDD (et dans le corps du document), **jamais** dans
`ImplementationInProgress.md`, qui ne fait que la référencer par son numéro `D…`.

## Portée

Ce fichier ne remplace pas les consignes générales : il ajoute une obligation de tenue du journal. Les
sections existantes (`Où on en est`, `Prochaine étape exacte`, `Questions ouvertes`, `Journal d'avancement`,
`Commandes utiles`) doivent rester en place ; on les met à jour, on ne les remplace pas par une note libre.
