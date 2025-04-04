#include "GameLogic.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <random>

int Character::TakeDamage(int damage) {
    // If this is healing (negative damage)
    if (damage < 0) {
        int healAmount = -damage; // Convert to positive
        int oldHealth = health;
        health = std::min(maxHealth, health + healAmount); // Can't heal beyond max health
        return health - oldHealth; // Return actual amount healed
    }

    // Normal damage handling
    if (rand() % 100 < avoidance) {
        Logger::info(name + " avoided the attack!");
        return 0;
    }

    // New damage calculation formula with some randomness
    float defenseReduction = static_cast<float>(defense) / (defense + 50);  // Defense has diminishing returns
    float damageMultiplier = (rand() % 30 + 85) / 100.0f;  // Random damage between 85% and 115%
    int damageTaken = static_cast<int>(damage * (1.0f - defenseReduction) * damageMultiplier);
    damageTaken = std::max(1, damageTaken);  // Always deal at least 1 damage
    
    health -= damageTaken;
    health = std::max(0, health);  // Don't go below 0
    
    std::stringstream ss;
    ss << name << " took " << damageTaken << " damage. Health: " << health;
    Logger::info(ss.str());
    
    return damageTaken;
}

void Character::LevelUp(int exp) {
    level++;
    maxHealth = static_cast<int>(maxHealth * 1.2f);  // 20% increase
    health = maxHealth;
    attack = static_cast<int>(attack * 1.15f);       // 15% increase
    defense = static_cast<int>(defense * 1.15f);     // 15% increase
    speed = static_cast<int>(speed * 1.1f);          // 10% increase
    avoidance = std::min(40, static_cast<int>(avoidance * 1.1f)); // 10% increase, capped at 40%
}

void Character::PrintStats() const {
    std::stringstream ss;
    ss << "Name: " << name << "\n"
       << "Health: " << health << "\n"
       << "Attack: " << attack << "\n"
       << "Defense: " << defense << "\n"
       << "Speed: " << speed << "\n"
       << "Avoidance: " << avoidance << "\n"
       << "Position: " << x << " - " << y << "\n"
       << "Level: " << level << "\n"
       << "Exp.: " << experience;
    Logger::info(ss.str());
}

bool Battle::Fight(Character& character1, Character& character2) {
    Logger::info("\n=== BATTLE START ===");
    Logger::info(character1.GetName() + " (HP: " + std::to_string(character1.GetHealth()) + ") VS " + 
                character2.GetName() + " (HP: " + std::to_string(character2.GetHealth()) + ")");
    
    bool playerTurn = character1.GetSpeed() > character2.GetSpeed();
    bool battleEnded = false;
    bool escaped = false;

    while (!battleEnded) {
        if (playerTurn) {
            Logger::info("\nYour turn! Choose your action:");
            Logger::info("1. Attack");
            Logger::info("2. Try to escape");
            
            // Wait for input (this will be handled by the game state)
            // For now, always attack
            int damage = character1.GetAttack();
            character2.TakeDamage(damage);
            
            Logger::info("\n" + character1.GetName() + " attacks " + character2.GetName() + "!");
            Logger::info("Enemy HP: " + std::to_string(character2.GetHealth()) + "/" + 
                        std::to_string(character2.GetMaxHealth()));
            
            if (character2.IsDefeated()) {
                battleEnded = true;
                break;
            }
        } else {
            // Enemy turn
            Logger::info("\nEnemy's turn!");
            int damage = character2.GetAttack();
            character1.TakeDamage(damage);
            
            Logger::info(character2.GetName() + " attacks " + character1.GetName() + "!");
            Logger::info("Your HP: " + std::to_string(character1.GetHealth()) + "/" + 
                        std::to_string(character1.GetMaxHealth()));
            
            if (character1.IsDefeated()) {
                battleEnded = true;
                break;
            }
        }
        playerTurn = !playerTurn;
    }

    bool result = !character1.IsDefeated();
    Logger::info("\n=== BATTLE " + std::string(result ? "WON!" : "LOST!") + " ===\n");
    return result;
}

void Battle::Reward(Character& winner, Character& loser) {
    int exp = loser.GetLevel() * 5;  // Base experience from defeated enemy
    if (loser.GetBoss()) {
        exp *= 2;  // Double experience for defeating a boss
    }
    winner.AddExperience(exp);  // Use AddExperience instead of direct LevelUp call
}

Map::Map(int width, int height)
    : width(width), height(height), 
      grid(width, std::vector<Character*>(height)),
      item_grid(width, std::vector<std::shared_ptr<Item>>(height)),
      rng(std::random_device()()) {
    PopulateMonsters(5);
    PopulateBoss();
    PopulateItems(8); // Add some items to the map
}

void Map::PlaceCharacter(Character& character) {
    std::uniform_int_distribution<int> distX(0, width - 1);
    std::uniform_int_distribution<int> distY(0, height - 1);
    int x = distX(rng);
    int y = distY(rng);

    while (grid[x][y] != nullptr) {
        x = distX(rng);
        y = distY(rng);
    }

    grid[x][y] = &character;
    character.SetX(x);
    character.SetY(y);
    
    Logger::info("Placed " + character.GetName() + " at position (" + 
                std::to_string(x) + ", " + std::to_string(y) + ")");
}

void Map::MoveCharacter(Character& character, int dx, int dy) {
    if ((dx != 0 && dy != 0) || (dx == 0 && dy == 0)) {
        Logger::info("Invalid move!");
        return;
    }

    int x = character.GetX();
    int y = character.GetY();
    int newX = x + dx;
    int newY = y + dy;

    if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
        Logger::info("Move out of bounds!");
        return;
    }

    if (grid[newX][newY] != nullptr) {
        Logger::info("Position occupied!");
        return;
    }

    grid[newX][newY] = &character;
    grid[x][y] = nullptr;
    character.SetX(newX);
    character.SetY(newY);
    
    Logger::info(character.GetName() + " moved to position (" + 
                std::to_string(newX) + ", " + std::to_string(newY) + ")");
}

void Map::PopulateMonsters(int n) {
    for (int i = 0; i < n; i++) {
        std::string name = "Monster lvl" + std::to_string(i);
        // Monsters get progressively stronger
        float difficultyMult = 1.0f + (i * 0.2f);  // Each monster is 20% stronger than the last
        
        int health = static_cast<int>(80 * difficultyMult);
        int attack = static_cast<int>(25 * difficultyMult);
        int defence = static_cast<int>(15 * difficultyMult);
        int speed = static_cast<int>(15 * difficultyMult);
        int avoidance = static_cast<int>(10 * difficultyMult);
        
        Character* monster = new Character(name, health, attack, defence, speed, avoidance);
        monster->SetLevel(i + 1);
        PlaceCharacter(*monster);
    }
}

void Map::PopulateBoss() {
    std::string name = GenerateRandomName();
    // Boss is significantly stronger
    int health = 200;
    int attack = 45;
    int defence = 25;
    int speed = 20;
    int avoidance = 15;
    
    Character* boss = new Character(name, health, attack, defence, speed, avoidance);
    boss->SetLevel(5);
    boss->SetBoss();
    PlaceCharacter(*boss);
}

std::string Map::GenerateRandomName() {
    std::string names[] = {
        "Gorath the Destroyer",
        "Zarak the Necromancer",
        "Korgath the Conqueror",
        "Morgath the Defiler",
        "Vorgath the Annihilator"
    };
    return names[rand() % 5];
}

int Map::GenerateRandomStat(int min, int max) {
    return min + (rand() % (max - min + 1));
}

void Map::RemoveEnemy(Character& enemy, int /*dx*/, int /*dy*/) {
    // Find and remove the enemy from the grid
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == &enemy) {
                grid[y][x] = nullptr;
                return;
            }
        }
    }
}

Character* Map::CheckNewPosition(Character& mainCharacter, int dx, int dy) {
    int newX = mainCharacter.GetX() + dx;
    int newY = mainCharacter.GetY() + dy;
    
    // Check boundaries
    if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
        return nullptr;
    }
    
    if (grid[newX][newY] != nullptr) {
        Logger::info("Found enemy: " + grid[newX][newY]->GetName());
        return grid[newX][newY];
    }
    return nullptr;
}

std::shared_ptr<Item> Map::GetItemAtPosition(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return item_grid[x][y];
    }
    return nullptr;
}

void Map::RemoveItemAtPosition(int x, int y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        item_grid[x][y] = nullptr;
    }
}

void Map::PlaceItem(std::shared_ptr<Item> item) {
    std::uniform_int_distribution<int> distX(0, width - 1);
    std::uniform_int_distribution<int> distY(0, height - 1);
    int x = distX(rng);
    int y = distY(rng);

    while (grid[x][y] != nullptr || item_grid[x][y] != nullptr) {
        x = distX(rng);
        y = distY(rng);
    }

    item_grid[x][y] = item;
    Logger::info("Placed item " + item->GetName() + " at position (" + 
                std::to_string(x) + ", " + std::to_string(y) + ")");
}

std::vector<std::shared_ptr<Item>> Map::CreateRandomItems(int count) {
    std::vector<std::shared_ptr<Item>> items;
    
    // Define possible items
    std::vector<std::shared_ptr<Item>> possibleItems = {
        // Potions
        std::make_shared<Item>("Health Potion", "Restores 30 HP", ItemType::POTION, 30),
        std::make_shared<Item>("Greater Health Potion", "Restores 50 HP", ItemType::POTION, 50),
        std::make_shared<Item>("Strength Potion", "Temporarily increases attack by 10", ItemType::POTION, 10),
        
        // Weapons
        std::make_shared<Item>("Sharp Sword", "Increases attack by 15", ItemType::WEAPON, 15),
        std::make_shared<Item>("Battle Axe", "Increases attack by 20", ItemType::WEAPON, 20),
        std::make_shared<Item>("Legendary Blade", "Increases attack by 30", ItemType::WEAPON, 30),
        
        // Objects
        std::make_shared<Item>("Boss Compass", "Reveals the boss location", ItemType::OBJECT, 0, ObjectEffect::REVEAL_BOSS),
        std::make_shared<Item>("Monster Radar", "Reveals all monsters", ItemType::OBJECT, 0, ObjectEffect::REVEAL_MONSTERS),
        std::make_shared<Item>("Treasure Map", "Reveals all items", ItemType::OBJECT, 0, ObjectEffect::REVEAL_ITEMS)
    };

    // Randomly select items
    std::uniform_int_distribution<int> dist(0, possibleItems.size() - 1);
    for (int i = 0; i < count; ++i) {
        items.push_back(possibleItems[dist(rng)]);
    }

    return items;
}

void Map::PopulateItems(int n) {
    auto items = CreateRandomItems(n);
    for (auto& item : items) {
        PlaceItem(item);
    }
} 