#pragma once
struct Vector3
{
    float x;
    float y;
    float z;

    // += 演算子のオーバーロード
    Vector3& operator+=(const Vector3& other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
        return *this;
    }

    // + 演算子のオーバーロード
    friend Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
        Vector3 result = lhs;
        result += rhs;
        return result;
    }

    // *= 演算子のオーバーロード
    Vector3& operator*=(float scalar) {
        this->x *= scalar;
        this->y *= scalar;
        this->z *= scalar;
        return *this;
    }

    // * 演算子のオーバーロード
    friend Vector3 operator*(const Vector3& vec, float scalar) {
        Vector3 result = vec;
        result *= scalar;
        return result;
    }

    // * 演算子のオーバーロード
    friend Vector3 operator*(float scalar, const Vector3& vec) {
        return vec * scalar;
    }
};