#pragma once
#include <memory>
class StageScene;
class IStageState {
public:
	virtual ~IStageState() = default;
	virtual void Enter(StageScene* scene) = 0;  // 状態に入ったときの初期化
	virtual void Update(StageScene* scene) = 0; // 毎フレームの更新
	virtual void Exit(StageScene* scene) = 0;   // 状態を抜けるときの後処理
};
