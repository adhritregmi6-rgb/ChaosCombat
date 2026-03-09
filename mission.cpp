#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip> // for std::fixed and std::setprecision

// Mission structure
struct Mission {
    int id;
    std::string description;
    int target;       // e.g., number of kills or objectives to complete
    int progress;     // current progress
    int rewardCoins;  // coins rewarded on completion
    bool completed;

    Mission(int i, std::string desc, int tgt, int reward)
        : id(i), description(std::move(desc)), target(tgt), progress(0), rewardCoins(reward), completed(false) {}

    // Update progress by amount, return true if mission completed this update
    bool UpdateProgress(int amount) {
        if (completed) return false;
        progress += amount;
        if (progress >= target) {
            progress = target;
            completed = true;
            return true;
        }
        return false;
    }

    void Display() const {
        std::cout << "Mission #" << id << ": " << description << "\n"
                  << "Progress: " << progress << "/" << target
                  << (completed ? " [COMPLETED]" : "") << "\n"
                  << "Reward: " << rewardCoins << " coins\n\n";
    }
};

// Player class managing coins, kill streaks, and missions
class Player {
public:
    Player() : coins(0), killStreak(0), maxKillStreak(0) {}

    void AddCoins(int amount) {
        coins += amount;
        std::cout << "[COINS] You earned " << amount << " coins! Total coins: " << coins << "\n";
    }

    void ResetKillStreak() {
        if (killStreak > maxKillStreak) maxKillStreak = killStreak;
        killStreak = 0;
        std::cout << "[KILLSTREAK] Kill streak reset.\n";
    }

    void RegisterKill() {
        killStreak++;
        if (killStreak > maxKillStreak) maxKillStreak = killStreak;

        float multiplier = GetKillStreakMultiplier();
        int baseReward = 10; // base coins per kill
        int reward = static_cast<int>(baseReward * multiplier);

        std::cout << "[KILL] Kill registered! Kill streak: " << killStreak
                  << ", Multiplier: " << std::fixed << std::setprecision(2) << multiplier
                  << ", Coins earned: " << reward << "\n";

        AddCoins(reward);
    }

    float GetKillStreakMultiplier() const {
        // 1 kill streak = 1.1, 2 = 1.2, 3 = 1.3, etc.
        // Cap multiplier at 2.0 for balance
        float multiplier = 1.0f + 0.1f * killStreak;
        if (multiplier > 2.0f) multiplier = 2.0f;
        return multiplier;
    }

    void CompleteMission(int missionId) {
        auto it = missions.find(missionId);
        if (it != missions.end()) {
            Mission& m = it->second;
            if (!m.completed) {
                m.completed = true;
                AddCoins(m.rewardCoins);
                std::cout << "[MISSION] Mission #" << missionId << " completed! Reward: " << m.rewardCoins << " coins.\n";
            }
        }
    }

    void AddMission(const Mission& mission) {
        missions[mission.id] = mission;
    }

    void UpdateMissionProgress(int missionId, int amount) {
        auto it = missions.find(missionId);
        if (it != missions.end()) {
            Mission& m = it->second;
            if (!m.completed) {
                bool justCompleted = m.UpdateProgress(amount);
                std::cout << "[MISSION] Updated mission #" << missionId << " progress by " << amount << ".\n";
                if (justCompleted) {
                    AddCoins(m.rewardCoins);
                    std::cout << "[MISSION] Mission #" << missionId << " completed! Reward: " << m.rewardCoins << " coins.\n";
                }
            }
        }
    }

    void DisplayHomeScreen() const {
        std::cout << "===== HOME SCREEN =====\n";
        std::cout << "Coins: " << coins << "\n";
        std::cout << "Current Kill Streak: " << killStreak << "\n";
        std::cout << "Max Kill Streak: " << maxKillStreak << "\n\n";

        std::cout << "Active Missions:\n";
        for (const auto& pair : missions) {
            pair.second.Display();
        }
        std::cout << "=======================\n\n";
    }

private:
    int coins;
    int killStreak;
    int maxKillStreak;
    std::unordered_map<int, Mission> missions;
};

// -------------------- Demo Usage --------------------

int main() {
    Player player;

    // Add some missions
    player.AddMission(Mission(1, "Get 5 kills", 5, 100));
    player.AddMission(Mission(2, "Complete 3 missions", 3, 150));
    player.AddMission(Mission(3, "Play 10 matches", 10, 200));

    player.DisplayHomeScreen();

    // Simulate gameplay events
    std::cout << "Simulating kills...\n";
    for (int i = 0; i < 7; ++i) {
        player.RegisterKill();
        player.UpdateMissionProgress(1, 1); // progress mission 1 by 1 kill
    }

    std::cout << "\nSimulating mission completions...\n";
    player.UpdateMissionProgress(2, 1);
    player.UpdateMissionProgress(2, 1);
    player.UpdateMissionProgress(2, 1); // completes mission 2

    player.DisplayHomeScreen();

    std::cout << "Kill streak broken.\n";
    player.ResetKillStreak();

    player.DisplayHomeScreen();

    return 0;
}
