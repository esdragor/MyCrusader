"""The glade sweep (2026-11-07, chantier Cartes; E-maps-review defect 5: "the forests are solid blobs, the glades are nearly invisible").

NOT a match map (it is not in Tools/Data/Maps.csv): a 400 m test map, /Game/Maps/GladeSweep, with the same forest laid out EIGHT times side by
side, each time with another number and size of glades, so that the user picks the look on the screen (scenario glade_sweep; Enter = the next
variant). Same seed everywhere: the outlines are alike, only the clearings change. Made by Tools/make_glade_sweep.ps1 (the editor alone).

A variant is (label, glades, scale, reach): reach = common.GLADE_REACH (how far out a glade may be centred; far out it eats the outline: a crescent), glades = common.GLADE_FORCE (0 = the draws of today's maps, a glade or a path of 3 m, half of the groves have
none), scale = common.GLADE_SCALE (the glade's radius is 2.5 to 3.8 m x scale). Every forest has FOREST_TREES trees.
"""
from maps import common

GROUND = "MI_Ground_Plain"
NO_EDITS = True  # common.make_map: no painted forests (the file is named Crown.umap)
FOREST_TREES = 300  # the real forests are 200 trees or more (E-maps-review); 300 shows a clearing better
SEED = 7
CELL = 7000.0  # cm between two forests' centres along x
ROW = 8000.0  # cm between the two rows
VARIANTS = [
    ("A", 0, 1.0, 0.55),  # today's maps
    ("B", 1, 1.5, 0.55),
    ("C", 1, 2.5, 0.15),  # one big glade near the middle: a ring of trees round it
    ("D", 2, 1.5, 0.55),
    ("E", 3, 1.5, 0.55),
    ("F", 3, 1.5, 0.35),  # three glades kept inside the blob
    ("G", 5, 1.0, 0.6),  # many small ones
    ("H", 2, 2.5, 0.3),  # two big ones kept inside
]


def centre(index):
    """(x, y) of the variant's forest: 4 columns, 2 rows, the map's middle in the middle of the grid."""
    column, row = index % 4, index // 4
    return ((column - 1.5) * CELL, (row - 0.5) * ROW)


def obstacles(recipe):
    return 0


def sources(recipe):
    placed = []
    saved = (common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH)
    try:
        for index, (_, glades, scale, reach) in enumerate(VARIANTS):
            common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH = glades, scale, reach
            common.SHAPES.clear()  # the shape cache does not know the knobs
            placed += [common.placed_source("Tree", x, y) for x, y in common.grove_points(SEED, centre(index), FOREST_TREES, reach=3500.0)]
    finally:
        common.GLADE_FORCE, common.GLADE_SCALE, common.GLADE_REACH = saved
        common.SHAPES.clear()
    return placed, len(VARIANTS)
