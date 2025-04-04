#include "GamePlayState.h"
#include "CharacterSelectionState.h"
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

GamePlayState::GamePlayState(int selectedCharacter, const std::string& playerName)
    : player(nullptr),
      gameMap(std::make_unique<Map>(15, 15)),
      currentEnemy(nullptr),
      combatState(CombatState::NOT_IN_COMBAT),
      selectedCharacter(selectedCharacter),
      playerName(playerName),
      gameOver(false),
      currentDiceValue(1),
      diceAnimationTime(0),
      isRollingDice(false) {
    
    // Initialize player shape
    playerShape.setSize(sf::Vector2f(40, 40));
    playerShape.setFillColor(sf::Color::Blue);
    playerShape.setOutlineColor(sf::Color::White);
    playerShape.setOutlineThickness(2);

    // Calculate layout dimensions for larger window
    const float leftColumnWidth = 400.0f;
    const float statsHeight = 180.0f;
    const float padding = 10.0f;

    // Initialize character info box
    characterInfoBox.setSize(sf::Vector2f(leftColumnWidth - 2 * padding, statsHeight));
    characterInfoBox.setPosition(padding, padding);
    characterInfoBox.setFillColor(sf::Color(60, 60, 60));
    characterInfoBox.setOutlineColor(sf::Color(200, 200, 200));
    characterInfoBox.setOutlineThickness(2);

    // Initialize combat log box
    combatLogBox.setSize(sf::Vector2f(leftColumnWidth - 2 * padding, 800 - statsHeight - 3 * padding));
    combatLogBox.setPosition(padding, statsHeight + 2 * padding);
    combatLogBox.setFillColor(sf::Color(60, 60, 60));
    combatLogBox.setOutlineColor(sf::Color(200, 200, 200));
    combatLogBox.setOutlineThickness(2);

    // Load class icon
    loadClassIcon(selectedCharacter);

    // Initialize character name text
    characterNameText.setFont(font);
    characterNameText.setCharacterSize(16);
    characterNameText.setFillColor(sf::Color::White);
    characterNameText.setPosition(10, 4);

    // Initialize health bar (leaving space for icon)
    healthBarBackground.setSize(sf::Vector2f(leftColumnWidth - 2 * padding - 48.0f, 20));
    healthBarBackground.setFillColor(sf::Color(100, 100, 100));
    healthBarBackground.setPosition(padding + 48.0f, 40);

    healthBar.setSize(sf::Vector2f(leftColumnWidth - 2 * padding - 48.0f, 20));
    healthBar.setFillColor(sf::Color::Green);
    healthBar.setPosition(padding + 48.0f, 40);

    // Initialize combat log with larger size
    combatLogBackground.setSize(sf::Vector2f(leftColumnWidth - 4 * padding, 800 - statsHeight - 5 * padding));
    combatLogBackground.setFillColor(sf::Color(0, 0, 0, 200));
    combatLogBackground.setPosition(padding * 2, statsHeight + 3 * padding);

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
    statsText.setPosition(padding * 2, 70);

    enemyText.setFont(font);
    enemyText.setCharacterSize(14);
    enemyText.setFillColor(sf::Color::White);
    enemyText.setPosition(padding * 2, 150);

    updateStatsText();
    updateEnemyDisplays();

    // Initialize dice
    diceShape.setSize(sf::Vector2f(60, 60));
    diceShape.setFillColor(sf::Color(200, 200, 200));
    diceShape.setOutlineColor(sf::Color::White);
    diceShape.setOutlineThickness(2);
    diceShape.setPosition(320, 120);  // Position near the combat options

    diceText.setFont(font);
    diceText.setCharacterSize(24);
    diceText.setFillColor(sf::Color::Black);
    updateDiceText();
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
    iconSprite.setPosition(10, 40); // Align with health bar
}

void GamePlayState::addCombatLogMessage(const std::string& message) {
    // Split message into lines if it contains newlines
    std::string currentLine;
    std::istringstream messageStream(message);
    while (std::getline(messageStream, currentLine)) {
        if (!currentLine.empty()) {  // Skip empty lines
            combatLog.push_front(currentLine);
        }
    }

    // Limit the number of lines
    while (combatLog.size() > MAX_LOG_LINES) {
        combatLog.pop_back();
    }

    // Update combat log texts
    combatLogTexts.clear();
    float yPos = 200; // Start below the stats section
    
    if (font.getInfo().family != "") {
        for (const auto& msg : combatLog) {
            sf::Text text;
            text.setFont(font);
            text.setString(msg);
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::White);
            text.setPosition(20, yPos);
            
            // Add a subtle background for combat messages
            if (msg.find("BATTLE") != std::string::npos || 
                msg.find("attacks") != std::string::npos ||
                msg.find("HP:") != std::string::npos) {
                sf::RectangleShape background;
                background.setSize(sf::Vector2f(360, 20));
                background.setPosition(15, yPos);
                background.setFillColor(sf::Color(60, 60, 60, 150));
                combatLogBackgrounds.push_back(background);
            }
            
            combatLogTexts.push_back(text);
            yPos += 24;  // Increased spacing between lines
        }
    }
}

void GamePlayState::initializeStats() {
    std::string className;
    int health, attack, defense, speed, avoidance;

    switch (selectedCharacter) {
        case 0: // Knight
            className = "Knight";
            health = 120;    // Reduced from 150
            attack = 25;     // Reduced from 30
            defense = 25;    // Increased from 20
            speed = 12;      // Reduced from 15
            avoidance = 8;   // Reduced from 10
            break;
        case 1: // Mage
            className = "Mage";
            health = 80;     // Reduced from 100
            attack = 35;     // Reduced from 40
            defense = 8;     // Reduced from 10
            speed = 15;      // Reduced from 20
            avoidance = 12;  // Reduced from 15
            break;
        case 2: // Archer
            className = "Archer";
            health = 90;     // Reduced from 120
            attack = 28;     // Reduced from 35
            defense = 12;    // Reduced from 15
            speed = 20;      // Reduced from 25
            avoidance = 18;  // Reduced from 20
            break;
        default:
            Logger::error("Invalid character selection!");
            return;
    }

    player = std::make_unique<Character>(playerName + " the " + className, health, attack, defense, speed, avoidance);
    gameMap->PlaceCharacter(*player);
    
    std::stringstream ss;
    ss << "Created " << player->GetName() << " with " << health << " HP";
    CombatLogger::log(ss.str());
}

void GamePlayState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (gameOver) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
            nextState = std::make_unique<CharacterSelectionState>(playerName);
            return;
        }
        return;
    }

    if (combatState != CombatState::NOT_IN_COMBAT) {
        if (event.type == sf::Event::KeyPressed) {
            if (combatState == CombatState::PLAYER_TURN) {
                if (event.key.code == sf::Keyboard::A) {
                    handlePlayerAttack();
                }
                else if (event.key.code == sf::Keyboard::E) {
                    handlePlayerEscape();
                }
            }
        }
        return;
    }

    if (event.type != sf::Event::KeyPressed) return;

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
    } else if (dx != 0 || dy != 0) {  // Only log movement if actually moving
        int newX = player->GetX() + dx;
        int newY = player->GetY() + dy;
        
        // Check map boundaries
        if (newX >= 0 && newX < 15 && newY >= 0 && newY < 15) {
            gameMap->MoveCharacter(*player, dx, dy);
            std::stringstream ss;
            ss << "\nMoved to position (" << player->GetX() << ", " << player->GetY() << ")";
            CombatLogger::log(ss.str());
        }
    }

    updateStatsText();
    updateEnemyDisplays();
}

void GamePlayState::handleCombat(Character* enemy) {
    if (!enemy) return;

    if (combatState == CombatState::NOT_IN_COMBAT) {
        currentEnemy = enemy;
        combatState = CombatState::PLAYER_TURN;
        std::stringstream ss;
        ss << "\n=== BATTLE START ===\n";
        ss << player->GetName() << " (HP: " << player->GetHealth() << ") VS " 
           << enemy->GetName() << " (HP: " << enemy->GetHealth() << ")";
        CombatLogger::log(ss.str());
        CombatLogger::log("\nYour turn! Choose your action:");
        CombatLogger::log("A. Attack");
        CombatLogger::log("E. Try to escape");
        return;
    }

    if (combatState == CombatState::PLAYER_TURN) {
        // Handle player's action (1 for attack, 2 for escape)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            handlePlayerAttack();
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            // Try to escape
            if (rand() % 100 < player->GetSpeed()) {
                CombatLogger::log("Successfully escaped!");
                combatState = CombatState::NOT_IN_COMBAT;
                currentEnemy = nullptr;
            } else {
                CombatLogger::log("Failed to escape!");
                combatState = CombatState::ENEMY_TURN;
                // Add small delay before enemy turn
                sf::sleep(sf::milliseconds(500));
                handleEnemyTurn(enemy);
            }
        }
    }
}

void GamePlayState::handleEnemyTurn(Character* enemy) {
    if (!enemy) return;
    
    // Apply bleed damage if wounded
    if (player->IsWounded()) {
        CombatLogger::log("\nYou are bleeding! (-5 HP)");
        player->ApplyBleedDamage();
        updateStatsText();
    }
    
    CombatLogger::log("\nEnemy's turn!");
    int damage = enemy->GetAttack();
    int actualDamage = player->TakeDamage(damage);
    
    std::stringstream ss;
    if (actualDamage > 0) {
        ss << enemy->GetName() << " attacks " << player->GetName() << " for " << actualDamage << " damage!";
        ss << "\nYour HP: " << player->GetHealth() << "/" << player->GetMaxHealth();
    } else {
        ss << player->GetName() << " dodged the attack!";
    }
    CombatLogger::log(ss.str());
    
    if (player->IsDefeated()) {
        CombatLogger::log("\n=== GAME OVER ===");
        gameOver = true;
        return;
    }
    
    combatState = CombatState::PLAYER_TURN;
    CombatLogger::log("\nYour turn! Choose your action:");
    CombatLogger::log("A. Attack");
    CombatLogger::log("E. Try to escape");
}

void GamePlayState::handleVictory(Character& enemy) {
    std::stringstream ss;
    ss << "\n=== BATTLE WON! ===";
    CombatLogger::log(ss.str());
    
    Battle::Reward(*player, enemy);
    gameMap->RemoveEnemy(enemy, 0, 0);
    
    if (enemy.GetBoss()) {
        CombatLogger::log("Congratulations! You've defeated the boss!");
        gameOver = true;
    }
    
    combatState = CombatState::NOT_IN_COMBAT;
    currentEnemy = nullptr;
}

void GamePlayState::update(float deltaTime) {
    // Update health bar size based on current health
    float healthPercent = static_cast<float>(player->GetHealth()) / player->GetMaxHealth();
    float healthBarWidth = healthBarBackground.getSize().x;
    healthBar.setSize(sf::Vector2f(healthBarWidth * healthPercent, healthBar.getSize().y));
    
    // Update dice animation
    updateDiceRoll(deltaTime);
}

void GamePlayState::updateStatsText() {
    // Update character name
    characterNameText.setString(player->GetName() + (player->IsWounded() ? " 🩸" : ""));
    sf::FloatRect nameBounds = characterNameText.getLocalBounds();
    characterNameText.setPosition(10, 10);

    // Calculate experience needed for next level
    int currentExp = player->GetExperience();
    int nextLevelExp = player->GetLevel() * 10;

    // Update stats with current values
    std::stringstream ss;
    ss << "Level: " << player->GetLevel() << "\n"
       << "HP: " << player->GetHealth() << "/" << player->GetMaxHealth() 
       << (player->IsWounded() ? " 🩸" : "") << "\n"
       << "Attack: " << player->GetAttack() << "\n"
       << "Defense: " << player->GetDefense() << "\n"
       << "Speed: " << player->GetSpeed() << "\n"
       << "Avoidance: " << player->GetAvoidance() << "%\n"
       << "Experience: " << currentExp << "/" << nextLevelExp;
    statsText.setString(ss.str());
}

void GamePlayState::updateEnemyDisplays() {
    if (gameOver) {
        std::stringstream ss;
        ss << (player->GetHealth() > 0 ? "Victory!" : "Game Over!");
        enemyText.setString(ss.str());
    } else {
        enemyText.setString("");  // Clear the enemy text since position is now in log
    }
}

void GamePlayState::handlePlayerAttack() {
    if (!currentEnemy) return;
    startDiceRoll();
}

void GamePlayState::handlePlayerEscape() {
    if (!currentEnemy) return;
    combatState = CombatState::TRYING_ESCAPE;
    startDiceRoll();
}

void GamePlayState::updateDiceText() {
    diceText.setString(std::to_string(currentDiceValue));
    // Center the text in the dice
    sf::FloatRect textBounds = diceText.getLocalBounds();
    diceText.setPosition(
        diceShape.getPosition().x + (diceShape.getSize().x - textBounds.width) / 2,
        diceShape.getPosition().y + (diceShape.getSize().y - textBounds.height) / 2
    );
}

void GamePlayState::startDiceRoll() {
    isRollingDice = true;
    diceAnimationTime = 0;
}

void GamePlayState::updateDiceRoll(float deltaTime) {
    if (!isRollingDice) return;

    diceAnimationTime += deltaTime;
    
    // Update dice value rapidly during animation
    if (diceAnimationTime < 1.0f) {  // Roll for 1 second
        currentDiceValue = (rand() % 20) + 1;
        updateDiceText();
    } else {
        isRollingDice = false;
        // Final roll
        currentDiceValue = rollD20();
        updateDiceText();
        
        // Handle the result based on current action
        if (combatState == CombatState::PLAYER_TURN) {
            handleDiceResult(currentDiceValue, true);  // true for attack
        } else if (combatState == CombatState::TRYING_ESCAPE) {
            handleDiceResult(currentDiceValue, false);  // false for escape
        }
    }
}

void GamePlayState::handleDiceResult(int roll, bool isAttack) {
    std::stringstream ss;
    ss << "\nDice roll: " << roll;
    
    if (isAttack) {
        if (roll == 20) {
            ss << " - CRITICAL HIT!";
            CombatLogger::log(ss.str());
            performPlayerAttack(true);  // true for critical hit
        } else if (roll == 1) {
            ss << " - CRITICAL MISS! You are wounded!";
            CombatLogger::log(ss.str());
            player->SetWounded(true);
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
        } else if (roll >= 10) {
            ss << " - Hit!";
            CombatLogger::log(ss.str());
            performPlayerAttack(false);  // false for normal hit
        } else {
            ss << " - Miss!";
            CombatLogger::log(ss.str());
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
        }
    } else {
        // Need 10 or higher to escape
        if (roll >= 10) {
            ss << " - Escape successful!";
            CombatLogger::log(ss.str());
            combatState = CombatState::NOT_IN_COMBAT;
            currentEnemy = nullptr;
            player->SetWounded(false);  // Clear wounded condition after escaping
        } else {
            ss << " - Escape failed!";
            CombatLogger::log(ss.str());
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
        }
    }
}

void GamePlayState::performPlayerAttack(bool isCritical) {
    if (!currentEnemy) return;
    
    int damage = player->GetAttack();
    if (isCritical) {
        damage *= 2;  // Double damage on critical hit
    }
    
    int actualDamage = currentEnemy->TakeDamage(damage);
    
    std::stringstream ss;
    if (actualDamage > 0) {
        ss << player->GetName() << " attacks " << currentEnemy->GetName() 
           << " for " << actualDamage << " damage";
        if (isCritical) {
            ss << " (CRITICAL HIT!)";
        }
        ss << "!\nEnemy HP: " << currentEnemy->GetHealth() << "/" << currentEnemy->GetMaxHealth();
    } else {
        ss << currentEnemy->GetName() << " dodged the attack!";
    }
    CombatLogger::log(ss.str());
    
    if (currentEnemy->IsDefeated()) {
        handleVictory(*currentEnemy);
        player->SetWounded(false);  // Clear wounded condition after victory
        return;
    }
    
    combatState = CombatState::ENEMY_TURN;
    sf::sleep(sf::milliseconds(500));
    handleEnemyTurn(currentEnemy);
}

void GamePlayState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(40, 40, 40));

    // Draw character info box with border
    window.draw(characterInfoBox);
    
    // Draw character name
    window.draw(characterNameText);
    
    // Draw health bar and icon
    window.draw(healthBarBackground);
    window.draw(healthBar);
    window.draw(iconSprite);
    window.draw(statsText);
    window.draw(enemyText);

    // Draw combat log box with border
    window.draw(combatLogBox);
    window.draw(combatLogBackground);
    
    // Draw combat log backgrounds first
    for (const auto& background : combatLogBackgrounds) {
        window.draw(background);
    }
    
    // Then draw the text
    for (const auto& text : combatLogTexts) {
        window.draw(text);
    }

    // Draw dice
    window.draw(diceShape);
    window.draw(diceText);

    // Draw right column (game map)
    drawGrid(window);

    // Draw game over/victory screen if needed
    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(overlay);

        sf::Text endText;
        endText.setFont(font);
        endText.setCharacterSize(48);
        
        if (player->GetHealth() > 0) {
            endText.setString("Victory!\nPress Enter to return to main menu");
            endText.setFillColor(sf::Color::Green);
        } else {
            endText.setString("Game Over!\nPress Enter to return to main menu");
            endText.setFillColor(sf::Color::Red);
        }
        
        sf::FloatRect textBounds = endText.getLocalBounds();
        endText.setPosition(
            (window.getSize().x - textBounds.width) / 2,
            (window.getSize().y - textBounds.height) / 2
        );
        
        window.draw(endText);
    }
}

void GamePlayState::drawGrid(sf::RenderWindow& window) {
    const int cellSize = 40;
    const int gridSize = 15;
    const float leftColumnWidth = 400.0f;
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