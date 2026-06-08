#pragma once

// ---------------------------------------------------------
// Engine Includes
// ---------------------------------------------------------
#include <Object3D.h>
#include <Collider/CollisionManager.h>
#include "src/Game//Sandstrom/FlyAway.h"

// =========================================================
// プレイヤークラス
// =========================================================
class Player : public Cygnus::ICollisionCallback
{
public:
	// =========================================================
	// Public Methods
	// =========================================================

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// デバッグ表示
	/// </summary>
	void Debug();

	/// <summary>
	/// 衝突時コールバック
	/// </summary>
	/// <param name="other"></param>
	void OnCollision(Cygnus::Collider* other) override;

	// =========================================================
	// Accessor
	// =========================================================

	/// <summary>
	/// 鉱石を拾えるか（所持最大数に達していないか）
	/// </summary>
	/// <returns></returns>
	bool CanPickUpOre() const { return oreCount_ < kMaxOreCount; }

	/// <summary>
	/// プレイヤーの所持鉱石を1増やす（落ちている鉱石取得時）
	/// </summary>
	void AddOreCount() { oreCount_++; }

	/// <summary>
	/// 鉱石所持数を取得
	/// </summary>
	uint32_t GetOreCount() { return oreCount_; }

	/// <summary>
	/// 指定した個数分だけ所持鉱石を減らす（歯車作成時の処理）
	/// </summary>
	/// <param name="amount">鉱石の消費数</param>
	void ConsumeOre(uint32_t amount) {
		if (oreCount_ >= amount) {
			oreCount_ -= amount;
		}
	}

	/// <summary>
	/// 歯車を拾えるか（所持最大数に達していないか）
	/// </summary>
	/// <returns></returns>
	bool CanPickUpGear() const { return gearCount_ < kMaxGearCount; }

	/// <summary>
	/// プレイヤーの所持歯車を1増やす（落ちている歯車取得時）
	/// </summary>
	void AddGearCount() { gearCount_++; }

	/// <summary>
	/// 歯車所持数を取得
	/// </summary>
	uint32_t GetGearCount() { return gearCount_; }

	/// <summary>
	/// 指定した個数分だけ歯車所持数を減らす（列車注入時の処理）
	/// </summary>
	/// <param name="amount"></param>
	void ConsumeGear(uint32_t amount) {
		if (gearCount_ >= amount) {
			gearCount_ -= amount;
		}
	}

	/// setter_座標位置
	/// </summary>
	/// <param name="translate">座標を設定</param>
	void SetTranslate(const Cygnus::Float3& translate) { object_->transform_.translate_ = translate; }
  
	/// <summary>
	/// プレイヤー座標を取得
	/// </summary>
	/// <returns></returns>
	Cygnus::Float3 GetPosition() const { return object_->transform_.translate_; }

private:
	// =========================================================
	// Internal Methods
	// =========================================================

	/// <summary>
	/// キーボード用入力取得
	/// </summary>
	/// <returns></returns>
	Cygnus::Float3 GetKeyInput();

	/// <summary>
	/// ゲームパッド用入力取得
	/// </summary>
	/// <returns></returns>
	Cygnus::Float3 GetPadInput();

private:
	// =========================================================
	// Constants
	// =========================================================

	const float kMoveSpeed = 20.0f;	// 移動速度
	const Cygnus::Float3 kColliderSize = {1.0f, 2.0f, 1.0f};	// コライダーサイズ
	
	float slowDown_ = 0.0f;    //減速する値
	const float kSlowDownMax_ = 15.0f;//最大減速

	const float kMiningOffset = 1.5f;	// 採掘時の前方オフセット
	const float kMiningRange = 1.2f;	// 採掘時のブレ許容値

	const uint32_t kMaxOreCount = 4;	// 所持できる鉱石の最大数
	const uint32_t kMaxGearCount = 2;	// 所持できる歯車の最大数

	const float kRequiredMinigTime = 1.0f;	// 採掘に必用な時間

	// =========================================================
	// Member Variables
	// =========================================================

	std::unique_ptr<Cygnus::Object3D> object_;	// オブジェクト
	std::unique_ptr<Cygnus::Collider> collider_;	// コライダー

	Cygnus::Float3 velocity_ = {0.0f, 0.0f, 0.0f};	// 速度ベクトル

	uint32_t oreCount_ = 0;	// 現在の所持鉱石数
	uint32_t gearCount_ = 0; // 現在の所持歯車数

	float miningTimer_ = 0.0f;	// 採掘の長押し時間カウント

	FlyAway flyAway_;
};

