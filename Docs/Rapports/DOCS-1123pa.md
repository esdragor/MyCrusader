# DOCS-1123pa (2026-11-23 15:3x, docs seulement)

Fait enregistre : l'utilisateur a repondu a 15:28, mot pour mot « A pour les 3 » a `Retours\Q-commanderie-palier1.md` (Sergent du Temple, Frere infirmier, Arbaletrier a pavois : garder les copies provisoires, a regler en test de jeu). A est deja le defaut code (CELEBORN-1123pa, 638a4ed sur integ-41) : aucun code, aucune tache neuve.

Fichiers touches :
- `GDD.md` : D896 ajoute apres D895 (GDD.md etait propre avant : `git diff --stat GDD.md` vide), « Prochain numero » = D897.
- `SpecializationInProgress.md` : ligne ajoutee (l. 134) apres celle de CELEBORN-1123pa.
- `EtapeDefini.md` : ligne ajoutee (l. 977) apres celle de CELEBORN-1123pa.
- `Auto_Journey_File.md` l. 28 : « valeurs = A (D896, 2026-11-23 15:28) » dans la parenthese CELEBORN-1123pa, reste de la ligne inchange.
- Fiche : titre [REPONDU], `Reponse : A pour les 3 (utilisateur, 2026-11-23 15:28, D896)`, `git mv` vers `Docs\Archive\Retours_2026-11-23\Q-commanderie-palier1.md` (fiche suivie par git).
- `Auto_Journey_Retours.md` : en-tete, section 1 (E-ai_garrison hors compte), section 2 recomptes.
- `Retours\LISEZ-MOI.md`, `Retours\ESSAIS-A-JOUER.md` : comptes recomptes, E-ai_garrison ajoute.

Comptes d'apres le dossier Retours\ (11 fichiers : 6 E + 2 BUG + 1 Q + LISEZ-MOI + ESSAIS) :
- Questions ouvertes : 1, PAS 0 : `Q-ligne-case-bloquee.md` (non suivie par git, creee par un autre agent) est [A REPONDRE]. Le « 0 » attendu par l'enonce est donc faux.
- [A ESSAYER] en titre : 4 (talents_heal, tower_wood, wall_line_long LOGIQUE SEULEMENT + ai_garrison VU A L'ECRAN sur integ-25, pas dans main) ; essais prets : 0.

NON VERIFIE : rien lance (docs seulement) ; le contenu de Q-ligne-case-bloquee et d'E-ai_garrison n'a ete lu que sur leurs premieres lignes ; D896 reprend les valeurs du tableau de la fiche, pas relues dans `Tools/Data/UnitTypes.csv`.

Anomalies : Q-ligne-case-bloquee non suivie par git, non commitee ici (pas mon fichier) ; Auto_Journey_File.md, EtapeDefini.md portaient deja des lignes d'autres agents (partent avec le commit, docs seulement).
