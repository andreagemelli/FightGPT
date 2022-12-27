#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "character.cpp"

// Include the Map class
#include "map.cpp"

// Include the Battle class
#include "battle.cpp"

int main() {
  std::string name;
  std::cout << "Enter your character's name: ";
  std::cin >> name;

  std::cout << "Choose your character:" << std::endl;
  std::cout << "1. Knight, hard to kill" << std::endl;
  std::cout << "2. Mage, sustained damage" << std::endl;
  std::cout << "3. Archer, mixed stats and escape skills" << std::endl;
  int characterClass;
  std::cin >> characterClass;

  Character mainCharacter;
  if (characterClass == 1) {
    mainCharacter = Character(name + " the Knight", 100, 20, 15, 5, 5);
  } else if (characterClass == 2) {
    mainCharacter = Character(name + " the Mage", 50, 50, 5, 10, 10);
  } else if (characterClass == 3) {
    mainCharacter = Character(name + " the Archer", 75, 35, 10, 20, 15);
  } else {
    std::cout << "Invalid character class" << std::endl;
    return 1;
  }
  
  std::cout << std::endl;
    
  Map map(5, 5);
  map.PlaceCharacter(mainCharacter);
  mainCharacter.PrintStats();
  
  std::cout << "You have been teleported into a devil world: fight to the death and kill the boss to escape! Good luck" << std::endl;
  bool gameOver = false;
  bool bossDefeated = false;

  while (!gameOver) {
    // Display the map and get the player's next move
    // TODO add graphic
    // map.Display();
    std::cout <<  std::endl;
    std::cout << "Enter your next move (u/d/l/r): ";
    char move;
    std::cin >> move;

    // Move the main character
    int dx = 0;
    int dy = 0;
    if (move == 'u') {
      std::cout << "Moving Up!" << std::endl;
      dx = -1;
    } else if (move == 'd') {
        std::cout << "Moving Down!" << std::endl;
      dx = 1;
    } else if (move == 'l') {
      std::cout << "Moving Left!" << std::endl;
      dy = -1;
    } else if (move == 'r') {
      std::cout << "Moving Right!" << std::endl;
      dy = 1;
    } else {
      std::cout << "Invalid move" << std::endl;
      continue;
    }

    // Check if an enemy was encountered
    Character* enemy = map.CheckNewPosition(mainCharacter, dx, dy);
    if (enemy != nullptr) {
      // Start a fight with the encountered enemy
      std::cout << "Want to fight? (Y/n): ";
      // TODO add escape dices
      char fight;
      std::cin >> fight;
      if (fight != 'Y') { continue; }
      Battle battle;
      bool victory = battle.Fight(mainCharacter, *enemy);
      // TODO if win against Boss, win the game
      if (victory) {
        // Reward the main character and restore their health
        std::cout << std::endl;
        std::cout << mainCharacter.GetName() << " defeated " << enemy->GetName() << std::endl;
        mainCharacter.ResetHealth();
        battle.Reward(mainCharacter, *enemy);
        map.RemoveEnemy(*enemy, dx, dy);
        if (enemy->GetBoss()){
            gameOver = true;
            bossDefeated = true;
        }
      } else {
        // The main character was defeated, game over
        gameOver = true;
      }
    }
    map.MoveCharacter(mainCharacter, dx, dy);
  }
  
  if (bossDefeated) {
    std::cout << "You defeated the boss of this evil world and survived! You won!" << std::endl;
  }
  else {
    std::cout << mainCharacter.GetName() << " was defeated: Game Over!" << std::endl;
  }
  return 0;
}
