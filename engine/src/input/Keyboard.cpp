#include "Keyboard.h"

namespace frostwind {
namespace {

bool validKey(SDL_Scancode key)
{
    return key > SDL_SCANCODE_UNKNOWN && key < SDL_SCANCODE_COUNT;
}

} // namespace

bool Keyboard::isDown(SDL_Scancode key) const
{
    return validKey(key) && down_[key];
}

bool Keyboard::wasPressed(SDL_Scancode key) const
{
    return validKey(key) && pressed_[key];
}

bool Keyboard::wasReleased(SDL_Scancode key) const
{
    return validKey(key) && released_[key];
}

void Keyboard::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP) {
        return;
    }
    const auto key = event.key.scancode;
    if (!validKey(key)) {
        return;
    }
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat && !down_[key]) {
        down_[key] = true;
        pressed_[key] = true;
    } else if (event.type == SDL_EVENT_KEY_UP && down_[key]) {
        down_[key] = false;
        released_[key] = true;
    }
}

void Keyboard::releaseAll()
{
    for (int key = 1; key < SDL_SCANCODE_COUNT; ++key) {
        released_[key] = released_[key] || down_[key];
        down_[key] = false;
    }
}

void Keyboard::clearTransitions()
{
    pressed_.fill(false);
    released_.fill(false);
}

} // namespace frostwind
