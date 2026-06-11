#include "Player.h"

// Engine
#include <Input/Input.h>
#include <ImguiWrapper.h>
#include <TimeManager.h>
#include <ParticleEffect/ParticleEffectManager.h>
#include <SoundManager.h>
#include <Collider/CollisionMath.h>
#include <src/Game/Particles/Debuff/DebuffManager.h>

// Application
#include <src/Game/Ore/OreManager.h>
#include <src/Game/UI/UIManager.h>

void Player::Initialize() {
	// オブジェクト生成
	object_ = std::make_unique<Cygnus::Object3D>();
	object_->model_ = &Cygnus::ModelManager::GetInstance()->GetModel("Player");

	// コライダー生成 + 登録
	auto aabb = std::make_unique<Cygnus::AABBCollider>();
	aabb->SetTag("Player");
	aabb->SetFollowTarget(&object_->transform_.translate_);
	aabb->SetSize(kColliderSize);
	aabb->SetOwner(this);

	collider_ = std::move(aabb);
	Cygnus::CollisionManager::GetInstance()->Register(collider_.get());
}

void Player::Update(float deltaTime) {
	auto input = Cygnus::Input::GetInstance();

#pragma region
	// キーボードとゲームパッド両方の入力を加算
	Cygnus::Float3 moveDir = { 0.0f, 0.0f, 0.0f };
	moveDir += GetKeyInput();
	moveDir += GetPadInput();

	// 入力がある場合のみ回転と移動を行う
	if (Cygnus::Float3::Length(moveDir) > 0.01f) {
		// 回転処理
		float angle = std::atan2f(moveDir.x, moveDir.z);	// 入力ベクトルから角度を計算
		const float kStep = Cygnus::PIf / 4.0f;	// 8方向に限定するため45度
		object_->transform_.rotate_.y = std::round(angle / kStep) * kStep;	// オブジェクト回転に反映

		// 移動処理
		if (Cygnus::Float3::Length(moveDir) > 1.0f) {
			// 正規化して一定の速度を保つように
			moveDir = Cygnus::Float3::Normalize(moveDir);
		}
	}


	// オブジェクト位置に反映
	object_->transform_.translate_ += moveDir * (kMoveSpeed - slowDown_) * deltaTime;
	
	//重力と砂嵐で飛ばされる処理
	flyAway_.Update(object_->transform_.translate_);


	//ミイラで遅くなる処理
	slowDown_ -= deltaTime;
	slowDown_ = std::clamp(slowDown_, 0.0f, kSlowDownMax_);

	// 移動パーティクル生成
	if (Cygnus::Float3::Length(moveDir) > 0.01f) {
		Cygnus::ParticleEffectManager::GetInstance()->Emit("move_dust", object_->transform_.translate_ + Cygnus::Float3(0, -2.0f, 0),
			1,
			Cygnus::Float3(0, 0, 0),
			0.0f
		);
	}


#pragma endregion

#pragma region 
	// 向きから前方のベクトルを作成する
	float angleY = object_->transform_.rotate_.y;
	Cygnus::Float3 frontVec = {std::sinf(angleY), 0.0f, std::cosf(angleY)};

	// プレイヤーの少し前方を判定の中心にする
	Cygnus::Float3 targetPos = {
		object_->transform_.translate_.x + frontVec.x * kMiningOffset, 
		object_->transform_.translate_.y, 
		object_->transform_.translate_.z + frontVec.z * kMiningOffset
	};

	// キー入力に関係なく、前方に鉱石があるならUI表示をリクエスト
	if (OreManager::GetInstance()->IsBreakableAt(targetPos, kMiningRange)) {
		UIManager::GetInstance()->RequestInteract(InteractGuide::ActionType::Mine);
	}

	// 実際の採掘実行（キー入力）
	if (input->TriggerKey(DIK_SPACE) || input->IsTriggerButton(0, XINPUT_GAMEPAD_A)) {
		// 鉱石採掘判定
		if (OreManager::GetInstance()->TryBreakAt(targetPos, kMiningRange)) {
			// 鉱石採掘時の処理
		}
	}
#pragma endregion

	// コライダー更新
	collider_->Update();
	// オブジェクト更新
	object_->UpdateMatrix();
}

void Player::Draw() {
	// オブジェクト描画
	object_->Draw();
}

void Player::Debug() {
#ifdef USE_IMGUI
	ImGui::Begin("Player");

	ImGui::DragFloat3("Translate", &object_->transform_.translate_.x, 0.01f);
	ImGui::Text("OreCount : %d", oreCount_);
	ImGui::Text("GearCount : %d", gearCount_);

	ImGui::End();
#endif
}

void Player::OnCollision(Cygnus::Collider* other) {
	// 押し戻しを行うオブジェクトとの衝突
	// : 鉱石オブジェクト, 工作台オブジェクト
	if (other->GetTag() == "Ore" || other->GetTag() == "WorkBench" || other->GetTag() == "Sphinx") {
		Cygnus::AABBCollider* myAABB = dynamic_cast<Cygnus::AABBCollider*>(collider_.get());
		Cygnus::AABBCollider* otherAABB = dynamic_cast<Cygnus::AABBCollider*>(other);

		// 押し戻し処理
		if (myAABB && otherAABB) {
			// 押し戻しベクトル取得
			Cygnus::Float3 pushVec = myAABB->GetPushBackVector(*otherAABB);
			// プレイヤー位置を補正
			object_->transform_.translate_ += pushVec;
			object_->UpdateMatrix();

			// コライダーも更新
			Cygnus::Float3 currentMin = myAABB->GetMin();
			Cygnus::Float3 currentMax = myAABB->GetMax();
			myAABB->SetMin(currentMin + pushVec);
			myAABB->SetMax(currentMax + pushVec);
		}
	}
	// 列車オブジェクト（OBB）
	if(other->GetTag() == "Carrier") {
		Cygnus::AABBCollider* myAABB = dynamic_cast<Cygnus::AABBCollider*>(collider_.get());
		Cygnus::OBBCollider* otherOBB = dynamic_cast<Cygnus::OBBCollider*>(other);

		if(myAABB && otherOBB) {
			// 自身のAABBを一時的にOBBにする
			Cygnus::OBBCollider myAsOBB;
			myAsOBB.SetCenter((myAABB->GetMin() + myAABB->GetMax()) * 0.5f);
			myAsOBB.SetSize((myAABB->GetMax() - myAABB->GetMin()) * 0.5f);
			myAsOBB.SetXAxis({ 1.0f, 0.0f, 0.0f });
			myAsOBB.SetYAxis({ 0.0f, 1.0f, 0.0f });
			myAsOBB.SetZAxis({ 0.0f, 0.0f, 1.0f });

			// OBB vs OBBの押し戻しベクトルを計算
			Cygnus::Float3 pushVec = Cygnus::CollisionMath::CalculatePushBackOBBvsOBB(otherOBB, &myAsOBB);

			// プレイヤー位置を補正
			object_->transform_.translate_ -= pushVec;
			object_->UpdateMatrix();

			// AABBコライダーも更新
			Cygnus::Float3 currentMin = myAABB->GetMin();
			Cygnus::Float3 currentMax = myAABB->GetMax();
			myAABB->SetMin(currentMin - pushVec);
			myAABB->SetMax(currentMax - pushVec);
		}
	}


	if (other->GetTag() == "Mummy") {
		slowDown_ = kSlowDownMax_; 
		Cygnus::SoundManager::GetInstance()->Play("slow", false, 0.5f);
		//デバフパーティクルを付ける
		DebuffManager::GetInstance()->Create(object_->transform_.translate_);
	}

	if (other->GetTag() == "Sandstorm") {
		flyAway_.InSandstorm();
	}

}

Cygnus::Float3 Player::GetKeyInput() {
	auto input = Cygnus::Input::GetInstance();
	Cygnus::Float3 dir = { 0.0f, 0.0f, 0.0f };
	// 移動キー入力
	if (input->PushKey(DIK_W)) dir.z += 1.0f;
	if (input->PushKey(DIK_S)) dir.z -= 1.0f;
	if (input->PushKey(DIK_A)) dir.x -= 1.0f;
	if (input->PushKey(DIK_D)) dir.x += 1.0f;
	// キー入力結果を返す
	return dir;
}

Cygnus::Float3 Player::GetPadInput() {
	auto input = Cygnus::Input::GetInstance();
	XINPUT_STATE state;

	// コントローラー取得
	if (input->GetJoystickState(0, state)) {
		// 左スティック入力結果を返す
		return {
			state.Gamepad.sThumbLX / 32767.0f,
			0.0f,
			state.Gamepad.sThumbLY / 32767.0f
		};
	}
	return { 0.0f, 0.0f, 0.0f };
}
