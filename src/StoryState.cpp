#include "StoryState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

StoryState::StoryState(int selectedCharacter, const std::string& playerName, const std::string& bossName)
    : selectedCharacter(selectedCharacter), 
      playerName(playerName), 
      bossName(bossName),
      textFadeIn(0.0f),
      pulseEffect(0.0f),
      continueTextDelay(0.0f) {
    
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Create dark overlay background
    background.setSize(sf::Vector2f(1200, 800));  // Window size
    background.setFillColor(sf::Color(0, 0, 0, 255));  // Pure black

    // Set up story text with padding
    const float horizontalPadding = 40.0f;  // Reduced padding
    const float verticalPadding = 40.0f;    // Reduced padding
    storyText.setFont(font);
    storyText.setCharacterSize(18);         // Even smaller font size
    storyText.setFillColor(sf::Color(255, 255, 255, 0));  // Start fully transparent
    storyText.setLineSpacing(1.3f);  // Slightly reduced line spacing

    // Generate the story content
    generateStory();

    // Format the text to fit within the screen
    formatText();

    // Get the formatted text bounds
    sf::FloatRect textBounds = storyText.getLocalBounds();

    // Create a semi-transparent text background with padding
    const float maxBackgroundWidth = 500.0f;  // Reduced maximum width
    textBackground.setSize(sf::Vector2f(
        std::min(maxBackgroundWidth, textBounds.width + (horizontalPadding * 2)),
        textBounds.height + (verticalPadding * 2)
    ));
    textBackground.setFillColor(sf::Color(0, 0, 0, 200));
    textBackground.setOrigin(textBackground.getSize().x / 2.0f, textBackground.getSize().y / 2.0f);
    textBackground.setPosition(1200 / 2.0f, 800 / 2.0f);

    // Center the text within the background
    storyText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    storyText.setPosition(1200 / 2.0f, 800 / 2.0f);

    // Set up the continue text
    continueText.setFont(font);
    continueText.setString("Press ENTER to begin your quest...");
    continueText.setCharacterSize(20);  // Smaller continue text
    continueText.setFillColor(sf::Color(255, 255, 255, 0));  // Start invisible
    
    // Center the continue text at the bottom of the text background
    sf::FloatRect continueBounds = continueText.getLocalBounds();
    continueText.setOrigin(continueBounds.width / 2.0f, continueBounds.height / 2.0f);
    continueText.setPosition(1200 / 2.0f, textBackground.getPosition().y + (textBackground.getSize().y / 2.0f) + 25.0f);
}

void StoryState::generateStory() {
    std::string classType;
    std::string questDescription;
    
    switch (selectedCharacter) {
        case 0: // Knight
            classType = "noble knight";
            questDescription = "Your martial prowess and unwavering courage may be the key to escaping this cursed realm.";
            break;
        case 1: // Mage
            classType = "skilled mage";
            questDescription = "Your mastery of the arcane arts could be the power needed to break free from this dark dimension.";
            break;
        case 2: // Archer
            classType = "swift archer";
            questDescription = "Your deadly precision and agility might be the perfect skills to survive and escape this nightmare.";
            break;
    }

    std::stringstream ss;
    ss << "In the depths of an ancient dungeon, where reality bends and darkness reigns supreme...\n\n"
       << "You, " << playerName << ", a " << classType << " from the realm of light, "
       << "have been pulled through a malevolent portal while investigating strange occurrences in your homeland.\n\n"
       << "This cursed place is known as the Shifting Dungeons, a prison dimension ruled by " << bossName 
       << ", a being of pure malice who feeds on the essence of trapped warriors.\n\n"
       << "For centuries, " << bossName << " has used these dungeons to lure mighty heroes, draining their power "
       << "to maintain their immortality. The walls themselves pulse with the trapped souls of fallen champions.\n\n"
       << questDescription << "\n\n"
       << "To escape this nightmare and prevent " << bossName << " from threatening your world, "
       << "you must grow stronger by defeating the dungeon's corrupted denizens, "
       << "find powerful artifacts to aid your quest, and ultimately face the tyrant in combat.\n\n"
       << "But beware - death here means your soul will join the countless others, forever powering the dungeon's dark magic...\n\n"
       << "Press ENTER to begin your quest...";

    storyText.setString(ss.str());
}

void StoryState::formatText() {
    // Set a fixed width for the text, accounting for padding
    const float maxWidth = 420.0f;  // Further reduced width of the text block
    
    std::string originalText = storyText.getString();
    std::string formattedText;
    std::string currentLine;
    std::istringstream words(originalText);
    std::string word;
    
    float currentLineWidth = 0.0f;
    
    // Create a temporary text object for width measurements
    sf::Text tempText;
    tempText.setFont(font);
    tempText.setCharacterSize(18);  // Match the story text size
    
    while (std::getline(words, word, ' ')) {
        // Handle newlines in the original text
        size_t newlinePos = word.find("\\n");
        if (newlinePos != std::string::npos) {
            if (newlinePos == 0) {
                formattedText += "\n";
            } else {
                formattedText += currentLine + word.substr(0, newlinePos) + "\n";
                currentLine = "";
            }
            currentLineWidth = 0.0f;
            continue;
        }
        
        // Measure this word
        tempText.setString(word + " ");
        float wordWidth = tempText.getLocalBounds().width;
        
        if (currentLineWidth + wordWidth > maxWidth) {
            // Line would be too long, add a newline
            formattedText += currentLine + "\n";
            currentLine = word + " ";
            currentLineWidth = wordWidth;
        } else {
            // Add to current line
            currentLine += word + " ";
            currentLineWidth += wordWidth;
        }
    }
    
    // Add the last line
    formattedText += currentLine;
    
    // Set the formatted text
    storyText.setString(formattedText);
}

void StoryState::handleEvent(const sf::Event& event, sf::RenderWindow& /*window*/) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
        nextState = std::make_unique<GamePlayState>(selectedCharacter, playerName, bossName);
    }
}

void StoryState::update(float deltaTime) {
    // Fade in text gradually
    if (textFadeIn < 1.0f) {
        textFadeIn += deltaTime * 2.0f; // Adjust speed by changing multiplier
        if (textFadeIn > 1.0f) textFadeIn = 1.0f;

        // Update story text opacity
        sf::Color storyColor = storyText.getFillColor();
        storyColor.a = static_cast<sf::Uint8>(255 * textFadeIn);
        storyText.setFillColor(storyColor);
    }

    // Handle continue text delay and effects
    if (textFadeIn >= 1.0f) {  // Only start counting delay after main text is fully visible
        continueTextDelay += deltaTime;
        
        if (continueTextDelay >= 5.0f) {  // After 5 seconds
            // Start pulsing the continue text
            pulseEffect += deltaTime * 3.0f;  // Control pulse speed
            float alpha = (std::sin(pulseEffect) + 1.0f) / 2.0f;  // Oscillate between 0 and 1
            
            // Apply pulse effect to continue text
            sf::Color textColor = continueText.getFillColor();
            textColor.a = static_cast<sf::Uint8>(155 + 100 * alpha);  // Pulse between 155 and 255 alpha
            continueText.setFillColor(textColor);
        } else {
            // Keep continue text invisible until delay is over
            continueText.setFillColor(sf::Color(255, 255, 255, 0));
        }
    }
}

void StoryState::draw(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(textBackground);
    window.draw(storyText);
    window.draw(continueText);
} 