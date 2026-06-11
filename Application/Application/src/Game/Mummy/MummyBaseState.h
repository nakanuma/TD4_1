#pragma once
#include "Float3.h"
#include <memory>

class Mummy;
namespace MummyState {
	class BaseState {
	public:
		virtual void Update(Mummy& mummy, float dt) = 0;

	protected:
		std::unique_ptr<BaseState> nextState_;
	};

	class SummonState : public BaseState {
	public:
		void Update(Mummy& mummy, float dt) override;
	private:
		float rotateY_ = 0.0f;
		float summonTime_ = 2.0f;
	};

	class MoveState : public BaseState {
	public:
		void Update(Mummy& mummy, float dt) override;
	private:

		Cygnus::Float3 velocity_ = { 0.0f, 0.0f, 0.0f };	// 速度ベクトル

		const float kMoveSpeed_ = 2.0f; //移動速度

		const float kLifeTimeMax_ = 6.0f;//生存時間
		float lifeTime_ = kLifeTimeMax_;//生存時間
	};

	class DeadState : public BaseState {
	public:
		void Update(Mummy& mummy, float dt) override;
	private:
		float performanceTime_ = 1.0f;//死亡演出時間
		float rotateY_ = 0.0f;
	};
}


