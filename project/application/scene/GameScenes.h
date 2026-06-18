#pragma once
///=====================================================///
/// ゲーム固有のシーン番号
/// （旧 engine/scene/IScene.h の enum SCENE をゲーム側へ移設）
///   - 値の並びは従来と同一に保つこと（遷移ロジック・SetSceneNo 互換のため）
///   - 名前を SCENE のままにして既存参照（SCENE::TITLE 等）を壊さない
///=====================================================///
enum SCENE { DEBUG, TITLE, STAGE, TUTORIAL, CLEAR, OVER };
