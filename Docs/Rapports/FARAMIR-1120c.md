# FARAMIR-1120c : pose serveur du Sceau de lumiere en script (socle des factions, morceau 3), 2026-11-20

APPELS_DECLARES: 17 (arret a 25 : arret dur 10:22 non atteint)
HEURE_DEBUT/HEURE_FIN: 10:06:40 / 10:11
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: beb093e (stub qui refuse toujours "pas l'Aube" + tests) ; assertit l'effet : oui (jauge ecrite, fin de recharge, refus et jauge intacte), rouge NON VU a l'execution (aucun quick_check dans l'agent)
COMMITS: 2 (beb093e test + stub, 7f66bbb regle + RPC), dernier a 10:10, merge_main_fait: non (integ-26 fusionnee en premier geste)
TESTS: aucun lance (NON COMPILE) ; TESTS_SCRIPT_AJOUTES: 2 (Test_FactionPowerCastChain, Test_FactionPowerCastRefusals) ; TESTS_TAUTOLOGIQUES: 0 (couts, jauge, fin de recharge calcules par HOLDawnPowers / HOLFactionResource / HOLGroundAbility)
ESSAIS: aucun
QUESTIONS: aucune creee ; ecart signale : recharge du livre HOLDawnPowers.SacredLight = 20000 ms, Sceau = 40 s (la regle prend 40000 ms, SealCooldownMs)
BUILDS_DEMANDES: 0
TYPES_A_EXPOSER: (1) origine du lanceur en cm (le RPC prend GetHeroBannerLocation, FIntPoint, unite non verifiee, pose de remplacement) ; (2) horloge de recharge : le RPC utilise Gameplay::GetTimeSeconds()*1000, les zones ont leur ZoneClockMs (UHOLHeroSubsystem::GetZoneNowMs) non expose ; l'interface devra lire PowerCooldownEndMs sur la meme horloge.
FAIT / RESTE:
- Script/Factions/HOLFactionPowerCast.as : FHOLFactionPowerDecision, HOLFactionPowerCast::Decide(Faction, PowerIndex, Gauge, CooldownEndMs, NowMs, Origin, Point) ; raisons 1 pas Aube, 2 pouvoir absent, 3 recharge, 4 portee (800 cm, distance arrondie au-dessus), 5 jauge ; SealRangeCm 800 / SealCooldownMs 40000.
- Script/Net/HOLRpcPlayer.as : ServerCastFactionPower(Slot, PowerIndex, Point) : refuse si Slot != emetteur ; StartLightSeal d'abord, puis SetFactionResource et SetPowerCooldownEndMs.
- Script/Tests/HOLFactionPowerCast_Test.as : chaine accepte, recharge a -1 ms refuse, a la fin accepte, portee exacte/+1 cm, jauge vide/un de moins/exacte, hors Aube, pouvoir hors livre.
- RESTE : preuve a l'execution ; accesseurs de TYPES_A_EXPOSER ; interface (autre agent).
NON VERIFIE: rien compile ni execute : syntaxe Angelscript (Math::CeilToInt sur double, FIntPoint .X/.Y, HOLHero::GetHeroBannerLocation, Gameplay::GetTimeSeconds, HOLZoneEffect::StartLightSeal avec FVector) ; la session principale doit lancer quick_check -Parts tests (apres un build des scripts si necessaire) et VOIR le rouge sur beb093e (2 tests rouges, stub) puis le vert sur 7f66bbb ; la pose dans un vrai monde (StartLightSeal, RPC, jauge et recharge ecrites) n'est pas testee en script.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: (docs voulus : AubeInProgress, EtapeDefini, Auto_Journey_File ; le reste vient des autres agents) oui
M AubeInProgress.md
 M AudioBuildingsInProgress.md
 M Auto_Journey_File.md
 M DuelInProgress.md
 M EtapeDefini.md
 M Placeholders.txt
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/CELEBORN-1120d.md
?? Docs/Rapports/GALADRIEL-1120d.md
?? Docs/Rapports/HALDIR-1120f.md
?? Docs/Rapports/HALDIR-1120g.md
?? Docs/Rapports/SAURON-1120e.md