# DOCS-1119r : reponses de l'utilisateur du 2026-11-19 ~22:55 reportees (docs seulement)

Aucun code, build, commit, ni modification de GDD.md / EtapeDefini.md / Auto_Journey_Retours.md / Auto_Journey_Jour.md.

## Fiches repondues puis deplacees (git mv, aucun doublon, pas de suffixe _soir)
Vers `Docs\Archive\Retours_2026-11-19\` : E-signals_altclick.md (OK + REGLAGE D860), E-deploy.md (BUG D861), E-specialization_button.md (OK), Q-hallebardier-valeurs.md (D858, option B, remplace le Lancier), Q-socle-factions-valeurs.md (D859, OK recommandees partout). Reponse ecrite en fin de fiche, ligne `Reponse (2026-11-19 ~22:55) : ...`.

## Journaux completes (une ligne datee « 2026-11-19 ~22:55, reponse de l'utilisateur »)
- SignalsInProgress.md : fin de fichier (section HALDIR-1116a). Rattache a H2 (minicarte animee, script) et H3 (Alt + clic minicarte), deja prevus.
- Palier3InProgress.md : fin de fichier (section Decisions, suite du journal G6). SiegeInProgress.md : fin de « Suivi » (seule mention du montage : l. 57).
- SpecializationInProgress.md : fin de fichier.
- AubeInProgress.md : A1 (D858), A3 (D859 lignes 6-7), A4 (D859 lignes 1-4), A6a (D859 ligne 5), A7 (D859 ligne 10). Valeurs lues dans le tableau de la fiche Q, pas de memoire.
- DuelInProgress.md : apres la ligne D7 du plan (D859 lignes 8-9).

## Auto_Journey_File.md, section « Socle commun des factions »
En tete (corrections, a faire a la prochaine seance) :
- (iii) D861 BUG deploy (a) demonte + attaque -> monter puis attaquer : C++ (Mass), HOLUnitOrderProcessor.cpp / HOLDeployProcessor.cpp.
- (iv) D861 BUG deploy (b) monte + deplacement -> garder l'ordre : C++ (Mass), memes fichiers ; HOLPlayerOrders.as a verifier.
- (v) D860 REGLAGE : mixte, a verifier. Minicarte animee = script (HOLGameScreenScript.as RefreshMinimap) ; marque au sol animee = C++ (HOLSelectionMarkers.cpp UpdateSignalMarks) ; Alt + clic minicarte = C++ (HOLMinimapView.cpp / HOLSignalInput.cpp, Slate).
Plus bas, a la place des deux `[?]` des questions : (i) D858 Aube A1 Hallebardier (SAURON-1119n1.md) ; (ii) D859 porter les 10 valeurs (CELEBORN-1119n2.md). La ligne `[?]` des 3 essais est supprimee (remplacee par iii a v). Longueurs des lignes : 262 a 296 caracteres.

## Retours\LISEZ-MOI.md et ESSAIS-A-JOUER.md
Ligne « MISE A JOUR DU 2026-11-19 ~23:00 : a jouer 0, questions sans reponse 0 ... » ajoutee en tete de la partie d'etat (le reste est laisse tel quel, donc les listes plus bas citent encore les fiches deplacees).

## Non trouve / NON VERIFIE
- Section « A6a » : pas de titre propre, c'est une puce de A6 ; la ligne est inseree apres les deux puces datees de A6a (l. ~120).
- Section « D7 » de DuelInProgress.md : pas de section propre, c'est une puce du Plan (l. 46) ; ligne inseree juste apres, en sous-puce.
- SiegeInProgress.md : pas de partie « montage » dediee (une seule mention, l. 57) ; ligne ajoutee en fin de fichier.
- NON VERIFIE : l'etiquette script / C++ du point (iii) a (v) vient d'un Grep de noms de fichiers, pas d'une lecture du code ; LISEZ-MOI.md (BOM conserve) et les autres fichiers relus seulement par grep de longueur / debut de ligne ; Placeholders.txt non touche (rien de provisoire ajoute ni retire ici).
- Auto_Journey_File.md a ete modifie par une autre session entre mes deux ecritures (avertissement de l'outil Edit) : mes lignes sont presentes, mais a relire par la session principale.
