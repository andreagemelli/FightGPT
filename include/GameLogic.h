#pragma once

#include <string>
#include <vector>
#include <random>
#include <memory>

class Character {
protected:
    std::string name;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int speed;
    int avoidance;
    int level;
    int experience;
    int x;
    int y;
    bool boss;
    bool isWounded;

public:
    Character() {}

    Character(const std::string& name, int health, int attack, int defense, int speed, int avoidance)
        : name(name), 
          health(health), 
          maxHealth(health), 
          attack(attack), 
          defense(defense),
          speed(speed), 
          avoidance(avoidance), 
          level(1),
          experience(0),
          x(0), 
          y(0),
          boss(false),
          isWounded(false) {}

    virtual ~Character() = default;

    // Returns actual damage dealt, or 0 if avoided
    virtual int TakeDamage(int damage);
    void LevelUp(int exp = 0);
    void ResetHealth() { health = maxHealth; }
    bool IsDefeated() { return health <= 0; }
    void PrintStats() const;

    int GetX() { return x; }
    int GetY() { return y; }
    void SetX(int new_x) { x = new_x; }
    void SetY(int new_y) { y = new_y; }
    int GetSpeed() { return speed; }
    int GetAttack() { return attack; }
    int GetLevel() { return level; }
    void SetLevel(int i) { level = i; }
    std::string GetName() { return name; }
    void SetBoss() { boss = true; }
    bool GetBoss() { return boss; }
    int GetHealth() { return health; }
    int GetMaxHealth() { return maxHealth; }
    int GetDefense() { return defense; }
    int GetExperience() const { return experience; }
    int GetAvoidance() const { return avoidance; }
    bool IsWounded() const { return isWounded; }
    void SetWounded(bool wounded) { isWounded = wounded; }
    void ApplyBleedDamage() {
        if (isWounded) {
            health = std::max(1, health - 5);
        }
    }

    void AddExperience(int exp) {
        experience += exp;
        while (experience >= level * 10) {
            LevelUp(exp);
        }
    }
};

class Battle {
public:
    static bool Fight(Character& character1, Character& character2);
    static void Reward(Character& winner, Character& loser);
};

class Map {
private:
    int width;
    int height;
    std::vector<std::vector<Character*>> grid;
    std::mt19937 rng;

public:
    Map(int width, int height);
    void PlaceCharacter(Character& character);
    void MoveCharacter(Character& character, int dx, int dy);
    void PopulateMonsters(int n);
    void PopulateBoss();
    std::string GenerateRandomName();
    int GenerateRandomStat(int min, int max);
    void RemoveEnemy(Character& enemy, int dx, int dy);
    Character* CheckNewPosition(Character& mainCharacter, int dx, int dy);
}; 