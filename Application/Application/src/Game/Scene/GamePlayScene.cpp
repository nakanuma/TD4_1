#include "GamePlayScene.h"
#include "DirectXBase.h"
#include "ImguiWrapper.h"
#include "RTVManager.h"
#include "SRVManager.h"
#include "SpriteCommon.h"
#include <Engine/3D/LightCamera.h>
#include <Engine/3D/LineDrawer.h>
#include <Engine/DirectX/ShadowMapManager.h>
#include <Engine/Model/SkyBoxManager.h>
#include <Engine/ParticleEffect/ParticleEffectManager.h>

// C++
#include <numbers>

// Engine
#include <Engine/Scene/SceneManager.h>
#include <CommandManager.h>

// Application
#include <src/Game/Path/PathManager.h>
#include <src/Game/Ore/OreManager.h>
#include <src/Game/WorkBench/WorkBenchManager.h>
#include <src/Game/Gear/GearManager.h>
#include <src/Game/Particles/Fireworks/FireworksManager.h>
#include <src/Game/Particles/SandRain/SandRainManager.h>
#include <src/Game/Particles/Interact/InteractManager.h>
#include <src/Game/Particles/Debuff/DebuffManager.h>
#include <src/Game/Particles/Storm/StormManager.h>

#include <src/Game/Sandstrom/SandstormManager.h>
#include <src/Game/GameData/GameDataManager.h>

#include <src/Game/GameData/GameDataManager.h>

void GamePlayScene::Initialize() {
	Cygnus::DirectXBase* dxBase = Cygnus::DirectXBase::GetInstance();

	// カメラのインスタンスを生成
	camera_ = std::make_unique<Cygnus::Camera>(Cygnus::Float3{ 0.0f, 80.0f, -60.0f }, Cygnus::Float3{ 1.0f, 0.0f, 0.0f }, 0.45f);
	Cygnus::Camera::Set(camera_.get()); // 現在のカメラをセット

	// SpriteCommonの生成と初期化
	spriteCommon_ = std::make_unique<Cygnus::SpriteCommon>();
	spriteCommon_->Initialize(Cygnus::DirectXBase::GetInstance());

	// TextureManagerの初期化
	Cygnus::TextureManager::Initialize(dxBase->GetDevice(), Cygnus::SRVManager::GetInstance());

	// Inputの初期化
	input_ = Cygnus::Input::GetInstance();

	// LightManagerの初期化
	lightManager_ = Cygnus::LightManager::GetInstance();
	lightManager_->Initialize();

	// シャドウマップ生成
	shadowMapHandle_ = Cygnus::ShadowMapManager::GetInstance()->CreateShadowMap(Cygnus::Window::GetWidth(), Cygnus::Window::GetHeight());

	// ポストエフェクト管理
	postEffectManager_ = std::make_unique<Cygnus::PostEffectManager>();
	postEffectManager_->Initialize();

	///
	///	↓ ゲームシーン用
	///
	
	// 地面オブジェクト生成
	objectGround_ = std::make_unique<Cygnus::Object3D>();
	objectGround_->model_ = &Cygnus::ModelManager::GetInstance()->GetModel("Plane"); // モデル設定
	objectGround_->transform_.rotate_ = {-Cygnus::PIf / 2.0f, 0.0f, 0.0f}; // 上向き
	objectGround_->transform_.scale_ = {500.0f, 500.0f, 1.0f}; // スケール変更
	objectGround_->materialCB_.data_->color = {0.95f, 0.9f, 0.56f, 1.0f}; // 色変更

	// 経路管理クラス初期化
	PathManager::GetInstance()->Initialize();

	// 鉱石オブジェクト管理クラス初期化
	OreManager::GetInstance()->Initialize();

	// 工作台オブジェクト管理クラス初期化
	WorkBenchManager::GetInstance()->Initialize();

	// 歯車オブジェクト管理クラス初期化
	GearManager::GetInstance()->Initialize();

	player_ = std::make_unique<Player>();
	player_->Initialize();

	sphinx_ = std::make_unique<Sphinx>();
	sphinx_->Initialize();

	// UI管理クラス初期化	
	UIManager::GetInstance()->Initialize();

	//ステージエディタ初期化
	stageEditor_ = std::make_unique<StageEditor>();
	stageEditor_->LoadJsonFile(GameDataManager::GetInstance()->GetStageJsonName());//ステージジェイソンファイルを読み込む
	stageEditor_->SpitObjects(player_,sphinx_);// プレイヤー生成 + 初期化

	//最初の線路に設置される
	// 経路に沿って移動するオブジェクト生成 + 初期化
	carrier_ = std::make_unique<Carrier>();
	carrier_->Initialize();

	// ゴールオブジェクト生成 + 初期化
	goal_ = std::make_unique<Goal>();
	goal_->Initialize();

	fieldManager_ = std::make_unique<FieldEventManager>();
	fieldManager_->Initialize(&*spriteCommon_, stageEditor_->GetEventRatio());

	MummyManager::GetInstance()->Initialize();

	SunLaser::GetInstance()->Initialize();
	Cygnus::SoundManager::GetInstance()->Play("bgm", true, 0.5f);
}

void GamePlayScene::Finalize()
{
	Cygnus::SoundManager::GetInstance()->Stop("sandstorm");
	Cygnus::SoundManager::GetInstance()->Stop("resshaugoku");
	Cygnus::SoundManager::GetInstance()->Stop("bgm");
}


void GamePlayScene::Update() {
	///
	///	共通更新処理
	/// 
	Cygnus::LightManager::GetInstance()->ClearEmissiveLights(); // エミッシブライトをクリア
	Cygnus::LightManager::GetInstance()->ClearAreaLights();     // エリアライトをクリア
	Cygnus::SkyBoxManager::GetInstance()->Update(); // SkyBox更新
	float dt = Cygnus::TimeManager::GetInstance()->GetDeltaTime();	// デルタタイム取得


	///
	///	他更新処理
	/// 

	// UIManagerのインタラクトUI表示要求をリセット（表示され続けないため）
	UIManager::GetInstance()->ClearInteractRequests();

	///
	///	オブジェクト更新処理
	/// 

	if (!carrier_->IsGoal())
	{
		// 地面オブジェクト更新
		objectGround_->UpdateMatrix();
		// プレイヤー更新
		player_->Update(dt);
		// 経路に沿って移動するオブジェクト更新
		carrier_->Update(dt);
		sphinx_->Update(dt, player_->GetPosition());
		// 鉱石オブジェクト管理クラス更新
		OreManager::GetInstance()->Update();
		// 工作台オブジェクト管理クラス更新
		WorkBenchManager::GetInstance()->Update();
		// 歯車オブジェクト管理クラス更新
		GearManager::GetInstance()->Update();
		// ゴールオブジェクト更新
		goal_->Update();

		fieldManager_->Update(dt);
		SunLaser::GetInstance()->Update(player_->GetPosition(), dt);

		//ミイラ召喚/管理クラスの更新
		MummyManager::GetInstance()->Update(player_->GetPosition(), dt);

		//　砂嵐管理クラスの更新
		SandstormManager::GetInstance()->Update();
	}

	//花火パーティクル更新
	FireworksManager::GetInstance()->Update(dt);
	//砂嵐パーティクル更新
	SandRainManager::GetInstance()->Update(dt);
	//インタラクトパーティクル更新
	InteractManager::GetInstance()->Update(dt);
	//デバフパーティクル更新
	DebuffManager::GetInstance()->Update(dt);
	//竜巻パーティクル更新
	StormManager::GetInstance()->Update(dt);

	///
	///	
	/// 
	
	// コライダー管理クラス更新
	Cygnus::CollisionManager::GetInstance()->Update();

	// UI管理クラス更新
	UIManager::GetInstance()->Update(
		player_->GetOreCount(), !player_->CanPickUpOre(), 
		player_->GetGearCount(), !player_->CanPickUpGear(), 
		player_->GetPosition(), 
		carrier_->GetTranslate(), carrier_->GetEnergyRatio()
	);

	// パーティクルエフェクト管理クラス更新
	Cygnus::ParticleEffectManager::GetInstance()->Update(dt);
}

void GamePlayScene::Draw() {
	Cygnus::DirectXBase* dxBase = Cygnus::DirectXBase::GetInstance();
	Cygnus::SRVManager* srvManager = Cygnus::SRVManager::GetInstance();
	auto* cmd = Cygnus::CommandManager::GetInstance()->GetCommandList();

	// 描画前処理
	dxBase->PreDraw();
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager->descriptorHeap_.heap_.Get() };
	cmd->SetDescriptorHeaps(1, descriptorHeaps);
	// ImGuiのフレーム開始処理
	Cygnus::ImguiWrapper::NewFrame();
	// カメラの定数バッファを設定
	Cygnus::Camera::TransferConstantBuffer();
	// ライトの定数バッファを設定
	lightManager_->TransferContantBuffer();
	// LightCameraの定数バッファを送信
	Cygnus::LightCamera::GetInstance()->TransferConstantBuffer();

	// ---------------------------------------------------------
	// シャドウマップ描画前処理
	// ---------------------------------------------------------

	// ライトカメラの更新
	Cygnus::LightCamera::GetInstance()->SetDirectionalLight(Cygnus::LightManager::GetInstance()->directionalLightCB_.data_->direction);

	// シャドウマップ描画開始
	Cygnus::ShadowMapManager::GetInstance()->BeginShadowPass(shadowMapHandle_);

	/// =========================================================
	/// ↓ ここから通常モデルのシャドウマップ描画
	/// =========================================================



	/// =========================================================
	/// ↑ ここまで通常モデルのシャドウマップ描画
	/// =========================================================

	// スキニングモデル用PSOをセット
	cmd->SetPipelineState(Cygnus::ShadowMapManager::GetInstance()->GetShadowSkinnedPSO());

	/// =========================================================
	/// ↓ ここからスキニングモデルのシャドウマップ描画
	/// =========================================================



	/// =========================================================
	/// ↑ ここまでスキニングモデルのシャドウマップ描画
	/// =========================================================

	Cygnus::ShadowMapManager::GetInstance()->EndShadowPass(shadowMapHandle_);

	/// =========================================================
	/// ↓ ここから3Dオブジェクト描画
	/// =========================================================

#pragma region
	postEffectManager_->BeginMainScene();

	// スカイボックス描画
	Cygnus::SkyBoxManager::GetInstance()->Draw();
	// -----------------------------------------------

	// 地面オブジェクト描画
	objectGround_->Draw();
	// プレイヤー描画
	player_->Draw();
	// 経路管理クラス描画
	PathManager::GetInstance()->Draw();
	// 経路に沿って移動するオブジェクト描画
	carrier_->Draw();
	sphinx_->Draw();
	// ゴールオブジェクト描画
	goal_->Draw();
	MummyManager::GetInstance()->Draw();
	// 鉱石オブジェクト管理クラス描画
	OreManager::GetInstance()->Draw();
	// 工作台オブジェクト管理クラス描画
	WorkBenchManager::GetInstance()->Draw();
	// 歯車オブジェクト管理クラス描画
	GearManager::GetInstance()->Draw();
	// 砂嵐管理クラスの描画
	SandstormManager::GetInstance()->Draw();

	SunLaser::GetInstance()->Draw();
	// -----------------------------------------------
	postEffectManager_->EndMainScene();
#pragma endregion

#pragma region バックバッファへの直接描画
	postEffectManager_->RestoreBackBuffer(true);
	// -----------------------------------------------

	// パーティクルエフェクト描画
	Cygnus::ParticleEffectManager::GetInstance()->Draw();
	// ライン描画
	Cygnus::LineDrawer::GetInstance()->Draw();

	// -----------------------------------------------
	postEffectManager_->RestoreDepthBufferState();

#pragma endregion

	/// =========================================================
	/// ↑ ここまで3Dオブジェクト描画
	/// =========================================================

	// Spriteの描画準備。全ての描画に共通のグラフィックスコマンドを積む
	spriteCommon_->PreDraw();

	/// =========================================================
	/// ↓ ここからスプライト描画
	/// =========================================================
	
	// UI管理クラス描画
	UIManager::GetInstance()->Draw();
	fieldManager_->Draw();

	/// =========================================================
	/// ↑ ここまでスプライト描画
	/// =========================================================

#ifdef _DEBUG // デバッグ表示
	// コライダー描画
	Cygnus::CollisionManager::GetInstance()->Draw();

	// ゲームシーン
	Debug();
	// プレイヤー
	player_->Debug();
	// 経路に沿って動くオブジェクト
	carrier_->Debug();
	//　ステージエディタの更新(jsonの生成処理)
	stageEditor_->Update();
	sphinx_->Debug();
	fieldManager_->Debug();
	MummyManager::GetInstance()->Debug();
	//　砂嵐管理クラスのデバッグ処理
	SandstormManager::GetInstance()->Debug();
#endif

	if (carrier_->IsGoal())
	{
		Cygnus::SceneManager::GetInstance()->ChangeScene("SELECT");
		Cygnus::CollisionManager::GetInstance()->Clear();
	}

	// ImGuiの内部コマンドを生成する
	Cygnus::ImguiWrapper::Render(cmd);
	// 描画後処理
	dxBase->PostDraw();
	// フレーム終了処理
	dxBase->EndFrame();
}

void GamePlayScene::Debug() {
#ifdef USE_IMGUI
	ImGui::Begin("GamePlaySceneInfo");

	ImGui::Text("fps:%.2f", ImGui::GetIO().Framerate);

	ImGui::End();
#endif

#ifdef USE_IMGUI
	ImGui::Begin("Camera");

	ImGui::DragFloat3("Translate", &camera_->transform_.translate_.x, 0.01f);
	ImGui::DragFloat3("Rotate", &camera_->transform_.rotate_.x, 0.01f);

	ImGui::End();
#endif

#ifdef USE_IMGUI
	//花火パーティクルのデバッグ表示
	FireworksManager::GetInstance()->Debug();
	//砂嵐パーティクルのデバッグ表示
	SandRainManager::GetInstance()->Debug();
	//インタラクトパーティクルのデバッグ表示
	InteractManager::GetInstance()->Debug();
	//デバフパーティクルのデバッグ表示
	DebuffManager::GetInstance()->Debug();
	//砂嵐管理クラスのデバッグ表示
	StormManager::GetInstance()->Debug();


#endif
}