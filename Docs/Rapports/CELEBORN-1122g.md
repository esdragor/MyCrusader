# CELEBORN-1122g : F8 T3, degats de la fleche de batiment (armure, credit au proprietaire). NON COMPILE

Joueur : la fleche d'une Tour de guet finie est reduite par l'armure a distance de la cible (comme une fleche d'archer) et, si elle tue, la mort est creditee au joueur proprietaire.
Technique : `HOLCombat::GetBuildingShotDamage` (inline, `HOLCombatRules.h`), appelee a la relache dans `ReleaseBuildingArrows` ; l'atterrissage existant gere deja un tireur absent.

APPELS_DECLARES: 24 (arret a 25 vus ; le seuil se juge sur `tool_uses` de la session principale)
HEURE_DEBUT/HEURE_FIN: 17:55 / 17:58 (Get-Date ; l'horloge a peu avance pendant la seance)
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: bbaac86 (bouchon qui compile + tests, meme commit), rouge NON VU a l'execution ; assertit l'effet : oui pour le test de la regle (valeur comparee a `HOLCombat::GetDamage` d'un archer, pas une constante) ; la chaine Mass verifie la mort et l'arrivee de la fleche   TEST_ROUGE_SEPARE: oui (bbaac86 rouge, 14585e2 vert)
TEST_ROUGE attendu sur bbaac86 : `HeroesOfLiberty.Combat.BuildingShotDamage` echoue (le bouchon rend Base : 5 au lieu de GetDamage, pas de minimum 1, pas de Forge ni de talent) ; les cas Mass (cible fragile morte, tour detruite en vol) passent deja sur le bouchon (T2 deja la), ils servent de garde-fou contre crash / regression.
COMMITS: 2 sur integ-41 (bbaac86, 14585e2), dernier 17:58 ; merge_main_fait: non (consigne : pas de fusion ; DLL non compilee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : `Tools\build.ps1 -LiveCoding=0` dans C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, puis `quick_check -Parts tests`, filtre `HeroesOfLiberty.Combat.BuildingShot`)

## Fait
- `HOLCombatRules.h` : `GetBuildingShotDamage(Base, Victim, VictimArmorBonus = 0, VictimArmorPercent = 0)` : `(Base - (RangedArmor + bonus talent)) * (100 - clamp(Forge, 0, 90)) / 100`, 1 au moins, `HeroTroopDamageFactor` sur un heros : meme formule que `GetDamage` d'un archer de niveau 1 sans bonus ni technologie d'attaque, aucun bonus invente. Armure de melee ignoree.
- `HOLCombatProcessor.cpp` : a la relache, `Arrow.Damage = GetBuildingShotDamage(BuildingAttack, *Enemy.Constants, GetTalentArmorBonus(Enemy), GetTechnologyArmorPercent(Enemy, Ranged))` (comme le degat d'un archer est fixe a la relache) ; le levier de moral reste lu a l'atterrissage comme pour toute fleche.
- Credit : l'atterrissage appelle `Kill(Enemy, Arrow.ShooterSlot, ...)` : `KillUnit` -> `MarkUnitDying(victime, KillerPlayer)` -> `RecordUnitDeath` : la mort va au slot du proprietaire. `FindFighter` sur une poignee vide rend INDEX_NONE (garde `!Entity.IsSet()`, l.1271) et chaque lecture de `Shooter` est gardee par `!= INDEX_NONE` (l.1491, 1541, 1561, 1594) : aucun crash. Risque trouve et corrige : `AwardKillXP` compare `KillerId == HeroId` ; l'identifiant d'un batiment (`GetUniqueID`) pouvait egaler l'identifiant d'un heros : l'appel passe maintenant 0 quand `Arrow.Shooter` n'est pas pose.
- Tests (`HOLBuildingShotTests.cpp`) : `HeroesOfLiberty.Combat.BuildingShotDamage` (armure reduit = valeur de `GetDamage` d'un archer, minimum 1, Forge 50 %, bonus talent, sans armure = 5 : aucun bonus invente) ; dans `HeroesOfLiberty.Combat.BuildingShot` : cible a 1 PV morte de la fleche sans tireur, tour detruite en vol : la fleche atterrit et blesse.

## RELECTURE_CPP (L20-2)
Relu (`git diff`) : `GetArmor` (l.22) et `HeroTroopDamageFactor` (l.28) declares avant l'usage dans le header ; `HasArmorCategory` et `EHOLArmorCategory::Hero` copies de `GetDamage` (cpp) ; `GetTalentArmorBonus(const FFighter&)` et `GetTechnologyArmorPercent(const FFighter&, EHOLAttackType)` membres (h l.226) appeles depuis un membre non const ; `FMath::Clamp/Max` deja utilises dans le header voisin ; `Arrow.Shooter` est un `FMassEntityHandle` (`IsSet()` utilise l.1271) ; `0u` et `uint32 ShooterId` : le ternaire est de type `uint32`. Cote test : `IsValid(Tower)` apres `Destroy()`, `FHOLUnitTypeConstants` construit par defaut avec `Attack` / `RangedArmor` / `MeleeArmor` / `AttackType` assignes (champs lus dans `HOLCombatRules.cpp`). Indentation : une ligne de `HOLCombatRules.h` a un tab de trop, cosmetique.
Sources de risque de compilation restantes : type exact de `FHOLUnitTypeConstants::Attack` (int ou float : `= 5` convient aux deux) ; `Archer`/`Victim` non `const` passes a `GetDamage` ; `EHOLArmorCategory::Hero` visible depuis `HOLCombatRules.h` (inclus par `HOLUnitTypes.h` ?) ; l'indentation du commentaire inseree dans `Kill` ; ajouts a la place du patron « bonus Hero » du test : le `Hero` n'est pas teste (categorie non posee).

## Defauts pris
1. Pas de bonus de technologie d'attaque du proprietaire ni de niveau (batiment : aucun).
2. Armure de la CIBLE : talent (bouclier) et pourcentage de Forge pris comme pour une fleche d'archer ; armure de melee jamais.
3. Pas de XP de heros attribuee comme « tue par le heros » (ID 0), mais le heros du proprietaire reste eligible au partage d'XP existant comme pour toute mort non causee par lui (non lu en detail).
4. Le credit `NoteHit` des camps neutres n'est pas fait (T3 ne l'inclut pas ; `Shooter` absent) : question de design deja listee dans 1122d section 4.6.

## Reste
Build + `quick_check -Parts tests` : rouge attendu sur bbaac86 (`BuildingShotDamage`), vert sur 14585e2. Puis T4 cadence (purge de `BuildingsShot`), T5 brouillard, T6 garnison, T7 affichage, T8 cout.

## NON VERIFIE
Compilation et tous les tests ; le rouge et le vert a l'execution ; que le Spearman a l'identifiant multiple de 4 (banc Shelter) ; que la cible a 1 PV meurt bien avant 10 s (flight < 10 s) ; que le credit `RecordUnitDeath` atteint le bon slot (lu dans le code, pas assertit : l'API de lecture de `UHOLMatchStatsSubsystem` n'a pas ete trouvee dans le budget d'appels, donc le test ne compare PAS le compteur de kills du joueur) ; la destruction de l'acteur tour dans un monde de test (`Tower->Destroy()`).

PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Root HeroesOfLiberty-Celeborn -Stop : « aucun processus detache »)
ANOMALIES_PROCESSUS: IN_USE.txt d'entree non ecrit (le budget d'appels l'a evince : le fichier portait deja `libre` de 1122f, CPP: non ; je l'ai reecrit `libre` a la sortie) ; 2 Edit PowerShell refuses (indentation attendue fausse) puis repris par regex.
GIT_STATUS_PRINCIPALE: inchangee hors docs voulus : oui (FortificationsInProgress.md, EtapeDefini.md, Auto_Journey_File.md : une ligne ajoutee apres la ligne 1122f de chaque fichier)
