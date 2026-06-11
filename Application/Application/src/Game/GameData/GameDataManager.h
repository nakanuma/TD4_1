#pragma once

class GameDataManager
{
public:
    static GameDataManager* GetInstance()
    {
        static GameDataManager instance;
        return &instance;
    }

    // 選択したステージのセッターとゲッター
    void SetTargetStage(int stageId) { targetStage_ = stageId; }
    int GetTargetStage() const { return targetStage_; }

    const std::string GetStageJsonName() {
        return "resources/stageEditor/stage_" + std::to_string(targetStage_) + ".json";
    }

private:
    GameDataManager() = default;
    ~GameDataManager() = default;

    int targetStage_ = 0; // デフォルトのステージ
};
