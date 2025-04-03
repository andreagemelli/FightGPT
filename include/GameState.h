#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

class GameState {
public:
    virtual ~GameState() = default;
    
    virtual void handleEvent(const sf::Event& event, sf::RenderWindow& window) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    
    virtual void onEnter() {}
    virtual void onExit() {}

protected:
    std::unique_ptr<GameState> nextState;

public:
    std::unique_ptr<GameState> getNextState() {
        return std::move(nextState);
    }
}; 