#pragma once

#include <string>
#include <vector>
#include <random>
#include <memory>

enum class ItemType {
    POTION,
    WEAPON,
    OBJECT
};

enum class ObjectEffect {
    REVEAL_BOSS,
    REVEAL_ITEMS,
    REVEAL_MONSTERS
};

class Item {
private:
    std::string name;
    std::string description;
    ItemType type;
    int effect_value;
    ObjectEffect object_effect;

public:
    Item(const std::string& name, const std::string& description, ItemType type, int effect_value = 0, ObjectEffect object_effect = ObjectEffect::REVEAL_BOSS)
        : name(name), description(description), type(type), effect_value(effect_value), object_effect(object_effect) {}

    std::string GetName() const { return name; }
    std::string GetDescription() const { return description; }
    ItemType GetType() const { return type; }
    int GetEffectValue() const { return effect_value; }
    ObjectEffect GetObjectEffect() const { return object_effect; }
};

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
    std::vector<std::shared_ptr<Item>> inventory;
    std::shared_ptr<Item> equipped_weapon;
    static const int MAX_INVENTORY_SIZE = 4;
    
    // Special ability properties
    bool abilityUsed;
    bool isRage;        // For Knight: when true, forces critical hit
    int markerCount;    // For Archer: counts remaining guaranteed hits
    bool burnActive;    // For Mage: tracks if enemy is burning

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
          isWounded(false),
          abilityUsed(false),
          isRage(false),
          markerCount(0),
          burnActive(false) {}

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

    // New inventory methods
    bool AddItem(std::shared_ptr<Item> item) {
        if (inventory.size() < MAX_INVENTORY_SIZE) {
            inventory.push_back(item);
            return true;
        }
        return false;
    }

    bool RemoveItem(int index) {
        if (index >= 0 && static_cast<size_t>(index) < inventory.size()) {
            inventory.erase(inventory.begin() + index);
            return true;
        }
        return false;
    }

    const std::vector<std::shared_ptr<Item>>& GetInventory() const {
        return inventory;
    }

    void EquipWeapon(int index) {
        if (index >= 0 && static_cast<size_t>(index) < inventory.size() && 
            inventory[index]->GetType() == ItemType::WEAPON) {
            equipped_weapon = inventory[index];
        }
    }

    std::shared_ptr<Item> GetEquippedWeapon() const {
        return equipped_weapon;
    }

    int GetTotalAttack() const {
        int total = attack;
        if (equipped_weapon) {
            total += equipped_weapon->GetEffectValue();
        }
        return total;
    }

    // Special ability methods
    bool HasUsedAbility() const { return abilityUsed; }
    void SetAbilityUsed(bool used) { abilityUsed = used; }
    
    void ResetAbility() { 
        abilityUsed = false;
        isRage = false;
        markerCount = 0;
        burnActive = false;
    }
    
    // Rage ability
    bool IsRageActive() const { return isRage; }
    void ActivateRage() { 
        isRage = true;
        abilityUsed = true;
    }
    void DeactivateRage() { isRage = false; }
    
    // Hunter's Mark ability
    bool HasMarker() const { return markerCount > 0; }
    int GetMarkerCount() const { return markerCount; }
    void ActivateMarker() {
        markerCount = 3;
        abilityUsed = true;
    }
    void DecrementMarker() {
        if (markerCount > 0) {
            markerCount--;
        }
    }
    
    // Burn ability
    bool IsBurning() const { return burnActive; }
    void ApplyBurn() {
        burnActive = true;
    }
    void ApplyBurnDamage() {
        if (burnActive) {
            health = std::max(1, health - 5); // 5 HP burn damage per turn
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
    std::vector<std::vector<std::shared_ptr<Item>>> item_grid;
    std::mt19937 rng;

public:
    Map(int width, int height);
    void PlaceCharacter(Character& character);
    void MoveCharacter(Character& character, int dx, int dy);
    void PopulateMonsters(int n);
    void PopulateBoss();
    void PopulateItems(int n);
    std::string GenerateRandomName();
    int GenerateRandomStat(int min, int max);
    void RemoveEnemy(Character& enemy, int dx, int dy);
    Character* CheckNewPosition(Character& mainCharacter, int dx, int dy);
    std::shared_ptr<Item> GetItemAtPosition(int x, int y) const;
    void RemoveItemAtPosition(int x, int y);
    void PlaceItem(std::shared_ptr<Item> item);
    std::vector<std::shared_ptr<Item>> CreateRandomItems(int count);
    Character* GetCharacterAt(int x, int y) const { return grid[x][y]; }
}; 