"""
Quoridor AI - Step 3: アルファベータ法 (防衛・妨害ブースト＋高速軽量化版)
=======================================
修正内容:
  - main() 内の構文エラー（リスト内包表記の重複）を修正。
  - プレイヤー接近時の妨害ブーストロジックを正常に実行可能に。
"""

import sys
import json
from collections import deque

BOARD_SIZE = 9
DEBUG_MODE =True

# ──────────────────────────────────────────
# 移動チェック (C++の仕様に完全同期)
# ──────────────────────────────────────────

def can_move(x, y, dx, dy, vw, hw):
    nx, ny = x + dx, y + dy
    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
        return False

    if dx == 1:
        if 0 <= x < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][x]: return False
            if y > 0 and vw[y - 1][x]: return False
    elif dx == -1:
        cx = x - 1
        if 0 <= cx < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][cx]: return False
            if y > 0 and vw[y - 1][cx]: return False
    elif dy == 1:
        if 0 <= y < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[y][x]: return False
            if x > 0 and hw[y][x - 1]: return False
    elif dy == -1:
        cy = y - 1
        if 0 <= cy < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[cy][x]: return False
            if x > 0 and hw[cy][x - 1]: return False

    return True


# ──────────────────────────────────────────
# BFS (最短距離・経路計算)
# ──────────────────────────────────────────

def bfs_dist(px, py, goal_y, vw, hw):
    if py == goal_y:
        return 0
    visited = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
    q = deque([(px, py, 0)])
    visited[py][px] = True
    while q:
        x, y, d = q.popleft()
        for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
            nx, ny = x + dx, y + dy
            if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
                continue
            if visited[ny][nx]:
                continue
            if not can_move(x, y, dx, dy, vw, hw):
                continue
            visited[ny][nx] = True
            if ny == goal_y:
                return d + 1
            q.append((nx, ny, d + 1))
    return 999


def bfs_path_cells(px, py, goal_y, vw, hw):
    if py == goal_y:
        return set()
    visited = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
    parent = {}
    q = deque([(px, py)])
    visited[py][px] = True
    goal_cell = None
    while q:
        x, y = q.popleft()
        if y == goal_y:
            goal_cell = (x, y)
            break
        for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
            nx, ny = x + dx, y + dy
            if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
                continue
            if visited[ny][nx]:
                continue
            if not can_move(x, y, dx, dy, vw, hw):
                continue
            visited[ny][nx] = True
            parent[(nx, ny)] = (x, y)
            q.append((nx, ny))
    if not goal_cell:
        return set()
    path = set()
    cur = goal_cell
    while cur != (px, py):
        path.add(cur)
        cur = parent[cur]
    path.add((px, py))
    return path


# ──────────────────────────────────────────
# 先読み用の移動手生成
# ──────────────────────────────────────────

def get_moves(px, py, ox, oy, vw, hw):
    result = []
    for dx, dy in [(0, -1), (1, 0), (-1, 0), (0, 1)]:
        if not can_move(px, py, dx, dy, vw, hw):
            continue
        nx, ny = px + dx, py + dy
        if nx == ox and ny == oy:
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
# 壁ユーティリティ
# ──────────────────────────────────────────

def can_place(wx, wy, is_v, vw, hw):
    if not (0 <= wx < BOARD_SIZE - 1 and 0 <= wy < BOARD_SIZE - 1):
        return False
    if is_v:
        if vw[wy][wx]: return False
        if wy > 0 and vw[wy - 1][wx]: return False
        if wy < BOARD_SIZE - 2 and vw[wy + 1][wx]: return False
        if hw[wy][wx]: return False
    else:
        if hw[wy][wx]: return False
        if wx > 0 and hw[wy][wx - 1]: return False
        if wx < BOARD_SIZE - 2 and hw[wy][wx + 1]: return False
        if vw[wy][wx]: return False
    return True


def apply_wall(wx, wy, is_v, vw, hw, val):
    if is_v:
        vw[wy][wx] = val
    else:
        hw[wy][wx] = val


def wall_candidates(path, d_diff, walls, vw, hw, opp_dist_to_goal):
    """
    ⚡妨害強化＆高速化ポイント:
    通常時は手数差が開いたら壁シミュレーションをスキップして高速化するが、
    相手がゴール間近（opp_dist_to_goal <= 4）のときは、死に物狂いで壁を探すために枝切りを解除する。
    """
    if walls <= 0:
        return []
        
    # 相手がゴールから遠いときだけ、手数差による高速化の枝切りを適用する
    if opp_dist_to_goal > 4:
        if d_diff < -2 or d_diff > 3:
            return []
        
    seen = set()
    result = []
    for (cx, cy) in path:
        # 相手の直近のマス周辺のみをターゲットにする
        for wy in range(max(0, cy - 1), min(8, cy + 2)):
            for wx in range(max(0, cx - 1), min(8, cx + 2)):
                for is_v in [True, False]:
                    key = (wx, wy, is_v)
                    if key in seen:
                        continue
                    seen.add(key)
                    if can_place(wx, wy, is_v, vw, hw):
                        result.append(key)
    return result


# ──────────────────────────────────────────
# アルファベータ法 (動的防衛強化型評価関数)
# ──────────────────────────────────────────

def alphabeta(p0x, p0y, w0, p1x, p1y, w1, vw, hw,
              depth, alpha, beta, is_cpu, use_wall):
    if p1y == 0: return 9999
    if p0y == 8: return -9999

    d0 = bfs_dist(p0x, p0y, 8, vw, hw) # プレイヤーの残り歩数
    d1 = bfs_dist(p1x, p1y, 0, vw, hw) # CPUの残り歩数

    if depth == 0:
        # ⚡【評価関数の動的チューニング】
        # プレイヤーがゴール（y=8）に近づくほど、プレイヤーの距離を引き延ばす妨害の価値を高める
        if d0 <= 2:
            # 残り1〜2歩の超緊急事態：自分の進軍を後回しにして、全力で妨害壁を建てる
            player_weight = 4.5
        elif d0 <= 4:
            # 残り3〜4歩の警戒状態：妨害の価値を通常時の3倍以上に引き上げる
            player_weight = 2.5
        else:
            # 通常時：元のバランス（自分の進軍をやや優先）
            player_weight = 0.8

        # 動的に決定した重みでベーススコアを算出
        base_score = (d0 * player_weight) - (d1 * 2.0)
        
        cpu_moves_count = len(get_moves(p1x, p1y, p0x, p0y, vw, hw))
        my_mobility_bonus = cpu_moves_count * 0.4
        opp_moves_count = len(get_moves(p0x, p0y, p1x, p1y, vw, hw))
        opp_mobility_penalty = (4 - opp_moves_count) * 0.15
        return base_score + my_mobility_bonus + opp_mobility_penalty

    if is_cpu:
        best = -9999
        moves = get_moves(p1x, p1y, p0x, p0y, vw, hw)
        moves.sort(key=lambda m: bfs_dist(m[0], m[1], 0, vw, hw))
        for nx, ny in moves:
            v = alphabeta(p0x, p0y, w0, nx, ny, w1, vw, hw,
                          depth - 1, alpha, beta, False, use_wall)
            if v > best: best = v
            alpha = max(alpha, v)
            if alpha >= beta: break

        if use_wall and w1 > 0:
            path0 = bfs_path_cells(p0x, p0y, 8, vw, hw)
            # 引数の末尾にプレイヤーのゴール距離「d0」を渡す
            for wx, wy, is_v in wall_candidates(path0, d1 - d0, w1, vw, hw, d0):
                apply_wall(wx, wy, is_v, vw, hw, True)
                nd0 = bfs_dist(p0x, p0y, 8, vw, hw)
                nd1 = bfs_dist(p1x, p1y, 0, vw, hw)
                if nd0 < 999 and nd1 < 999:
                    v = alphabeta(p0x, p0y, w0, p1x, p1y, w1 - 1, vw, hw,
                                  depth - 1, alpha, beta, False, use_wall)
                    if v > best: best = v
                    alpha = max(alpha, v)
                apply_wall(wx, wy, is_v, vw, hw, False)
                if alpha >= beta: break
        return best

    else:
        best = 9999
        moves = get_moves(p0x, p0y, p1x, p1y, vw, hw)
        moves.sort(key=lambda m: bfs_dist(m[0], m[1], 8, vw, hw))
        for nx, ny in moves:
            v = alphabeta(nx, ny, w0, p1x, p1y, w1, vw, hw,
                          depth - 1, alpha, beta, True, use_wall)
            if v < best: best = v
            beta = min(beta, v)
            if alpha >= beta: break

        if use_wall and w0 > 0:
            path1 = bfs_path_cells(p1x, p1y, 0, vw, hw)
            # 引数の末尾にCPUのゴール距離「d1」を渡す
            for wx, wy, is_v in wall_candidates(path1, d0 - d1, w0, vw, hw, d1):
                apply_wall(wx, wy, is_v, vw, hw, True)
                nd0 = bfs_dist(p0x, p0y, 8, vw, hw)
                nd1 = bfs_dist(p1x, p1y, 0, vw, hw)
                if nd0 < 999 and nd1 < 999:
                    v = alphabeta(p0x, p0y, w0 - 1, p1x, p1y, w1, vw, hw,
                                  depth - 1, alpha, beta, True, use_wall)
                    if v < best: best = v
                    beta = min(beta, v)
                apply_wall(wx, wy, is_v, vw, hw, False)
                if alpha >= beta: break
        return best


# ──────────────────────────────────────────
# ルート探索（最善手を決定）
# ──────────────────────────────────────────

def choose_action(state):
    p0x, p0y = state["players"][0]
    p1x, p1y = state["players"][1]
    vw = [row[:] for row in state["vertical_walls"]]
    hw = [row[:] for row in state["horizontal_walls"]]
    w0 = state["remaining_walls"][0]
    w1 = state["remaining_walls"][1]

    root_moves = [tuple(m) for m in state["move_candidates"]]
    
    d0 = bfs_dist(p0x, p0y, 8, vw, hw) # プレイヤー0のゴール(y=8)への距離
    d1 = bfs_dist(p1x, p1y, 0, vw, hw) # CPU(プレイヤー1)のゴール(y=0)への距離
    
    d_diff_cpu = d0 - d1 

    # フェーズと手数差による深さ(depth)の動的制御
    if d1 <= 4:
        depth = 2
        use_wall = False
    elif abs(d_diff_cpu) >= 3:
        depth = 2
        use_wall = True
    else:
        depth = 3
        use_wall = True

    best_score = -9999
    best_action = None

    if not root_moves:
        root_moves = [(p1x, p1y)]

    root_moves_sorted = sorted(root_moves, key=lambda m: bfs_dist(m[0], m[1], 0, vw, hw))

    for nx, ny in root_moves_sorted:
        v = alphabeta(p0x, p0y, w0, nx, ny, w1, vw, hw,
                      depth - 1, best_score, 9999, False, use_wall)
        if v > best_score:
            best_score = v
            best_action = {"type": "move", "x": nx, "y": ny}

    if use_wall and w1 > 0:
        path0 = bfs_path_cells(p0x, p0y, 8, vw, hw)
        # 引数の末尾にプレイヤーのゴール距離「d0」を渡す
        for wx, wy, is_v in wall_candidates(path0, d_diff_cpu, w1, vw, hw, d0):
            apply_wall(wx, wy, is_v, vw, hw, True)
            nd0 = bfs_dist(p0x, p0y, 8, vw, hw)
            nd1 = bfs_dist(p1x, p1y, 0, vw, hw)
            if nd0 < 999 and nd1 < 999:
                v = alphabeta(p0x, p0y, w0, p1x, p1y, w1 - 1, vw, hw,
                              depth - 1, best_score, 9999, False, use_wall)
                if v > best_score:
                    best_score = v
                    best_action = {
                        "type": "wall",
                        "x": wx, "y": wy,
                        "vertical": is_v
                    }
            apply_wall(wx, wy, is_v, vw, hw, False)

    if best_action is None:
        best_action = {"type": "move", "x": root_moves_sorted[0][0], "y": root_moves_sorted[0][1]}

    return best_action


# ──────────────────────────────────────────
# メインループ (C++パイプ通信対応版)
# ──────────────────────────────────────────

def main():
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            state = json.loads(line)

            # 【修正箇所】重複行を削除し、正しく型変換する形式に修正
            state["vertical_walls"] = [
                [bool(int(v)) for v in row]
                for row in state["vertical_walls"]
            ]
            state["horizontal_walls"] = [
                [bool(int(v)) for v in row]
                for row in state["horizontal_walls"]
            ]

            action = choose_action(state)

            print(json.dumps(action), flush=True)

        except Exception as e:
            sys.stderr.write(f"Python Error: {e}\n")
            sys.stderr.flush()

if __name__ == "__main__":
    main()