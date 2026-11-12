# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir) et `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Dernier bilan : 2026-11-11 (13:24-18:02, détail dans l'archive du 2026-11-12)

- Main = fd03266 après 3 fusions (rien poussé) ; tests 1173/1173, aiserver, aiduel PASS ; net non relancé sur le résultat fusionné.
- Fusionné : N7 état des camps, Palier 3 G4-G6 d (bouton), T4b Mains du guérisseur, Légions L4 (rage, IA, traductions), S5 prouvé sur le réseau, notereset/preflight, neutrals 3v3, essais de Sauron ; build de main réparé par Haldir (39ba5ec).
- Non fusionné : galadriel-tab a3e90a8 (conflit notereset avec fog), net-playtest c8bde0e/WIP S6 (test ROUGE), visual f0670cf+ (scénarios) : lignes 95, 100 et 57 de `Auto_Journey_File.md`.
- Scénario neutrals ramené à 3 Hommes d'armes : masque le résidu D641 (noté dans `NeutralsInProgress.md` et `Retours\E-neutrals.md`).
- Réponses de l'utilisateur du 2026-11-11 aux Q-*.md reportées (commit bacee4b) ; `Retours\ESSAIS-A-JOUER.md` nettoyé le 2026-11-12 (0 essai prêt).
- Leçons non appliquées le 2026-11-11 : plafond de 45 appels (agents à 46-64), commits de docs sur main pendant une fusion, calls.md partagé ; 2 tips rouges commités par des agents.

## Chapitres du GDD non commencés (relevé Arwen du 2026-10-30, état revu le 2026-11-12 au seul nom des `*InProgress.md`)

Relevé complet d'origine : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`, section du même nom. Depuis, un chantier est ouvert pour : Volcan (`VolcanoInProgress.md`), Technologies (`TechnologiesInProgress.md`), Arbre de talents (`TalentsInProgress.md`), Duel (`DuelInProgress.md`), Marché (`MarketInProgress.md`), Siège (`SiegeInProgress.md`), Neutres (`NeutralsInProgress.md`), Relais (`RelaysInProgress.md`), Légions Noires (`LegionsInProgress.md`). Reste sans chantier :
- **§ 16.2 World State Manager** (D29) : se construit avec le volcan, pas à part.
- **§ 5.2 Ressources spéciales, § 6.2 Bonus de hauteur** : décidés ; la hauteur est exclue du prototype pour les effets de terrain (M-Q2) ; à relire avant d'ouvrir.
- **D384 Centres secondaires** (XP de territoire) : à vérifier contre `RelaysInProgress.md` avant d'ouvrir.
- **Après le prototype, ne se prennent pas** : § 13 les quatre autres factions (Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu, Templiers), § 12.3 Événements futurs (D209), § 14.3 Modes Battlefield et Reliques (D416, D102), § 9.9 Progression hors partie.
