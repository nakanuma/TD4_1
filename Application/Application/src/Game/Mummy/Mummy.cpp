#include "Mummy.h"
#include "src/Game/Player/Player.h"
#include "ImguiWrapper.h"
#include <Collider/CollisionMath.h>

void Mummy::Initialize(const Cygnus::Float3& translate) {
	object_ = std::make_unique<Cygnus::Object3D>();
	object_->model_ = &Cygnus::ModelManager::GetInstance()->GetModel("Mummy");
	object_->transform_.translate_ = translate;

	state_ = std::make_unique<MummyState::SummonState>();
}

void Mummy::Update(const Cygnus::Float3& playerPos, float deltaTime) {

	//プレイヤーとの距離
	playerAndMummyLength_ = playerPos - object_->transform_.translate_;

	state_->Update(*this, deltaTime);

	// コライダー更新
	if (collider_) {
		collider_->Update();
	}

	// オブジェクト更新
	object_->UpdateMatrix();
}

void Mummy::Draw() {
	object_->Draw();
}

void Mummy::Debug() {
	
}

void Mummy::OnCollision(Cygnus::Collider* other) {
	if (other->GetTag() == "Ore" || other->GetTag() == "WorkBench") {

		Cygnus::OBBCollider* myOBB = dynamic_cast<Cygnus::OBBCollider*>(collider_.get());
		Cygnus::AABBCollider* otherAABB = dynamic_cast<Cygnus::AABBCollider*>(other);

		// 押し戻し処理
		if (myOBB && other)
		{
			// 相手のAABBを一時的にOBBとして扱う
			Cygnus::OBBCollider otherAsOBB;
			otherAsOBB.SetCenter((otherAABB->GetMin() + otherAABB->GetMax()) * 0.5f);
			otherAsOBB.SetSize((otherAABB->GetMax() - otherAABB->GetMin()) * 0.5f);
			otherAsOBB.SetXAxis({ 1.0f, 0.0f, 0.0f });
			otherAsOBB.SetYAxis({ 0.0f, 1.0f, 0.0f });
			otherAsOBB.SetZAxis({ 0.0f, 0.0f, 1.0f });

			// 押し戻しベクトルを計算
			Cygnus::Float3 pushVec = Cygnus::CollisionMath::CalculatePushBackOBBvsOBB(myOBB, &otherAsOBB);

			// 位置を補正
			Move(pushVec);

			// コライダーも更新
			myOBB->Update();
		}
	}

	if (other->GetTag() == "Player" || other->GetTag() == "Sphinx" || other->GetTag() == "SunLaser") {
		ChangeMummyState(std::make_unique<MummyState::DeadState>());
	}

}

void Mummy::Move(const Cygnus::Float3& move) {
	object_->transform_.translate_ += move;
	object_->UpdateMatrix();
}

void Mummy::Rotate(const Cygnus::Float3 rotate) {
	object_->transform_.rotate_ = rotate;
	object_->UpdateMatrix();
}


void Mummy::CreateCollider() {
	// コライダー生成 + 登録
	auto aabb = std::make_unique<Cygnus::OBBCollider>();
	aabb->SetTag("Mummy");
	aabb->SetFollowTarget(&object_->transform_.translate_);
	aabb->SetFollowRotation(&object_->transform_.rotate_);
	aabb->SetSize(kColliderSize_);
	aabb->SetOwner(this);

	collider_ = std::move(aabb);
	Cygnus::CollisionManager::GetInstance()->Register(collider_.get());
}

void Mummy::DeleteCollider() {
	Cygnus::CollisionManager::GetInstance()->Unregister(&*collider_);//コライダー削除
	collider_.reset();
}

void Mummy::ChangeMummyState(std::unique_ptr<MummyState::BaseState> nextState) {
	state_ = std::move(nextState);
}
