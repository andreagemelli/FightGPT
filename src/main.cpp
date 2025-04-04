#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <memory>
#include "GameState.h"
#include "NameInputState.h"
#include "CharacterSelectionState.h"
#include "GamePlayState.h"
#include "Logger.h"

int main() {
    Logger::info("Starting FightGPT");
    
    // Create the main window with larger size
    sf::RenderWindow window(sf::VideoMode(1200, 800), "FightGPT");
    window.setVerticalSyncEnabled(true);

    // Create the game state manager starting with name input
    std::unique_ptr<GameState> currentState = std::make_unique<NameInputState>();
    sf::Clock clock;

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                Logger::info("Window closed by user");
                window.close();
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                Logger::info("Game exited via ESC key");
                window.close();
            } else {
                // Handle state-specific events
                currentState->handleEvent(event, window);
            }
        }
        
        float deltaTime = clock.restart().asSeconds();
        currentState->update(deltaTime);

        // Check for state transition
        if (auto nextState = currentState->getNextState()) {
            Logger::info("Transitioning to new game state");
            currentState = std::move(nextState);
        }
        
        // Clear the window with dark background
        window.clear(sf::Color(20, 20, 20));

        // Draw the current state
        currentState->draw(window);

        // Display the window
        window.display();
    }
    
    Logger::info("Game terminated successfully");
    return 0;
} 