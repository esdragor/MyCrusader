"""Shared writer of the art assets' fields (AR0-d2 3b, D925): an editor script that makes a model writes it into
/Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type> (UHOLArtVisual), never into a column of a type CSV.
STUB (red): writes nothing yet.
"""


def set_art_field(family, type_name, field, value):
    """Sets DA_Art_<type_name>.<field> to value (an asset path) and saves the asset. STUB: does nothing."""
    return None
