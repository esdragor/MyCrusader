# DOCS-1123pe (2026-11-23, 17:2x) : tableau des agents + Priorite_Socle_Factions

Impact joueur : aucun (docs seulement). Aucun code, aucun build.

## Fait
1. `Auto_Journey_Jour.md`, section « Agents du 2026-11-23 » : 4 resumes completes (integ-62, FARAMIR-1123pf, DOCS-1123pc, VERDICT-1123pa ; Fini = heure du rapport) et 8 lignes ajoutees apres build-GALADRIEL-4 : ARWEN-1123pe, FARAMIR-1123pg, DOCS-1123pd, RECETTE-1123pc, sync-Haldir-17h, build-ARWEN, build-FARAMIR-3, integ-63. Aucune ligne deja remplie modifiee. GALADRIEL-1123pf et GLORFINDEL-1123ph etaient deja remplies : inchangees.
2. `Docs\Priorite_Socle_Factions_2026-11-16.md` remis a jour pour 17:2x (ancienne version : `Docs\Archive\Priorite_Socle_Factions_2026-11-16_avant_maj_2026-11-23_soir.md`), meme structure.

## Ecart de methode
`agents_table.ps1` estampille Lance / Fini avec Get-Date (heure d'ecriture, pas l'heure reelle) : j'ai edite les lignes avec un script Python (EOL conserve, assertion « resume vide » avant chaque remplissage) pour garder les heures des rapports.

## NON ETABLI
- Lance de FARAMIR-1123pg, DOCS-1123pd, RECETTE-1123pc (« ? ») ; worktree d'integ-63 (« ? ») ; aucun rapport DOCS-1123pd.md.
- `git log fc41498..main --first-parent` a renvoye un seul commit inattendu (b7186d1) : sortie non exploitee ; les 5 avances (b5a285b 15:37, c3578b1 16:00, 86a2e15 16:22, a984c61 16:46, 3b6d29c 17:07) viennent de `git log -1` par sha, et main = 3b6d29c de `git log --first-parent`.
- Contenu des avances b5a285b a a984c61 (seul b5a285b est decrit dans le Jour), etat de Celeborn integ-41, Elrond integ-25 (bouton du Moine), Sauron integ-55, S2-a/b, D893 : non relus, marques « non revalides » dans la priorite.
- integ-63 : resultat inconnu (INTEG-63.txt s'arrete a « tests des rouges ... » a 17:15).
- Q-commanderie-batiment, Q-moral-attenuation, Q-sp3-libelles : titre seul lu. Reponse de l'utilisateur a faction_aim_free : aucune vue. Troisieme unite de D896 non relevee (texte tronque).
- Anomalie : `Auto_Journey_Jour.md` avait deja des modifications non commitees d'une autre session (embarquees par le commit par chemins). Placeholders.txt non touche (aucun provisoire ajoute ni retire).
