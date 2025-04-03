#pragma once

#include "GameState.h"
#include "GameLogic.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <vector>
#include <deque>

// Forward declarations of the original game classes
class Character;
class Map;

class GamePlayState : public GameState {
public:
    GamePlayState(int selectedCharacter);
    ~GamePlayState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    void initializeStats();
    void updateStatsText();
    void updateEnemyDisplays();
    void drawGrid(sf::RenderWindow& window);
    void handleCombat(Character* enemy);
    void addCombatLogMessage(const std::string& message);
    void loadClassIcon(int selectedCharacter);

    std::unique_ptr<Character> player;
    std::unique_ptr<Map> gameMap;
    std::unique_ptr<Battle> battle;

    sf::Font font;
    sf::Text statsText;
    sf::Text enemyText;
    sf::RectangleShape playerShape;
    sf::RectangleShape healthBar;
    sf::RectangleShape healthBarBackground;
    sf::Texture classIcon;
    sf::Sprite iconSprite;
    
    // Combat log elements
    sf::RectangleShape combatLogBackground;
    std::deque<std::string> combatLog;
    std::vector<sf::Text> combatLogTexts;
    static const size_t MAX_LOG_LINES = 10;

    int selectedCharacter;
    bool gameOver;
}; 