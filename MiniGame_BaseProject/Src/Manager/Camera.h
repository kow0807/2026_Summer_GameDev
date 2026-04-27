#pragma once
#include <DxLib.h>
#include "../Common/Quaternion.h"
class ResourceManager;
class Transform;

class Camera
{

public:

	
	static constexpr float SPEED = 1.0f;										// カメラスピード(度)
	static constexpr float CAMERA_NEAR = 10.0f;									// カメラクリップ：NEAR
	static constexpr float CAMERA_FAR = 30000.0f;								// カメラクリップ：FAR	
	static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 100.0f, -500.0f };		// カメラの初期座標
	static constexpr VECTOR LOCAL_F2C_POS = { 0.0f, 50.0f, -400.0f };			// 追従位置からカメラ位置までの相対座標
	static constexpr VECTOR LOCAL_F2T_POS = { 0.0f, 0.0f, 500.0f };				// 追従位置から注視点までの相対座標	
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);			// カメラのX回転上限度角
	static constexpr float LIMIT_X_DW_RAD = 15.0f * (DX_PI_F / 180.0f);			// カメラのX回転下限度角
	static constexpr VECTOR RELATIVE_C2T_POS = { 0.0f, -100.0f, 220.0f };		// カメラ位置から注視点までの相対座標
	static constexpr VECTOR RELATIVE_F2C_POS_FOLLOW = { 0.0f, 280.0f, -350.0f };// 追従位置からカメラ位置までの相対座標(追従モード)
	static constexpr float PAT_CAMERA_MAX_RX = 100.0f;							// パットの時のカメラの制限(x上限)
	static constexpr float PAT_CAMERA_MIN_RX = -100.0f;							// パットの時のカメラの制限(x下限)
	static constexpr float PAT_CAMERA_MAX_RY = 100.0f;							// パットの時のカメラの制限(y上限)
	static constexpr float PAT_CAMERA_MIN_RY = -100.0f;							// パットの時のカメラの制限(y下限)
	static constexpr float CAMERA_MAX_ANGLE_X = 60.0f;							// カメラのX回転上限度角	
	static constexpr float CAMERA_MIN_ANGLE_X = -60.0f;							// カメラのX回転下限度角
	static constexpr float CAMERA_FREE_MOVE_SPEED = 20.0f;						// フリーモードのカメラ移動速度


	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FOLLOW,
		FOLLOW_MOUSE,
		SELF_SHOT,
		EVENT,
		MINI_GAME,
		FREE// デバッグ用
	};

	enum class GAME_CAMERA
	{
		NONE,
		MOUSE,
		TARGET
	};

	enum class GAME_TYPE
	{
		NONE,
		QUORIDOR,
		HARE_AND_HOUNDS
	};

	Camera(void);
	~Camera(void);

	void Init(void);
	void Update(void);
	void SetBeforeDraw(void);
	void Draw(void);

	// カメラ位置
	VECTOR GetPos(void) const;
	// カメラの操作角度
	VECTOR GetAngles(void) const;
	// カメラの注視点
	VECTOR GetTargetPos(void) const;

	// カメラ角度
	Quaternion GetQuaRot(void) const;
	// X回転を抜いたカメラ角度
	Quaternion GetQuaRotOutX(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);
	void ChangeGameCamera(GAME_CAMERA gameCamera);
	void ChangeGameTypeCamera(GAME_TYPE gameType);

	// 追従対象の設定
	void SetFollow(const Transform* follow);
	void SetFollowEnemy(const Transform* followEnemy);

	// マウス移動
	void MouseMove(float* x_m, float* y_m, const float fov_per);

	// ロックオンマーカー
	void DrawLockonMarker(void);
	void LockonSoundLoad(void);

	// イベントカメラの状態をリセット
	void ResetEventCameraState(void);

	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラの右方向
	VECTOR GetRight(void) const;

	// カメラの上方向
	VECTOR GetUp(void) const;

	// 座標設定
	void SetPos(const VECTOR& pos);

	// 角度設定
	void SetAngles(const VECTOR& angles);

	// カメラ操作設定
	void SetOperatable(bool flag);

private:


	// シングルトン参照
	ResourceManager& resMng_;

	// カメラが追従対象とするTransform
	const Transform* followTransform_;
	const Transform* followEnemyTransform_;

	// カメラモード
	MODE mode_;

	// ゲーム中のカメラ
	GAME_CAMERA gameCamera_;

	// ゲームの種類
	GAME_TYPE gameType_;

	// カメラの位置
	VECTOR pos_;

	// カメラ角度(rad)
	VECTOR angles_;

	// カメラ移動方向
	VECTOR moveDir_;

	// X軸回転が無い角度
	Quaternion rotOutX_;

	// カメラ角度
	Quaternion rot_;

	// 注視点
	VECTOR targetPos_;

	// カメラの上方向
	VECTOR cameraUp_;

	// ロックオンフレーム
	int lockonFrame_;

	// 音
	int lockonSe_;

	// カメラの初期化を行うかどうか
	bool isInitialized_;

	// カメラが操作可能であるか
	bool isOperatable_;

	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(void);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFollow(void);
	void SetBeforeDrawFollowMouse(void);
	void SetBeforeDrawSelfShot(void);
	void SetBeforeDrawEvent(void);
	void SetBeforDrawMiniGame(void);
	void SetBeforeDrawFree(void);

	// ゲーム種別更新ステップ
	void SetBeforeDrawQuoridor(void);
};
