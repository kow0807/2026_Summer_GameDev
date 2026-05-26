"""
Quoridor AI - Step 2
====================
Step1 からの追加:
  - 壁設置の判断を追加
  - 評価関数: opp_dist - cpu_dist を最大化
    (相手のゴールまでの距離が大きく、自分が小さいほど有利)
  - 移動と壁設置を同じスコアで比較して最善手を選ぶ
  - 両者がゴールに到達できる壁のみ許可（詰み禁止ルール）
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
    C++ BuildBoardJson の出力形式:
      vertical_walls[y][x]:   8行(y=0..7) × 9列(x=0..8)
      horizontal_walls[y][x]: 8行(y=0..7) × 9列(x=0..8)
    """
    nx, ny = x + dx, y + dy
    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
        return False
    if dx ==  1 and y <= 7 and x <= 7 and vw[y][x]:    return False
    if dx == -1 and y <= 7 and x >= 1 and vw[y][x-1]:  return False
    if dy ==  1 and y <= 7 and x <= 8 and hw[y][x]:    return False
    if dy == -1 and y >= 1 and x <= 8 and hw[y-1][x]:  return False
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
    result = []
    for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
        if not can_move(p1x, p1y, dx, dy, vw, hw):
            continue
        nx, ny = p1x + dx, p1y + dy
        if nx == p0x and ny == p0y:
            if can_move(nx, ny, dx, dy, vw, hw):
                result.append((nx + dx, ny + dy))
            else:
                for sx, sy in [(-dy, dx), (dy, -dx)]:
                    if can_move(nx, ny, sx, sy, vw, hw):
                        result.append((nx + sx, ny + sy))
        else:
            result.append((nx, ny))
    return result


# ──────────────────────────────────────────
# 壁設置ユーティリティ
# ──────────────────────────────────────────

def can_place_wall(wx, wy, is_v, vw, hw):
    """
    壁を置けるか判定
    縦壁: vw[wy][wx] と vw[wy+1][wx] を使う → wx=0..7, wy=0..6
    横壁: hw[wy][wx] と hw[wy][wx+1] を使う → wx=0..7, wy=0..7
    """
    if is_v:
        if not (0 <= wx <= 7 and 0 <= wy <= 6):
            return False
        if vw[wy][wx] or vw[wy+1][wx]:        # すでに壁がある
            return False
        if hw[wy][wx] and hw[wy][wx+1]:        # 十字チェック
            return False
    else:
        if not (0 <= wx <= 7 and 0 <= wy <= 7):
            return False
        if hw[wy][wx] or hw[wy][wx+1]:
            return False
        if wy <= 6 and vw[wy][wx] and vw[wy+1][wx]:
            return False
    return True


def apply_wall(wx, wy, is_v, vw, hw, val):
    """壁を置く(val=True) または 外す(val=False)"""
    if is_v:
        vw[wy][wx]   = val
        vw[wy+1][wx] = val
    else:
        hw[wy][wx]   = val
        hw[wy][wx+1] = val


# ──────────────────────────────────────────
# 行動選択
# ──────────────────────────────────────────

def choose_action(state):
    p0x, p0y = state["players"][0]   # プレイヤー (ゴール: y=8)
    p1x, p1y = state["players"][1]   # CPU        (ゴール: y=0)
    vw = [row[:] for row in state["vertical_walls"]]
    hw = [row[:] for row in state["horizontal_walls"]]
    walls_left = state["remaining_walls"][1]

    best_score = -9999
    best_action = None

    # ── 移動手の評価 ──────────────────────────
    move_cands = get_move_candidates(p1x, p1y, p0x, p0y, vw, hw)
    if not move_cands:
        move_cands = [(p1x, p1y)]

    for nx, ny in move_cands:
        cpu_dist = bfs_dist(nx, ny, 0, vw, hw)
        opp_dist = bfs_dist(p0x, p0y, 8, vw, hw)
        score = opp_dist - cpu_dist
        if score > best_score:
            best_score = score
            best_action = {"type": "move", "x": nx, "y": ny}

    # ── 壁設置の評価 ──────────────────────────
    if walls_left > 0:
        for wy in range(8):
            for wx in range(8):
                for is_v in [True, False]:
                    if not can_place_wall(wx, wy, is_v, vw, hw):
                        continue

                    apply_wall(wx, wy, is_v, vw, hw, True)

                    opp_dist = bfs_dist(p0x, p0y, 8, vw, hw)
                    cpu_dist = bfs_dist(p1x, p1y, 0, vw, hw)

                    # 両者がゴールに到達できることを確認（詰み禁止）
                    if opp_dist < 999 and cpu_dist < 999:
                        score = opp_dist - cpu_dist
                        if score > best_score:
                            best_score = score
                            best_action = {
                                "type": "wall",
                                "x": wx,
                                "y": wy,
                                "vertical": is_v
                            }

                    apply_wall(wx, wy, is_v, vw, hw, False)

    return best_action


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

        action = choose_action(state)

        sys.stdout.write(json.dumps(action) + "\n")
        sys.stdout.flush()

    except Exception as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.stderr.flush()