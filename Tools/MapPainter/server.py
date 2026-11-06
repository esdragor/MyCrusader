"""The map painter, served on http://localhost:8765 (run: python Tools/MapPainter/server.py).
GET /            the page, built on each request from Saved/MapLayouts/*.json (written by every map the editor generates) and the saved
                 edits of Tools/Data/MapEdits/*.json;
POST /save/<map> writes Tools/Data/MapEdits/<map>.json (the strokes the next generation of that map applies, common.apply_edits)."""
import glob
import http.server
import json
import os
import re

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LAYOUTS = os.path.join(ROOT, "Saved", "MapLayouts")
EDITS = os.path.join(ROOT, "Tools", "Data", "MapEdits")
TEMPLATE = os.path.join(os.path.dirname(__file__), "painter_template.html")


def page():
    maps, edits = {}, {}
    for path in sorted(glob.glob(os.path.join(LAYOUTS, "*.json"))):
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
        data["slots"] = [[round(a), round(b)] for a, b in data["slots"]]
        data["relays"] = [[round(a), round(b), round(c)] for a, b, c in data["relays"]]
        data["half"] = round(data["half"])
        maps[data["map"]] = data
        saved = os.path.join(EDITS, data["map"] + ".json")
        if os.path.exists(saved):
            with open(saved, encoding="utf-8") as handle:
                edits[data["map"]] = json.load(handle)
    with open(TEMPLATE, encoding="utf-8") as handle:
        text = handle.read()
    return text.replace("/*MAPS*/{}", json.dumps(maps, separators=(",", ":"))).replace("/*EDITS*/{}", json.dumps(edits, separators=(",", ":")))


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        body = ("<!doctype html><html><head><meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\"></head><body>" + page() + "</body></html>").encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        match = re.fullmatch(r"/save/([A-Za-z0-9_]+)", self.path)
        if not match:
            self.send_error(404)
            return
        data = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        os.makedirs(EDITS, exist_ok=True)
        path = os.path.join(EDITS, match.group(1) + ".json")
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data, handle, separators=(",", ":"))
        # 2026-11-06: every painted map gets the validated spacing (hex lattice of 0.90 m, Forest_Medium's) and is stretched to its other sizes
        # (common.stretch_edits) unless its settings file already exists (the user's own settings are never overwritten).
        settings = os.path.join(EDITS, match.group(1) + ".settings.json")
        if not os.path.exists(settings):
            with open(settings, "w", encoding="utf-8") as handle:
                json.dump({"keep": 1.0, "gap": 0, "lattice": 90, "stretch": True}, handle)
        body = ("%d gommes, %d ajouts" % (len(data.get("remove", [])), len(data.get("add", [])))).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *args):
        pass


if __name__ == "__main__":
    print("Map painter on http://localhost:8765 (Ctrl+C stops)")
    http.server.ThreadingHTTPServer(("127.0.0.1", 8765), Handler).serve_forever()
