#pragma once

struct Vector2
{
	float x;
	float y;

    // += 演算子のオーバーロード
    Vector2& operator+=(const Vector2& other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }
};
