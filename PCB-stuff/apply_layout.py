#!/usr/bin/env python3
"""Apply layout.yaml part placements to a COPY of glance-pcb.kicad_pcb.

Run with KiCad's own Python (it contains pcbnew), e.g. on macOS:
  /Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/Current/bin/python3 apply_layout.py --dry-run

KiCad version: NOT checked (KiCad was not installed where this was written).
Written for the KiCad 9/10 pcbnew API; the .kicad_pcb file header says
version 20260206, which suggests KiCad 10. The script prints the real
pcbnew version at runtime so you can confirm.
Needs PyYAML in the same Python (pip install pyyaml) -- if KiCad's Python lacks it.
"""
import argparse
import os
import sys

import pcbnew
import yaml

IN_PATH = "glance-pcb.kicad_pcb"
OUT_PATH = "glance-pcb-placed.kicad_pcb"
YAML_PATH = "layout.yaml"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true", help="print summary, write nothing")
    args = ap.parse_args()

    if os.path.realpath(IN_PATH) == os.path.realpath(OUT_PATH):
        sys.exit("Refusing to run: output path equals input path.")

    print("pcbnew version:", pcbnew.Version())
    with open(YAML_PATH) as f:
        parts = {p["reference"]: p for p in yaml.safe_load(f)["parts"]}

    board = pcbnew.LoadBoard(IN_PATH)
    # YAML origin is the board's top-left corner: take it from Edge.Cuts (read-only).
    box = board.GetBoardEdgesBoundingBox()
    ox, oy = box.GetX(), box.GetY()
    print("board origin (mm): %.3f, %.3f" % (pcbnew.ToMM(ox), pcbnew.ToMM(oy)))

    fps = {fp.GetReferenceAsString(): fp for fp in board.GetFootprints()}

    for ref in sorted(set(parts) - set(fps)):
        print("YAML reference missing from board:", ref)
    for ref in sorted(set(fps) - set(parts)):
        print("Board footprint missing from YAML (left untouched):", ref)

    rows = []
    for ref, p in parts.items():
        fp = fps.get(ref)
        if fp is None:
            continue
        old = fp.GetPosition()
        old_rot = fp.GetOrientationDegrees()
        old_side = "bottom" if fp.IsFlipped() else "top"

        want = p.get("side", "top")
        if want == "bottom" and not fp.IsFlipped():
            # Flip() mirrors orientation too, so it must come before SetOrientationDegrees.
            fp.Flip(fp.GetPosition(), False)  # False = flip top<->bottom (KiCad 8/9 signature)
        elif want == "top" and fp.IsFlipped():
            print("WARNING: %s is on the bottom but YAML says top; side not changed" % ref)

        fp.SetOrientationDegrees(float(p.get("rotation", 0)))
        fp.SetPosition(pcbnew.VECTOR2I(ox + pcbnew.FromMM(p["x"]), oy + pcbnew.FromMM(p["y"])))

        new = fp.GetPosition()
        rows.append((ref,
                     "%.3f, %.3f" % (pcbnew.ToMM(old.x - ox), pcbnew.ToMM(old.y - oy)),
                     "%.3f, %.3f" % (pcbnew.ToMM(new.x - ox), pcbnew.ToMM(new.y - oy)),
                     "%g -> %g" % (old_rot, fp.GetOrientationDegrees()),
                     "%s -> %s" % (old_side, "bottom" if fp.IsFlipped() else "top")))

    print("\n%-5s %-20s %-20s %-14s %s" % ("ref", "old x,y (mm)", "new x,y (mm)", "rotation", "side"))
    for r in rows:
        print("%-5s %-20s %-20s %-14s %s" % r)

    if args.dry_run:
        print("\n--dry-run: nothing written.")
        return
    pcbnew.SaveBoard(OUT_PATH, board)
    print("\nWrote", OUT_PATH)


if __name__ == "__main__":
    main()
