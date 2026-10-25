// HOLManager: keeps the lobbies of the multiplayer playtest and starts one dedicated server per match (M6, R2).
// The protocol is described in Manager/PROTOCOL.md.
using System.Runtime.InteropServices;
using HOLManager;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;
using HOLManager.Versions;

var builder = WebApplication.CreateBuilder(args);

// Listens on every address (Tailscale) on port 7700 unless --urls says otherwise. Set in code, not in
// appsettings.json: the startup shortcut may start the manager from any folder.
if (string.IsNullOrEmpty(builder.Configuration["urls"]))
{
    builder.WebHost.UseUrls("http://0.0.0.0:7700");
}

var settings = builder.Configuration.Get<ManagerSettings>() ?? new ManagerSettings();

// Log file next to the manager (R2.12): the startup shortcut runs it minimized, the console may be lost.
if (!string.IsNullOrWhiteSpace(settings.LogDir))
{
    builder.Logging.AddProvider(new FileLoggerProvider(Path.GetFullPath(settings.LogDir, AppContext.BaseDirectory)));
}
var gameVersion = settings.ResolveGameVersion();

builder.Services.AddSingleton<IClock, SystemClock>();
// Created by the container, which disposes it at shutdown: the servers still running are stopped.
builder.Services.AddSingleton<IServerLauncher>(_ => new ProcessServerLauncher(settings));
builder.Services.AddSingleton(sp => new LobbyStore(gameVersion, sp.GetRequiredService<IClock>(),
    launcher: sp.GetRequiredService<IServerLauncher>()));

// Versions served to the launcher (L2), with their own settings.
builder.Services.AddSingleton(new VersionStore((builder.Configuration.Get<VersionSettings>() ?? new VersionSettings()).ResolveDir()));

// Housekeeping every second (R2.10): servers gone, silent or lingering are followed even when no one calls.
builder.Services.AddHostedService<HousekeepingService>();

var app = builder.Build();
app.UseLobbyErrors();

app.MapGet("/status", (LobbyStore store, IServerLauncher launcher) => new
{
    protocolVersion = ManagerSettings.ProtocolVersion,
    gameVersion,
    lobbies = store.Count,
    servers = launcher.Running,
    maxServers = settings.MaxServers,
});

app.Lifetime.ApplicationStarted.Register(() => app.Logger.LogInformation(
    "HOLManager started: game version {GameVersion}, server {ServerExe}, max {MaxServers} servers.",
    gameVersion, Path.GetFullPath(settings.ServerExe, AppContext.BaseDirectory), settings.MaxServers));
app.Lifetime.ApplicationStopping.Register(() => app.Logger.LogInformation("HOLManager stopping."));

app.MapLobbyRoutes();
app.MapServerRoutes();
app.MapVersionRoutes();

// Closing the console window (CTRL_CLOSE, SIGHUP in .NET) kills the process a few seconds later without the usual
// shutdown: stop the app and wait for it, so that the game servers still running are stopped (R2.12).
using var stopped = new ManualResetEventSlim();
using var closeWindow = PosixSignalRegistration.Create(PosixSignal.SIGHUP, context =>
{
    context.Cancel = true;
    app.Lifetime.StopApplication();
    stopped.Wait(TimeSpan.FromSeconds(4));
});

app.Run();
stopped.Set();

// Lets the tests start the app with WebApplicationFactory<Program>.
public partial class Program
{
}
