#include <iostream>
#include <string>

// Simulated persistent storage (e.g., file, database)
class PersistentStorage {
public:
    static bool LoadTutorialCompleted() {
        // In real game, load from file or player prefs
        return tutorialCompleted;
    }

    static void SaveTutorialCompleted(bool completed) {
        tutorialCompleted = completed;
        std::cout << "[Storage] Tutorial completion saved: " << (completed ? "true" : "false") << "\n";
    }

private:
    static bool tutorialCompleted;
};

bool PersistentStorage::tutorialCompleted = false;

// Game states
enum class GameState {
    Tutorial,
    Playing,
    Paused,
    Exiting
};

// Tutorial system
class Tutorial {
public:
    Tutorial() : currentStep(0), isActive(false) {}

    void Start() {
        isActive = true;
        currentStep = 0;
        std::cout << "Tutorial started. Press 'N' to go to next step, 'S' to skip tutorial.\n";
        ShowCurrentStep();
    }

    void Update(char input) {
        if (!isActive) return;

        if (input == 'N' || input == 'n') {
            NextStep();
        } else if (input == 'S' || input == 's') {
            Skip();
        } else {
            std::cout << "Invalid input. Press 'N' for next, 'S' to skip.\n";
        }
    }

    bool IsActive() const { return isActive; }

private:
    int currentStep;
    bool isActive;

    void ShowCurrentStep() {
        switch (currentStep) {
            case 0:
                std::cout << "[Tutorial] Use WASD to move your character.\n";
                break;
            case 1:
                std::cout << "[Tutorial] Use Left Mouse Button to shoot.\n";
                break;
            case 2:
                std::cout << "[Tutorial] Complete missions to earn coins.\n";
                break;
            case 3:
                std::cout << "[Tutorial] You can skip the tutorial anytime by pressing 'S'.\n";
                break;
            case 4:
                std::cout << "[Tutorial] Tutorial complete! Enjoy the game.\n";
                Complete();
                break;
            default:
                Complete();
                break;
        }
    }

    void NextStep() {
        currentStep++;
        if (currentStep > 4) {
            Complete();
        } else {
            ShowCurrentStep();
        }
    }

    void Skip() {
        std::cout << "Tutorial skipped.\n";
        Complete();
    }

    void Complete() {
        isActive = false;
        PersistentStorage::SaveTutorialCompleted(true);
    }
};

// Main game class
class Game {
public:
    Game() {
        tutorialCompleted = PersistentStorage::LoadTutorialCompleted();
        if (!tutorialCompleted) {
            state = GameState::Tutorial;
            tutorial.Start();
        } else {
            state = GameState::Playing;
            std::cout << "Welcome back! Starting game normally.\n";
        }
    }

    void Run() {
        while (state != GameState::Exiting) {
            if (state == GameState::Tutorial) {
                char input;
                std::cout << "Enter input (N=next, S=skip): ";
                std::cin >> input;
                tutorial.Update(input);
                if (!tutorial.IsActive()) {
                    state = GameState::Playing;
                    std::cout << "Tutorial finished. Starting game...\n";
                }
            } else if (state == GameState::Playing) {
                // Here you would run your main game loop
                std::cout << "[Game] Playing... Press Q to quit.\n";
                char input;
                std::cin >> input;
                if (input == 'Q' || input == 'q') {
                    state = GameState::Exiting;
                } else {
                    std::cout << "Game input received: " << input << "\n";
                    // Process other game inputs here
                }
            }
        }
        std::cout << "Exiting game. Goodbye!\n";
    }

private:
    GameState state;
    Tutorial tutorial;
    bool tutorialCompleted;
};

int main() {
    Game game;
    game.Run();
    return 0;
}
