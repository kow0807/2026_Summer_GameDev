"""
Quoridor AI - Step 1 (v2)
=========================

"""

import sys
import json
from collections import deque

BOARD_SIZE = 9


# ──────────────────────────────────────────
# 壁チェック
# ──────────────────────────────────────────

def can_move(x, y, dx, dy, vw, hw):
    """
    (x,y) から (dx,dy) 方向へ移動できるか（壁考慮）

    C++ BuildBoardJson の出力形式:
      vertical_walls[y][x]:   8行(y=0..7) × 9列(x=0..8)
        縦壁 = セル(x,y)と(x+1,y)の間 → vw[y][x], x=0..7
      horizontal_walls[y][x]: 8行(y=0..7) × 9列(x=0..8)
        横壁 = セル(x,y)と(x,y+1)の間 → hw[y][x], x=0..8, y=0..7
    """
    nx, ny = x + dx, y + dy
    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
        return False

    # 縦壁チェック（x と x+1 の間）
    if dx ==  1 and y <= 7 and x <= 7 and vw[y][x]:    return False  # 右
    if dx == -1 and y <= 7 and x >= 1 and vw[y][x-1]:  return False  # 左

    # 横壁チェック（y と y+1 の間）x は 0..8 すべてが対象
    if dy ==  1 and y <= 7 and x <= 8 and hw[y][x]:    return False  # 下
    if dy == -1 and y >= 1 and x <= 8 and hw[y-1][x]:  return False  # 上

    return True


# ──────────────────────────────────────────
# BFS 最短距離
# ──────────────────────────────────────────

def bfs_dist(px, py, goal_y, vw, hw):
    """(px,py) からゴールライン goal_y までの最短手数。到達不能なら 999。"""
    if py == goal_y:
        return 0
    visited = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
    q = deque([(px, py, 0)])
    visited[px][py] = True
    while q:
        x, y, d = q.popleft()
        for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
            nx, ny = x + dx, y + dy
            if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
                continue
            if visited[nx][ny]:
                continue
            if not can_move(x, y, dx, dy, vw, hw):
                continue
            visited[nx][ny] = True
            if ny == goal_y:
                return d + 1
            q.append((nx, ny, d + 1))
    return 999


# ──────────────────────────────────────────
# 合法移動手の生成（ジャンプ込み）
# ──────────────────────────────────────────

def get_move_candidates(p1x, p1y, p0x, p0y, vw, hw):
    """CPU(p1) の合法移動手。相手がいるマスへはジャンプ or 側面移動。"""
    result = []
    for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
        if not can_move(p1x, p1y, dx, dy, vw, hw):
            continue
        nx, ny = p1x + dx, p1y + dy
        if nx == p0x and ny == p0y:            # 相手がいる
            if can_move(nx, ny, dx, dy, vw, hw):
                result.append((nx + dx, ny + dy))      # 直線ジャンプ
            else:
                for sx, sy in [(-dy, dx), (dy, -dx)]:  # 側面移動
                    if can_move(nx, ny, sx, sy, vw, hw):
                        result.append((nx + sx, ny + sy))
        else:
            result.append((nx, ny))
    return result


# ──────────────────────────────────────────
# 行動選択
# ──────────────────────────────────────────

def choose_move(state):
    p0x, p0y = state["players"][0]   # プレイヤー (ゴール: y=8)
    p1x, p1y = state["players"][1]   # CPU        (ゴール: y=0)
    vw = state["vertical_walls"]
    hw = state["horizontal_walls"]

    cands = get_move_candidates(p1x, p1y, p0x, p0y, vw, hw)

    if not cands:
        return {"type": "move", "x": p1x, "y": p1y}

    # ゴールまでの BFS 最短距離が最小の手を選ぶ
    best = min(cands, key=lambda pos: bfs_dist(pos[0], pos[1], 0, vw, hw))
    return {"type": "move", "x": best[0], "y": best[1]}


# ──────────────────────────────────────────
# メインループ
# ──────────────────────────────────────────

for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    try:
        state = json.loads(line)

        state["vertical_walls"] = [
            [bool(int(v)) for v in row]
            for row in state["vertical_walls"]
        ]
        state["horizontal_walls"] = [
            [bool(int(v)) for v in row]
            for row in state["horizontal_walls"]
        ]

        action = choose_move(state)

        sys.stdout.write(json.dumps(action) + "\n")
        sys.stdout.flush()

    except Exception as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.stderr.flush()