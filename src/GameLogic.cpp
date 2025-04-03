#include "GameLogic.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <random>

void Character::TakeDamage(int damage) {
    if (rand() % 100 < avoidance) {
        Logger::info(name + " avoided the attack!");
        return;
    }

    int damageTaken = damage / (defense + 1);
    health -= damageTaken;
    
    std::stringstream ss;
    ss << name << " took " << damageTaken << " damage. Health: " << health;
    Logger::info(ss.str());
}

void Character::LevelUp(int exp) {
    experience += exp;

    while (experience >= 10 * (level + 1)) {
        level++;
        health *= 1.1;
        maxHealth = health;
        attack *= 1.1;
        defense *= 1.1;
        speed *= 1.1;
        avoidance *= 1.1;
    }

    std::stringstream ss;
    ss << "Gained " << exp << " exp and reached lvl " << level;
    Logger::info(ss.str());
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
    Logger::info("Battle started between " + character1.GetName() + " and " + character2.GetName());
    
    while (!character1.IsDefeated() && !character2.IsDefeated()) {
        if (character1.GetSpeed() > character2.GetSpeed()) {
            character2.TakeDamage(character1.GetAttack());
            if (character2.IsDefeated()) break;
            character1.TakeDamage(character2.GetAttack());
        } else {
            character1.TakeDamage(character2.GetAttack());
            if (character1.IsDefeated()) break;
            character2.TakeDamage(character1.GetAttack());
        }
    }

    bool result = !character1.IsDefeated();
    Logger::info("Battle ended. Winner: " + (result ? character1.GetName() : character2.GetName()));
    return result;
}

void Battle::Reward(Character& winner, Character& loser) {
    int exp = loser.GetLevel() * 10;
    winner.LevelUp(exp);
    Logger::info(winner.GetName() + " gained " + std::to_string(exp) + " experience");
}

Map::Map(int width, int height)
    : width(width), height(height), grid(width, std::vector<Character*>(height)),
      rng(std::random_device()()) {
    PopulateMonsters(5);
    PopulateBoss();
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
        std::string name = "Monster " + std::to_string(i);
        int health = GenerateRandomStat(i, 100);
        int attack = GenerateRandomStat(i, 50);
        int defence = GenerateRandomStat(i, 15);
        int speed = GenerateRandomStat(i, 20);
        int avoidance = GenerateRandomStat(i, 15);
        Character* monster = new Character(name, health, attack, defence, speed, avoidance);
        monster->SetLevel(i);
        PlaceCharacter(*monster);
    }
}

void Map::PopulateBoss() {
    std::string name = GenerateRandomName();
    int health = GenerateRandomStat(50, 100);
    int attack = GenerateRandomStat(20, 50);
    int defence = GenerateRandomStat(15, 15);
    int speed = GenerateRandomStat(5, 20);
    int avoidance = GenerateRandomStat(5, 15);
    Character* boss = new Character(name, attack, health, defence, speed, avoidance);
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

void Map::RemoveEnemy(Character& enemy, int dx, int dy) {
    grid[enemy.GetX()][enemy.GetY()] = nullptr;
    Logger::info("Removed " + enemy.GetName() + " from the map");
}

Character* Map::CheckNewPosition(Character& mainCharacter, int dx, int dy) {
    int newX = mainCharacter.GetX() + dx;
    int newY = mainCharacter.GetY() + dy;
    
    if (grid[newX][newY] != nullptr) {
        Logger::info("Found enemy: " + grid[newX][newY]->GetName());
        return grid[newX][newY];
    }
    return nullptr;
} 