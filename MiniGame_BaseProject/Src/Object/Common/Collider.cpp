#include <DxLib.h>
#include <cmath>
#include "Collider.h"


Collider::Collider(TYPE type, int modelId)
{
	type_ = type;
	modelId_ = modelId;
}

Collider::~Collider(void)
{
}

bool Collider::IsCollidiingWith(const Collider& other) const
{
	VECTOR aMin = VSub(center_,halfSize_);
	VECTOR aMax = VAdd(center_, halfSize_);
	VECTOR bMin = VSub(other.center_,other.halfSize_);
	VECTOR bMax = VAdd(other.center_,other.halfSize_);

	return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
		(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
		(aMin.z <= bMax.z && aMax.z >= bMin.z);
}

VECTOR Collider::GetPushVector(const Collider& other) const
{
	VECTOR delta = VSub(other.center_,center_);
	VECTOR intersect = {
		(halfSize_.x + other.halfSize_.x) - std::abs(delta.x),
		(halfSize_.y + other.halfSize_.y) - std::abs(delta.y),
		(halfSize_.z + other.halfSize_.z) - std::abs(delta.z)
	};

	// Ç«Ç±Ç…Ç‡ÇﬂÇËçûÇÒÇ≈Ç¢Ç»Ç¢Ç∆Ç´
	if (intersect.x <= 0.0f || intersect.y <= 0.0f || intersect.z <= 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}

	// ç≈Ç‡è¨Ç≥Ç¢é≤ï˚å¸Ç…âüÇµèoÇ∑
	if (intersect.x < intersect.y && intersect.x < intersect.z) {
		return { (delta.x < 0.0f ? -intersect.x : intersect.x), 0.0f, 0.0f };
	}
	else if (intersect.y < intersect.z) {
		return { 0.0f, (delta.y < 0.0f ? -intersect.y : intersect.y), 0.0f };
	}
	else {
		return { 0.0f, 0.0f, (delta.z < 0.0f ? -intersect.z : intersect.z) };
	}
}

void Collider::SetPosition(const VECTOR& pos)
{
	center_ = pos;
}

VECTOR Collider::GetPosition(void) const
{
	return center_;
}
