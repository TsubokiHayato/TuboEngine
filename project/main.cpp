#include"DirectXcommon.h"
#include"D3DResourceLeakChecker.h"
#include"MT_Matrix.h"
#include "Input.h"

#include"SpriteCommon.h"
#include"Sprite.h"
#include"TextureManager.h"

#include"Object3dCommon.h"
#include"Object3d.h"
#include"ModelCommon.h"
#include"Model.h"
#include"ModelManager.h"

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")


# define PI 3.14159265359f

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


	/*-----------------------------------------------------------------------------------------------------------
	|																											|
	|												初期化処理													|
	|																											|
	-----------------------------------------------------------------------------------------------------------*/
#pragma region 基盤システムの初期化

	//ウィンドウズアプリケーション
	WinApp* winApp = nullptr;
	winApp = new WinApp();
	winApp->Initialize();

	//リークチェッカー
	D3DResourceLeakChecker leakChecker;


	//DirectX共通部分
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//スプライト共通部分
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);



	//オブジェクト3Dの共通部分
	Object3dCommon* object3dCommon = nullptr;
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	//モデル共通部分
	ModelCommon* modelCommon = nullptr;
	modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);


#pragma endregion 基盤システムの初期化


#pragma region TextureManegerの初期化
	//テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	//テクスチャマネージャに追加する画像ハンドル
	std::string uvCheckerTextureHandle = "Resources/uvChecker.png";
	std::string monsterBallTextureHandle = "Resources/monsterBall.png";

	//画像ハンドルをテクスチャマネージャに挿入する
	TextureManager::GetInstance()->LoadTexture(uvCheckerTextureHandle);
	TextureManager::GetInstance()->LoadTexture(monsterBallTextureHandle);

#pragma endregion TextureManegerの初期化

#pragma region ModelManagerの初期化
	//モデルマネージャーの初期化
	ModelManager::GetInstance()->initialize(dxCommon);

	//モデルディレクトリパス
	const std::string modelDirectoryPath = "Resources";
	//モデルファイルパス
	const std::string modelFileNamePath = "plane.obj";
	//モデルファイルパス2
	const std::string modelFileNamePath2 = "barrier.obj";

	ModelManager::GetInstance()->LoadModel(modelFileNamePath);
	ModelManager::GetInstance()->LoadModel(modelFileNamePath2);

#pragma endregion ModelManagerの初期化
	//入力初期化
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	/*---------------
		スプライト
	---------------*/
#pragma region スプライトの初期化
	//左右反転フラグ
	bool isFlipX_;
	//上下反転フラグ
	bool isFlipY_;
	//テクスチャの左上座標
	Vector2 textureLeftTop;
	//テクスチャから初期サイズを得るフラグ
	bool isAdjustTextureSize;


	// スプライト初期化
	std::vector<Sprite*> sprites;
	for (uint32_t i = 0; i < 9; ++i) {

		Sprite* sprite = new Sprite();

		//もしfor文のiが偶数なら
		if (i % 2 == 0) {
			//モンスターボールを表示させる
			sprite->Initialize(spriteCommon, winApp, dxCommon, monsterBallTextureHandle);
		}
		else {
			//uvCheckerを表示させる
			sprite->Initialize(spriteCommon, winApp, dxCommon, uvCheckerTextureHandle);
		}


		// 各スプライトに異なる位置やプロパティを設定する
		Vector2 spritePosition = { i * -1280.0f, 0.0f }; // スプライトごとに異なる位置
		float spriteRotation = 0.0f;                 // 回転は任意
		Vector4 spriteColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 色は白（RGBA）
		Vector2 size = { 50.0f, 50.0f };             // 任意のサイズ

		//各種機能を使えるようにする
		isFlipX_ = sprite->GetFlipX();
		isFlipY_ = sprite->GetFlipY();
		textureLeftTop = sprite->GetTextureLeftTop();
		isAdjustTextureSize = sprite->GetIsAdjustTextureSize();


		sprite->SetPosition(spritePosition);
		sprite->SetRotation(spriteRotation);
		sprite->SetColor(spriteColor);
		sprite->SetSize(size);
		sprite->SetTextureLeftTop(textureLeftTop);
		sprite->SetGetIsAdjustTextureSize(isAdjustTextureSize);

		sprites.push_back(sprite);
	}
#pragma endregion スプライトの初期化

	/*---------------
	  オブジェクト3D
	---------------*/
#pragma region 3Dモデルの初期化
	//オブジェクト3D
	Object3d* object3d;
	object3d = new Object3d();
	object3d->Initialize(object3dCommon, winApp, dxCommon);

	Vector3 modelPosition = { -1.0f,0.0f,0.0f };
	Vector3 modelRotation = { 0.0f,0.0f,0.0f };
	Vector3 modelScale = { 1.0f,1.0f,1.0f };

	
	//モデル
	Model* model = nullptr;
	model = new Model();
	model->Initialize(modelCommon,modelDirectoryPath,modelFileNamePath);

	object3d->SetModel(model);
	object3d->SetModel("plane.obj");

	////////////////////////////////////////////////////////////////////////

	

	//オブジェクト3D
	Object3d* object3d2;
	object3d2 = new Object3d();
	object3d2->Initialize(object3dCommon, winApp, dxCommon);

	Vector3 modelPosition2 = { 1.0f,0.0f,0.0f };
	Vector3 modelRotation2 = { 0.0f,0.0f,0.0f };
	Vector3 modelScale2 = { 1.0f,1.0f,1.0f };


	//モデル
	Model* model2 = nullptr;
	model2 = new Model();
	model2->Initialize(modelCommon,modelDirectoryPath, modelFileNamePath2);

	object3d2->SetModel(model2);
	object3d2->SetModel(modelFileNamePath2);

#pragma endregion 3Dモデルの初期化






	//ウィンドウの×ボタンんが押されるまでループ
	while (true) {
		/*-------------------
		 Windowsメッセージ処理
		-------------------*/
		if (winApp->ProcessMessage()) {
			break;
		}

		//ウィンドウにメッセージが来てたら最優先で処理させる


		/*-----------------------------------------------------------------------------------------------------------
		|																											|
		|												更新処理														|
		|																											|
		-----------------------------------------------------------------------------------------------------------*/

		/*-------------------
			 入力の更新
		-------------------*/



		/*-------
		  ImGui
		-------*/
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//スプライトのImGui
		for (Sprite* sprite : sprites) {
			if (sprite) {
				ImGui::Begin("Sprite");

				ImGui::Checkbox("isFlipX", &isFlipX_);
				ImGui::Checkbox("isFlipY", &isFlipY_);
				ImGui::Checkbox("isAdjustTextureSize", &isAdjustTextureSize);
				ImGui::DragFloat2("textureLeftTop", &textureLeftTop.x);
				ImGui::End();
			}
		}
		ImGui::Begin("Object3D");
		ImGui::DragFloat3("Position", &modelPosition.x);
		ImGui::DragFloat3("Rotation", &modelRotation.x);
		ImGui::DragFloat3("Scale", &modelScale.x);
		ImGui::End();
		ImGui::Begin("Object3D2");
		ImGui::DragFloat3("Position", &modelPosition2.x);
		ImGui::DragFloat3("Rotation", &modelRotation2.x);
		ImGui::DragFloat3("Scale", &modelScale2.x);
		ImGui::End();





		ImGui::ShowDemoWindow();
		ImGui::Render();



		/*--------------
		   ゲームの処理
		--------------*/
		//入力の更新
		input->Update();

		modelRotation.y += 0.01f;

		modelRotation2.x -= 0.01f;
		//modelRotation2.y -= 0.01f;
		modelRotation2.z -= 0.01f;

		//オブジェクト3Dの更新
		object3d->Update();

		object3d->SetPosition(modelPosition);
		object3d->SetRotation(modelRotation);
		object3d->SetScale(modelScale);

		object3d2->Update();

		object3d2->SetPosition(modelPosition2);
		object3d2->SetRotation(modelRotation2);
		object3d2->SetScale(modelScale2);


		//スプライトの更新
		for (Sprite* sprite : sprites) {
			if (sprite) {
				// ここでは各スプライトの位置や回転を更新する処理を行う
				// 例: X軸方向に少しずつ移動させる
				Vector2 currentPosition = sprite->GetPosition();
				currentPosition.x += 4.0f; // 毎フレーム少しずつ右に動かす
				float currentRotation = sprite->GetRotation();

				sprite->SetPosition(currentPosition);
				sprite->SetRotation(currentRotation);
				sprite->SetTextureLeftTop(textureLeftTop);
				sprite->SetFlipX(isFlipX_);
				sprite->SetFlipY(isFlipY_);
				sprite->SetGetIsAdjustTextureSize(isAdjustTextureSize);

				sprite->Update();
			}
		}




		/*-----------------------------------------------------------------------------------------------------------
		|																											|
		|												描画処理														|
		|																											|
		-----------------------------------------------------------------------------------------------------------*/
#pragma region 描画処理

		/*-------------------
		　　DirectX描画開始
		　-------------------*/
		dxCommon->PreDraw();

		/*-------------------
		　　シーンの描画
	　　-------------------*/



	  //3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->DrawSettingsCommon();

		//オブジェクト3Dの描画

#pragma region Draw3D

		object3d->Draw();
		object3d2->Draw();

#pragma endregion Draw3D

#pragma region Draw2D
		/*-------------------
				2D
		--------------------*/

		//2Dオブジェクトの描画準備。2Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		spriteCommon->DrawSettingsCommon();

		// 描画処理
		for (Sprite* sprite : sprites) {
			if (sprite) {
				sprite->Draw();
			}
		}

#pragma endregion Draw2D

		/*-------------------
		　　DirectX描画終了
	  　　-------------------*/

		dxCommon->PostDraw();

#pragma endregion 描画処理
	}

#pragma region AllRelease



	//リソースリークチェック

	//WindowsAppの削除
	winApp->Finalize();
	delete winApp;
	winApp = nullptr;

	//DirectX共通部分の削除
	CloseHandle(dxCommon->GetFenceEvent());
	delete dxCommon;

	//入力の削除
	delete input;

	//スプライト共通部分の削除
	delete spriteCommon;


	for (Sprite* sprite : sprites) {
		if (sprite) {
			delete sprite; // メモリを解放
		}
	}

	delete object3dCommon;
	delete object3d;

	delete modelCommon;
	delete model;


	delete model2;
	delete object3d2;
	sprites.clear(); // ポインタをクリア

	//テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	//モデルマネージャーの終了
	ModelManager::GetInstance()->Finalize();

	//ImGui
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//警告時に止まる
	//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

#pragma endregion AllRelease


	return 0;

}
>>>>>>> a
