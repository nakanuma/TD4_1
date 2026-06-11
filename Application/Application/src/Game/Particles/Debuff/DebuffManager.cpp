#include "DebuffManager.h"
//C++
#include <random>


//application
#include <ParticleEffect/ParticleEffectManager.h>

//Engine
#include <ImguiWrapper.h>

DebuffManager* DebuffManager::GetInstance() {
	static DebuffManager instance;
	return &instance;
}

void DebuffManager::Update(float dt) {
	// 全てのパーティクル更新
	for (size_t i = 0; i < debuffs_.size(); ) {

		auto& debuff = debuffs_[i];

		// =====================================================
		// 時間更新
		// =====================================================
		debuff.time += dt;

		//発生高度と発生範囲からランダムの位置からパーティクルを発生させる
		if (debuff.time < kMaxTime_) {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> disX(-kRange_.x, kRange_.x);
			std::uniform_real_distribution<float> disZ(-kRange_.y, kRange_.y);
			std::uniform_int_distribution<int> countDis(0, 1); // 1から2の整数を生成する分布

			int count = countDis(gen); // 生成された整数を取得

			Cygnus::ParticleEffectManager::GetInstance()->Emit("debuff", debuff.position + Cygnus::Float3{ disX(gen),  kHeight_,  disZ(gen) }, count);

		}

		++i;
	}
}

void DebuffManager::Debug() {
#ifdef _DEBUG
	ImGui::Begin("Debuffs Manager Debug");
	//発生位置の入力
	ImGui::DragFloat3("Emit Position", &emitPosition_.x, 0.1f);
	//発生
	if (ImGui::Button("Create")) {
		Create(emitPosition_);
	}

	ImGui::End();
#endif // _DEBUG

}

void DebuffManager::Create(const Cygnus::Float3& position) {
	// インタラクトの生成
	DebuffParticle debuff;
	debuff.position = position;
	debuff.time = 0.0f;

	// パーティクルをマップに追加
	debuffs_.push_back(debuff);
}
