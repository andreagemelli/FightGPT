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

    sf::Font font;
    sf::Text title;
    std::array<sf::Text, 3> options;
    std::array<sf::RectangleShape, 3> optionBoxes;
    sf::RectangleShape background;
    int selectedOption;
    
    void initializeText();
    void initializeBoxes();
}; 