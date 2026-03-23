#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <cctype>

/* =========================
   DATA STRUCTURES
========================= */

struct Operator {
    std::string id, name, role, faction, perk;
};

struct Mission {
    std::string id, title, description, type;
    int goal, progress, reward;
    bool done;
};

struct Weapon {
    std::string id, name, type;
    int price, dmg, rpm, control;
    bool owned;
};

enum class GameState {
    Lobby,
    Missions,
    Armory,
    Multiplayer
};

/* =========================
   GAME STATE
========================= */

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

/* =========================
   UI HELPERS
========================= */

sf::Text createText(const std::string& str, sf::Font& font, int size, sf::Color color = sf::Color::White) {
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    return text;
}

/* =========================
   BUTTON
========================= */

class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;

    Button() {}

    Button(sf::Font& font, const std::string& label, float x, float y, float w = 200, float h = 50) {
        shape.setSize({w, h});
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color(30,30,60));
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(1);

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(18);
        text.setPosition(x + 10, y + 10);
    }

    bool isClicked(sf::Vector2i mouse) {
        return shape.getGlobalBounds().contains((float)mouse.x, (float)mouse.y);
    }

    void draw(sf::RenderWindow& win) {
        win.draw(shape);
        win.draw(text);
    }
};

/* =========================
   INIT DATA
========================= */

void initGameData() {
    game.operators = {
        {"ghostline","GHOSTLINE","Assault","NATO","Fast Hands"},
        {"ironveil","IRONVEIL","Engineer","NATO","Field Repair"}
    };

    game.missions = {
        {"m1","GET KILLS","Kill enemies",5,0,50,false,"kills"},
        {"m2","BUY GUN","Buy weapon",1,0,60,false,"purchase"}
    };

    game.shop = {
        {"w1","RIFLE","AR",120,60,80,70,false},
        {"w2","SMG","SMG",90,50,90,60,false}
    };
}

/* =========================
   GAME LOGIC
========================= */

void simulateKill() {
    game.profile.kills++;
    game.profile.coins += 5;

    for(auto& m : game.missions) {
        if(!m.done && m.type=="kills") {
            m.progress++;
            if(m.progress >= m.goal) {
                m.done = true;
                game.profile.coins += m.reward;
                game.profile.missionsDone++;
            }
        }
    }
}

void buyWeapon(int i) {
    if(i < 0 || i >= game.shop.size()) return;

    Weapon& w = game.shop[i];

    if(w.owned) return;

    if(game.profile.coins >= w.price) {
        game.profile.coins -= w.price;
        w.owned = true;

        for(auto& m : game.missions) {
            if(!m.done && m.type=="purchase") {
                m.progress++;
                if(m.progress >= m.goal) {
                    m.done = true;
                    game.profile.coins += m.reward;
                    game.profile.missionsDone++;
                }
            }
        }
    }
}

/* =========================
   MAIN
========================= */

int main() {
    sf::RenderWindow window(sf::VideoMode(900,700), "Game");
    sf::Font font;

    if(!font.loadFromFile("arial.ttf")) {
        std::cout<<"Font missing\n";
        return -1;
    }

    initGameData();

    std::vector<std::string> navLabels = {"Lobby","Missions","Armory","Multiplayer"};
    std::vector<Button> navButtons;

    for(int i=0;i<navLabels.size();i++) {
        navButtons.emplace_back(font, navLabels[i], 20 + i*110, 20, 100, 40);
    }

    std::vector<Button> buyButtons;

    while(window.isOpen()) {
        sf::Event e;

        while(window.pollEvent(e)) {
            if(e.type == sf::Event::Closed)
                window.close();

            if(e.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);

                for(int i=0;i<navButtons.size();i++) {
                    if(navButtons[i].isClicked(mouse))
                        game.state = (GameState)i;
                }

                if(game.state == GameState::Armory) {
                    for(int i=0;i<buyButtons.size();i++) {
                        if(buyButtons[i].isClicked(mouse))
                            buyWeapon(i);
                    }
                }
            }

            if(e.type == sf::Event::KeyPressed) {
                if(game.state == GameState::Multiplayer) {
                    if(e.key.code == sf::Keyboard::K) simulateKill();
                }
            }
        }

        window.clear(sf::Color(10,10,15));

        // Draw nav
        for(int i=0;i<navButtons.size();i++) {
            if((int)game.state == i)
                navButtons[i].shape.setFillColor(sf::Color::Yellow);
            else
                navButtons[i].shape.setFillColor(sf::Color(30,30,60));

            navButtons[i].draw(window);
        }

        // Draw coins
        auto coins = createText("Coins: "+std::to_string(game.profile.coins), font, 20);
        coins.setPosition(20,80);
        window.draw(coins);

        // Draw states
        if(game.state == GameState::Armory) {
            float y = 150;
            buyButtons.clear();

            for(int i=0;i<game.shop.size();i++) {
                auto& w = game.shop[i];

                auto txt = createText(w.name+" $"+std::to_string(w.price), font, 18);
                txt.setPosition(40,y);
                window.draw(txt);

                Button b(font, w.owned ? "Owned" : "Buy", 700, y, 100, 30);
                buyButtons.push_back(b);
                b.draw(window);

                y += 40;
            }
        }

        if(game.state == GameState::Lobby) {
            auto t = createText("Lobby Screen", font, 30);
            t.setPosition(300,300);
            window.draw(t);
        }

        if(game.state == GameState::Multiplayer) {
            auto t = createText("Press K = Kill (+coins)", font, 20);
            t.setPosition(250,300);
            window.draw(t);
        }

        window.display();
    }

    return 0;
}