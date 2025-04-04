#include "CharacterSelectionState.h"
#include "GamePlayState.h"
#include "Logger.h"
#include <sstream>

CharacterSelectionState::CharacterSelectionState(const std::string& name) 
    : selectedOption(0), playerName(name) {
    
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

    // Load character images
    std::vector<std::string> characterPaths = {
        "assets/characters/knight.png",
        "assets/characters/mage.png",
        "assets/characters/archer.png"
    };

    for (size_t i = 0; i < 3; ++i) {
        // Load class icons
        if (!classIcons[i].loadFromFile(iconPaths[i])) {
            Logger::error("Failed to load icon: " + iconPaths[i]);
            return;
        }
        iconSprites[i].setTexture(classIcons[i]);
        
        // Scale icons to 48x48 for better visibility
        float scale = 48.0f / std::max(classIcons[i].getSize().x, classIcons[i].getSize().y);
        iconSprites[i].setScale(scale, scale);

        // Load character images
        if (!characterTextures[i].loadFromFile(characterPaths[i])) {
            Logger::error("Failed to load character image: " + characterPaths[i]);
            return;
        }
        characterSprites[i].setTexture(characterTextures[i]);
        
        // Scale character images to fit in the log panel
        float charScale = 250.0f / std::max(characterTextures[i].getSize().x, characterTextures[i].getSize().y);
        characterSprites[i].setScale(charScale, charScale);
    }

    // Set up title
    title.setFont(font);
    title.setString(name + ", Select Your Class");  // Simplified title
    title.setCharacterSize(36);  // Slightly smaller but still visible
    title.setFillColor(sf::Color(255, 215, 0));  // Gold color
    title.setStyle(sf::Text::Bold);  // Make it bold for better visibility

    // Set up class names
    const std::array<std::string, 3> classNames = {{"Knight", "Mage", "Archer"}};

    // Set up detailed descriptions for the log panel
    const std::array<std::string, 3> descriptions = {{
        "The Knight\n\n"
        "Attributes:\n"
        "HP: 120 - High survivability in combat\n"
        "Attack: 25 - Balanced damage output\n"
        "Defense: 25 - Strong protection against attacks\n"
        "Speed: 12 - Moderate movement and action speed\n"
        "Avoidance: 8% - Low chance to dodge attacks\n\n"
        "Special Ability: Rage\n"
        "Your next attack becomes a guaranteed critical hit,\n"
        "dealing double damage to the enemy.\n\n"
        "Perfect for beginners, excelling in defense",

        "The Mage\n\n"
        "Attributes:\n"
        "HP: 80 - Lower health pool\n"
        "Attack: 35 - High magical damage output\n"
        "Defense: 8 - Very light armor protection\n"
        "Speed: 15 - Good mobility in combat\n"
        "Avoidance: 12% - Decent chance to dodge\n\n"
        "Special Ability: Fireball\n"
        "Cast a devastating spell dealing 15% max HP damage\n"
        "and burns the enemy for 5 HP each turn.\n\n"
        "Glass cannon, high risk, high reward",

        "The Archer\n\n"
        "Attributes:\n"
        "HP: 90 - Moderate health pool\n"
        "Attack: 28 - Good ranged damage\n"
        "Defense: 12 - Light armor protection\n"
        "Speed: 20 - Excellent mobility\n"
        "Avoidance: 18% - High chance to dodge\n\n"
        "Special Ability: Hunter's Mark\n"
        "Mark your target, making your next three\n"
        "attacks impossible to dodge.\n\n"
        "Agile fighter, specializing in evasion"
    }};

    // Set up log panel
    logPanel.setSize(sf::Vector2f(500, 600));
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
        options[i].setString(classNames[i]);  // Only show class name
        options[i].setCharacterSize(24);  // Slightly smaller text
        options[i].setFillColor(sf::Color(220, 220, 220));

        optionBoxes[i].setSize(sf::Vector2f(250, 50));  // Reduced height from 80 to 50
        optionBoxes[i].setFillColor(sf::Color(60, 60, 60));
        optionBoxes[i].setOutlineThickness(2);

        // Store descriptions separately
        descriptions_[i] = descriptions[i];
    }

    updatePositions();
}

void CharacterSelectionState::updatePositions() {
    const float windowWidth = 1200.0f;
    const float windowHeight = 800.0f;
    const float padding = 20.0f;

    // Title positioning
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(
        (windowWidth - titleBounds.width) / 2,
        80.0f  // Moved down a bit to be more visible
    );

    // Left column for class selection
    const float leftColumnX = 50.0f;
    const float leftColumnStartY = 160.0f;  // Adjusted to account for new title position
    const float optionSpacing = 20.0f;  // Reduced spacing between options

    // Right column for class details - also starts lower
    const float rightColumnX = 400.0f;
    const float rightColumnY = 140.0f;  // Moved down to match left column

    // Position option boxes and their contents
    for (size_t i = 0; i < 3; ++i) {
        float boxY = leftColumnStartY + i * (optionBoxes[i].getSize().y + optionSpacing);
        
        // Position the box
        optionBoxes[i].setPosition(leftColumnX, boxY);
        
        // Position the icon
        iconSprites[i].setPosition(
            leftColumnX + 10.0f,
            boxY + (optionBoxes[i].getSize().y - 48) / 2
        );
        
        // Position the text
        options[i].setPosition(
            leftColumnX + 70.0f,
            boxY + (optionBoxes[i].getSize().y - options[i].getCharacterSize()) / 2
        );

        // Position character sprites in the log panel
        characterSprites[i].setPosition(
            rightColumnX + (logPanel.getSize().x - 250) / 2,  // Center in panel
            rightColumnY + 300  // Fixed position below description
        );
    }

    // Position and size the details panel
    logPanel.setSize(sf::Vector2f(700, 550));
    logPanel.setPosition(rightColumnX, rightColumnY);
    
    // Position the description text
    logText.setPosition(rightColumnX + 20.0f, rightColumnY + 20.0f);
    logText.setString(descriptions_[selectedOption]);
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
    // Clear with a darker background
    window.clear(sf::Color(30, 30, 30));

    // Draw title
    window.draw(title);

    // Draw option boxes and their contents
    for (size_t i = 0; i < 3; ++i) {
        // Draw box background
        window.draw(optionBoxes[i]);
        
        // Draw icon and text
        window.draw(iconSprites[i]);
        window.draw(options[i]);
    }

    // Draw details panel
    window.draw(logPanel);
    window.draw(logText);
    window.draw(characterSprites[selectedOption]);

    // Draw instruction text at bottom
    sf::Text instruction;
    instruction.setFont(font);
    instruction.setString("Use UP/DOWN arrows or mouse to select, ENTER to confirm");
    instruction.setCharacterSize(24);
    instruction.setFillColor(sf::Color(255, 215, 0));
    
    sf::FloatRect instructionBounds = instruction.getLocalBounds();
    instruction.setPosition(
        (window.getSize().x - instructionBounds.width) / 2,
        window.getSize().y - 60.0f
    );
    
    window.draw(instruction);
} 