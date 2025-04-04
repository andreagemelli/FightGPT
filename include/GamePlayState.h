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
    GamePlayState(int selectedCharacter, const std::string& playerName, const std::string& bossName);
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
    std::string bossName;
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
    
    // New declarations
    sf::Texture playerTexture; // Texture for the player character
    sf::Sprite playerSprite;   // Sprite for the player character
    
    // Add new texture and sprite members for monsters, boss, and items
    sf::Texture monsterTexture;
    sf::Texture bossTexture;
    sf::Texture itemTexture;
    sf::Sprite monsterSprite;
    sf::Sprite bossSprite;
    sf::Sprite itemSprite;
    
    // Dice-related members
    sf::Texture diceTexture;
    sf::Sprite diceSprite;
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

    // Map-related textures
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture wallTexture;
    sf::Sprite wallSprite;

    // Methods
    void initializeStats();
    void updateStatsText();
    void updateEnemyDisplays();
    void drawGrid(sf::RenderWindow& window);
    void drawBackground(sf::RenderWindow& window);
    void drawWalls(sf::RenderWindow& window);
    void handleCombat(Character* enemy);
    void handleEnemyTurn(Character* enemy);
    void handleVictory(Character& enemy);
    void handlePlayerAttack();
    void handlePlayerEscape();
    void addCombatLogMessage(const std::string& message);
    void loadClassIcon(int selectedCharacter);
    
    // Special ability methods
    void handlePlayerAbility();
    void performRageAbility();
    void performFireballAbility();
    void performHeadshotAbility();
    std::string getAbilityDescription() const;
    
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