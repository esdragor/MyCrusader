namespace HOLLauncher.Tests;

/** L23: one launcher per game folder; each test uses its own kernel object name (never a real launcher's). */
public class SingleInstanceTests
{
    private static string TestName() => @"Local\HOLLauncher.Tests." + Guid.NewGuid().ToString("N");

    [Fact]
    public void SameFolderSameNameWhateverItsForm()
    {
        Assert.Equal(SingleInstance.NameFor(@"C:\Games\HOL"), SingleInstance.NameFor(@"c:\games\hol\"));
        Assert.Equal(SingleInstance.NameFor(@"C:\Games\HOL"), SingleInstance.NameFor(@"C:\Games\Other\..\HOL"));
        Assert.NotEqual(SingleInstance.NameFor(@"C:\Games\HOL"), SingleInstance.NameFor(@"D:\Games\HOL"));
        Assert.StartsWith(@"Local\HeroesOfLibertyLauncher.", SingleInstance.NameFor(@"C:\Games\HOL"));
        Assert.DoesNotContain(@"\Games", SingleInstance.NameFor(@"C:\Games\HOL")[6..]);
    }

    [Fact]
    public void ASecondStartShowsTheFirstUnlessItIsATrayStart()
    {
        Assert.True(SingleInstance.ShowsFirst(LauncherOptions.Parse([], @"C:\HOL")));
        Assert.True(SingleInstance.ShowsFirst(LauncherOptions.Parse(["--update"], @"C:\HOL")));
        Assert.False(SingleInstance.ShowsFirst(LauncherOptions.Parse(["--tray"], @"C:\HOL")));
    }

    [Fact]
    public void OnlyTheFirstClaimHoldsTheName()
    {
        var name = TestName();
        using var first = SingleInstance.Claim(name);
        Assert.NotNull(first);
        Assert.Null(SingleInstance.Claim(name));
    }

    [Fact]
    public void TheNameIsFreeOnceTheFirstIsGone()
    {
        var name = TestName();
        SingleInstance.Claim(name)!.Dispose();
        Assert.False(SingleInstance.Signal(name));
        using var next = SingleInstance.Claim(name);
        Assert.NotNull(next);
    }

    [Fact]
    public void ASignalReachesTheFirst()
    {
        var name = TestName();
        using var first = SingleInstance.Claim(name)!;
        using var signalled = new ManualResetEventSlim();
        first.Listen(signalled.Set);

        Assert.True(SingleInstance.Signal(name));
        Assert.True(signalled.Wait(TimeSpan.FromSeconds(5)));

        // Every later start too, not only the first one.
        signalled.Reset();
        Assert.True(SingleInstance.Signal(name));
        Assert.True(signalled.Wait(TimeSpan.FromSeconds(5)));
    }

    [Fact]
    public void ReleaseLetsTheRestartedLauncherClaimAndReclaimWaitsForIt()
    {
        var name = TestName();
        using var old = SingleInstance.Claim(name)!;
        old.Release();
        using (var restarted = SingleInstance.Claim(name))
        {
            Assert.NotNull(restarted);
            Assert.False(old.Reclaim()); // the new launcher has it
        }
        Assert.True(old.Reclaim()); // the new launcher could not start: the old one claims it again
        Assert.Null(SingleInstance.Claim(name));
    }
}
