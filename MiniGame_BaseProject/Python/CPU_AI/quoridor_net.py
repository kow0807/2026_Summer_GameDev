"""
quoridor_net.py  —  Phase 2: NN モデル定義 & 特徴量エンジニアリング
====================================================================
【役割】
  1. QuoridorNet  : αβ法の評価関数を置き換える小〜中規模の全結合NN
  2. board_to_tensor() : 盤面状態 dict -> 入力テンソルへの変換
  3. load_model() / save_model() : モデルの保存・読み込みユーティリティ

【入力特徴量 (合計 185 次元)】
  ─ 駒位置        :  9×9 = 81 次元 × 2 プレイヤー = 162 次元 (one-hot)
  ─ 縦壁マップ    :  9×8 =  72 次元 (実際は BOARD_SIZE×(BOARD_SIZE-1))
  ─ 横壁マップ    :  8×9 =  72 次元
  ─ 残り壁数      :  2 次元 (0〜10 を 0.0〜1.0 に正規化)
  ─ BFS手数差     :  1 次元 (d_player - d_cpu を -8〜8 にクリップして正規化)
  合計: 2 + 72 + 72 + 2 + 1 = 149 + 2×9 = 149 + 18 ... 
  ※ 実際の次元数は board_to_tensor() の中でカウントされます

【NNアーキテクチャ】
  Linear(input_dim, 256) -> BatchNorm -> ReLU -> Dropout(0.3)
  Linear(256, 128)        -> BatchNorm -> ReLU -> Dropout(0.2)
  Linear(128,  64)        -> ReLU
  Linear( 64,   1)        -> tanh  (出力: -1.0〜+1.0)

  出力の意味: +1.0 に近いほど CPU(player1) に有利、-1.0 に近いほど不利
  → αβ法の depth==0 の return 値をそのまま置き換えられる

【使い方】
  from quoridor_net import QuoridorNet, board_to_tensor, load_model, save_model

  model = load_model("model.pth")          # 学習済みを読み込む (なければ新規作成)
  score = model.predict_score(state_dict)  # 盤面 dict からスコアを返す
"""

from __future__ import annotations

import json
from pathlib import Path
from collections import deque
from typing import Optional

import torch
import torch.nn as nn

# ───────────────────────────────────────────────────────
# 定数
# ───────────────────────────────────────────────────────
BOARD_SIZE = 9
MAX_WALLS  = 10

# モデルのデフォルト保存先（Quoridor_Ai.py と同じディレクトリに置く想定）
DEFAULT_MODEL_PATH = Path(__file__).parent / "model.pth"


# ───────────────────────────────────────────────────────
# BFS (特徴量計算用)
# ───────────────────────────────────────────────────────
def _can_move(x, y, dx, dy, vw, hw):
    nx, ny = x + dx, y + dy
    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
        return False
    if dx == 1:
        if 0 <= x < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][x]:   return False
            if y > 0            and vw[y-1][x]:    return False
    elif dx == -1:
        cx = x - 1
        if 0 <= cx < BOARD_SIZE - 1:
            if y < BOARD_SIZE - 1 and vw[y][cx]:  return False
            if y > 0            and vw[y-1][cx]:   return False
    elif dy == 1:
        if 0 <= y < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[y][x]:   return False
            if x > 0            and hw[y][x-1]:    return False
    elif dy == -1:
        cy = y - 1
        if 0 <= cy < BOARD_SIZE - 1:
            if x < BOARD_SIZE - 1 and hw[cy][x]:  return False
            if x > 0            and hw[cy][x-1]:   return False
    return True


def _bfs_dist(px, py, goal_y, vw, hw) -> int:
    if py == goal_y:
        return 0
    visited = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
    q = deque([(px, py, 0)])
    visited[py][px] = True
    while q:
        x, y, d = q.popleft()
        for dx, dy in [(1,0),(-1,0),(0,1),(0,-1)]:
            nx, ny = x+dx, y+dy
            if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
                continue
            if visited[ny][nx]:
                continue
            if not _can_move(x, y, dx, dy, vw, hw):
                continue
            visited[ny][nx] = True
            if ny == goal_y:
                return d + 1
            q.append((nx, ny, d+1))
    return 999


# ───────────────────────────────────────────────────────
# 特徴量変換
# ───────────────────────────────────────────────────────
def board_to_tensor(state: dict) -> torch.Tensor:
    """
    盤面状態 dict -> 1D float32 テンソル に変換する。

    state の形式は BuildBoardJson() / build_state_snapshot() と同じ:
      {
        "players": [[p0x, p0y], [p1x, p1y]],
        "remaining_walls": [w0, w1],
        "vertical_walls":   [[0/1, ...], ...],   # [y][x], shape=(9,8)
        "horizontal_walls": [[0/1, ...], ...],   # [y][x], shape=(8,9)
      }
    """
    features: list[float] = []

    players = state["players"]
    p0x, p0y = players[0]
    p1x, p1y = players[1]

    vw_raw = state["vertical_walls"]    # list of list
    hw_raw = state["horizontal_walls"]

    # vw[y][x] が bool or int → bool に統一
    vw = [[bool(v) for v in row] for row in vw_raw]
    hw = [[bool(v) for v in row] for row in hw_raw]

    # ── 1. 駒位置 one-hot (各 BOARD_SIZE×BOARD_SIZE = 81 次元) ──
    for px, py in [(p0x, p0y), (p1x, p1y)]:
        for cy in range(BOARD_SIZE):
            for cx in range(BOARD_SIZE):
                features.append(1.0 if (cx == px and cy == py) else 0.0)

    # ── 2. 縦壁マップ (BOARD_SIZE × (BOARD_SIZE-1) = 9×8 = 72 次元) ──
    for y in range(BOARD_SIZE):
        for x in range(BOARD_SIZE - 1):
            features.append(1.0 if vw[y][x] else 0.0)

    # ── 3. 横壁マップ ((BOARD_SIZE-1) × BOARD_SIZE = 8×9 = 72 次元) ──
    for y in range(BOARD_SIZE - 1):
        for x in range(BOARD_SIZE):
            features.append(1.0 if hw[y][x] else 0.0)

    # ── 4. 残り壁数 (正規化: 0〜10 -> 0.0〜1.0) ──
    w0, w1 = state["remaining_walls"]
    features.append(w0 / MAX_WALLS)
    features.append(w1 / MAX_WALLS)

    # ── 5. BFS手数差 (cpu_dist - player_dist を -1〜+1 に正規化) ──
    #   正の値 → CPUがリード、負の値 → プレイヤーがリード
    d0 = _bfs_dist(p0x, p0y, BOARD_SIZE - 1, vw, hw)  # player0 のゴールまで
    d1 = _bfs_dist(p1x, p1y, 0,              vw, hw)  # player1(CPU) のゴールまで
    diff = float(d0 - d1)
    diff_norm = max(-1.0, min(1.0, diff / 8.0))        # 8手差でクリップ
    features.append(diff_norm)

    return torch.tensor(features, dtype=torch.float32)


# ───────────────────────────────────────────────────────
# NN モデル
# ───────────────────────────────────────────────────────
class QuoridorNet(nn.Module):
    """
    コリドール評価関数NN。
    入力: board_to_tensor() で得られる 1D 特徴量ベクトル
    出力: スカラー (-1.0〜+1.0)
           +1.0 → CPU(player1) に有利
           -1.0 → player0 に有利
    """

    def __init__(self, input_dim: Optional[int] = None):
        super().__init__()

        # input_dim が未指定なら自動計算（ダミー盤面で一度通す）
        if input_dim is None:
            dummy_state = _make_dummy_state()
            input_dim = board_to_tensor(dummy_state).shape[0]

        self.input_dim = input_dim

        self.net = nn.Sequential(
            # Block 1
            nn.Linear(input_dim, 256),
            nn.BatchNorm1d(256),
            nn.ReLU(),
            nn.Dropout(0.3),

            # Block 2
            nn.Linear(256, 128),
            nn.BatchNorm1d(128),
            nn.ReLU(),
            nn.Dropout(0.2),

            # Block 3
            nn.Linear(128, 64),
            nn.ReLU(),

            # 出力
            nn.Linear(64, 1),
            nn.Tanh(),
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return self.net(x).squeeze(-1)

    def predict_score(self, state: dict) -> float:
        """
        盤面 state dict を受け取り、float スコアを返す。
        αβ法の depth==0 の return 値として差し込む。
        """
        self.eval()
        with torch.no_grad():
            x = board_to_tensor(state).unsqueeze(0)  # (1, input_dim)
            score = self(x).item()                   # -1.0〜+1.0
        # αβ法の従来スコアに合わせてスケール変換 (-10〜+10 程度)
        return score * 10.0


# ───────────────────────────────────────────────────────
# ダミー盤面 (input_dim 自動計算用)
# ───────────────────────────────────────────────────────
def _make_dummy_state() -> dict:
    return {
        "players":         [[4, 0], [4, 8]],
        "remaining_walls": [10, 10],
        "vertical_walls":  [[0] * (BOARD_SIZE - 1) for _ in range(BOARD_SIZE)],
        "horizontal_walls":[[0] * BOARD_SIZE       for _ in range(BOARD_SIZE - 1)],
    }


# ───────────────────────────────────────────────────────
# 保存 / 読み込み
# ───────────────────────────────────────────────────────
def save_model(model: QuoridorNet, path: Path = DEFAULT_MODEL_PATH) -> None:
    """モデルの重みと input_dim を保存する。"""
    torch.save({
        "input_dim":   model.input_dim,
        "state_dict":  model.state_dict(),
    }, path)
    print(f"[QuoridorNet] モデルを保存しました: {path}")


def load_model(path: Path = DEFAULT_MODEL_PATH) -> QuoridorNet:
    """
    保存済みモデルを読み込む。
    ファイルが存在しない場合は新規作成して返す。
    """
    if not Path(path).exists():
        print(f"[QuoridorNet] モデルファイルなし → 新規作成: {path}")
        model = QuoridorNet()
        return model

    checkpoint = torch.load(path, map_location="cpu", weights_only=True)
    model = QuoridorNet(input_dim=checkpoint["input_dim"])
    model.load_state_dict(checkpoint["state_dict"])
    print(f"[QuoridorNet] モデルを読み込みました: {path}  (input_dim={model.input_dim})")
    return model


# ───────────────────────────────────────────────────────
# 動作確認 (単体実行)
# ───────────────────────────────────────────────────────
if __name__ == "__main__":
    print("=== QuoridorNet 動作確認 ===")

    dummy = _make_dummy_state()
    tensor = board_to_tensor(dummy)
    print(f"  特徴量次元数 : {tensor.shape[0]}")

    model = QuoridorNet()
    print(f"  モデル input_dim : {model.input_dim}")

    total_params = sum(p.numel() for p in model.parameters())
    print(f"  総パラメータ数   : {total_params:,}")

    score = model.predict_score(dummy)
    print(f"  初期盤面のスコア : {score:.4f}  (未学習なのでランダムに近い値)")

    # 保存 → 読み込みテスト
    tmp_path = Path("/tmp/quoridor_net_test.pth")
    save_model(model, tmp_path)
    model2 = load_model(tmp_path)
    score2 = model2.predict_score(dummy)
    print(f"  読み込み後スコア : {score2:.4f}  (保存前と一致するはず)")
    assert abs(score - score2) < 1e-5, "保存/読み込みに誤りがあります"
    print("  保存/読み込みテスト: OK")
