#include <iostream>
#include <cstdlib>

// Base class for all characters
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

  public:
    Character(){}

    Character(std::string name, int health, int attack, int defense, int speed, int avoidance)
        : name(name), health(health), attack(attack), defense(defense), speed(speed), avoidance(avoidance) {
            level = 1;
            experience = 0;
            x = 0;
            y = 0;
            maxHealth = health;
            boss = false;
          }

    // Virtual function for taking damage
    virtual void TakeDamage(int damage) {
      // Check if the character should avoid the attack
      if (rand() % 100 < avoidance) {
        std::cout << name << " avoided the attack!" << std::endl;
        return;
      }

      // Calculate the amount of damage to apply based on defense
      int damageTaken = damage / (defense + 1);
      health -= damageTaken;

      std::cout << name <<" health: "<< health << std::endl;
    }

    // Function for increasing level and attributes
    void LevelUp(int exp) {

      experience += exp;

      // Check if the character has enough experience to level up
      while (experience >= 10 * (level + 1)) {
        level++;

        // Increase attributes by 10%
        health *= 1.1;
        maxHealth = health;
        attack *= 1.1;
        defense *= 1.1;
        speed *= 1.1;
        avoidance *= 1.1;
      }

      std::cout << "Gained " << exp << " exp and reached lvl " << level << std::endl;
    }

    void ResetHealth() { health = maxHealth; }

    // Function for checking if the character is defeated
    bool IsDefeated() { return health <= 0; }

    void PrintStats() const {
    std::cout << "Name: " << name << std::endl;
    std::cout << "Health: " << health << std::endl;
    std::cout << "Attack: " << attack << std::endl;
    std::cout << "Defense: " << defense << std::endl;
    std::cout << "Speed: " << speed << std::endl;
    std::cout << "Avoidance: " << avoidance << std::endl;
    std::cout << "Position: " << x << " - " << y << std::endl;
    std::cout << "Level: " << level << std::endl;
    std::cout << "Exp.: " << experience << std::endl;
  }

    // Getters and Setters
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
    
};
