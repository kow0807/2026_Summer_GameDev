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
		QUORIDOR,
		MINI_SHOGI,
		MAX
	};

	enum class SELECT_STATE
	{
		GAME_SELECT,
		EXPLANATION,
		RUNTIME_LOADING,
		TRANSITION_OUT,
		TRANSITION_IN,
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
	int gameTitleFont_;

	int backImg_;
	int gameThumbnail_[static_cast<int>(MINI_STATE::MAX)];
	int firstPressExplanationImg_;
	int quizExplanationImg_;
	int reversiExplanationImg_;
	int buttonMashExplanationImg_;
	bool isKeyboard_;

	bool isBgm_;
	int bgm_;
	int moveSe_;
	int cancelSe_;
	int selectSe_;
	int menuSe_;

	int leftArrowAnim_;
	int rightArrowAnim_;

	int decideSEH_;

	bool isYes_;

	int fadeAlpha_;

	bool isPause_;
	float pauseX_;
	int pauseSelect_;

	// ミニゲームの基底クラス
	std::unique_ptr<GameBase> gameBase_;

	// ミニゲーム選択操作
	void SelectGameUpdate(void);
	void ExplanationUpdate(void);
	void GameUpdate(void);

	bool PauseUpdate(void);
	void PauseDraw(void);


	void DrawGame(void);

	// ミニゲーム選択UI
	void SelectGameDrawUI(void);
	void ExplanationDrawUI(void);

	// 各ミニゲームの説明UI
	void ExplanationFirstPressDrawUI(void);
	void ExplanationQuizDrawUI(void);
	void ExplanationReversiDrawUI(void);
	void ExplanationButtonMashDrawUI(void);
	void ExplanationQuoridorDrawUI(void);
	void ExplanationMiniShogiUI(void);

	void DrawRunTimeLoading(void);

	//　ミニゲーム生成
	void CreateMiniGame(void);

	void DrawFade(void);
};
