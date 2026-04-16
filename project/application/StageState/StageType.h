#pragma once

enum class StageType {
	Ready,
	Playing,
	Pause,
	StageClear,
	Boss,
	GameClear,
	GameOver,
	Tutorial,
	Transition, // プレイヤーイージング付きステージ遷移
};
