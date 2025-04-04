#include "CharacterSelectionState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

CharacterSelectionState::CharacterSelectionState(const std::string& playerName) 
    : selectedOption(0), playerName(playerName) {
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Set up title
    title.setFont(font);
    title.setString("Choose Your Class, " + playerName);
    title.setCharacterSize(32);
    title.setFillColor(sf::Color(255, 215, 0)); // Gold color
    
    // Load class icons
    loadIcons();
    
    // Set up character options
    const std::string descriptions[3] = {
        "Knight\n\nHigh HP and Defense\nBalanced Attack",
        "Mage\n\nHigh Attack Power\nLow Defense",
        "Archer\n\nHigh Speed\nMedium Attack"
    };

    for (int i = 0; i < 3; ++i) {
        options[i].setFont(font);
        options[i].setString(descriptions[i]);
        options[i].setCharacterSize(24);
        options[i].setFillColor(sf::Color(220, 220, 220));

        optionBoxes[i].setSize(sf::Vector2f(350, 140));
        optionBoxes[i].setFillColor(sf::Color(60, 60, 60));
        optionBoxes[i].setOutlineThickness(2);
    }

    updatePositions();
}

void CharacterSelectionState::loadIcons() {
    const std::string iconPaths[3] = {
        "assets/icons/sword.png",    // Knight
        "assets/icons/hat.png",      // Mage
        "assets/icons/bow.png"       // Archer
    };

    for (int i = 0; i < 3; ++i) {
        if (!classIcons[i].loadFromFile(iconPaths[i])) {
            Logger::error("Failed to load icon: " + iconPaths[i]);
            continue;
        }
        iconSprites[i].setTexture(classIcons[i]);
        
        // Scale icon to reasonable size (48x48 pixels)
        float scale = 48.0f / std::max(classIcons[i].getSize().x, classIcons[i].getSize().y);
        iconSprites[i].setScale(scale, scale);
    }
}

void CharacterSelectionState::updatePositions() {
    // Center the title
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(400 - titleBounds.width/2, 50);

    // Position the option boxes, text and icons
    for (int i = 0; i < 3; ++i) {
        optionBoxes[i].setPosition(250, 150 + i * 140);
        
        // Position icon on the left side of the box
        iconSprites[i].setPosition(260, 150 + i * 140 + 36); // Center vertically in box
        
        // Center the text within the box, but leave space for the icon
        sf::FloatRect textBounds = options[i].getLocalBounds();
        options[i].setPosition(
            320, // Start text after icon
            150 + i * 140 + (140 - textBounds.height) / 2
        );
    }
}

void CharacterSelectionState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
                if (selectedOption > 0) selectedOption--;
                break;
            case sf::Keyboard::Down:
                if (selectedOption < 2) selectedOption++;
                break;
            case sf::Keyboard::Return:
                Logger::info("Selected character: " + std::to_string(selectedOption));
                nextState = std::make_unique<GamePlayState>(selectedOption, playerName);
                break;
            default:
                break;
        }
    }
}

void CharacterSelectionState::update(float deltaTime) {
    // Update the option boxes' colors
    for (int i = 0; i < 3; ++i) {
        if (i == selectedOption) {
            optionBoxes[i].setOutlineColor(sf::Color(255, 215, 0)); // Gold color for selected
        } else {
            optionBoxes[i].setOutlineColor(sf::Color(100, 100, 100));
        }
    }
}

void CharacterSelectionState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(40, 40, 40));

    window.draw(title);
    
    for (int i = 0; i < 3; ++i) {
        window.draw(optionBoxes[i]);
        window.draw(iconSprites[i]);
        window.draw(options[i]);
    }
} 