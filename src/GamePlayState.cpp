#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

class CombatLogger {
public:
    static void setCallback(std::function<void(const std::string&)> cb) {
        callback = cb;
    }
    
    static void log(const std::string& message) {
        Logger::info(message);
        if (callback) {
            callback(message);
        }
    }

private:
    static std::function<void(const std::string&)> callback;
};

std::function<void(const std::string&)> CombatLogger::callback;

GamePlayState::GamePlayState(int selectedCharacter)
    : selectedCharacter(selectedCharacter), gameOver(false) {
    
    // Initialize game map (10x10 grid)
    gameMap = std::make_unique<Map>(10, 10);

    // Initialize player shape
    playerShape.setSize(sf::Vector2f(50, 50));
    playerShape.setFillColor(sf::Color::Blue);
    playerShape.setOutlineColor(sf::Color::White);
    playerShape.setOutlineThickness(2);

    // Initialize health bar
    healthBarBackground.setSize(sf::Vector2f(200, 20));
    healthBarBackground.setFillColor(sf::Color(100, 100, 100));
    healthBarBackground.setPosition(10, 10);

    healthBar.setSize(sf::Vector2f(200, 20));
    healthBar.setFillColor(sf::Color::Green);
    healthBar.setPosition(10, 10);

    // Initialize combat log
    combatLogBackground.setSize(sf::Vector2f(300, 200));
    combatLogBackground.setFillColor(sf::Color(0, 0, 0, 200));
    combatLogBackground.setPosition(480, 10);

    // Set up the combat logger callback
    CombatLogger::setCallback([this](const std::string& message) {
        addCombatLogMessage(message);
    });

    // Initialize stats
    initializeStats();

    // Try to load font
    if (!font.loadFromFile("assets/fonts/Jacquard12-Regular.ttf")) {
        Logger::error("Failed to load font!");
        // Set up text elements with default font
        statsText.setCharacterSize(16);
        statsText.setFillColor(sf::Color::White);
        statsText.setPosition(10, 40);

        enemyText.setCharacterSize(14);
        enemyText.setFillColor(sf::Color::White);
        enemyText.setPosition(10, 120);
    } else {
        // Initialize text elements with loaded font
        statsText.setFont(font);
        statsText.setCharacterSize(16);
        statsText.setFillColor(sf::Color::White);
        statsText.setPosition(10, 40);

        enemyText.setFont(font);
        enemyText.setCharacterSize(14);
        enemyText.setFillColor(sf::Color::White);
        enemyText.setPosition(10, 120);
    }

    updateStatsText();
    updateEnemyDisplays();
}

void GamePlayState::addCombatLogMessage(const std::string& message) {
    combatLog.push_front(message);
    if (combatLog.size() > MAX_LOG_LINES) {
        combatLog.pop_back();
    }

    // Update combat log texts
    combatLogTexts.clear();
    float yPos = 20;
    
    // Only create text objects if font is loaded
    if (font.getInfo().family != "") {
        for (const auto& msg : combatLog) {
            sf::Text text;
            text.setFont(font);
            text.setString(msg);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::White);
            text.setPosition(490, yPos);
            combatLogTexts.push_back(text);
            yPos += 20;
        }
    }
}

void GamePlayState::initializeStats() {
    std::string name;
    int health, attack, defense, speed, avoidance;

    switch (selectedCharacter) {
        case 0: // Knight
            name = "Knight";
            health = 150;
            attack = 30;
            defense = 20;
            speed = 15;
            avoidance = 10;
            break;
        case 1: // Mage
            name = "Mage";
            health = 100;
            attack = 40;
            defense = 10;
            speed = 20;
            avoidance = 15;
            break;
        case 2: // Archer
            name = "Archer";
            health = 120;
            attack = 35;
            defense = 15;
            speed = 25;
            avoidance = 20;
            break;
        default:
            Logger::error("Invalid character selection!");
            return;
    }

    player = std::make_unique<Character>(name, health, attack, defense, speed, avoidance);
    gameMap->PlaceCharacter(*player);
    
    std::stringstream ss;
    ss << "Created " << name << " with " << health << " HP";
    CombatLogger::log(ss.str());
}

void GamePlayState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type != sf::Event::KeyPressed || gameOver) return;

    int dx = 0, dy = 0;

    switch (event.key.code) {
        case sf::Keyboard::Left:  dx = -1; break;
        case sf::Keyboard::Right: dx = 1;  break;
        case sf::Keyboard::Up:    dy = -1; break;
        case sf::Keyboard::Down:  dy = 1;  break;
        default: return;
    }

    Character* enemy = gameMap->CheckNewPosition(*player, dx, dy);
    if (enemy) {
        handleCombat(enemy);
    } else {
        gameMap->MoveCharacter(*player, dx, dy);
        std::stringstream ss;
        ss << "Moved to position (" << player->GetX() << ", " << player->GetY() << ")";
        CombatLogger::log(ss.str());
    }

    updateStatsText();
    updateEnemyDisplays();
}

void GamePlayState::handleCombat(Character* enemy) {
    if (!enemy) return;

    std::stringstream ss;
    ss << "Combat started with " << enemy->GetName();
    CombatLogger::log(ss.str());

    bool playerWon = Battle::Fight(*player, *enemy);

    if (playerWon) {
        ss.str("");
        ss << "Victory! Defeated " << enemy->GetName();
        CombatLogger::log(ss.str());
        
        Battle::Reward(*player, *enemy);
        gameMap->RemoveEnemy(*enemy, 0, 0);
        
        if (enemy->GetBoss()) {
            CombatLogger::log("Congratulations! You've defeated the boss!");
            gameOver = true;
        }
    } else {
        ss.str("");
        ss << "Defeat! " << enemy->GetName() << " was too strong!";
        CombatLogger::log(ss.str());
        gameOver = true;
    }
}

void GamePlayState::update(float deltaTime) {
    // Update health bar size based on current health
    float healthPercent = static_cast<float>(player->GetHealth()) / player->GetMaxHealth();
    healthBar.setSize(sf::Vector2f(200 * healthPercent, 20));
}

void GamePlayState::updateStatsText() {
    std::stringstream ss;
    ss << "Level: " << player->GetLevel() << "\n"
       << "HP: " << player->GetHealth() << "/" << player->GetMaxHealth() << "\n"
       << "Attack: " << player->GetAttack() << "\n"
       << "Defense: " << player->GetDefense() << "\n"
       << "Speed: " << player->GetSpeed() << "\n"
       << "Experience: " << player->GetExperience();
    statsText.setString(ss.str());
}

void GamePlayState::updateEnemyDisplays() {
    std::stringstream ss;
    ss << "Position: (" << player->GetX() << ", " << player->GetY() << ")\n";
    if (gameOver) {
        ss << (player->GetHealth() > 0 ? "Victory!" : "Game Over!");
    }
    enemyText.setString(ss.str());
}

void GamePlayState::drawGrid(sf::RenderWindow& window) {
    const int cellSize = 50;
    const int gridSize = 10;
    const float offsetX = (window.getSize().x - gridSize * cellSize) / 2;
    const float offsetY = (window.getSize().y - gridSize * cellSize) / 2;

    // Draw grid lines
    for (int i = 0; i <= gridSize; ++i) {
        sf::RectangleShape line(sf::Vector2f(gridSize * cellSize, 2));
        line.setPosition(offsetX, offsetY + i * cellSize);
        line.setFillColor(sf::Color(100, 100, 100));
        window.draw(line);

        line.setSize(sf::Vector2f(2, gridSize * cellSize));
        line.setPosition(offsetX + i * cellSize, offsetY);
        window.draw(line);
    }

    // Draw player
    playerShape.setPosition(offsetX + player->GetX() * cellSize, 
                          offsetY + player->GetY() * cellSize);
    window.draw(playerShape);
}

void GamePlayState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(40, 40, 40));

    drawGrid(window);
    window.draw(healthBarBackground);
    window.draw(healthBar);
    
    // Only draw text if font is loaded
    if (font.getInfo().family != "") {
        window.draw(statsText);
        window.draw(enemyText);
        
        // Draw combat log
        window.draw(combatLogBackground);
        for (const auto& text : combatLogTexts) {
            window.draw(text);
        }
    }
} 