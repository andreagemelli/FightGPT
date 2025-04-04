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
    diceShape.setSize(sf::Vector2f(60, 60));
    diceShape.setFillColor(sf::Color(200, 200, 200));
    diceShape.setOutlineColor(sf::Color::White);
    diceShape.setOutlineThickness(2);
    diceShape.setPosition(320, 120);  // Position near the combat options

    diceText.setFont(font);
    diceText.setCharacterSize(24);
    diceText.setFillColor(sf::Color::Black);
    updateDiceText();

    initializeInventoryUI();
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
                msg.find("I. Use Item") != std::string::npos) {
                bgColor = sf::Color(100, 100, 100, 150);  // Grey for clickable options
            }
            else if (msg.find("You found:") != std::string::npos ||
                     msg.find("BATTLE START") != std::string::npos ||
                     msg.find("VS") != std::string::npos) {
                bgColor = sf::Color(255, 255, 0, 100);  // Yellow for discoveries and battle starts
            }
            else if (msg.find(player->GetName()) != std::string::npos ||
                     msg.find("Hit!") != std::string::npos ||
                     msg.find("dodged") != std::string::npos ||
                     msg.find("Healed") != std::string::npos ||
                     msg.find("Equipped") != std::string::npos) {
                bgColor = sf::Color(0, 0, 255, 100);  // Blue for player actions
            }
            else if (msg.find("Enemy's turn") != std::string::npos ||
                     msg.find("attacks") != std::string::npos ||
                     msg.find("Enemy HP") != std::string::npos ||
                     msg.find("CRITICAL MISS") != std::string::npos ||
                     msg.find("Escape failed") != std::string::npos) {
                bgColor = sf::Color(150, 50, 50, 150);  // Red for enemy/danger
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

void GamePlayState::handleEvent(const sf::Event& event, sf::RenderWindow& /*window*/) {
    if (gameOver) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
            nextState = std::make_unique<CharacterSelectionState>("");
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
    
    updateStatsText();  // Update stats display after taking damage
    
    combatState = CombatState::PLAYER_TURN;
    CombatLogger::log("\nYour turn! Choose your action:");
    CombatLogger::log("A. Attack");
    CombatLogger::log("E. Try to escape");
    CombatLogger::log("I. Use Item/Change Weapon");
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

    // Check for items at player's position
    if (!showingItemPrompt && combatState == CombatState::NOT_IN_COMBAT) {
        checkForItems();
    }

    updateInventoryDisplay();
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
        ss << " (Total: " << player->GetTotalAttack() << ")";
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
    float xPos = diceShape.getPosition().x + (diceShape.getSize().x - textBounds.width) / 2.0f - textBounds.left;
    float yPos = diceShape.getPosition().y + (diceShape.getSize().y - textBounds.height) / 2.0f - textBounds.top;
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
    // Clear previous messages
    combatLog.clear();
    combatLogTexts.clear();
    combatLogBackgrounds.clear();
    
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
    
    // Clear previous messages
    combatLog.clear();
    combatLogTexts.clear();
    combatLogBackgrounds.clear();
    
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
        addCombatLogMessage("Inventory is full!");
    }
}

void GamePlayState::handleItemUse(int index) {
    const auto& inventory = player->GetInventory();
    if (index < 0 || static_cast<size_t>(index) >= inventory.size()) return;

    auto item = inventory[index];
    std::stringstream ss;

    switch (item->GetType()) {
        case ItemType::POTION:
            if (item->GetName().find("Health") != std::string::npos) {
                int healAmount = item->GetEffectValue();
                int oldHealth = player->GetHealth();
                int maxHeal = player->GetMaxHealth() - oldHealth;
                int actualHeal = std::min(healAmount, maxHeal);
                
                if (actualHeal > 0) {
                    player->TakeDamage(-healAmount); // Negative damage = healing
                    ss << "Used " << item->GetName() << ". Healed for " << actualHeal << " HP";
                    ss << "\nHP: " << player->GetHealth() << "/" << player->GetMaxHealth();
                    player->RemoveItem(index);
                } else {
                    ss << "You are already at full health!";
                }
            } else {
                // Handle other potion effects
                ss << "Used " << item->GetName() << ". Effect: " << item->GetDescription();
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
    const float offsetY = (window.getSize().y - gridSize * cellSize) / 2 - 50;

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
                // Draw yellow diamond for items
                sf::RectangleShape marker(sf::Vector2f(12, 12));
                marker.setFillColor(sf::Color(255, 215, 0, isVisible ? 200 : 100)); // More transparent if revealed
                marker.setPosition(offsetX + x * cellSize + cellSize/2 - 6, 
                                offsetY + y * cellSize + cellSize/2 - 6);
                marker.setRotation(45); // Rotate to make it diamond-shaped
                window.draw(marker);
            }

            // Draw enemies if visible or revealed
            if (enemy && enemy != player.get()) {
                bool shouldDrawEnemy = isVisible || 
                                     (enemy->GetBoss() && bossRevealed) || 
                                     (!enemy->GetBoss() && monstersRevealed);
                
                if (shouldDrawEnemy) {
                    if (enemy->GetBoss()) {
                        // Draw larger red diamond for bosses
                        sf::RectangleShape marker(sf::Vector2f(14, 14));
                        marker.setFillColor(sf::Color(255, 0, 0, isVisible ? 200 : 100));
                        marker.setPosition(offsetX + x * cellSize + cellSize/2 - 7,
                                        offsetY + y * cellSize + cellSize/2 - 7);
                        marker.setRotation(45);
                        window.draw(marker);
                    } else {
                        // Draw red circle for regular enemies
                        sf::CircleShape marker(6);
                        marker.setFillColor(sf::Color(255, 0, 0, isVisible ? 200 : 100));
                        marker.setPosition(offsetX + x * cellSize + cellSize/2 - 6,
                                        offsetY + y * cellSize + cellSize/2 - 6);
                        window.draw(marker);
                    }
                }
            }
        }
    }

    // Draw player position
    sf::CircleShape playerMarker(6);
    playerMarker.setFillColor(sf::Color::Blue);
    playerMarker.setPosition(offsetX + player->GetX() * cellSize + cellSize/2 - 6,
                           offsetY + player->GetY() * cellSize + cellSize/2 - 6);
    window.draw(playerMarker);
} 