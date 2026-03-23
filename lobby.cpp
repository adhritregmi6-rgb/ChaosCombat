#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <functional>

// ========== Data Structures ==========

struct Operator {
    std::string id;
    std::string name;
    std::string role;
    std::string faction;
    std::string perk;
};

struct Mission {
    std::string id;
    std::string title;
    std::string description;
    int goal;
    int progress;
    int reward;
    bool done;
    std::string type; // "kills", "rounds", "purchase"
};

struct Weapon {
    std::string id;
    std::string name;
    std::string type;
    int price;
    int dmg;
    int rpm;
    int control;
    bool owned;
};

enum class GameState {
    Lobby,
    Missions,
    Armory,
    Multiplayer
};

// ========== Global State ==========

struct Profile {
    int coins = 100;
    int kills = 0;
    int missionsDone = 0;
    std::string selectedOperatorId = "ghostline";
    std::string primaryWeaponId = "";
};

struct GameData {
    std::vector<Operator> operators;
    std::vector<Mission> missions;
    std::vector<Weapon> shop;
    Profile profile;
    GameState state = GameState::Lobby;
};

GameData game;

// ========== Utility Functions ==========

sf::Text createText(const std::string& str, sf::Font& font, unsigned int size, sf::Color color = sf::Color::White) {
    sf::Text text(font, str, size);
    text.setFillColor(color);
    return text;
}

std::string toUpper(const std::string& s) {
    std::string out = s;
    for (auto& c : out) c = toupper(c);
    return out;
}

// ========== UI Components ==========

class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> onClick;

    Button(sf::Font& font, const std::string& label, float x, float y, float w = 200, float h = 50)
        : text(font, label, 20) {
        shape.setSize({w, h});
        shape.setPosition({x, y});
        shape.setFillColor(sf::Color(30, 30, 60));
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(1.f);

        text.setFillColor(sf::Color::White);
        text.setPosition({x + 10, y + 10});
    }

    bool isClicked(sf::Vector2i mousePos) {
        return shape.getGlobalBounds().contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }
};

// ========== Initialization ==========

void initGameData() {
    game.operators = {
        {"ghostline", "GHOSTLINE", "Assault", "NATO", "Fast Hands"},
        {"ironveil", "IRONVEIL", "Engineer", "NATO", "Field Repair"},
        {"nightjar", "NIGHTJAR", "Recon", "PMC", "Sensor Sweep"},
        {"bulwark", "BULWARK", "Support", "PMC", "Ammo Drop"}
    };

    game.missions = {
        {"m1", "CLEAR THE STAIRWELL", "Get 6 kills while capturing objectives.", 6, 0, 70, false, "kills"},
        {"m2", "HOLD THE LINE", "Win 2 rounds in Domination.", 2, 0, 90, false, "rounds"},
        {"m3", "SUPPLY RUN", "Purchase 1 weapon in the Armory.", 1, 0, 50, false, "purchase"}
    };

    game.shop = {
        {"w1", "M4 CARBINE", "AR", 120, 62, 78, 70, false},
        {"w2", "KRAKEN-9", "SMG", 95, 52, 92, 64, false},
        {"w3", "LONGSIGHT", "DMR", 160, 82, 52, 74, false},
        {"w4", "BARRICADE", "LMG", 180, 74, 68, 55, false},
        {"w5", "SIDEWINDER", "Pistol", 55, 44, 70, 86, false}
    };
}

// ========== Render Functions ==========

void drawCenteredText(sf::RenderWindow& window, sf::Text& text, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
    text.setPosition({window.getSize().x / 2.f, y});
    window.draw(text);
}

void drawHeader(sf::RenderWindow& window, sf::Font& font) {
    sf::Text title = createText("WARROOM LOBBY PROTOTYPE", font, 28, sf::Color(255, 176, 0));
    drawCenteredText(window, title, 40);

    std::string coinsStr = "Coins: " + std::to_string(game.profile.coins);
    sf::Text coinsText = createText(coinsStr, font, 22, sf::Color::Yellow);
    coinsText.setPosition({20, 80});
    window.draw(coinsText);

    std::string killsStr = "Kills: " + std::to_string(game.profile.kills);
    sf::Text killsText = createText(killsStr, font, 22, sf::Color::Green);
    killsText.setPosition({20, 110});
    window.draw(killsText);

    std::string missionsStr = "Missions Done: " + std::to_string(game.profile.missionsDone) + "/" + std::to_string(game.missions.size());
    sf::Text missionsText = createText(missionsStr, font, 22, sf::Color::Cyan);
    missionsText.setPosition({20, 140});
    window.draw(missionsText);
}

void drawLobby(sf::RenderWindow& window, sf::Font& font) {
    sf::Text header = createText("Operator Selection", font, 24, sf::Color::White);
    header.setPosition({20, 180});
    window.draw(header);

    float y = 220.f;
    for (const auto& op : game.operators) {
        sf::Text opText = createText(op.name + " - " + op.role + " (" + op.faction + ") Perk: " + op.perk, font, 18);
        opText.setPosition({40, y});
        if (op.id == game.profile.selectedOperatorId) {
            opText.setFillColor(sf::Color::Yellow);
        }
        window.draw(opText);
        y += 30.f;
    }

    sf::Text missionHeader = createText("Missions", font, 24, sf::Color::White);
    missionHeader.setPosition({20, y + 20});
    window.draw(missionHeader);

    y += 60.f;
    for (const auto& m : game.missions) {
        std::string status = m.done ? "[DONE]" : "[" + std::to_string(m.progress) + "/" + std::to_string(m.goal) + "]";
        sf::Text missionText = createText(m.title + " " + status + " Reward: " + std::to_string(m.reward) + " coins", font, 18);
        missionText.setPosition({40, y});
        if (m.done) missionText.setFillColor(sf::Color::Green);
        window.draw(missionText);
        y += 30.f;
    }
}

void drawArmory(sf::RenderWindow& window, sf::Font& font) {
    sf::Text header = createText("Armory - Buy Weapons", font, 24, sf::Color::White);
    header.setPosition({20, 180});
    window.draw(header);

    float y = 220.f;
    for (const auto& w : game.shop) {
        std::string ownedStr = w.owned ? "[OWNED]" : "[Price: " + std::to_string(w.price) + "]";
        sf::Text weaponText = createText(w.name + " (" + w.type + ") " + ownedStr, font, 18);
        weaponText.setPosition({40, y});
        if (w.owned) weaponText.setFillColor(sf::Color::Green);
        window.draw(weaponText);
        y += 30.f;
    }
}

void drawMultiplayer(sf::RenderWindow& window, sf::Font& font) {
    sf::Text header = createText("Multiplayer Simulation", font, 24, sf::Color::White);
    header.setPosition({20, 180});
    window.draw(header);

    sf::Text info = createText("Press K to simulate a kill (+5 coins)", font, 18, sf::Color::Cyan);
    info.setPosition({40, 220});
    window.draw(info);

    sf::Text info2 = createText("Press M to simulate mission completion", font, 18, sf::Color::Cyan);
    info2.setPosition({40, 250});
    window.draw(info2);
}

// ========== Game Logic ==========

void simulateKill() {
    game.profile.kills++;
    game.profile.coins += 5;
    // Update missions of type kills
    for (auto& m : game.missions) {
        if (!m.done && m.type == "kills") {
            m.progress++;
            if (m.progress >= m.goal) {
                m.done = true;
                game.profile.coins += m.reward;
                game.profile.missionsDone++;
                std::cout << "Mission completed: " << m.title << " Reward: " << m.reward << " coins\n";
            }
        }
    }
}

void simulateMissionCompletion() {
    // Complete first incomplete mission that is not kills type
    for (auto& m : game.missions) {
        if (!m.done && m.type != "kills") {
            m.done = true;
            game.profile.coins += m.reward;
            game.profile.missionsDone++;
            std::cout << "Mission completed: " << m.title << " Reward: " << m.reward << " coins\n";
            break;
        }
    }
}

void buyWeapon(int index) {
    if (index < 0 || index >= (int)game.shop.size()) return;
    Weapon& w = game.shop[index];
    if (w.owned) {
        std::cout << "Weapon already owned.\n";
        return;
    }
    if (game.profile.coins >= w.price) {
        game.profile.coins -= w.price;
        w.owned = true;
        game.profile.primaryWeaponId = w.id;
        std::cout << "Bought weapon: " << w.name << "\n";
        // Update purchase missions
        for (auto& m : game.missions) {
            if (!m.done && m.type == "purchase") {
                m.progress++;
                if (m.progress >= m.goal) {
                    m.done = true;
                    game.profile.coins += m.reward;
                    game.profile.missionsDone++;
                    std::cout << "Mission completed: " << m.title << " Reward: " << m.reward << " coins\n";
                }
            }
        }
    } else {
        std::cout << "Not enough coins to buy " << w.name << "\n";
    }
}

// ========== Main ==========

int main() {
    sf::RenderWindow window(sf::VideoMode({900, 700}), "WARROOM Lobby Prototype");
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Failed to load font arial.ttf\n";
        return -1;
    }

    initGameData();

    // Buttons for navigation
    std::vector<Button> navButtons;
    const float navX = 20.f;
    const float navY = 20.f;
    const float navSpacing = 110.f;
    std::vector<std::string> navLabels = {"Lobby", "Missions", "Armory", "Multiplayer"};
    for (int i = 0; i < (int)navLabels.size(); ++i) {
        Button b(font, navLabels[i], navX + i * navSpacing, navY, 100, 40);
        navButtons.push_back(b);
    }

    // Armory buy buttons
    std::vector<Button> buyButtons;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // Navigation buttons
                for (int i = 0; i < (int)navButtons.size(); ++i) {
                    if (navButtons[i].isClicked(mousePos)) {
                        game.state = static_cast<GameState>(i);
                        buyButtons.clear();
                    }
                }

                // Armory buy buttons
                if (game.state == GameState::Armory) {
                    for (int i = 0; i < (int)buyButtons.size(); ++i) {
                        if (buyButtons[i].isClicked(mousePos)) {
                            buyWeapon(i);
                        }
                    }
                }
            }

            if (event->is<sf::Event::KeyPressed>()) {
                auto keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (game.state == GameState::Multiplayer) {
                    if (keyEvent->code == sf::Keyboard::Key::K) {
                        simulateKill();
                    }
                    if (keyEvent->code == sf::Keyboard::Key::M) {
                        simulateMissionCompletion();
                    }
                }
            }
        }

        window.clear(sf::Color(7, 9, 11));

        // Draw header and coins/kills/missions
        drawHeader(window, font);

        // Draw navigation buttons
        for (int i = 0; i < (int)navButtons.size(); ++i) {
            // Highlight active
            if (i == (int)game.state) {
                navButtons[i].shape.setFillColor(sf::Color(255, 176, 0));
                navButtons[i].text.setFillColor(sf::Color::Black);
            } else {
                navButtons[i].shape.setFillColor(sf::Color(30, 30, 60));
                navButtons[i].text.setFillColor(sf::Color::White);
            }
            navButtons[i].draw(window);
        }

        // Draw current view
        switch (game.state) {
            case GameState::Lobby:
                drawLobby(window, font);
                break;
            case GameState::Missions:
                drawLobby(window, font); // reuse lobby for missions for simplicity
                break;
            case GameState::Armory: {
                drawArmory(window, font);
                // Draw buy buttons
                buyButtons.clear();
                float y = 220.f;
                for (int i = 0; i < (int)game.shop.size(); ++i) {
                    const Weapon& w = game.shop[i];
                    Button buyBtn(font, w.owned ? "Owned" : "Buy", 700, y, 100, 30);
                    if (w.owned) {
                        buyBtn.shape.setFillColor(sf::Color(50, 50, 50));
                        buyBtn.text.setFillColor(sf::Color(150, 150, 150));
                    } else if (game.profile.coins >= w.price) {
                        buyBtn.shape.setFillColor(sf::Color(255, 176, 0));
                        buyBtn.text.setFillColor(sf::Color::Black);
                    } else {
                        buyBtn.shape.setFillColor(sf::Color(100, 100, 100));
                        buyBtn.text.setFillColor(sf::Color(180, 180, 180));
                    }
                    buyBtn.draw(window);
                    buyButtons.push_back(buyBtn);
                    y += 30.f;
                }
                break;
            }
            case GameState::Multiplayer:
                drawMultiplayer(window, font);
                break;
        }

        window.display();
    }

    return 0;
}
