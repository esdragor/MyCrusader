// Switches on the Dawn's Honor earned by healing (S1-e, AubeInProgress.md): the Mass heal processor (UHOLHealProcessor) credits one Honor per
// HonorHealPerPoint hit points healed, but stays off (0) until it is configured. The numbers are the Dawn's (HOLDawnValues, ONE source, D839).
// A world subsystem (UScriptWorldSubsystem): the first tick of each world configures the processor, which also clears the heal remainders of
// the previous match. Installed on every machine, harmless on a client (the heal processor only runs on the server).
class UHOLDawnHealHonorInstaller : UScriptWorldSubsystem
{
	bool bConfigured = false;

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		if (bConfigured)
			return;
		HOLZoneEffect::ConfigureHealHonor(HOLDawnValues::HonorHealPerPoint(), HOLDawnValues::HonorCap(),
			HOLDawnValues::HonorKillGain(), HOLDawnValues::HonorKillRadiusCm());
		bConfigured = true;
	}
}
