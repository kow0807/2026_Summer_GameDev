"""
Quoridor 自己対戦 棋譜収集スクリプト (Phase 2)
=============================================
【目的】
  現在のαβ法AIを両プレイヤーに使って自己対戦させ、
  学習用棋譜データを .jsonl 形式で収集する。

【出力ファイル】
  games_record.jsonl  ← 1行 = 1ゲームの全履歴 + 最終勝者

【1レコードの構造】
  {
    "winner": 0 or 1,
    "steps": [
      {
        "state": { ...盤面JSON... },
        "action": { "type": "move"/"wall", ... }
      },
      ...
    ]
  }

【使い方】
  python self_play_collector.py                        # デフォルト100ゲーム収集（学習なし）
  python self_play_collector.py --games 500            # 500ゲーム収集（学習なし）
  python self_play_collector.py --games 500 --learn    # 収集後にNN学習まで実行
  python self_play_collector.py --games 500 --out my_data.jsonl --learn

【--learn フラグについて】
  指定すると棋譜収集完了後、自動的に train_from_record() が呼ばれる。
  Phase2 でニューラルネットワークの実装が完成したら、
  train_from_record() の中身を本実装に差し替えるだけでOK。
  現時点ではプレースホルダーとして収集済みデータの統計を表示する。
"""

import sys
import json
import argparse
import copy
import time
from collections import deque
from pathlib import Path

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader

# Phase2: NNモデルと特徴量変換を読み込む
# quoridor_net.py が同じディレクトリにあること
try:
    from quoridor_net import QuoridorNet, board_to_tensor, load_model, save_model, DEFAULT_MODEL_PATH
    _NN_AVAILABLE = True
except ImportError:
    _NN_AVAILABLE = False
    print("[警告] quoridor_net.py が見つかりません。--learn は統計表示のみになります。", file=sys.stderr)

# ───────────────────────────────────────────────────────
# 定数
# ───────────────────────────────────────────────────────
BOARD_SIZE = 9
MAX_WALLS  = 10
MAX_STEPS  = 300   # 無限ループ防止：この手数を超えたら引き分け扱いで破棄


# ───────────────────────────────────────────────────────
# 移動チェック (C++ Board::CanMove に完全同期)
# ───────────────────────────────────────────────────────
def can_move(x, y, dx, dy, vw, hw):
    nx, ny = x + dx, y + dy
    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
        return False

    if dx == 1:
        if 0 <= x < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][x]:     return False
            if y > 0            and vw[y - 1][x]:   return False
    elif dx == -1:
        cx = x - 1
        if 0 <= cx < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][cx]:    return False
            if y > 0            and vw[y - 1][cx]:  return False
    elif dy == 1:
        if 0 <= y < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[y][x]:     return False
            if x > 0            and hw[y][x - 1]:   return False
    elif dy == -1:
        cy = y - 1
        if 0 <= cy < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[cy][x]:    return False
            if x > 0            and hw[cy][x - 1]:  return False
    return True


# ───────────────────────────────────────────────────────
# BFS ユーティリティ
# ───────────────────────────────────────────────────────
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
    parent  = {}
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
    path, cur = set(), goal_cell
    while cur != (px, py):
        path.add(cur)
        cur = parent[cur]
    path.add((px, py))
    return path


# ───────────────────────────────────────────────────────
# 移動手生成
# ───────────────────────────────────────────────────────
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


def get_all_move_candidates(px, py, ox, oy, vw, hw):
    """C++ GetAllMoveCandidates に相当"""
    return get_moves(px, py, ox, oy, vw, hw)


# ───────────────────────────────────────────────────────
# 壁ユーティリティ
# ───────────────────────────────────────────────────────
def can_place(wx, wy, is_v, vw, hw):
    if not (0 <= wx < BOARD_SIZE - 1 and 0 <= wy < BOARD_SIZE - 1):
        return False
    if is_v:
        if vw[wy][wx]:                                    return False
        if wy > 0          and vw[wy - 1][wx]:            return False
        if wy < BOARD_SIZE - 2 and vw[wy + 1][wx]:        return False
        if hw[wy][wx]:                                    return False
    else:
        if hw[wy][wx]:                                    return False
        if wx > 0          and hw[wy][wx - 1]:            return False
        if wx < BOARD_SIZE - 2 and hw[wy][wx + 1]:        return False
        if vw[wy][wx]:                                    return False
    return True


def can_reach_goal(px, py, goal_y, vw, hw):
    return bfs_dist(px, py, goal_y, vw, hw) < 999


def apply_wall(wx, wy, is_v, vw, hw, val):
    if is_v:
        vw[wy][wx] = val
    else:
        hw[wy][wx] = val


def wall_candidates(path, d_diff, walls, vw, hw, opp_dist):
    if walls <= 0:
        return []
    if opp_dist > 4:
        if d_diff < -2 or d_diff > 3:
            return []
    seen, result = set(), []
    for (cx, cy) in path:
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


# ───────────────────────────────────────────────────────
# αβ法 (Quoridor_Ai.py と同一)
# ───────────────────────────────────────────────────────
def alphabeta(p0x, p0y, w0, p1x, p1y, w1, vw, hw,
              depth, alpha, beta, is_cpu, use_wall):
    if p1y == 0: return  9999
    if p0y == 8: return -9999

    d0 = bfs_dist(p0x, p0y, 8, vw, hw)
    d1 = bfs_dist(p1x, p1y, 0, vw, hw)

    if depth == 0:
        if   d0 <= 2: pw = 4.5
        elif d0 <= 4: pw = 2.5
        else:         pw = 0.8
        base = d0 * pw - d1 * 2.0
        mob_cpu = len(get_moves(p1x, p1y, p0x, p0y, vw, hw)) * 0.4
        mob_opp = (4 - len(get_moves(p0x, p0y, p1x, p1y, vw, hw))) * 0.15
        return base + mob_cpu + mob_opp

    if is_cpu:
        best  = -9999
        moves = sorted(get_moves(p1x, p1y, p0x, p0y, vw, hw),
                       key=lambda m: bfs_dist(m[0], m[1], 0, vw, hw))
        for nx, ny in moves:
            v = alphabeta(p0x, p0y, w0, nx, ny, w1, vw, hw,
                          depth - 1, alpha, beta, False, use_wall)
            if v > best: best = v
            alpha = max(alpha, v)
            if alpha >= beta: break

        if use_wall and w1 > 0:
            path0 = bfs_path_cells(p0x, p0y, 8, vw, hw)
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
        best  = 9999
        moves = sorted(get_moves(p0x, p0y, p1x, p1y, vw, hw),
                       key=lambda m: bfs_dist(m[0], m[1], 8, vw, hw))
        for nx, ny in moves:
            v = alphabeta(nx, ny, w0, p1x, p1y, w1, vw, hw,
                          depth - 1, alpha, beta, True, use_wall)
            if v < best: best = v
            beta = min(beta, v)
            if alpha >= beta: break

        if use_wall and w0 > 0:
            path1 = bfs_path_cells(p1x, p1y, 0, vw, hw)
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


# ───────────────────────────────────────────────────────
# 手を選ぶ (choose_action) — ターン番号を引数で渡す
#   turn=0 → players_[0] (ゴール y=8)
#   turn=1 → players_[1] (ゴール y=0, 現在のCPU)
# ───────────────────────────────────────────────────────
def choose_action_for(turn, p0x, p0y, w0, p1x, p1y, w1, vw, hw):
    """
    turn=0 のときはプレイヤー0視点、turn=1 のときはCPU(プレイヤー1)視点で手を選ぶ。
    αβ法は常に「自分=CPU(turn=1)」として書かれているため、
    turn=0 のときは盤面を左右対称に入れ替えて呼び出す。
    """
    if turn == 1:
        # 既存のChoose_action と同じ
        root_moves = get_all_move_candidates(p1x, p1y, p0x, p0y, vw, hw)
        d0 = bfs_dist(p0x, p0y, 8, vw, hw)
        d1 = bfs_dist(p1x, p1y, 0, vw, hw)
        d_diff = d0 - d1

        if d1 <= 4:
            depth, use_wall = 2, False
        elif abs(d_diff) >= 3:
            depth, use_wall = 2, True
        else:
            depth, use_wall = 3, True

        best_score  = -9999
        best_action = None
        root_sorted = sorted(root_moves, key=lambda m: bfs_dist(m[0], m[1], 0, vw, hw))
        if not root_sorted:
            root_sorted = [(p1x, p1y)]

        for nx, ny in root_sorted:
            v = alphabeta(p0x, p0y, w0, nx, ny, w1, vw, hw,
                          depth - 1, best_score, 9999, False, use_wall)
            if v > best_score:
                best_score  = v
                best_action = {"type": "move", "x": nx, "y": ny}

        if use_wall and w1 > 0:
            path0 = bfs_path_cells(p0x, p0y, 8, vw, hw)
            for wx, wy, is_v in wall_candidates(path0, d_diff, w1, vw, hw, d0):
                apply_wall(wx, wy, is_v, vw, hw, True)
                nd0 = bfs_dist(p0x, p0y, 8, vw, hw)
                nd1 = bfs_dist(p1x, p1y, 0, vw, hw)
                if nd0 < 999 and nd1 < 999:
                    v = alphabeta(p0x, p0y, w0, p1x, p1y, w1 - 1, vw, hw,
                                  depth - 1, best_score, 9999, False, use_wall)
                    if v > best_score:
                        best_score  = v
                        best_action = {"type": "wall", "x": wx, "y": wy, "vertical": is_v}
                apply_wall(wx, wy, is_v, vw, hw, False)

        if best_action is None:
            best_action = {"type": "move", "x": root_sorted[0][0], "y": root_sorted[0][1]}
        return best_action

    else:
        # turn=0: 盤面を「プレイヤー0がCPU役」として入れ替えて呼び出す
        # ゴールは y=8 (BOARD_SIZE-1) なので、y座標を反転して y=0 に揃える
        def flip_y(y):
            return BOARD_SIZE - 1 - y

        # 壁配列を上下反転させた新しい配列を作る
        vw_flip = [row[:] for row in reversed(vw)]
        hw_flip = [row[:] for row in reversed(hw)]

        fp0x, fp0y = p0x, flip_y(p0y)   # player0 → CPU役 (ゴール y=0)
        fp1x, fp1y = p1x, flip_y(p1y)   # player1 → 相手役 (ゴール y=8)

        root_moves = get_all_move_candidates(fp0x, fp0y, fp1x, fp1y, vw_flip, hw_flip)
        d0f = bfs_dist(fp1x, fp1y, 8, vw_flip, hw_flip)  # 相手 (p1) のゴール距離
        d1f = bfs_dist(fp0x, fp0y, 0, vw_flip, hw_flip)  # 自分 (p0) のゴール距離
        d_diff = d0f - d1f

        if d1f <= 4:
            depth, use_wall = 2, False
        elif abs(d_diff) >= 3:
            depth, use_wall = 2, True
        else:
            depth, use_wall = 3, True

        best_score  = -9999
        best_action_flip = None
        root_sorted = sorted(root_moves, key=lambda m: bfs_dist(m[0], m[1], 0, vw_flip, hw_flip))
        if not root_sorted:
            root_sorted = [(fp0x, fp0y)]

        for nx, ny in root_sorted:
            v = alphabeta(fp1x, fp1y, w1, nx, ny, w0, vw_flip, hw_flip,
                          depth - 1, best_score, 9999, False, use_wall)
            if v > best_score:
                best_score       = v
                best_action_flip = {"type": "move", "x": nx, "y": ny}

        if use_wall and w0 > 0:
            path1f = bfs_path_cells(fp1x, fp1y, 8, vw_flip, hw_flip)
            for wx, wy, is_v in wall_candidates(path1f, d_diff, w0, vw_flip, hw_flip, d0f):
                apply_wall(wx, wy, is_v, vw_flip, hw_flip, True)
                nd0f = bfs_dist(fp1x, fp1y, 8, vw_flip, hw_flip)
                nd1f = bfs_dist(fp0x, fp0y, 0, vw_flip, hw_flip)
                if nd0f < 999 and nd1f < 999:
                    v = alphabeta(fp1x, fp1y, w1, fp0x, fp0y, w0 - 1, vw_flip, hw_flip,
                                  depth - 1, best_score, 9999, False, use_wall)
                    if v > best_score:
                        best_score       = v
                        best_action_flip = {"type": "wall", "x": wx, "y": wy, "vertical": is_v}
                apply_wall(wx, wy, is_v, vw_flip, hw_flip, False)

        if best_action_flip is None:
            best_action_flip = {"type": "move",
                                "x": root_sorted[0][0], "y": root_sorted[0][1]}

        # y座標を元に戻す
        if best_action_flip["type"] == "move":
            return {"type": "move",
                    "x": best_action_flip["x"],
                    "y": flip_y(best_action_flip["y"])}
        else:
            return {"type": "wall",
                    "x": best_action_flip["x"],
                    "y": flip_y(best_action_flip["y"]),
                    "vertical": best_action_flip["vertical"]}


# ───────────────────────────────────────────────────────
# ゲームシミュレーター
# ───────────────────────────────────────────────────────
class QuoridorGame:
    def __init__(self):
        self.reset()

    def reset(self):
        # player0: y=0 スタート, ゴール y=8
        # player1: y=8 スタート, ゴール y=0
        self.pos   = [[4, 0], [4, 8]]
        self.walls = [MAX_WALLS, MAX_WALLS]
        self.vw    = [[False] * (BOARD_SIZE - 1) for _ in range(BOARD_SIZE)]
        self.hw    = [[False] * BOARD_SIZE       for _ in range(BOARD_SIZE - 1)]
        self.turn  = 0
        self.steps = []

    def check_winner(self):
        if self.pos[0][1] == BOARD_SIZE - 1: return 0
        if self.pos[1][1] == 0:              return 1
        return -1

    def build_state_snapshot(self):
        """現在の盤面を辞書として返す (記録・学習入力用)"""
        p0x, p0y = self.pos[0]
        p1x, p1y = self.pos[1]
        cands = get_all_move_candidates(
            self.pos[self.turn][0], self.pos[self.turn][1],
            self.pos[1 - self.turn][0], self.pos[1 - self.turn][1],
            self.vw, self.hw
        )
        return {
            "turn":            self.turn,
            "players":         [list(p) for p in self.pos],
            "remaining_walls": list(self.walls),
            "move_candidates": [list(c) for c in cands],
            "vertical_walls":  [[int(v) for v in row] for row in self.vw],
            "horizontal_walls":[[int(v) for v in row] for row in self.hw],
        }

    def apply_action(self, action):
        """actionを盤面に反映。成功したら True を返す"""
        t = self.turn
        px, py = self.pos[t]
        ox, oy = self.pos[1 - t]

        if action["type"] == "move":
            nx, ny = action["x"], action["y"]
            cands = get_all_move_candidates(px, py, ox, oy, self.vw, self.hw)
            if (nx, ny) not in cands:
                return False
            self.pos[t] = [nx, ny]

        elif action["type"] == "wall":
            wx, wy, is_v = action["x"], action["y"], action["vertical"]
            if self.walls[t] <= 0:
                return False
            if not can_place(wx, wy, is_v, self.vw, self.hw):
                return False
            apply_wall(wx, wy, is_v, self.vw, self.hw, True)
            # BFS確認
            p0x, p0y = self.pos[0]
            p1x, p1y = self.pos[1]
            ok = (can_reach_goal(p0x, p0y, BOARD_SIZE - 1, self.vw, self.hw) and
                  can_reach_goal(p1x, p1y, 0,              self.vw, self.hw))
            if not ok:
                apply_wall(wx, wy, is_v, self.vw, self.hw, False)
                return False
            self.walls[t] -= 1
        else:
            return False

        self.turn = 1 - t
        return True

    def run_one_game(self):
        """
        1ゲームを自己対戦で完走させ、結果を返す。
        戻り値: {"winner": 0 or 1, "steps": [...]} or None (引き分け/エラー)
        """
        self.reset()
        steps = []

        for step_i in range(MAX_STEPS):
            winner = self.check_winner()
            if winner >= 0:
                return {"winner": winner, "steps": steps}

            state_snap = self.build_state_snapshot()
            t = self.turn
            p0x, p0y = self.pos[0]
            p1x, p1y = self.pos[1]
            w0, w1   = self.walls[0], self.walls[1]

            try:
                action = choose_action_for(
                    t, p0x, p0y, w0, p1x, p1y, w1,
                    [row[:] for row in self.vw],
                    [row[:] for row in self.hw]
                )
            except Exception as e:
                print(f"  [!] choose_action error at step {step_i}, turn {t}: {e}",
                      file=sys.stderr)
                return None

            ok = self.apply_action(action)
            if not ok:
                # フォールバック: 移動候補から最初の手を使う
                p = self.pos[t]
                o = self.pos[1 - t]
                cands = get_all_move_candidates(p[0], p[1], o[0], o[1], self.vw, self.hw)
                if not cands:
                    return None   # 詰み（通常は起こらないはず）
                fallback = {"type": "move", "x": cands[0][0], "y": cands[0][1]}
                self.apply_action(fallback)
                action = fallback

            steps.append({"state": state_snap, "action": action})

        # MAX_STEPS 超過 → 引き分け扱いで破棄
        return None



# ───────────────────────────────────────────────────────
# Dataset
# ───────────────────────────────────────────────────────
class QuoridorDataset(Dataset):
    """
    games_record.jsonl から (特徴量テンソル, rewardラベル) のペアを作る Dataset。
    reward: +1 (勝者の手) / -1 (敗者の手) -> float32 に変換して返す。
    """
    def __init__(self, jsonl_path: Path):
        self.samples: list[tuple[torch.Tensor, float]] = []
        with jsonl_path.open("r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                record = json.loads(line)
                for step in record.get("steps", []):
                    state  = step["state"]
                    reward = float(step.get("reward", 0))
                    try:
                        tensor = board_to_tensor(state)
                        self.samples.append((tensor, reward))
                    except Exception:
                        pass  # 壊れたレコードはスキップ

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        x, y = self.samples[idx]
        return x, torch.tensor(y, dtype=torch.float32)


# ───────────────────────────────────────────────────────
# 学習処理 (Phase2 本実装)
# ───────────────────────────────────────────────────────
def train_from_record(jsonl_path: Path,
                      epochs:     int  = 20,
                      batch_size: int  = 256,
                      lr:         float = 1e-3,
                      model_path: Path = None):
    """
    収集した棋譜データでQuoridorNetを学習する。

    引数:
        jsonl_path : 棋譜ファイル (.jsonl)
        epochs     : 学習エポック数 (デフォルト: 20)
        batch_size : ミニバッチサイズ (デフォルト: 256)
        lr         : 学習率 (デフォルト: 0.001)
        model_path : 保存先 .pth パス (デフォルト: quoridor_net.py の DEFAULT_MODEL_PATH)
    """
    print()
    print("=== 学習フェーズ開始 (Phase2) ===")

    if not _NN_AVAILABLE:
        print("  [!] quoridor_net.py が見つからないため学習をスキップします。")
        return

    if not jsonl_path.exists():
        print(f"  [!] 棋譜ファイルが見つかりません: {jsonl_path}")
        return

    if model_path is None:
        model_path = DEFAULT_MODEL_PATH

    # ── 1. データセット作成 ──
    print(f"  棋譜読み込み中... ({jsonl_path})")
    dataset = QuoridorDataset(jsonl_path)
    if len(dataset) == 0:
        print("  [!] 有効なサンプルが0件です。棋譜を確認してください。")
        return

    # train / val = 9:1 に分割
    val_size   = max(1, len(dataset) // 10)
    train_size = len(dataset) - val_size
    train_ds, val_ds = torch.utils.data.random_split(dataset, [train_size, val_size])

    train_loader = DataLoader(train_ds, batch_size=batch_size, shuffle=True,  drop_last=False)
    val_loader   = DataLoader(val_ds,   batch_size=batch_size, shuffle=False, drop_last=False)

    print(f"  サンプル数  : {len(dataset)}  (train={train_size}, val={val_size})")

    # ── 2. モデル・オプティマイザ準備 ──
    model     = load_model(model_path)          # 既存モデルがあれば継続学習
    optimizer = optim.Adam(model.parameters(), lr=lr, weight_decay=1e-4)
    scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=epochs)
    loss_fn   = nn.MSELoss()

    # NN の出力は -1〜+1 (tanh)、ラベルも -1 or +1 なのでそのまま MSE で学習
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model.to(device)
    print(f"  デバイス    : {device}")
    print(f"  エポック数  : {epochs}")
    print(f"  バッチサイズ: {batch_size}")
    print(f"  学習率      : {lr}")
    print()

    best_val_loss = float("inf")

    for epoch in range(1, epochs + 1):
        # ── Train ──
        model.train()
        train_loss = 0.0
        for x_batch, y_batch in train_loader:
            x_batch = x_batch.to(device)
            y_batch = y_batch.to(device)
            optimizer.zero_grad()
            pred = model(x_batch)
            loss = loss_fn(pred, y_batch)
            loss.backward()
            nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)
            optimizer.step()
            train_loss += loss.item() * len(x_batch)
        train_loss /= train_size

        # ── Validation ──
        model.eval()
        val_loss = 0.0
        with torch.no_grad():
            for x_batch, y_batch in val_loader:
                x_batch = x_batch.to(device)
                y_batch = y_batch.to(device)
                pred     = model(x_batch)
                val_loss += loss_fn(pred, y_batch).item() * len(x_batch)
        val_loss /= val_size

        scheduler.step()

        mark = " ★" if val_loss < best_val_loss else ""
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            # ベストモデルを即座に保存
            model.to("cpu")
            save_model(model, model_path)
            model.to(device)

        print(f"  Epoch [{epoch:>3}/{epochs}]  "
              f"train_loss={train_loss:.4f}  val_loss={val_loss:.4f}{mark}")

    print()
    print(f"  最良 val_loss : {best_val_loss:.4f}")
    print(f"  モデル保存先  : {model_path.resolve()}")
    print("=== 学習フェーズ終了 ===")



# ───────────────────────────────────────────────────────
# メイン: 指定回数だけ自己対戦して .jsonl に書き出す
# ───────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(
        description="Quoridor 自己対戦 棋譜収集",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
例:
  python self_play_collector.py                        # 100ゲーム収集（学習なし）
  python self_play_collector.py --games 500 --learn    # 500ゲーム収集 -> 学習まで実行
        """
    )
    parser.add_argument("--games", type=int, default=100,
                        help="収集するゲーム数 (デフォルト: 100)")
    parser.add_argument("--out",   type=str, default="games_record.jsonl",
                        help="出力ファイルパス (デフォルト: games_record.jsonl)")
    parser.add_argument("--learn", action="store_true",
                        help="収集完了後に学習フェーズ(train_from_record)を実行する")
    args = parser.parse_args()

    out_path = Path(args.out)
    game     = QuoridorGame()

    collected = 0
    discarded = 0
    win_count = [0, 0]
    t_start   = time.time()

    print("=== Quoridor 自己対戦 棋譜収集 ===")
    print(f"  目標ゲーム数 : {args.games}")
    print(f"  出力ファイル : {out_path}")
    print(f"  学習フェーズ : {'あり (--learn)' if args.learn else 'なし'}")
    print()

    with out_path.open("w", encoding="utf-8") as f:
        while collected < args.games:
            result = game.run_one_game()

            if result is None:
                discarded += 1
                continue

            winner = result["winner"]
            win_count[winner] += 1

            for step in result["steps"]:
                step_turn = step["state"]["turn"]
                step["reward"] = 1 if step_turn == winner else -1

            f.write(json.dumps(result, ensure_ascii=False) + "\n")
            f.flush()
            collected += 1

            elapsed = time.time() - t_start
            avg     = elapsed / collected
            remain  = avg * (args.games - collected)
            print(
                f"\r  [{collected:>5}/{args.games}]  "
                f"P0勝:{win_count[0]}  P1勝:{win_count[1]}  "
                f"破棄:{discarded}  "
                f"経過:{elapsed:.0f}s  残り推定:{remain:.0f}s  ",
                end="", flush=True
            )

    print()
    print()
    print("=== 収集完了 ===")
    print(f"  収集ゲーム数 : {collected}")
    print(f"  破棄ゲーム数 : {discarded}")
    print(f"  Player0 勝利: {win_count[0]} ({win_count[0]/collected*100:.1f}%)")
    print(f"  Player1 勝利: {win_count[1]} ({win_count[1]/collected*100:.1f}%)")
    print(f"  合計時間    : {time.time() - t_start:.1f}s")
    print(f"  出力ファイル: {out_path.resolve()}")

    # ── --learn が指定されていたら学習フェーズへ ──
    if args.learn:
        train_from_record(out_path)


if __name__ == "__main__":
    main()