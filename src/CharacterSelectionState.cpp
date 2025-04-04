#include "CharacterSelectionState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

CharacterSelectionState::CharacterSelectionState(const std::string& name) 
    : playerName(name), selectedOption(0) {
    
    if (!font.loadFromFile("assets/fonts/Jersey15-Regular.ttf")) {
        Logger::error("Failed to load font!");
        return;
    }

    // Load class icons
    std::vector<std::string> iconPaths = {
        "assets/icons/sword.png",
        "assets/icons/hat.png",
        "assets/icons/bow.png"
    };

    for (size_t i = 0; i < 3; ++i) {
        if (!classIcons[i].loadFromFile(iconPaths[i])) {
            Logger::error("Failed to load icon: " + iconPaths[i]);
            return;
        }
        iconSprites[i].setTexture(classIcons[i]);
        
        // Scale icons to 32x32
        float scale = 32.0f / std::max(classIcons[i].getSize().x, classIcons[i].getSize().y);
        iconSprites[i].setScale(scale, scale);
    }

    // Set up title
    title.setFont(font);
    title.setString("Welcome, " + playerName + "!\nChoose Your Class");
    title.setCharacterSize(40);
    title.setFillColor(sf::Color(255, 215, 0));  // Gold color
    title.setLineSpacing(1.5);

    // Set up character options with detailed attributes
    const std::array<std::string, 3> descriptions = {{
        "HP: 120 - High survivability in combat\n"
        "Attack: 25 - Balanced damage output\n"
        "Defense: 25 - Strong protection against attacks\n"
        "Speed: 12 - Moderate movement and action speed\n"
        "Avoidance: 8% - Low chance to dodge attacks\n\n"
        "Perfect for beginners, excelling in defense",

        "HP: 80 - Lower health pool\n"
        "Attack: 35 - High magical damage output\n"
        "Defense: 8 - Very light armor protection\n"
        "Speed: 15 - Good mobility in combat\n"
        "Avoidance: 12% - Decent chance to dodge\n\n"
        "Glass cannon, high risk, high reward",

        "HP: 90 - Moderate health pool\n"
        "Attack: 28 - Good ranged damage\n"
        "Defense: 12 - Light armor protection\n"
        "Speed: 20 - Excellent mobility\n"
        "Avoidance: 18% - High chance to dodge\n\n"
        "Agile fighter, specializing in evasion"
    }};

    // Set up log panel
    logPanel.setSize(sf::Vector2f(400, 600));
    logPanel.setFillColor(sf::Color(60, 60, 60));
    logPanel.setOutlineColor(sf::Color(100, 100, 100));
    logPanel.setOutlineThickness(2);

    // Set up log text
    logText.setFont(font);
    logText.setCharacterSize(20);
    logText.setFillColor(sf::Color::White);

    // Initialize option boxes and texts
    for (size_t i = 0; i < 3; ++i) {
        options[i].setFont(font);
        options[i].setString(descriptions[i]);
        options[i].setCharacterSize(20);
        options[i].setFillColor(sf::Color(220, 220, 220));

        optionBoxes[i].setSize(sf::Vector2f(500, 180));
        optionBoxes[i].setFillColor(sf::Color(60, 60, 60));
        optionBoxes[i].setOutlineThickness(2);
    }

    updatePositions();
}

void CharacterSelectionState::updatePositions() {
    // Center the title
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(
        (1200 - titleBounds.width) / 2,
        50
    );

    // Position options and boxes
    float startY = 180.f;
    float optionHeight = 180.f;
    float padding = 20.f;
    float leftMargin = 50.f;

    for (size_t i = 0; i < 3; ++i) {
        optionBoxes[i].setPosition(leftMargin, startY + i * (optionHeight + padding));
        options[i].setPosition(leftMargin + 70, startY + i * (optionHeight + padding) + 20);
        iconSprites[i].setPosition(leftMargin + 20, startY + i * (optionHeight + padding) + 20);
    }

    // Position log panel and text
    logPanel.setPosition(750, 180);
    logText.setPosition(770, 200);
    logText.setString(options[selectedOption].getString());
}

void CharacterSelectionState::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
            case sf::Keyboard::Up:
                if (selectedOption > 0) selectedOption--;
                updatePositions(); // Update log when selection changes
                break;
            case sf::Keyboard::Down:
                if (selectedOption < 2) selectedOption++;
                updatePositions(); // Update log when selection changes
                break;
            case sf::Keyboard::Return:
                Logger::info("Selected character: " + std::to_string(selectedOption));
                nextState = std::make_unique<GamePlayState>(selectedOption, playerName);
                break;
            default:
                break;
        }
    }
    else if (event.type == sf::Event::MouseMoved) {
        // Get mouse position
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Check if mouse is over any option box
        for (int i = 0; i < 3; ++i) {
            sf::FloatRect bounds = optionBoxes[i].getGlobalBounds();
            if (bounds.contains(mousePos.x, mousePos.y)) {
                if (selectedOption != i) {
                    selectedOption = i;
                    updatePositions(); // Update log when hovering over a new option
                }
                break;
            }
        }
    }
}

void CharacterSelectionState::update(float /*deltaTime*/) {
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

    // Draw title
    window.draw(title);

    // Draw options
    for (size_t i = 0; i < 3; ++i) {
        window.draw(optionBoxes[i]);
        window.draw(iconSprites[i]);
        window.draw(options[i]);
    }

    // Draw log panel and text
    window.draw(logPanel);
    window.draw(logText);

    // Draw instruction text
    sf::Text instruction;
    instruction.setFont(font);
    instruction.setString("Use UP/DOWN arrows to select, ENTER to confirm");
    instruction.setCharacterSize(20);
    instruction.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect instructionBounds = instruction.getLocalBounds();
    instruction.setPosition(
        (window.getSize().x - instructionBounds.width) / 2,
        window.getSize().y - 50
    );
    window.draw(instruction);
} 