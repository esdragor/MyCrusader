"""A stand-in `unreal` module, so that the map recipes (Tools/Editor/maps) run without the editor: their geometry is plain Python, the editor is
only needed to lay actors and save the file. Versioned here (V4b, 2026-11-02) after being kept by hand next to the *_dump.py scripts.
  import maps_stub; ROOT = maps_stub.install()      # then: from maps import common, steppe
Any attribute of the module is a harmless object that can be called; unreal.Vector keeps x, y, z.
"""
import os
import sys
import types

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))


class Any:
    """Any attribute or call of the stand-in module."""

    def __init__(self, *args, **kwargs):
        pass

    def __getattr__(self, name):
        return Any()

    def __call__(self, *args, **kwargs):
        return Any()


class Vector:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


def install():
    """Puts the stand-in `unreal` in sys.modules and Tools/Editor on the path; returns the project root."""
    fake = types.ModuleType("unreal")
    fake.Vector = Vector
    fake.LinearColor = lambda *values: values
    fake.Paths = types.SimpleNamespace(convert_relative_path_to_full=lambda path: path, project_dir=lambda: ROOT + os.sep)
    fake.__getattr__ = lambda name: Any()
    sys.modules["unreal"] = fake
    sys.path.insert(0, os.path.join(ROOT, "Tools", "Editor"))
    return ROOT