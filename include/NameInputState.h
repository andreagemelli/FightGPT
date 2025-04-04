#pragma once

#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <string>

class NameInputState : public GameState {
public:
    NameInputState();
    ~NameInputState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    void updateText();

    sf::Font font;
    sf::Text titleText;
    sf::Text inputText;
    sf::Text instructionText;
    sf::RectangleShape inputBox;
    
    std::string playerName;
    static const size_t MAX_NAME_LENGTH = 20;
}; 