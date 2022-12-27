// Battle class
class Battle {
  public:
    // Function for fighting between two characters
    static bool Fight(Character& character1, Character& character2) {
      // Continue fighting until one character is defeated
      while (!character1.IsDefeated() && !character2.IsDefeated()) {
        // Determine which character attacks first based on GetSpeed
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
    
      if (character1.IsDefeated()){
        return false;
      }
      else {
        return true;
      }
    }

    // Function for rewarding the winning character with experience
    static void Reward(Character& winner, Character& loser) {
      // Give the winner some experience based on the loser's level
      winner.LevelUp(loser.GetLevel() * 10);
      winner.PrintStats();
    }
};
