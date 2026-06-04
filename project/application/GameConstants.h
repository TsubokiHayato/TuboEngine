#pragma once

namespace GameConstants {
    // 固定タイムステップ (60fps 相当)
    // 実際の dt を受け取らない更新処理で使用する
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;
}
