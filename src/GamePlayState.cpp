#include "GamePlayState.h"
#include "CharacterSelectionState.h"
#include "Logger.h"
#include <sstream>
#include <cstdlib>
#include <ctime>

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

GamePlayState::GamePlayState(int selectedCharacter, const std::string& playerName, const std::string& bossName)
    : player(nullptr),
      gameMap(std::make_unique<Map>(15, 15)),
      currentEnemy(nullptr),
      combatState(CombatState::NOT_IN_COMBAT),
      selectedCharacter(selectedCharacter),
      playerName(playerName),
      bossName(bossName),
      gameOver(false),
      currentDiceValue(1),
      diceAnimationTime(0),
      isRollingDice(false),
      showingItemPrompt(false),
      currentItem(nullptr),
      bossRevealed(false),
      itemsRevealed(false),
      monstersRevealed(false) {

    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }
    
    // Load prison background
    if (!backgroundTexture.loadFromFile("assets/maps/prison.png")) {
        Logger::error("Failed to load prison background!");
        return;
    }
    backgroundSprite.setTexture(backgroundTexture);

    // Load wall texture
    if (!wallTexture.loadFromFile("assets/maps/wall.png")) {
        Logger::error("Failed to load wall texture!");
        return;
    }
    wallSprite.setTexture(wallTexture);
    
    // Load character image for the player
    std::string characterPath;
    switch (selectedCharacter) {
        case 0: characterPath = "assets/characters/knight.png"; break; // Knight
        case 1: characterPath = "assets/characters/mage.png"; break;   // Mage
        case 2: characterPath = "assets/characters/archer.png"; break; // Archer
        default: characterPath = "assets/characters/knight.png"; break;
    }

    if (!playerTexture.loadFromFile(characterPath)) {
        Logger::error("Failed to load character image!");
        return;
    }
    playerSprite.setTexture(playerTexture);
    
    // Scale the character image to fit within a 40x40 tile
    float scaleX = 40.0f / playerTexture.getSize().x;
    float scaleY = 40.0f / playerTexture.getSize().y;
    playerSprite.setScale(scaleX, scaleY);

    // Load class icon
    loadClassIcon(selectedCharacter);

    // Initialize player shape
    playerShape.setSize(sf::Vector2f(30, 30));
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

    // Initialize inventory box with smaller height
    inventoryBox.setSize(sf::Vector2f(leftColumnWidth - 2 * padding, 120));  // Reduced height
    inventoryBox.setPosition(padding, statsHeight + padding);
    inventoryBox.setFillColor(sf::Color(60, 60, 60));
    inventoryBox.setOutlineColor(sf::Color(200, 200, 200));
    inventoryBox.setOutlineThickness(2);

    // Initialize combat log box - position it below inventory with proper spacing
    float combatLogY = statsHeight + inventoryBox.getSize().y + 2 * padding;
    float combatLogHeight = 800 - combatLogY - 2 * padding;
    
    combatLogBox.setSize(sf::Vector2f(leftColumnWidth - 2 * padding, combatLogHeight));
    combatLogBox.setPosition(padding, combatLogY);
    combatLogBox.setFillColor(sf::Color(60, 60, 60));
    combatLogBox.setOutlineColor(sf::Color(200, 200, 200));
    combatLogBox.setOutlineThickness(2);

    // Initialize combat log background
    combatLogBackground.setSize(sf::Vector2f(leftColumnWidth - 4 * padding, combatLogHeight - 2 * padding));
    combatLogBackground.setFillColor(sf::Color(0, 0, 0, 200));
    combatLogBackground.setPosition(padding * 2, combatLogY + padding);

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

    // Set up the combat logger callback
    CombatLogger::setCallback([this](const std::string& message) {
        addCombatLogMessage(message);
    });

    // Initialize stats
    initializeStats();

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
    if (!diceTexture.loadFromFile("assets/icons/dice.png")) {
        Logger::error("Failed to load dice texture!");
        return;
    }
    
    diceSprite.setTexture(diceTexture);
    // Scale dice sprite to 60x60 pixels
    float diceScale = 60.0f / std::max(diceTexture.getSize().x, diceTexture.getSize().y);
    diceSprite.setScale(diceScale, diceScale);
    diceSprite.setPosition(320, 120);  // Position near the combat options

    diceText.setFont(font);
    diceText.setCharacterSize(24);
    diceText.setFillColor(sf::Color::Black);  // Changed to black for better visibility on dice
    diceText.setStyle(sf::Text::Bold);  // Make the text bold for better readability
    updateDiceText();

    initializeInventoryUI();

    // Load monster texture
    if (!monsterTexture.loadFromFile("assets/characters/monster.png")) {
        Logger::error("Failed to load monster texture!");
        return;
    }
    monsterSprite.setTexture(monsterTexture);
    float monsterScale = 40.0f / std::max(monsterTexture.getSize().x, monsterTexture.getSize().y);
    monsterSprite.setScale(monsterScale, monsterScale);
    
    // Load boss texture
    if (!bossTexture.loadFromFile("assets/characters/boss.png")) {
        Logger::error("Failed to load boss texture!");
        return;
    }
    bossSprite.setTexture(bossTexture);
    float bossScale = 40.0f / std::max(bossTexture.getSize().x, bossTexture.getSize().y);
    bossSprite.setScale(bossScale, bossScale);
    
    // Load item texture
    if (!itemTexture.loadFromFile("assets/characters/item.png")) {
        Logger::error("Failed to load item texture!");
        return;
    }
    itemSprite.setTexture(itemTexture);
    float itemScale = 40.0f / std::max(itemTexture.getSize().x, itemTexture.getSize().y);
    itemSprite.setScale(itemScale, itemScale);
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
    combatLogBackgrounds.clear();
    
    // Calculate starting Y position - now positioned below inventory
    float yPos = combatLogBox.getPosition().y + 10;
    
    if (font.getInfo().family != "") {
        for (const auto& msg : combatLog) {
            sf::Text text;
            text.setFont(font);
            text.setString(msg);
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::White);
            text.setPosition(combatLogBox.getPosition().x + 10, yPos);
            
            // Add backgrounds with different colors for different message types
            sf::Color bgColor;
            if (msg.find("Choose your action") != std::string::npos ||
                msg.find("Press") != std::string::npos ||
                msg.find("A. Attack") != std::string::npos ||
                msg.find("E. Try to escape") != std::string::npos ||
                msg.find("I. Use Item") != std::string::npos ||
                msg.find("S. Use Special Ability") != std::string::npos ||  // Added special ability option
                msg.find("1.") != std::string::npos ||  // Inventory slots
                msg.find("2.") != std::string::npos ||
                msg.find("3.") != std::string::npos ||
                msg.find("4.") != std::string::npos) {
                bgColor = sf::Color(100, 100, 100, 150);  // Grey for all keyboard input options
            }
            else if (msg.find("Enemy") != std::string::npos ||
                     msg.find("attacks") != std::string::npos ||
                     msg.find("CRITICAL MISS") != std::string::npos ||
                     msg.find("Escape failed") != std::string::npos ||
                     msg.find("Failed to escape") != std::string::npos ||
                     msg.find("burning") != std::string::npos) {
                bgColor = sf::Color(150, 50, 50, 150);  // Red for enemy actions and negative events
            }
            else if (msg.find(player->GetName()) != std::string::npos ||
                     msg.find("Hit!") != std::string::npos ||
                     msg.find("dodged") != std::string::npos ||
                     msg.find("Healed") != std::string::npos ||
                     msg.find("Equipped") != std::string::npos ||
                     msg.find("RAGE") != std::string::npos ||
                     msg.find("FIREBALL") != std::string::npos ||
                     msg.find("HUNTER'S MARK") != std::string::npos ||
                     msg.find("Your HP:") != std::string::npos ||
                     msg.find("Successfully escaped") != std::string::npos) {
                bgColor = sf::Color(0, 0, 255, 100);  // Blue for player actions and positive events
            }
            else if (msg.find("You found:") != std::string::npos ||
                     msg.find("BATTLE START") != std::string::npos ||
                     msg.find("VS") != std::string::npos ||
                     msg.find("=== BATTLE WON! ===") != std::string::npos) {
                bgColor = sf::Color(255, 255, 0, 100);  // Yellow for discoveries and battle events
            }
            else {
                bgColor = sf::Color(0, 0, 0, 0);  // Transparent for normal messages
            }

            if (bgColor.a > 0) {
                sf::RectangleShape background;
                background.setSize(sf::Vector2f(combatLogBox.getSize().x - 20, 20));
                background.setPosition(combatLogBox.getPosition().x + 5, yPos);
                background.setFillColor(bgColor);
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
            nextState = std::make_unique<CharacterSelectionState>("");  // Pass empty string to return to name screen
            return;
        }
        return;
    }

    if (event.type == sf::Event::KeyPressed) {
        // Handle inventory hotkeys (1-4) - allow using items anytime
        if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num4) {
            int index = event.key.code - sf::Keyboard::Num1;
            handleItemUse(index);
            return;
        }

        if (combatState != CombatState::NOT_IN_COMBAT) {
            if (combatState == CombatState::PLAYER_TURN) {
                if (event.key.code == sf::Keyboard::A) {
                    handlePlayerAttack();
                }
                else if (event.key.code == sf::Keyboard::E) {
                    handlePlayerEscape();
                }
                else if (event.key.code == sf::Keyboard::I) {
                    CombatLogger::log("\nChoose item (1-4) to use or change weapon");
                    CombatLogger::log("Current inventory:");
                    displayInventoryInLog();
                }
                else if (event.key.code == sf::Keyboard::S && !player->HasUsedAbility()) {
                    handlePlayerAbility();
                }
            }
            return;
        }

        // Handle item interaction
        if (showingItemPrompt) {
            if (event.key.code == sf::Keyboard::P) {
                handleItemPickup();
                return;
            } else if (event.key.code == sf::Keyboard::L) {
                // Remove item from map but don't add to inventory
                gameMap->RemoveItemAtPosition(player->GetX(), player->GetY());
                showingItemPrompt = false;
                currentItem = nullptr;
                addCombatLogMessage("You left the item behind.");
                return;
            }
            return;
        }

        // Handle movement
        if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right ||
            event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) {
            int dx = 0, dy = 0;
            if (event.key.code == sf::Keyboard::Left)  dx = -1;
            else if (event.key.code == sf::Keyboard::Right) dx = 1;
            else if (event.key.code == sf::Keyboard::Up)    dy = -1;
            else if (event.key.code == sf::Keyboard::Down)  dy = 1;

            Character* enemy = gameMap->CheckNewPosition(*player, dx, dy);
            if (enemy) {
                handleCombat(enemy);
            } else if (dx != 0 || dy != 0) {  // Only move if actually moving
                int newX = player->GetX() + dx;
                int newY = player->GetY() + dy;
                
                // Check map boundaries and walls
                if (newX >= 0 && newX < 15 && newY >= 0 && newY < 15 && !gameMap->HasWall(newX, newY)) {
                    gameMap->MoveCharacter(*player, dx, dy);
                    // Move monsters after player's turn
                    gameMap->MoveMonsters(*player);
                    std::stringstream ss;
                    ss << "\nMoved to position (" << player->GetX() << ", " << player->GetY() << ")";
                    CombatLogger::log(ss.str());
                }
            }

            updateStatsText();
            updateEnemyDisplays();
            return;
        }
    }

    // Check for items at player's position
    if (!showingItemPrompt && combatState == CombatState::NOT_IN_COMBAT) {
        checkForItems();
    }

    updateInventoryDisplay();
}

void GamePlayState::handleCombat(Character* enemy) {
    if (!enemy) return;

    if (combatState == CombatState::NOT_IN_COMBAT) {
        currentEnemy = enemy;
        combatState = CombatState::PLAYER_TURN;
        player->ResetAbility(); // Reset ability at the start of combat
        
        // Clear previous combat messages
        combatLog.clear();
        combatLogTexts.clear();
        combatLogBackgrounds.clear();
        
        std::stringstream ss;
        ss << "\n=== BATTLE START ===\n";
        ss << player->GetName() << " (HP: " << player->GetHealth() << "/" << player->GetMaxHealth() << ") VS " 
           << enemy->GetName() << " (HP: " << enemy->GetHealth() << "/" << enemy->GetMaxHealth() << ")";
        CombatLogger::log(ss.str());
        CombatLogger::log("\nYour turn! Choose your action:");
        CombatLogger::log("A. Attack");
        CombatLogger::log("E. Try to escape");
        CombatLogger::log("I. Use Item/Change Weapon");
        if (!player->HasUsedAbility()) {
            CombatLogger::log("S. Use Special Ability: " + getAbilityDescription());
        }
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
    
    // Apply status effects first
    if (player->IsWounded()) {
        player->ApplyBleedDamage();
        updateStatsText();
    }
    
    if (enemy->IsBurning()) {
        CombatLogger::log("\nEnemy is burning! (-5 HP)");
        enemy->ApplyBurnDamage();
        
        if (enemy->IsDefeated()) {
            handleVictory(*enemy);
            return;
        }
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
    
    updateStatsText();
    
    // Deactivate Rage after enemy's turn if it was active
    if (player->IsRageActive()) {
        player->DeactivateRage();
    }
    
    combatState = CombatState::PLAYER_TURN;
    CombatLogger::log("\nYour turn! Choose your action:");
    CombatLogger::log("A. Attack");
    CombatLogger::log("E. Try to escape");
    CombatLogger::log("I. Use Item/Change Weapon");
    if (!player->HasUsedAbility()) {
        CombatLogger::log("S. Use Special Ability: " + getAbilityDescription());
    }
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
        nextState = std::make_unique<CharacterSelectionState>("");  // Pass empty string to return to name screen
    }
    
    combatState = CombatState::NOT_IN_COMBAT;
    currentEnemy = nullptr;
}

void GamePlayState::update(float deltaTime) {
    // Update health bar size based on current health
    float healthPercent = static_cast<float>(player->GetHealth()) / player->GetMaxHealth();
    float healthBarWidth = healthBarBackground.getSize().x;
    healthBar.setSize(sf::Vector2f(healthBarWidth * healthPercent, healthBar.getSize().y));
    
    // Update buffs
    player->UpdateBuffs(deltaTime);
    
    // Update dice animation
    updateDiceRoll(deltaTime);

    // Check for items at player's position
    if (!showingItemPrompt && combatState == CombatState::NOT_IN_COMBAT) {
        checkForItems();
    }

    updateInventoryDisplay();
    updateStatsText();  // Update stats to reflect any buff changes
}

void GamePlayState::updateStatsText() {
    // Update character name
    characterNameText.setString(player->GetName() + (player->IsWounded() ? " 🩸" : ""));
    characterNameText.setPosition(10, 10);

    // Calculate experience needed for next level
    int currentExp = player->GetExperience();
    int nextLevelExp = player->GetLevel() * 10;

    // Update stats with current values
    std::stringstream ss;
    ss << "Level: " << player->GetLevel() << "\n"
       << "HP: " << player->GetHealth() << "/" << player->GetMaxHealth() 
       << (player->IsWounded() ? " 🩸" : "") << "\n"
       << "Base Attack: " << player->GetAttack();
    
    // Show total attack if different from base
    if (player->GetTotalAttack() != player->GetAttack()) {
        ss << " (Total: " << player->GetTotalAttack();
        if (player->HasStrengthBuff()) {
            ss << " [+" << player->GetTotalAttack() - player->GetAttack() << " for " 
               << static_cast<int>(player->GetStrengthBuffDuration()) << "s]";
        }
        ss << ")";
    }
    ss << "\n"
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
    
    // If Rage is active, skip dice roll and perform critical hit immediately
    if (player->IsRageActive()) {
        CombatLogger::log("\nRAGE CRITICAL HIT!");
        performPlayerAttack(true);  // Force critical hit
        player->DeactivateRage();  // Consume Rage after use
        return;
    }
    
    startDiceRoll();
}

void GamePlayState::handlePlayerEscape() {
    if (!currentEnemy) return;
    combatState = CombatState::TRYING_ESCAPE;
    startDiceRoll();
}

void GamePlayState::updateDiceText() {
    diceText.setString(std::to_string(currentDiceValue));
    
    // Get the actual bounds of the dice sprite
    sf::FloatRect diceBounds = diceSprite.getGlobalBounds();
    
    // Get the text bounds
    sf::FloatRect textBounds = diceText.getLocalBounds();
    
    // Calculate the center position of the dice sprite
    float centerX = diceBounds.left + (diceBounds.width / 2.0f);
    float centerY = diceBounds.top + (diceBounds.height / 2.0f);
    
    // Position the text at the center, accounting for the text bounds offset
    float xPos = centerX - (textBounds.width / 2.0f) - textBounds.left;
    float yPos = centerY - (textBounds.height / 2.0f) - textBounds.top;
    
    diceText.setPosition(xPos, yPos);
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
        // If rage is active, show only high numbers during animation for effect
        if (player->IsRageActive()) {
            currentDiceValue = 20;
        } else {
            currentDiceValue = (rand() % 20) + 1;
        }
        updateDiceText();
    } else {
        isRollingDice = false;
        
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
    CombatLogger::log(ss.str());
    
    if (isAttack) {
        // If Rage is active, force critical hit and deactivate rage
        if (player->IsRageActive()) {
            ss.str("");  // Clear stringstream
            ss << " - RAGE CRITICAL HIT!";
            CombatLogger::log(ss.str());
            performPlayerAttack(true);  // Force critical hit
            player->DeactivateRage();  // Deactivate rage after use
            return;
        }
        
        // Normal attack resolution
        if (roll == 20) {
            ss << " - CRITICAL HIT!";
            CombatLogger::log(ss.str());
            performPlayerAttack(true);
        } else if (roll == 1 && !player->HasMarker()) {
            ss << " - CRITICAL MISS! You are wounded!";
            CombatLogger::log(ss.str());
            player->SetWounded(true);
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
        } else if (roll >= 10 || player->HasMarker()) {
            ss << (player->HasMarker() ? " - MARKED TARGET HIT!" : " - Hit!");
            CombatLogger::log(ss.str());
            performPlayerAttack(false);
            
            if (player->HasMarker()) {
                player->DecrementMarker();
            }
        } else {
            ss << " - Miss!";
            CombatLogger::log(ss.str());
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
        }
    } else {
        // Escape logic remains unchanged
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
    
    int damage = player->GetTotalAttack(); // Use total attack including weapon bonus
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
        if (player->HasMarker()) {
            ss << " (MARKED TARGET: " << player->GetMarkerCount() << " marks remaining)";
        }
        if (auto weapon = player->GetEquippedWeapon()) {
            ss << " with " << weapon->GetName();
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

void GamePlayState::initializeInventoryUI() {
    const float padding = 10.0f;
    const float startX = inventoryBox.getPosition().x + padding;
    const float startY = inventoryBox.getPosition().y + padding;

    // Create inventory title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("Inventory");
    titleText.setCharacterSize(16);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(startX, startY);
    inventoryTexts.push_back(titleText);

    // Create 4 inventory slots as simple text
    for (int i = 0; i < 4; ++i) {
        sf::Text slotText;
        slotText.setFont(font);
        slotText.setCharacterSize(14);
        slotText.setFillColor(sf::Color::White);
        slotText.setPosition(startX, startY + 25 + i * 20);  // Compact vertical spacing
        slotText.setString(std::to_string(i + 1) + ".");
        inventoryTexts.push_back(slotText);
    }
}

void GamePlayState::updateInventoryDisplay() {
    // Skip the title text (first element)
    const auto& inventory = player->GetInventory();
    for (size_t i = 0; i < 4; ++i) {
        std::string displayText = std::to_string(i + 1) + ".";
        if (i < inventory.size()) {
            const auto& item = inventory[i];
            displayText += " " + item->GetName();
            if (item == player->GetEquippedWeapon()) {
                displayText += " (E)";
            }
        }
        inventoryTexts[i + 1].setString(displayText);  // +1 to skip title
    }
}

void GamePlayState::checkForItems() {
    if (showingItemPrompt) return; // Don't check for items if we're already showing a prompt

    auto item = gameMap->GetItemAtPosition(player->GetX(), player->GetY());
    if (item) {
        currentItem = item;
        showingItemPrompt = true;
        std::stringstream ss;
        ss << "You found: " << item->GetName() << "\n" << item->GetDescription() << "\n";
        ss << "Press 'P' to pick up or 'L' to leave";
        addCombatLogMessage(ss.str());
    }
}

void GamePlayState::handleItemPickup() {
    if (!currentItem || !showingItemPrompt) return;

    // Try to add item to inventory
    if (player->AddItem(currentItem)) {
        // Successfully added to inventory, remove from map
        gameMap->RemoveItemAtPosition(player->GetX(), player->GetY());
        addCombatLogMessage("Picked up " + currentItem->GetName() + ".");
        showingItemPrompt = false;
        currentItem = nullptr;
    } else {
        addCombatLogMessage("Inventory is full! Press 'L' to leave the item.");
    }
}

void GamePlayState::handleItemUse(int index) {
    const auto& inventory = player->GetInventory();
    if (index < 0 || static_cast<size_t>(index) >= inventory.size()) return;

    auto item = inventory[index];
    std::stringstream ss;

    switch (item->GetType()) {
        case ItemType::POTION:
            if (item->GetName().find("Health") != std::string::npos || item->GetName() == "Apple") {
                int healAmount = item->GetEffectValue();
                int oldHealth = player->GetHealth();
                int maxHeal = player->GetMaxHealth() - oldHealth;
                int actualHeal = std::min(healAmount, maxHeal);
                
                if (actualHeal > 0) {
                    player->TakeDamage(-actualHeal); // Use actualHeal instead of healAmount
                    ss << "Used " << item->GetName() << ". Healed for " << actualHeal << " HP";
                    ss << "\nHP: " << player->GetHealth() << "/" << player->GetMaxHealth();
                    player->RemoveItem(index);
                } else {
                    ss << "You are already at full health!";
                }
            } else if (item->GetName().find("Strength") != std::string::npos) {
                // Handle strength potion
                int strengthBonus = item->GetEffectValue();
                player->ApplyStrengthBuff(strengthBonus);
                ss << "Used " << item->GetName() << ". Attack increased by " << strengthBonus;
                ss << "\nNew Attack Power: " << player->GetTotalAttack();
                player->RemoveItem(index);
            }
            break;

        case ItemType::WEAPON:
            player->EquipWeapon(index);
            ss << "Equipped " << item->GetName();
            ss << "\nNew Attack Power: " << player->GetTotalAttack();
            break;

        case ItemType::OBJECT:
            // Handle object effects based on type
            switch (item->GetObjectEffect()) {
                case ObjectEffect::REVEAL_BOSS:
                    bossRevealed = true;
                    ss << "Used " << item->GetName() << ". The boss location is now revealed!";
                    break;
                case ObjectEffect::REVEAL_ITEMS:
                    itemsRevealed = true;
                    ss << "Used " << item->GetName() << ". All items are now revealed!";
                    break;
                case ObjectEffect::REVEAL_MONSTERS:
                    monstersRevealed = true;
                    ss << "Used " << item->GetName() << ". All monsters are now revealed!";
                    break;
            }
            player->RemoveItem(index);
            break;
    }

    CombatLogger::log(ss.str());
    updateInventoryDisplay();
    updateStatsText();

    // If in combat, show combat options again after using item
    if (combatState == CombatState::PLAYER_TURN) {
        CombatLogger::log("\nYour turn! Choose your action:");
        CombatLogger::log("A. Attack");
        CombatLogger::log("E. Try to escape");
        CombatLogger::log("I. Use Item/Change Weapon");
    }
}

void GamePlayState::displayInventoryInLog() {
    const auto& inventory = player->GetInventory();
    for (size_t i = 0; i < 4; ++i) {
        std::string displayText = std::to_string(i + 1) + ".";
        if (i < inventory.size()) {
            const auto& item = inventory[i];
            displayText += " " + item->GetName();
            if (item == player->GetEquippedWeapon()) {
                displayText += " (E)";
            }
            displayText += " - " + item->GetDescription();
        } else {
            displayText += " Empty";
        }
        CombatLogger::log(displayText);
    }
}

void GamePlayState::handlePlayerAbility() {
    if (!currentEnemy || player->HasUsedAbility()) return;
 
    switch (selectedCharacter) {
        case 0: // Knight
            performRageAbility();
            // Knight should NOT go to enemy turn, just activate rage and wait for next attack
            break;
        case 1: // Mage
            performFireballAbility();
            break;
        case 2: // Archer
            performHeadshotAbility();
            // Archer waits for next turn
            combatState = CombatState::ENEMY_TURN;
            sf::sleep(sf::milliseconds(500));
            handleEnemyTurn(currentEnemy);
            break;
    }
}

void GamePlayState::performRageAbility() {
    player->ActivateRage();
    player->SetAbilityUsed(true);  // Mark ability as used
    CombatLogger::log("\nRAGE ACTIVATED! Your next attack will be a critical hit!");
    // Show combat options again since we're staying in player turn
    CombatLogger::log("\nYour turn! Choose your action:");
    CombatLogger::log("A. Attack");
    CombatLogger::log("E. Try to escape");
    CombatLogger::log("I. Use Item/Change Weapon");
}

void GamePlayState::performFireballAbility() {
    // Calculate 15% of enemy's max health for Fireball damage
    float damagePercent = 0.15f;  // 15%
    int maxHealth = currentEnemy->GetMaxHealth();
    int burnDamage = static_cast<int>(maxHealth * damagePercent);
    
    // Ensure minimum damage of 1
    burnDamage = std::max(1, burnDamage);
    
    int actualDamage = currentEnemy->TakeDamage(burnDamage);
    currentEnemy->ApplyBurn();
    player->SetAbilityUsed(true);
    
    std::stringstream ss;
    ss << "\nFIREBALL! Dealt " << actualDamage << " damage (" << (damagePercent * 100) << "% of max HP) and applied burn effect!";
    ss << "\nEnemy HP: " << currentEnemy->GetHealth() << "/" << currentEnemy->GetMaxHealth();
    CombatLogger::log(ss.str());
    
    if (currentEnemy->IsDefeated()) {
        handleVictory(*currentEnemy);
        return;
    }
    
    combatState = CombatState::ENEMY_TURN;
    sf::sleep(sf::milliseconds(500));
    handleEnemyTurn(currentEnemy);
}

void GamePlayState::performHeadshotAbility() {
    player->ActivateMarker();
    CombatLogger::log("\nHUNTER'S MARK ACTIVATED! Your next three attacks cannot miss!");
}

std::string GamePlayState::getAbilityDescription() const {
    switch (selectedCharacter) {
        case 0: // Knight
            return "Rage (Next attack is a critical hit)";
        case 1: // Mage
            return "Fireball (15% max HP damage + 5 damage/turn)";
        case 2: // Archer
            return "Hunter's Mark (Next 3 attacks cannot miss)";
        default:
            return "";
    }
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
    window.draw(diceSprite);
    window.draw(diceText);

    // Draw inventory
    window.draw(inventoryBox);
    for (const auto& text : inventoryTexts) {
        window.draw(text);
    }

    // Draw right column (game map)
    drawGrid(window);

    // Draw current enemy info if in combat
    if (currentEnemy && combatState != CombatState::NOT_IN_COMBAT) {
        // Create enemy info box below the map
        const float leftColumnWidth = 400.0f;
        const float mapSize = 15 * 40.0f; // gridSize * cellSize
        const float offsetX = leftColumnWidth + ((window.getSize().x - leftColumnWidth) - mapSize) / 2;
        const float offsetY = (window.getSize().y - mapSize) / 2 - 50; // Match the map's new position
        const float infoBoxY = offsetY + mapSize + 20; // Position below map with padding

        sf::RectangleShape enemyInfoBox;
        enemyInfoBox.setSize(sf::Vector2f(mapSize, 100));
        enemyInfoBox.setPosition(offsetX, infoBoxY);
        enemyInfoBox.setFillColor(sf::Color(60, 60, 60));
        enemyInfoBox.setOutlineColor(sf::Color(200, 200, 200));
        enemyInfoBox.setOutlineThickness(2);
        window.draw(enemyInfoBox);

        // Create enemy stats text
        sf::Text enemyStatsText;
        enemyStatsText.setFont(font);
        enemyStatsText.setCharacterSize(14);
        enemyStatsText.setFillColor(sf::Color::White);
        enemyStatsText.setPosition(offsetX + 10, infoBoxY + 10);

        std::stringstream ss;
        ss << "Enemy: " << currentEnemy->GetName() << (currentEnemy->GetBoss() ? " (BOSS)" : "") << "\n"
           << "HP: " << currentEnemy->GetHealth() << "/" << currentEnemy->GetMaxHealth() << "\n"
           << "Attack: " << currentEnemy->GetAttack() << "\n"
           << "Defense: " << currentEnemy->GetDefense() << "\n"
           << "Speed: " << currentEnemy->GetSpeed();
        enemyStatsText.setString(ss.str());
        window.draw(enemyStatsText);
    }

    // Draw game over/victory screen if needed
    if (gameOver) {
        // Draw semi-transparent overlay
        sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(overlay);

        // Create the main text
        sf::Text endText;
        endText.setFont(font);
        endText.setCharacterSize(72);  // Increased size for more impact
        
        if (player->GetHealth() > 0) {
            // Victory screen
            endText.setString("VICTORY!");
            endText.setFillColor(sf::Color(50, 255, 50));  // Bright green
            
            // Create subtitle text
            sf::Text subtitleText;
            subtitleText.setFont(font);
            subtitleText.setCharacterSize(36);
            subtitleText.setString("Press Enter to return to main menu");
            subtitleText.setFillColor(sf::Color(100, 255, 100));  // Lighter green
            
            // Position both texts
            sf::FloatRect mainBounds = endText.getLocalBounds();
            sf::FloatRect subBounds = subtitleText.getLocalBounds();
            
            endText.setPosition(
                (window.getSize().x - mainBounds.width) / 2,
                (window.getSize().y - (mainBounds.height + subBounds.height + 20)) / 2
            );
            
            subtitleText.setPosition(
                (window.getSize().x - subBounds.width) / 2,
                endText.getPosition().y + mainBounds.height + 20
            );
            
            window.draw(endText);
            window.draw(subtitleText);
        } else {
            // Game Over screen
            endText.setString("GAME OVER");
            endText.setFillColor(sf::Color::Red);
            
            // Create subtitle text
            sf::Text subtitleText;
            subtitleText.setFont(font);
            subtitleText.setCharacterSize(36);
            subtitleText.setString("Press Enter to return to main menu");
            subtitleText.setFillColor(sf::Color(255, 100, 100));
            
            // Position both texts
            sf::FloatRect mainBounds = endText.getLocalBounds();
            sf::FloatRect subBounds = subtitleText.getLocalBounds();
            
            endText.setPosition(
                (window.getSize().x - mainBounds.width) / 2,
                (window.getSize().y - (mainBounds.height + subBounds.height + 20)) / 2
            );
            
            subtitleText.setPosition(
                (window.getSize().x - subBounds.width) / 2,
                endText.getPosition().y + mainBounds.height + 20
            );
            
            window.draw(endText);
            window.draw(subtitleText);
        }
    }
}

void GamePlayState::drawGrid(sf::RenderWindow& window) {
    const int cellSize = 40;
    const int gridSize = 15;
    const float leftColumnWidth = 400.0f;
    const float offsetX = leftColumnWidth + ((window.getSize().x - leftColumnWidth) - gridSize * cellSize) / 2;
    const float offsetY = (window.getSize().y - gridSize * cellSize) / 2 - 50;

    // Draw background first
    drawBackground(window);

    // Draw grid lines (fainter now that we have a background)
    for (int i = 0; i <= gridSize; ++i) {
        sf::RectangleShape line(sf::Vector2f(gridSize * cellSize, 1));
        line.setPosition(offsetX, offsetY + i * cellSize);
        line.setFillColor(sf::Color(100, 100, 100, 128)); // Semi-transparent
        window.draw(line);

        line.setSize(sf::Vector2f(1, gridSize * cellSize));
        line.setPosition(offsetX + i * cellSize, offsetY);
        window.draw(line);
    }

    // Draw walls
    drawWalls(window);

    // Draw visible cells and markers
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            bool isVisible = false;
            // Check if cell is within visibility range (3 cells from player) or revealed by items
            int dx = abs(x - player->GetX());
            int dy = abs(y - player->GetY());
            if (dx <= 3 && dy <= 3) {
                isVisible = true;
            }

            // Check what's in the cell
            Character* enemy = gameMap->GetCharacterAt(x, y);
            std::shared_ptr<Item> item = gameMap->GetItemAtPosition(x, y);
            
            // Draw items if visible or revealed
            if ((isVisible || itemsRevealed) && item) {
                itemSprite.setPosition(offsetX + x * cellSize + cellSize/2 - itemSprite.getGlobalBounds().width/2,
                                     offsetY + y * cellSize + cellSize/2 - itemSprite.getGlobalBounds().height/2);
                itemSprite.setColor(sf::Color(255, 255, 255, isVisible ? 200 : 100));
                window.draw(itemSprite);
            }

            // Draw enemies if visible or revealed
            if (enemy && enemy != player.get()) {
                bool shouldDrawEnemy = isVisible || 
                                     (enemy->GetBoss() && bossRevealed) || 
                                     (!enemy->GetBoss() && monstersRevealed);
                
                if (shouldDrawEnemy) {
                    // Calculate available space for sprite (cell height minus health bar height)
                    float availableHeight = cellSize - 4; // 4 pixels for health bar
                    float spriteScale = availableHeight / std::max(monsterTexture.getSize().y, bossTexture.getSize().y);

                    // Draw enemy sprite
                    if (enemy->GetBoss()) {
                        bossSprite.setScale(spriteScale, spriteScale);
                        bossSprite.setPosition(offsetX + x * cellSize + cellSize/2 - bossSprite.getGlobalBounds().width/2,
                                            offsetY + y * cellSize + cellSize/2 - bossSprite.getGlobalBounds().height/2 - 2); // Move up slightly
                        bossSprite.setColor(sf::Color(255, 255, 255, isVisible ? 200 : 100));
                        window.draw(bossSprite);
                    } else {
                        monsterSprite.setScale(spriteScale, spriteScale);
                        monsterSprite.setPosition(offsetX + x * cellSize + cellSize/2 - monsterSprite.getGlobalBounds().width/2,
                                                offsetY + y * cellSize + cellSize/2 - monsterSprite.getGlobalBounds().height/2 - 2); // Move up slightly
                        monsterSprite.setColor(sf::Color(255, 255, 255, isVisible ? 200 : 100));
                        window.draw(monsterSprite);
                    }

                    // Draw health bar at the bottom of the cell
                    float infoY = offsetY + y * cellSize + cellSize - 4; // Position at bottom of cell
                    float infoX = offsetX + x * cellSize;

                    // Draw health bar background
                    sf::RectangleShape healthBarBg(sf::Vector2f(cellSize, 4));
                    healthBarBg.setFillColor(sf::Color(100, 100, 100));
                    healthBarBg.setPosition(infoX, infoY);
                    window.draw(healthBarBg);

                    // Draw health bar
                    float healthPercent = static_cast<float>(enemy->GetHealth()) / enemy->GetMaxHealth();
                    sf::RectangleShape healthBar(sf::Vector2f(cellSize * healthPercent, 4));
                    healthBar.setFillColor(sf::Color::Red);
                    healthBar.setPosition(infoX, infoY);
                    window.draw(healthBar);

                    // Draw enemy name within the health bar
                    sf::Text enemyName;
                    enemyName.setFont(font);
                    enemyName.setString(enemy->GetName());
                    enemyName.setCharacterSize(10);
                    enemyName.setFillColor(sf::Color::Black);
                    enemyName.setPosition(infoX + 2, infoY + 1); // Slightly offset from edges
                    window.draw(enemyName);
                }
            }
        }
    }

    // Draw player position with character image
    playerSprite.setPosition(offsetX + player->GetX() * cellSize + cellSize/2 - playerSprite.getGlobalBounds().width/2,
                             offsetY + player->GetY() * cellSize + cellSize/2 - playerSprite.getGlobalBounds().height/2);
    window.draw(playerSprite);
}

void GamePlayState::drawBackground(sf::RenderWindow& window) {
    const float leftColumnWidth = 400.0f;
    const int gridSize = 15;
    const int cellSize = 40;
    const float offsetX = leftColumnWidth + ((window.getSize().x - leftColumnWidth) - gridSize * cellSize) / 2;
    const float offsetY = (window.getSize().y - gridSize * cellSize) / 2 - 50;

    // Scale and position background to fit grid
    float scaleX = (gridSize * cellSize) / static_cast<float>(backgroundTexture.getSize().x);
    float scaleY = (gridSize * cellSize) / static_cast<float>(backgroundTexture.getSize().y);
    backgroundSprite.setScale(scaleX, scaleY);
    backgroundSprite.setPosition(offsetX, offsetY);
    window.draw(backgroundSprite);
}

void GamePlayState::drawWalls(sf::RenderWindow& window) {
    const float leftColumnWidth = 400.0f;
    const int gridSize = 15;
    const int cellSize = 40;
    const float offsetX = leftColumnWidth + ((window.getSize().x - leftColumnWidth) - gridSize * cellSize) / 2;
    const float offsetY = (window.getSize().y - gridSize * cellSize) / 2 - 50;

    // Scale wall sprite to fit cell size
    float scaleX = cellSize / static_cast<float>(wallTexture.getSize().x);
    float scaleY = cellSize / static_cast<float>(wallTexture.getSize().y);
    wallSprite.setScale(scaleX, scaleY);

    // Draw visible walls
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            // Check if wall is within visibility range (3 cells from player)
            int dx = abs(x - player->GetX());
            int dy = abs(y - player->GetY());
            if (dx <= 3 && dy <= 3 && gameMap->HasWall(x, y)) {
                wallSprite.setPosition(offsetX + x * cellSize, offsetY + y * cellSize);
                window.draw(wallSprite);
            }
        }
    }
} 