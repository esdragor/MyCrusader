# [À RÉPONDRE] Nomade : combien de temps pour bâtir le premier forum ? (HALDIR-1115i, 2026-11-15)

**Contexte.** D402 dit « forum posé et construit normalement » (écarté : « forum achevé à la pose »), mais aucune décision ne donne sa durée : la table (`Tools\Data\BuildingTypes.csv`, ligne `MainCentre`, colonne `BuildTime`) vaut **0 s**, car un forum de départ standard est posé fini. Résultat vu dans l'essai `nomad_ai` : le forum de l'IA Nomade s'achève au **premier coup** de son premier Paysan (« after 1 steps of work »), donc pratiquement achevé à la pose ; le joueur humain Nomade a exactement la même règle (même table, même calcul, aucun cas particulier).

Rappel des durées (pour 1 bâtisseur ; avec N Paysans la courbe N^0,63 de D312 : 5 Paysans ≈ durée / 2,76) : maison 10 s, caserne 20 s (D557, durées AoE divisées par 2) ; reconstruction d'un forum en ruine 120 s seule, 60 s avec 1 Paysan (D418, réglage à part `ForumRebuildTime`, inchangé quelle que soit la réponse).

**Options**
- **A : 75 s pour 1 Paysan** (Centre-ville d'AoE2/AoE4, 150 s, divisé par 2 comme D557). Vu en partie : avec les 5 Paysans réunis ≈ 27 s ; avec 1 seul (ce que fait l'IA aujourd'hui, les autres arrivent de loin) ≈ 75 s ; une vraie fenêtre où l'adversaire peut casser le chantier (ressources perdues, D402).
- **B : 150 s pour 1 Paysan** (valeur AoE pleine). Vu : 5 Paysans ≈ 54 s, 1 Paysan 2 min 30 ; départ Nomade nettement plus lent qu'un départ standard ; forte prime à rassembler ses Paysans.
- **C : 60 s pour 1 Paysan** (= la reconstruction d'un forum avec 1 Paysan, D418). Vu : 5 Paysans ≈ 22 s ; début de partie Nomade rapide, chantier peu exposé.
- **Autre** : à préciser (valeur, ou durée différente pour l'IA).

**Recommandation : A (75 s)** : cohérent avec D557 (durées AoE divisées par 2) et avec D402 (le chantier doit exister et pouvoir tomber).

**Défaut codé** : rien n'est changé tant que la question est ouverte (aujourd'hui 0 s = achevé au premier coup). Une ligne à changer : `Tools\Data\BuildingTypes.csv`, `MainCentre`, colonne `BuildTime` (puis régénérer la table `.uasset`) ; elle ne touche que les forums **construits** (le forum de départ standard est posé fini). Test à écrire avec : fondation MainCentre d'un siège Nomade avec N Paysans toujours en chantier avant `GetBuildSteps(BuildTime) / N^0,63` pas.

Reponse (2026-11-15) : B : 150 s pour 1 Paysan (valeur AoE pleine ; environ 54 s avec 5 Paysans, courbe N^0,63 de D312) ; PAS la recommandation A (75 s). Une ligne à changer : MainCentre BuildTime dans BuildingTypes.csv. (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D816) et le journal du chantier.
