"""Apply layout.json part placements to a COPY of glance-pcb.kicad_pcb.

Run inside KiCad's Scripting Console:  exec(open(r"path\to\apply_layout.py").read())
Edit the four variables below first.

KiCad version: NOT checked (KiCad was not installed where this was written).
Written for the KiCad 9/10 pcbnew API; the .kicad_pcb file header says
version 20260206, which suggests KiCad 10. The script prints the real
pcbnew version at runtime so you can confirm.
"""
import json
import os

import pcbnew

INPUT_BOARD = r"C:\Users\uromba\OneDrive - McGill University\glance-pcb\.history\glance-pcb.kicad_pcb"
LAYOUT_FILE = r"C:\Users\uromba\OneDrive - McGill University\glance-pcb\placement\layout.json"
OUTPUT_BOARD = r"C:\Users\uromba\OneDrive - McGill University\glance-pcb\.history\glance-pcb-placed.kicad_pcb"
DRY_RUN = True  # True = print summary only, write nothing

if os.path.realpath(INPUT_BOARD) == os.path.realpath(OUTPUT_BOARD):
    print("Refusing to run: OUTPUT_BOARD equals INPUT_BOARD.")
else:
    print("pcbnew version:", pcbnew.Version())
    try:
        with open(LAYOUT_FILE, encoding="utf-8") as f:
            parts = {p["reference"]: p for p in json.load(f)["parts"]}
    except (OSError, json.JSONDecodeError) as e:
        parts = None
        print("Could not read %s: %s" % (LAYOUT_FILE, e))

    if parts is not None:
        board = pcbnew.LoadBoard(INPUT_BOARD)
        # Layout origin is the board's top-left corner: take it from Edge.Cuts (read-only).
        box = board.GetBoardEdgesBoundingBox()
        ox, oy = box.GetX(), box.GetY()
        print("board origin (mm): %.3f, %.3f" % (pcbnew.ToMM(ox), pcbnew.ToMM(oy)))

        fps = {fp.GetReferenceAsString(): fp for fp in board.GetFootprints()}

        for ref in sorted(set(parts) - set(fps)):
            print("Layout reference missing from board:", ref)
        for ref in sorted(set(fps) - set(parts)):
            print("Board footprint missing from layout (left untouched):", ref)

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
                print("WARNING: %s is on the bottom but layout says top; side not changed" % ref)

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

        if DRY_RUN:
            print("\nDRY_RUN is True: nothing written.")
        else:
            pcbnew.SaveBoard(OUTPUT_BOARD, board)
            print("\nWrote", OUTPUT_BOARD)
