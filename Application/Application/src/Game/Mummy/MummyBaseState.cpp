#include "MummyBaseState.h"
#include "src/Game/Mummy/Mummy.h"

namespace MummyState {

	void SummonState::Update(Mummy& mummy, float dt) {
		summonTime_ -= dt;
		rotateY_ += dt;

		mummy.Rotate({0,rotateY_,0});
		mummy.Move({0,dt * 2.0f,0});

		if (summonTime_ < 0.0f) {
			mummy.CreateCollider();//コライダーを作成 + 登録
			mummy.ChangeMummyState(std::make_unique<MummyState::MoveState>());
		}
	}

	void MoveState::Update(Mummy& mummy, float dt) {

		Cygnus::Float3 moveNormal = Cygnus::Float3::Normalize(mummy.GetPlayerMummyLength());//移動方向設定 [プレイヤーのいる方向に]
		mummy.Move(moveNormal * kMoveSpeed_ * dt);

		float targetAngle = std::atan2(moveNormal.x, moveNormal.z);

		mummy.Rotate({ 0.0f,targetAngle,0.0f });
	
		//生存時間
		lifeTime_ -= dt;

		if (lifeTime_ < 0.0f) {
			mummy.ChangeMummyState(std::make_unique<MummyState::DeadState>());
		}
	}

	void DeadState::Update(Mummy& mummy, float dt) {
		if (performanceTime_ >= 1.0f) {
			mummy.DeleteCollider();//コライダー削除
		}

		performanceTime_ -= dt;
		rotateY_ -= dt * 2.0f;


		mummy.Rotate({ 0,rotateY_,0 });
		mummy.Move({ 0,-0.1f,0 });

		if (performanceTime_ < 0.0f) {
			mummy.OnDeadFlag();//死亡フラグ
		}
	}
}


