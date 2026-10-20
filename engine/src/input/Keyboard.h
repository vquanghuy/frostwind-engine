#pragma once

#include <array>

#include <SDL3/SDL.h>

namespace frostwind {

// State accumulated by Application's SDL events, consumed once per update.
class Keyboard {
public:
    bool isDown(SDL_Scancode key) const;
    bool wasPressed(SDL_Scancode key) const;
    bool wasReleased(SDL_Scancode key) const;

    void handleEvent(const SDL_Event& event);
    void releaseAll();
    void clearTransitions();

private:
    std::array<bool, SDL_SCANCODE_COUNT> down_{};
    std::array<bool, SDL_SCANCODE_COUNT> pressed_{};
    std::array<bool, SDL_SCANCODE_COUNT> released_{};
};

} // namespace frostwind
