#include "NameInputState.h"
#include "CharacterSelectionState.h"
#include "Logger.h"

NameInputState::NameInputState() : windowWidth(1200), windowHeight(800), playerName("") {
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Load logo
    if (!logoTexture.loadFromFile("assets/logo.png")) {
        Logger::error("Failed to load logo!");
    } else {
        logoSprite.setTexture(logoTexture);
        // Scale logo to reasonable size (300px width)
        float scaleX = 300.0f / logoTexture.getSize().x;
        float scaleY = 300.0f / logoTexture.getSize().y;
        logoSprite.setScale(scaleX, scaleY);
        float logoY = 50.f;  // Move logo higher up
        logoSprite.setPosition((windowWidth - logoSprite.getGlobalBounds().width) / 2.f, logoY);

        // Set up title (now positioned below logo)
        titleText.setFont(font);
        titleText.setString("Enter the name");
        titleText.setCharacterSize(40);
        titleText.setFillColor(sf::Color(255, 215, 0));  // Gold color
        float titleY = logoY + logoSprite.getGlobalBounds().height + 50.f;  // Add spacing after logo
        titleText.setPosition((windowWidth - titleText.getGlobalBounds().width) / 2.f, titleY);
    }
    
    // Set up input box
    inputBox.setSize(sf::Vector2f(400, 50));
    inputBox.setFillColor(sf::Color(60, 60, 60));
    inputBox.setOutlineColor(sf::Color(200, 200, 200));  // Lighter outline
    inputBox.setOutlineThickness(2);
    
    // Set up input text
    inputText.setFont(font);
    inputText.setCharacterSize(32);
    inputText.setFillColor(sf::Color::White);
    
    // Set up instruction text
    instructionText.setFont(font);
    instructionText.setString("Press Enter to confirm");
    instructionText.setCharacterSize(24);
    instructionText.setFillColor(sf::Color(255, 215, 0));  // Gold color to match title
    
    updateText();
}

void NameInputState::handleEvent(const sf::Event& event, sf::RenderWindow& /*window*/) {
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

void NameInputState::update(float /*deltaTime*/) {
    // No update needed
}

void NameInputState::updateText() {
    // Center title below logo with proper spacing
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(
        (windowWidth - titleBounds.width) / 2,
        titleText.getPosition().y
    );
    
    // Center input box below title
    float inputBoxY = titleText.getPosition().y + 80.f;  // Position input box below title
    inputBox.setPosition(
        (windowWidth - inputBox.getSize().x) / 2,
        inputBoxY
    );
    
    // Center input text in box
    inputText.setString(playerName + "_");
    sf::FloatRect textBounds = inputText.getLocalBounds();
    inputText.setPosition(
        (windowWidth - textBounds.width) / 2,
        inputBoxY + (inputBox.getSize().y - textBounds.height) / 2
    );
    
    // Center instruction text below input box
    sf::FloatRect instructionBounds = instructionText.getLocalBounds();
    instructionText.setPosition(
        (windowWidth - instructionBounds.width) / 2,
        inputBoxY + inputBox.getSize().y + 20
    );
}

void NameInputState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(40, 40, 40));
    
    window.draw(logoSprite);
    window.draw(titleText);
    window.draw(inputBox);
    window.draw(inputText);
    window.draw(instructionText);
} 