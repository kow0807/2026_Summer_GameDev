#pragma once
#include <memory>
#include "SceneBase.h"
class PixelMaterial;
class PixelRenderer;
class GameBase;


class GameScene : public SceneBase
{

public:

	enum class MINI_STATE
	{
		FIRST_PRESS,
		QUIZ,
		REVERSI,
		BUTTON_MASH,
		FLASH_CALC,
		QUORIDOR,
		HARE_AND_HOUNDS,
		MINI_SHOGI,
		MAX
	};

	enum class SELECT_STATE
	{
		GAME_SELECT,
		EXPLANATION,
		RUNTIME_LOADING,
		PLAYING
	};


	// 定数
	// ----------------------------

	// ----------------------------

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void) override;

private:

	MINI_STATE miniState_;

	SELECT_STATE selectState_;

	int explanationFontHandle_;

	int firstPressExplanationImg_;
	int quizExplanationImg_;
	int buttonMashExplanationImg_;

	bool isYes_;

	// ミニゲームの基底クラス
	std::unique_ptr<GameBase> gameBase_;

	// ミニゲーム選択操作
	void SelectGameUpdate(void);
	void ExplanationUpdate(void);
	void GameUpdate(void);

	void DrawGame(void);

	// ミニゲーム選択UI
	void SelectGameDrawUI(void);
	void ExplanationDrawUI(void);

	// 各ミニゲームの説明UI
	void ExplanationFirstPressDrawUI(void);
	void ExplanationQuizDrawUI(void);
	void ExplanationButtonMashDrawUI(void);
	void ExplanationQuoridorDrawUI(void);

	void DrawRunTimeLoading(void);

	//　ミニゲーム生成
	void CreateMiniGame(void);
};
