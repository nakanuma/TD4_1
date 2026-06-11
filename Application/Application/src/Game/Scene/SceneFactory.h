#pragma once

// ---------------------------------------------------------
// Engine Includes
// ---------------------------------------------------------
#include <Scene/AbstractSceneFactory.h>

// =========================================================
// このゲーム用のシーン工場
// =========================================================
class SceneFactory : public Cygnus::AbstractSceneFactory {
public:
	// =========================================================
	// Public Methods
	// =========================================================

	/// <summary>
	/// シングルトンインスタンスの取得を行います。
	/// </summary>
	/// <returns>シングルトンインスタンス</returns>
	static SceneFactory* GetInstance();

	/// <summary>
	/// シーンの生成を行います。
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	/// <returns>生成したシーン</returns>
	std::unique_ptr<Cygnus::BaseScene> CreateScene(const std::string& sceneName) override;

	/// <summary>
	/// 初期化時のシーン名を取得します。
	/// </summary>
	/// <returns></returns>
	std::string GetInitialSceneName() override;

private:
	// =========================================================
	// Member Variables
	// =========================================================

	// 初期化時シーン名
	std::string initialSceneName = "TITLE";
};