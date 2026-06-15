#include "SceneRegistration.h"

#include "SceneManager.h"
#include "GameScenes.h"

#include "DebugScene.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "ClearScene.h"
#include "OverScene.h"

#include <memory>

void RegisterGameScenes() {
	SceneManager* sm = SceneManager::GetInstance();

	// シーン番号 → 生成関数 を登録（TUTORIAL は現状どおり StageScene を流用）
	sm->RegisterScene(DEBUG,    [] { return std::make_unique<DebugScene>(); }, "Debug");
	sm->RegisterScene(TITLE,    [] { return std::make_unique<TitleScene>(); }, "Title");
	sm->RegisterScene(STAGE,    [] { return std::make_unique<StageScene>(); }, "Stage");
	sm->RegisterScene(TUTORIAL, [] { return std::make_unique<StageScene>(); }, "Tutorial");
	sm->RegisterScene(CLEAR,    [] { return std::make_unique<ClearScene>(); }, "Clear");
	sm->RegisterScene(OVER,     [] { return std::make_unique<OverScene>(); }, "Over");
}
