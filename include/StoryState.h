#pragma once

#include "GameState.h"
#include "GamePlayState.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

class StoryState : public GameState {
public:
    StoryState(int selectedCharacter, const std::string& playerName, const std::string& bossName);
    ~StoryState() override = default;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    void generateStory();
    void formatText();

    sf::Font font;
    sf::Text storyText;
    sf::Text continueText;
    sf::RectangleShape background;
    sf::RectangleShape textBackground;
    std::string playerName;
    int selectedCharacter;
    float textFadeIn;
    float pulseEffect;
    float continueTextDelay;
    std::string bossName;
}; 