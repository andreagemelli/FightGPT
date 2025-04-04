#include "NameInputState.h"
#include "CharacterSelectionState.h"
#include "Logger.h"

NameInputState::NameInputState() : playerName("") {
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Set up title
    titleText.setFont(font);
    titleText.setString("Enter Your Name");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    
    // Set up input box
    inputBox.setSize(sf::Vector2f(400, 50));
    inputBox.setFillColor(sf::Color(60, 60, 60));
    inputBox.setOutlineColor(sf::Color(100, 100, 100));
    inputBox.setOutlineThickness(2);
    
    // Set up input text
    inputText.setFont(font);
    inputText.setCharacterSize(32);
    inputText.setFillColor(sf::Color::White);
    
    // Set up instruction text
    instructionText.setFont(font);
    instructionText.setString("Press Enter when done");
    instructionText.setCharacterSize(24);
    instructionText.setFillColor(sf::Color(180, 180, 180));
    
    updateText();
}

void NameInputState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::TextEntered) {
        // Handle backspace
        if (event.text.unicode == 8 && !playerName.empty()) {
            playerName.pop_back();
        }
        // Handle regular text input
        else if (event.text.unicode < 128 && playerName.length() < MAX_NAME_LENGTH) {
            char inputChar = static_cast<char>(event.text.unicode);
            if (std::isalnum(inputChar) || inputChar == ' ') {
                playerName += inputChar;
            }
        }
        updateText();
    }
    else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Return && !playerName.empty()) {
            Logger::info("Player name set to: " + playerName);
            nextState = std::make_unique<CharacterSelectionState>(playerName);
        }
    }
}

void NameInputState::update(float deltaTime) {
    // Nothing to update
}

void NameInputState::updateText() {
    inputText.setString(playerName + "_");
    
    // Center everything
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(600 - titleBounds.width/2, 200);
    
    inputBox.setPosition(400, 300);
    
    sf::FloatRect textBounds = inputText.getLocalBounds();
    inputText.setPosition(
        400 + (400 - textBounds.width)/2,
        300 + (50 - textBounds.height)/2
    );
    
    sf::FloatRect instructionBounds = instructionText.getLocalBounds();
    instructionText.setPosition(
        600 - instructionBounds.width/2,
        380
    );
}

void NameInputState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(40, 40, 40));
    
    window.draw(titleText);
    window.draw(inputBox);
    window.draw(inputText);
    window.draw(instructionText);
} 