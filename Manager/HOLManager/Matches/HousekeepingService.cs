using HOLManager.Lobbies;

namespace HOLManager.Matches;

/**
 * Runs the store's housekeeping every second (R2.10): a server gone, silent 120 s after its launch or lingering
 * 10 s after "empty" is followed even when no client calls. The rules themselves are in LobbyStore, under its lock.
 */
public sealed class HousekeepingService(LobbyStore store) : BackgroundService
{
    public static readonly TimeSpan Period = TimeSpan.FromSeconds(1);

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        using var timer = new PeriodicTimer(Period);
        while (await timer.WaitForNextTickAsync(stoppingToken))
        {
            store.Housekeep();
        }
    }
}
