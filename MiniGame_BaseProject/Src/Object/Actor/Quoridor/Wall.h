#pragma once
#include "../ActorBase.h"

class Wall :
    public ActorBase
{
public:

	static constexpr VECTOR DEFAULT_SCALE = { 0.25f, 0.25f, 0.25f };
	static constexpr VECTOR DEFAULT_ROTATION = { 0.0f, 0.0f, 0.0f };

	enum class TYPE
	{
		VERTICAL,
		HORIZONTAL
	};

	// コンストラクタ
	Wall(void);
	// デストラクタ
	virtual ~Wall(void);
	
	virtual void Init(void) override;
	
	void InitTransform(void);
	
	virtual void Update(void) override;
	virtual void Draw(void) override;

	int GetX(void) const;
	int GetY(void) const;

	TYPE GetType(void) const;

	// プレビュー描画
	void DrawPreview(bool canPlace);

	// 座標更新
	void SetBoardPosition(int x, int y);

	// 向き変更
	void SetType(TYPE type);

	void RefreshTransform(void);

private:

	// 盤面座標
	int x_, y_;

	// 向き
	TYPE type_;


	void UpdateTransform(void);
};

