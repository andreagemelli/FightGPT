#include "CharacterSelectionState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

CharacterSelectionState::CharacterSelectionState() : selectedOption(0) {
    if (!font.loadFromFile("assets/fonts/Jacquard12-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Set up title
    title.setFont(font);
    title.setString("Select Your Character");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color(255, 215, 0)); // Gold color
    
    // Set up character options
    const std::string descriptions[3] = {
        "Knight\nHigh HP and Defense\nBalanced Attack",
        "Mage\nHigh Attack Power\nLow Defense\nMagic Abilities",
        "Archer\nHigh Speed\nMedium Attack\nRanged Combat"
    };

    for (int i = 0; i < 3; ++i) {
        options[i].setFont(font);
        options[i].setString(descriptions[i]);
        options[i].setCharacterSize(20);
        options[i].setFillColor(sf::Color(220, 220, 220));

        optionBoxes[i].setSize(sf::Vector2f(300, 120));
        optionBoxes[i].setFillColor(sf::Color(60, 60, 60));
        optionBoxes[i].setOutlineThickness(2);
    }

    updatePositions();
}

void CharacterSelectionState::updatePositions() {
    // Center the title
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(400 - titleBounds.width/2, 50);

    // Position the option boxes and text
    for (int i = 0; i < 3; ++i) {
        optionBoxes[i].setPosition(250, 150 + i * 140);
        
        // Center the text within the box
        sf::FloatRect textBounds = options[i].getLocalBounds();
        options[i].setPosition(
            250 + (300 - textBounds.width) / 2,
            150 + i * 140 + (120 - textBounds.height) / 2
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
                nextState = std::make_unique<GamePlayState>(selectedOption);
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
        window.draw(options[i]);
    }
} 