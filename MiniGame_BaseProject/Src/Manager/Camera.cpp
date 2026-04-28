#include <math.h>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include <algorithm>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Setting.h"
#include "../Object/Common/Transform.h"
#include "Camera.h"

Camera::Camera(void) 
	:
	resMng_(ResourceManager::GetInstance()),
	angles_({}),
	cameraUp_({}),
	mode_(Camera::MODE::NONE),
	gameCamera_(GAME_CAMERA::NONE),
	gameType_(GAME_TYPE::NONE),
	pos_({}),
	targetPos_({}),
	moveDir_({}),
	lockonFrame_(0),
	followTransform_(nullptr),
	followEnemyTransform_(nullptr),
	lockonSe_(-1),
	isInitialized_(false),
	isOperatable_(true)
{
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	ChangeMode(MODE::FIXED_POINT);
	ChangeGameCamera(GAME_CAMERA::MOUSE);
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{
	auto& ins = InputManager::GetInstance();

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	case Camera::MODE::FOLLOW_MOUSE:
		if (gameCamera_ == GAME_CAMERA::MOUSE)
		{
			DINPUT_JOYSTATE input;

			// 入力状態を取得
			GetJoypadDirectInputState(DX_INPUT_PAD1, &input);

			if (input.Rx > PAT_CAMERA_MAX_RX)
			{
				angles_.y += 1.5f;
			}
			if (input.Rx < PAT_CAMERA_MIN_RX)
			{
				angles_.y -= 1.5f;
			}
			if (input.Ry > PAT_CAMERA_MAX_RY && angles_.x < CAMERA_MAX_ANGLE_X)
			{
				angles_.x += 1.5f;
			}
			if (input.Ry < PAT_CAMERA_MIN_RY && angles_.x > CAMERA_MIN_ANGLE_X)
			{
				angles_.x -= 1.5f;
			}
			MouseMove(&angles_.y, &angles_.x, 1.f);
		}
		SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);
		SetBeforeDrawFollowMouse();
		break;
	case Camera::MODE::SELF_SHOT:

		break;
	case Camera::MODE::EVENT:
		SetBeforeDrawEvent();
		break;
	case Camera::MODE::MINI_GAME:
		SetBeforDrawMiniGame();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		pos_,
		targetPos_,
		cameraUp_
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::Draw(void)
{
	/*DrawFormatString(10, 100, 0x000000, "(%2f, %2f, %2f)", angles_.x, angles_.y, angles_.z);
	DrawFormatString(10, 120, 0x000000, "(%2f, %2f, %2f)", pos_.x, pos_.y, pos_.z);*/
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

void Camera::SetFollowEnemy(const Transform* followEnemy)
{
	followEnemyTransform_ = followEnemy;
}

void Camera::MouseMove(float* x_m, float* y_m, const float fov_per)
{
	int x_t, y_t;
	GetMousePoint(&x_t, &y_t);
	*x_m += float(std::clamp(x_t - Application::SCREEN_SIZE_X / 2, -120, 120)) * fov_per / GetFPS() * 5;
	*y_m += float(std::clamp(y_t - Application::SCREEN_SIZE_Y / 2, -120, 120)) * fov_per / GetFPS() * 5;
	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);
	if (angles_.x > 10.0f)
	{
		angles_.x = 10.0f;
	}
	if (angles_.x < -20.0f)
	{
		angles_.x = -20.0f;
	}
}

VECTOR Camera::GetPos(void) const
{
	return pos_;
}

VECTOR Camera::GetAngles(void) const
{
	return angles_;
}

VECTOR Camera::GetTargetPos(void) const
{
	return targetPos_;
}

Quaternion Camera::GetQuaRot(void) const
{
	return rot_;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, pos_));
}

VECTOR Camera::GetRight(void) const
{
	VECTOR forward = GetForward();
	VECTOR up = VGet(0.0f, 1.0f, 0.0f);	// ワールド座標系の上方向
	VECTOR right = VNorm(VCross(up, forward));

	return right;
}

VECTOR Camera::GetUp(void) const
{
	VECTOR forward = GetForward();
	VECTOR right = GetRight();
	VECTOR up = VNorm(VCross(forward, right));
	return up;
}

void Camera::SetPos(const VECTOR& pos)
{
	pos_ = pos;
}

void Camera::SetAngles(const VECTOR& angles)
{
	angles_ = angles;
}

void Camera::SetOperatable(bool flag)
{
	isOperatable_ = flag;
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetMouseDispFlag(TRUE);
		break;
	case Camera::MODE::FOLLOW:
		SetMouseDispFlag(TRUE);
		break;
	case Camera::MODE::FOLLOW_MOUSE:
		SetMouseDispFlag(FALSE);
		break;
	}

}

void Camera::ChangeGameCamera(GAME_CAMERA gameCamera)
{
	gameCamera_ = gameCamera;

	// 変更時の初期化処理
	switch (gameCamera_)
	{
	case Camera::GAME_CAMERA::NONE:
		break;
	case Camera::GAME_CAMERA::MOUSE:
		//angles_.x = -9.0f;
		break;
	case Camera::GAME_CAMERA::TARGET:
		break;
	}
}

void Camera::ChangeGameTypeCamera(GAME_TYPE gameType)
{
	gameType_ = gameType;

	switch (gameType_)
	{
	case Camera::GAME_TYPE::NONE:
		break;
	case Camera::GAME_TYPE::QUORIDOR:
		break;
	case Camera::GAME_TYPE::HARE_AND_HOUNDS:
		break;
	default:
		break;
	}
}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	pos_ = DEFAULT_CAMERA_POS;

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.x = AsoUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	rot_ = Quaternion();

}

void Camera::SyncFollow(void)
{

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// 重力の方向制御に従う
	// 正面から設定されたY軸分、回転させる
	rotOutX_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// 正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点(通常重力でいうところのY値を追従対象と同じにする)
	localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

}

void Camera::ProcessRot(void)
{

	auto& ins = InputManager::GetInstance();

	float movePow = 5.0f;

	// カメラ回転
	if (ins.IsNew(KEY_INPUT_RIGHT))
	{
		// 右回転
		angles_.y += AsoUtility::Deg2RadF(1.0f);
	}
	if (ins.IsNew(KEY_INPUT_LEFT))
	{
		// 左回転
		angles_.y += AsoUtility::Deg2RadF(-1.0f);
	}

	// 上回転
	if (ins.IsNew(KEY_INPUT_UP))
	{
		angles_.x += AsoUtility::Deg2RadF(1.0f);
		if (angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
	}

	// 下回転
	if (ins.IsNew(KEY_INPUT_DOWN))
	{
		angles_.x += AsoUtility::Deg2RadF(-1.0f);
		if (angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}

}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何もしない
}

void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作
	ProcessRot();

	// 追従対象との相対位置を同期
	SyncFollow();

}

void Camera::SetBeforeDrawFollowMouse(void)
{
	auto& ins = InputManager::GetInstance();

	static bool prevRStickPressed = false;
	DINPUT_JOYSTATE input;
	GetJoypadDirectInputState(DX_INPUT_PAD1, &input);

	bool nowRStickPressed = input.Buttons[9] != 0;

	bool isToggleTriggered = false;
	if (nowRStickPressed && !prevRStickPressed)
	{
		isToggleTriggered = true;
	}

	prevRStickPressed = nowRStickPressed;

	if (gameCamera_ == GAME_CAMERA::MOUSE && (ins.IsTrgMouseMiddle() || isToggleTriggered))
	{
		PlaySoundMem(lockonSe_, DX_PLAYTYPE_BACK, true);
		ChangeGameCamera(GAME_CAMERA::TARGET);
	}
	else if (gameCamera_ == GAME_CAMERA::TARGET && (ins.IsTrgMouseMiddle() || isToggleTriggered))
	{
		PlaySoundMem(lockonSe_, DX_PLAYTYPE_BACK, true);
		ChangeGameCamera(GAME_CAMERA::MOUSE);
	}
	// 追従対象の位置
	VECTOR followPos = followTransform_->pos;


	Quaternion rotX = Quaternion::AngleAxis(AsoUtility::Deg2RadF(angles_.x), AsoUtility::AXIS_X);
	Quaternion rotY = Quaternion::AngleAxis(AsoUtility::Deg2RadF(angles_.y), AsoUtility::AXIS_Y);
	Quaternion mixRot = rotY.Mult(rotX);

	// 追従対象からカメラまでの相対座標
	VECTOR relativeCPos = rotY.PosAxis(RELATIVE_F2C_POS_FOLLOW);

	// カメラ位置の更新
	pos_ = VAdd(followPos, relativeCPos);

	// カメラ位置から注視点までの相対座標
	VECTOR relativeTPos = mixRot.PosAxis(RELATIVE_C2T_POS);

	if (gameCamera_ == GAME_CAMERA::TARGET)
	{
		targetPos_ = AsoUtility::Lerp(targetPos_, followEnemyTransform_->pos, 0.1f);

		// 敵の位置への方向ベクトル
		VECTOR toEnemy = VSub(followEnemyTransform_->pos, pos_);

		// 角度を度数法で計算
		angles_.y = 
			static_cast<float>(
			AsoUtility::LerpDeg(static_cast<double>(angles_.y), 
			static_cast<double>(AsoUtility::Rad2DegF(atan2f(toEnemy.x, toEnemy.z))), 
			static_cast<double>(0.1f))
			);
	}
	else
	{
		// 注視点の更新
		targetPos_ = AsoUtility::Lerp(targetPos_, VAdd(pos_, relativeTPos), 0.3f);
	}

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.z = 0.0f;
}

void Camera::SetBeforeDrawSelfShot(void)
{
}

void Camera::DrawLockonMarker(void)
{
	if (gameCamera_ == GAME_CAMERA::TARGET)
	{
		VECTOR enemyPos = followEnemyTransform_->pos;
		enemyPos.y += 150.0f;

		float screenX, screenY;
		if (GetTransformPosition(&enemyPos, &screenX, &screenY) == 0)
		{
			float baseRadius = 18.0f;

			// ratio を計算し、0未満なら0にする
			float ratio = 1.0f - lockonFrame_ * 0.05f;
			if (ratio < 0.0f) ratio = 0.0f;

			// 外から内に縮む半径
			float moveRadius = baseRadius + 200.0f * ratio;

			float scale = 1.0f + 0.05f * sin(lockonFrame_ * 0.1f);
			float radius = moveRadius * scale;

			float angle = lockonFrame_ * 0.05f;

			int alpha = 160 + static_cast<int>(50 * sin(lockonFrame_ * 0.12f));
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

			int x = static_cast<int>(screenX);
			int y = static_cast<int>(screenY);
			int colorOuter = GetColor(255, 0, 0);   // 鮮やかな赤（外円）
			int colorInner = GetColor(255, 255, 0); // 明るい黄色（内円）

			// 外円・中円を重ねて描画
			for (int i = -1; i <= 1; ++i)
			{
				DrawCircle(x, y, static_cast<int>(radius) + i, colorOuter, false);
				DrawCircle(x, y, static_cast<int>(radius / 2) + i, colorInner, false);
			}

			// 回転する十字線
			for (int i = 0; i < 4; ++i)
			{
				float a = angle + (3.14159265f * 2.0f) * i / 4.0f;
				int lx = static_cast<int>(x + cos(a) * radius);
				int ly = static_cast<int>(y + sin(a) * radius);
				DrawLine(x, y, lx, ly, colorInner);
			}

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			lockonFrame_++;
		}
	}
	else
	{
		lockonFrame_ = 0;
	}
}

void Camera::LockonSoundLoad(void)
{
}

void Camera::ResetEventCameraState(void)
{
	isInitialized_ = false;
}

void Camera::SetBeforeDrawEvent(void)
{
}

void Camera::SetBeforDrawMiniGame(void)
{
	switch (gameType_)
	{
	case Camera::GAME_TYPE::NONE:
		break;
	case Camera::GAME_TYPE::QUORIDOR:
		SetBeforeDrawQuoridor();
		break;
	case Camera::GAME_TYPE::HARE_AND_HOUNDS:
		break;
	default:
		break;
	}

}

void Camera::SetBeforeDrawFree(void)
{
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	int centerX = Setting::GetInstance().GetWindowSize().width_ / 2;
	int centerY = Setting::GetInstance().GetWindowSize().height_ / 2;

	int dx = mouseX - centerX;
	int dy = mouseY - centerY;

	// マウス感度
	float sensitivity = 0.002f;

	// 操作可能時のみ
	if (isOperatable_)
	{
		// 視点角度に反映
		angles_.y += dx * sensitivity;	// 水平回転
		angles_.x -= dy * sensitivity;	// 垂直回転

		// 垂直回転の制限
		if (angles_.x > AsoUtility::Deg2RadF(89.0f))angles_.x = AsoUtility::Deg2RadF(89.0f);
		if (angles_.x < AsoUtility::Deg2RadF(-89.0f))angles_.x = AsoUtility::Deg2RadF(-89.0f);

		// 移動方向のリセット
		moveDir_ = AsoUtility::VECTOR_ZERO;

		// 入力関連
		auto& ins = InputManager::GetInstance();

		DINPUT_JOYSTATE pad;
		GetJoypadDirectInputState(DX_INPUT_PAD1, &pad);

		// 前後左右入力
		if (ins.IsNew(KEY_INPUT_W) || pad.Y < -500) moveDir_ = VAdd(moveDir_, GetForward());
		if (ins.IsNew(KEY_INPUT_S) || pad.Y > 500) moveDir_ = VAdd(moveDir_, VScale(GetForward(), -1.0f));
		if (ins.IsNew(KEY_INPUT_D) || pad.X > 500) moveDir_ = VAdd(moveDir_, GetRight());
		if (ins.IsNew(KEY_INPUT_A) || pad.X < -500) moveDir_ = VAdd(moveDir_, VScale(GetRight(), -1.0f));
		if (ins.IsNew(KEY_INPUT_Q)) moveDir_ = VAdd(moveDir_, VScale(GetUp(), -1.0f));
		if (ins.IsNew(KEY_INPUT_E)) moveDir_ = VAdd(moveDir_, GetUp());

		pos_ = VAdd(pos_, moveDir_);

		// 前方方向ベクトルを計算
		VECTOR forward = {
			cosf(angles_.x) * sinf(angles_.y),
			sinf(angles_.x),
			cosf(angles_.x) * cosf(angles_.y)
		};

		// 注視点を更新(カメラ位置 + 前方方向ベクトル)
		targetPos_ = VAdd(pos_, VScale(forward, 200.0f));

		// 上方向は常にY+
		cameraUp_ = AsoUtility::DIR_U;

		// 毎フレームマウスを中央に戻す
		SetMousePoint(centerX, centerY);

		// 値を小さくするほど、回転速度が遅くなります。
		const float GAMEPAD_SENSITIVITY = 0.025f;

		// スティックの最大値で割るための定数（-1000 ～ 1000 の範囲を想定）
		const float MAX_ANALOG_VALUE = 1000.0f;

		// デッドゾーンの閾値（スティックの遊び、ノイズ対策）
		const int DEADZONE_THRESHOLD = 200; // -200 から 200 の範囲は無視

		DINPUT_JOYSTATE input;

		// 入力状態を取得
		if (GetJoypadDirectInputState(DX_INPUT_PAD1, &input) == 0)
		{
			int rx = input.Rx;

			// デッドゾーン処理
			if (abs(rx) < DEADZONE_THRESHOLD)
			{
				rx = 0;
			}

			if (rx != 0)
			{
				// スティック値を正規化 (-1.0f ～ 1.0f)
				float normalizedRx = (float)rx / MAX_ANALOG_VALUE;

				// 視点角度に反映: angles_.y は水平回転
				angles_.y += normalizedRx * GAMEPAD_SENSITIVITY;
			}

			// -----------------------------------------------------------------
			// 2. 垂直回転 (右スティックの Y軸: Ry)
			// -----------------------------------------------------------------
			int ry = input.Ry;

			// デッドゾーン処理
			if (abs(ry) < DEADZONE_THRESHOLD)
			{
				ry = 0;
			}

			if (ry != 0)
			{
				// スティック値を正規化 (-1.0f ～ 1.0f)
				float normalizedRy = (float)ry / MAX_ANALOG_VALUE;

				// 垂直回転の加算（元のコードの角度制限部分を統合し、感度を適用）
				// Rx, Ryは正の値で右/下方向を指すことが多いが、
				// カメラの Y軸回転(angles_.x)は '下方向' が角度減少、'上方向' が角度増加になるように調整が必要
				// マウス操作と合わせる場合: マウス 'dy' は '下移動' で正、'angles_.x' は減少 (-dy)
				// ゲームパッド 'ry' は '下倒し' で正、'angles_.x' は減少 (-normalizedRy) とします。
				float deltaAngleX = -normalizedRy * GAMEPAD_SENSITIVITY;

				// 角度制限を考慮して加算
				float newAngleX = angles_.x + deltaAngleX;

				// 垂直回転の制限
				float minRad = AsoUtility::Deg2RadF(-89.0f);
				float maxRad = AsoUtility::Deg2RadF(89.0f);

				if (newAngleX > maxRad)
				{
					angles_.x = maxRad;
				}
				else if (newAngleX < minRad)
				{
					angles_.x = minRad;
				}
				else
				{
					angles_.x = newAngleX;
				}
			}
		}
	}
}

void Camera::SetBeforeDrawQuoridor(void)
{
	// 盤面に合わせる
	const int CELL_SIZE = 50.0f;
	const int BOARD_SIZE = 9;

	float center = (BOARD_SIZE - 1) * CELL_SIZE * 0.5;

	// カメラ位置
	pos_ = VGet(
		center + 60.0f,
		300.0f,
		center - 80.0f
	);

	// 注視点 (盤面中央)
	targetPos_ = VGet(center, 0.0f, center);

	// 上方向
	cameraUp_ = VGet(0, 0, 1);
}
