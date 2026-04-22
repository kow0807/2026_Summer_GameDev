#pragma once
class Collider
{

public :

	// 衝突種別
	enum class TYPE
	{
		STAGE,
	};

	// コンストラクタ
	Collider(TYPE type, int modelId);

	// デストラクタ
	~Collider(void);

	bool IsCollidiingWith(const Collider& other) const;
	VECTOR GetPushVector(const Collider& other) const;	// 押出ベクトルを返す
	void SetPosition(const VECTOR& pos);				// モデルと同期
	VECTOR GetPosition(void) const;

	// 衝突種別
	TYPE type_;

	// モデルのハンドルID
	int modelId_;

	// 中心座標
	VECTOR center_;

	// 幅/高さ/奥行きの半分
	VECTOR halfSize_;
};
