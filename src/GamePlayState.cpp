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
    playerShape.setSize(sf::Vector2f(40, 40));
    playerShape.setFillColor(sf::Color::Blue);
    playerShape.setOutlineColor(sf::Color::White);
    playerShape.setOutlineThickness(2);

    // Calculate layout dimensions
    const float leftColumnWidth = 300.0f;
    const float statsHeight = 200.0f;
    const float padding = 10.0f;

    // Load class icon
    loadClassIcon(selectedCharacter);

    // Initialize health bar (leaving space for icon)
    healthBarBackground.setSize(sf::Vector2f(leftColumnWidth - 2 * padding - 48.0f, 20));
    healthBarBackground.setFillColor(sf::Color(100, 100, 100));
    healthBarBackground.setPosition(padding + 48.0f, padding); // Position after icon

    healthBar.setSize(sf::Vector2f(leftColumnWidth - 2 * padding - 48.0f, 20));
    healthBar.setFillColor(sf::Color::Green);
    healthBar.setPosition(padding + 48.0f, padding); // Position after icon

    // Initialize combat log
    combatLogBackground.setSize(sf::Vector2f(leftColumnWidth - 2 * padding, 600 - statsHeight - 3 * padding));
    combatLogBackground.setFillColor(sf::Color(0, 0, 0, 200));
    combatLogBackground.setPosition(padding, statsHeight + 2 * padding);

    // Set up the combat logger callback
    CombatLogger::setCallback([this](const std::string& message) {
        addCombatLogMessage(message);
    });

    // Initialize stats
    initializeStats();

    // Try to load font
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
    }
    
    // Initialize text elements
    statsText.setFont(font);
    statsText.setCharacterSize(16);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition(padding, 40);

    enemyText.setFont(font);
    enemyText.setCharacterSize(14);
    enemyText.setFillColor(sf::Color::White);
    enemyText.setPosition(padding, 120);

    updateStatsText();
    updateEnemyDisplays();
}

void GamePlayState::loadClassIcon(int selectedCharacter) {
    const std::string iconPath = selectedCharacter == 0 ? "assets/icons/sword.png" :
                                selectedCharacter == 1 ? "assets/icons/hat.png" :
                                                       "assets/icons/bow.png";
    
    if (!classIcon.loadFromFile(iconPath)) {
        Logger::error("Failed to load icon: " + iconPath);
        return;
    }
    
    iconSprite.setTexture(classIcon);
    
    // Scale icon to 32x32 pixels for the health bar area
    float scale = 32.0f / std::max(classIcon.getSize().x, classIcon.getSize().y);
    iconSprite.setScale(scale, scale);
    
    // Position icon to the left of the health bar
    iconSprite.setPosition(10, 4); // Align vertically with health bar
}

void GamePlayState::addCombatLogMessage(const std::string& message) {
    combatLog.push_front(message);
    if (combatLog.size() > MAX_LOG_LINES) {
        combatLog.pop_back();
    }

    // Update combat log texts
    combatLogTexts.clear();
    float yPos = 220; // Start below the stats section
    
    if (font.getInfo().family != "") {
        for (const auto& msg : combatLog) {
            sf::Text text;
            text.setFont(font);
            text.setString(msg);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::White);
            text.setPosition(20, yPos);
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
    const int cellSize = 40;
    const int gridSize = 10;
    const float leftColumnWidth = 300.0f;
    const float offsetX = leftColumnWidth + ((window.getSize().x - leftColumnWidth) - gridSize * cellSize) / 2;
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

    // Draw left column elements
    window.draw(healthBarBackground);
    window.draw(healthBar);
    window.draw(iconSprite);  // Draw the class icon
    window.draw(statsText);
    window.draw(enemyText);
    window.draw(combatLogBackground);
    
    for (const auto& text : combatLogTexts) {
        window.draw(text);
    }

    // Draw right column (game map)
    drawGrid(window);
} 