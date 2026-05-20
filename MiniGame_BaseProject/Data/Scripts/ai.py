import sys
import json

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    try:
        state = json.loads(line)
        cands = state["move_candidates"]

        if not cands:
            px, py = state["players"][1]
            move = {"type": "move", "x": px, "y": py}
        else:
            # ゴール(y=0)に最も近い手を選ぶ
            best = min(cands, key=lambda pos: pos[1])
            move = {"type": "move", "x": best[0], "y": best[1]}

        sys.stdout.write(json.dumps(move) + "\n")
        sys.stdout.flush()

    except Exception as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.stderr.flush()