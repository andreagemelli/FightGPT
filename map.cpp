#include <iostream>
#include <vector>
#include <random>

// Forward declaration of Character class
class Character;

// Map class
class Map {
  private:
    int width;
    int height;
    std::vector<std::vector<Character*> > grid;
    std::mt19937 rng;

  public:
    Map(int width, int height)
        : width(width), height(height), grid(width, std::vector<Character*>(height)),
          rng(std::random_device()()) {
            PopulateMonsters(5);
            PopulateBoss();
          }

    // Function for placing a character on the map
    void PlaceCharacter(Character& character) {
      // Generate a random position
      std::uniform_int_distribution<int> distX(0, width - 1);
      std::uniform_int_distribution<int> distY(0, height - 1);
      int x = distX(rng);
      int y = distY(rng);

      // Check if the position is already occupied
      while (grid[x][y] != nullptr) {
        x = distX(rng);
        y = distY(rng);
      }

      // Place the character on the map
      grid[x][y] = &character;
      character.SetX(x);
      character.SetY(y);
    }

    // Function for moving a character on the map
    void MoveCharacter(Character& character, int dx, int dy) {
      // Check if the move is valid (only one position at a time)
      if (dx != 0 && dy != 0 || dx == 0 && dy == 0) {
        std::cout << "Invalid move!" << std::endl;
        return;
      }

      // Get the character's current position
      int x = character.GetX();
      int y = character.GetY();

      // Calculate the new position
      int newX = x + dx;
      int newY = y + dy;
      std::cout << "New Position: " << newX << " - " << newY << std::endl;

      // Check if the new position is within the map bounds
      if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
        std::cout << "Invalid move!" << std::endl;
        return;
      }

      // Check if the new position is occupied or invalid
      // TODO it blocks here!
      if (grid[newX][newY] != nullptr || grid[newX][newY] == (Character*) -1) {
        std::cout << "Position occupied or invalid!" << std::endl;
        return;
      }

      // Move the character to the new position
      grid[newX][newY] = &character;
      grid[x][y] = nullptr;
      character.SetX(newX);
      character.SetY(newY);
    }

    // PopulateMonsters creates n random monsters and places them on the map
  void PopulateMonsters(int n) {
    for (int i = 0; i < n; i++) {
      std::string name = "Monster " + std::to_string(i);
      int health = GenerateRandomStat(i, 100);
      int attack = GenerateRandomStat(i, 50);
      int defence = GenerateRandomStat(i, 15);
      int speed = GenerateRandomStat(i, 20);
      int avoidance = GenerateRandomStat(i, 15);
      Character* monster = new Character(name, attack, health, defence, speed, avoidance);
      monster->SetLevel(i);
      PlaceCharacter(*monster);
      // monsters_.push_back(monster);
    }
  }

  // PopulateBoss creates a random boss and places it on the map
  void PopulateBoss() {
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

  // GenerateRandomName returns a random fantasy name
  std::string GenerateRandomName() {
    // Generate a random number between 0 and 4
    int nameIndex = rand() % 5;
    std::string names[] = { "Gorath the Destroyer", "Zarak the Necromancer", "Korgath the Conqueror", "Morgath the Defiler", "Vorgath the Annihilator" };
    return names[nameIndex];
  }

  // GenerateRandomStat returns a random stat between min and max
  int GenerateRandomStat(int min, int max) {
    return min + (rand() % (max - min + 1));
  }

  void RemoveEnemy(Character& enemy, int dx, int dy){
    grid[enemy.GetX()][enemy.GetY()] = nullptr;
  }

  Character* CheckNewPosition(Character& mainCharacter, int dx, int dy) {
    // Check if the new position is within the map bounds
    int newX = mainCharacter.GetX() + dx;
    int newY = mainCharacter.GetY() + dy;
    if (grid[newX][newY] != nullptr) {
        std::cout << "Found an Enemy: " << grid[newX][newY]->GetName() << " wants to fight!" << std::endl;
        return grid[newX][newY];
    }

    // No enemy was found at the given position
    return nullptr;
  }
};
