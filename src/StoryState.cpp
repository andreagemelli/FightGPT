#include "StoryState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

StoryState::StoryState(int selectedCharacter, const std::string& playerName)
    : playerName(playerName), selectedCharacter(selectedCharacter), textFadeIn(0.0f) {
    
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Set up background
    background.setSize(sf::Vector2f(1200, 800));
    background.setFillColor(sf::Color(0, 0, 0, 255));

    // Generate boss name
    const std::string bossFirstNames[] = {"Shadowlord", "Dreadking", "Nightbringer", "Soulreaver", "Doomweaver"};
    const std::string bossLastNames[] = {"Vex", "Morthul", "Grimm", "Darkfang", "Bloodthorn"};
    bossName = bossFirstNames[rand() % 5] + " " + bossLastNames[rand() % 5];

    // Set up story text
    storyText.setFont(font);
    storyText.setCharacterSize(24);
    storyText.setFillColor(sf::Color(255, 255, 255, 0)); // Start fully transparent

    // Set up continue text
    continueText.setFont(font);
    continueText.setString("Press ENTER to continue...");
    continueText.setCharacterSize(20);
    continueText.setFillColor(sf::Color(150, 150, 150, 0)); // Start fully transparent
    
    generateStory();
    formatText();
}

void StoryState::generateStory() {
    std::string classType;
    std::string questDescription;
    
    switch (selectedCharacter) {
        case 0: // Knight
            classType = "noble knight";
            questDescription = "Your first task is to find a legendary weapon in the ancient armory, "
                             "which lies in the eastern part of this cursed realm.";
            break;
        case 1: // Mage
            classType = "skilled mage";
            questDescription = "Your first task is to locate the arcane scrolls of power, "
                             "hidden within the forgotten library to the east.";
            break;
        case 2: // Archer
            classType = "swift archer";
            questDescription = "Your first task is to retrieve the enchanted arrows "
                             "from the sacred grove in the eastern forests.";
            break;
    }

    std::stringstream ss;
    ss << "In a world where light once prevailed, darkness has taken hold...\n\n"
       << playerName << ", a " << classType << " from the realm of peace, "
       << "you have been pulled through a mysterious portal into this corrupted land.\n\n"
       << "This cursed realm is ruled by " << bossName << ", "
       << "a terrifying entity whose very presence drains the life from the land. "
       << "Known for commanding legions of twisted creatures and wielding powers that defy reality itself, "
       << "the tyrant has enslaved countless souls.\n\n"
       << "To return home, you must defeat " << bossName << " and break their hold over this world. "
       << "But first, you must grow stronger...\n\n"
       << questDescription;

    storyText.setString(ss.str());
}

void StoryState::formatText() {
    // Add horizontal padding to story text
    const float horizontalPadding = 200.0f; // 200 pixels on each side
    
    // Center the story text with padding
    sf::FloatRect textBounds = storyText.getLocalBounds();
    storyText.setPosition(
        horizontalPadding,
        (800 - textBounds.height) / 2 - 50
    );

    // Adjust text width to account for padding
    float maxWidth = 1200 - (horizontalPadding * 2);
    std::string currentText = storyText.getString();
    
    // Word wrap the text to fit within the padding
    std::string wrappedText;
    std::istringstream words(currentText);
    std::string word;
    float currentLineWidth = 0;
    while (std::getline(words, word, ' ')) {
        sf::Text tempText;
        tempText.setFont(font);
        tempText.setCharacterSize(24);
        tempText.setString(word + " ");
        float wordWidth = tempText.getLocalBounds().width;
        
        if (currentLineWidth + wordWidth > maxWidth) {
            wrappedText += "\n" + word + " ";
            currentLineWidth = wordWidth;
        } else {
            wrappedText += word + " ";
            currentLineWidth += wordWidth;
        }
    }
    
    storyText.setString(wrappedText);

    // Position continue text at the bottom
    sf::FloatRect continueBounds = continueText.getLocalBounds();
    continueText.setPosition(
        (1200 - continueBounds.width) / 2,
        700
    );
}

void StoryState::handleEvent(const sf::Event& event, sf::RenderWindow& /*window*/) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
        // Only allow proceeding if text is fully visible
        if (textFadeIn >= 1.0f) {
            nextState = std::make_unique<GamePlayState>(selectedCharacter, playerName);
        }
    }
}

void StoryState::update(float deltaTime) {
    // Fade in text gradually
    if (textFadeIn < 1.0f) {
        textFadeIn += deltaTime * 0.5f; // Adjust speed by changing multiplier
        if (textFadeIn > 1.0f) textFadeIn = 1.0f;

        int alpha = static_cast<int>(255 * textFadeIn);
        storyText.setFillColor(sf::Color(255, 255, 255, alpha));
        continueText.setFillColor(sf::Color(150, 150, 150, alpha));
    }
}

void StoryState::draw(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(storyText);
    window.draw(continueText);
} 