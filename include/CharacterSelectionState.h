#pragma once

#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <array>

class CharacterSelectionState : public GameState {
public:
    CharacterSelectionState();
    ~CharacterSelectionState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    void updatePositions();
    void loadIcons();

    sf::Font font;
    sf::Text title;
    std::array<sf::Text, 3> options;
    std::array<sf::RectangleShape, 3> optionBoxes;
    std::array<sf::Texture, 3> classIcons;
    std::array<sf::Sprite, 3> iconSprites;
    sf::RectangleShape background;
    int selectedOption;
    
    void initializeText();
    void initializeBoxes();
}; 