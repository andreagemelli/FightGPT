#pragma once

#include "GameState.h"
#include "StoryState.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

class CharacterSelectionState : public GameState {
public:
    explicit CharacterSelectionState(const std::string& name);
    ~CharacterSelectionState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    void updatePositions();

    // Text elements
    sf::Font font;
    sf::Text title;
    std::array<sf::Text, 3> options;
    sf::Text logText;

    // Visual elements
    std::array<sf::RectangleShape, 3> optionBoxes;
    std::array<sf::Texture, 3> classIcons;
    std::array<sf::Sprite, 3> iconSprites;
    std::array<sf::Texture, 3> characterTextures;
    std::array<sf::Sprite, 3> characterSprites;
    sf::RectangleShape logPanel;

    // Content storage
    std::array<std::string, 3> descriptions_;

    // State
    int selectedOption;
    std::string playerName;
}; 