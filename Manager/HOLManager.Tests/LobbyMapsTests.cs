using HOLManager.Lobbies;

namespace HOLManager.Tests;

/** C0.2 (Cartes): the maps the manager serves mirror Tools/Data/Maps.csv, the one table of the match maps. */
public class LobbyMapsTests
{
    /** Tools/Data/Maps.csv, found from the test's folder up to the project's. */
    private static string MapsCsv()
    {
        for (var dir = new DirectoryInfo(AppContext.BaseDirectory); dir != null; dir = dir.Parent)
        {
            var path = Path.Combine(dir.FullName, "Tools", "Data", "Maps.csv");
            if (File.Exists(path))
            {
                return path;
            }
        }
        throw new FileNotFoundException("Tools/Data/Maps.csv not found above " + AppContext.BaseDirectory);
    }

    [Fact]
    public void MapsCsvNamesInOrderAreTheMapsServed()
    {
        var lines = File.ReadAllLines(MapsCsv()).Where(l => l.Trim() != "").ToList();
        Assert.Equal("Name,DisplayName,Recipe,Budget", lines[0].Trim());
        var names = lines.Skip(1).Select(l => l.Split(',')[0].Trim()).ToList();
        Assert.NotEmpty(names);
        Assert.Equal(names, LobbyRules.Maps.Select(m => m.Name));
        Assert.Same(LobbyRules.Maps, LobbyRules.DefaultMaps);
        Assert.Equal("Plain", LobbyRules.DefaultMaps[0].Name);
    }

    [Fact]
    public void EveryMapServedHasTheFiveSizesAndEightPlaces()
    {
        Assert.All(LobbyRules.Maps, map =>
        {
            Assert.Equal(LobbyRules.SlotCount, map.MaxPlayers);
            Assert.Same(LobbyRules.MapSizes, map.Sizes);
            Assert.Equal($"/Game/Maps/{map.Name}.{map.Name}", map.ObjectPath);
        });
    }
}
