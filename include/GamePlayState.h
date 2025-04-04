#pragma once

#include "GameState.h"
#include "GameLogic.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <functional>
#include <sstream>


// Forward declarations of the original game classes
class Character;
class Map;

class GamePlayState : public GameState {
public:
    GamePlayState(int selectedCharacter, const std::string& playerName);
    ~GamePlayState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    enum class CombatState {
        NOT_IN_COMBAT,
        PLAYER_TURN,
        ENEMY_TURN,
        TRYING_ESCAPE
    };

    // Member variables (ordered to match initialization)
    std::unique_ptr<Character> player;
    std::unique_ptr<Map> gameMap;
    Character* currentEnemy;
    CombatState combatState;
    int selectedCharacter;
    std::string playerName;
    bool gameOver;
    int currentDiceValue;
    float diceAnimationTime;
    bool isRollingDice;
    bool showingItemPrompt;
    std::shared_ptr<Item> currentItem;
    bool bossRevealed;
    bool itemsRevealed;
    bool monstersRevealed;

    // Graphics-related members
    sf::Font font;
    sf::Text characterNameText;
    sf::Text statsText;
    sf::Text enemyText;
    sf::RectangleShape playerShape;
    sf::RectangleShape healthBar;
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape characterInfoBox;
    sf::RectangleShape combatLogBox;
    sf::Texture classIcon;
    sf::Sprite iconSprite;
    
    // Dice-related members
    sf::RectangleShape diceShape;
    sf::Text diceText;
    
    // Inventory-related members
    sf::RectangleShape inventoryBox;
    std::vector<sf::Text> inventoryTexts;
    std::vector<sf::RectangleShape> inventorySlots;
    
    // Combat log elements
    sf::RectangleShape combatLogBackground;
    std::deque<std::string> combatLog;
    std::vector<sf::Text> combatLogTexts;
    std::vector<sf::RectangleShape> combatLogBackgrounds;
    static const size_t MAX_LOG_LINES = 25;

    // Methods
    void initializeStats();
    void updateStatsText();
    void updateEnemyDisplays();
    void drawGrid(sf::RenderWindow& window);
    void handleCombat(Character* enemy);
    void handleEnemyTurn(Character* enemy);
    void handleVictory(Character& enemy);
    void handlePlayerAttack();
    void handlePlayerEscape();
    void addCombatLogMessage(const std::string& message);
    void loadClassIcon(int selectedCharacter);
    
    // Inventory-related methods
    void initializeInventoryUI();
    void updateInventoryDisplay();
    void handleItemPickup();
    void handleItemUse(int index);
    void checkForItems();
    void displayInventoryInLog();
    
    // Dice related methods
    void startDiceRoll();
    void updateDiceRoll(float deltaTime);
    void updateDiceText();
    void handleDiceResult(int roll, bool isAttack);
    void performPlayerAttack(bool isCritical);
    int rollD20() { return (rand() % 20) + 1; }
}; 