#include <app/Application.h>

#include <cmath>
#include <cstdio>
#include <exception>
#include <memory>
#include <stdexcept>

#include <glad/gl.h>

// Replace platform time/window operations at link time. No video/context is
// created; these checks exercise Application's real callback coordination.
namespace {
Uint64 now = 0;
Uint64 renderWork = 2000000;
Uint64 swapWork = 4000000;
Uint64 waited = 0;
bool isMinimized = false;
bool zeroSize = false;
bool throwUpdate = false;
bool throwInitialize = false;
bool failInitialize = false;
bool pressed = false;
bool held = false;
bool destroyed = false;
float delta = -1;
int updates = 0;
int renders = 0;
int presents = 0;
SDL_AppResult updateResult = SDL_APP_CONTINUE;

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void near(float actual, float expected)
{
    require(std::abs(actual - expected) < 0.000001f, "Unexpected update delta.");
}
} // namespace

extern "C" Uint64 SDL_GetTicksNS() { return now; }
extern "C" void SDL_DelayPrecise(Uint64 ns) { now += ns; waited = ns; }
extern "C" void SDL_Delay(Uint32 ms) { now += Uint64{ms} * SDL_NS_PER_MS; }

namespace frostwind {
Window::~Window() = default;
bool Window::initialize(const WindowConfig&) { return !window_ && !context_ && !videoInitialized_; }
bool Window::minimized() const { return isMinimized; }
bool Window::pixelSize(int& width, int& height) const
{
    width = zeroSize ? 0 : 800;
    height = 600;
    return true;
}
bool Window::makeCurrent() const { return true; }
bool Window::present() const { ++presents; now += swapWork; return true; }
} // namespace frostwind

namespace frostwind::detail {
void requireOpenGL(const char*) {}
void checkOpenGL(const char*) {}
} // namespace frostwind::detail

namespace {
class Probe final : public frostwind::Application {
public:
    ~Probe() override { destroyed = true; }

protected:
    bool initialize() override
    {
        if (throwInitialize) {
            throw std::runtime_error("Expected initialization exception.");
        }
        return !failInitialize;
    }
    SDL_AppResult handleEvent(const SDL_Event& event) override
    {
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            require(keyboard().isDown(event.key.scancode), "Input state must precede example event dispatch.");
        }
        return SDL_APP_CONTINUE;
    }
    SDL_AppResult update(float seconds) override
    {
        ++updates;
        delta = seconds;
        pressed = keyboard().wasPressed(SDL_SCANCODE_W);
        held = keyboard().isDown(SDL_SCANCODE_W);
        if (throwUpdate) {
            throw std::runtime_error("Expected update exception.");
        }
        return updateResult;
    }
    SDL_AppResult render(frostwind::Renderer&, int, int) override
    {
        ++renders;
        require(!keyboard().wasPressed(SDL_SCANCODE_W), "Key edges must be consumed after update.");
        now += renderWork;
        return SDL_APP_CONTINUE;
    }
};

std::unique_ptr<frostwind::Application> factory() { return std::make_unique<Probe>(); }

void* start(unsigned int fps = 60)
{
    void* state = nullptr;
    frostwind::ApplicationConfig config;
    config.maxFps = fps;
    require(frostwind::Application::onInit(&state, factory, config) == SDL_APP_CONTINUE && state,
        "Probe initialization failed.");
    return state;
}

void frame(void* state)
{
    waited = 0;
    require(frostwind::Application::onIterate(state) == SDL_APP_CONTINUE, "Probe frame failed.");
}

void event(void* state, Uint32 type, SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN)
{
    SDL_Event value{};
    value.type = type;
    value.key.scancode = scancode;
    require(frostwind::Application::onEvent(state, &value) == SDL_APP_CONTINUE, "Event failed.");
}

void checks()
{
    glad_glViewport = [](GLint, GLint, GLsizei, GLsizei) {};
    void* state = start();
    frame(state);
    near(delta, 0);
    require(waited == 10666667, "Render and swap work must consume the pacing budget.");
    event(state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_W);
    frame(state);
    near(delta, 0.016666667f);
    require(pressed && held, "Update missed accumulated key state.");
    frame(state);
    require(!pressed && held, "Key edges persisted or held state was lost.");
    now += 5 * SDL_NS_PER_SECOND;
    frame(state);
    near(delta, 0.1f);
    event(state, SDL_EVENT_WINDOW_FOCUS_LOST);
    const int beforeFocus = updates;
    frame(state);
    require(updates == beforeFocus, "Unfocused movement must pause.");
    event(state, SDL_EVENT_WINDOW_FOCUS_GAINED);
    frame(state);
    near(delta, 0);
    require(!pressed && !held, "Focus return retained old input.");
    isMinimized = true;
    const int beforeMinimize = presents;
    frame(state);
    require(presents == beforeMinimize, "Minimized window presented.");
    isMinimized = false;
    frame(state);
    near(delta, 0);
    zeroSize = true;
    const int beforeZero = renders;
    frame(state);
    require(renders == beforeZero, "Zero-size window rendered.");
    zeroSize = false;
    frame(state);
    near(delta, 0);
    for (auto result : {SDL_APP_SUCCESS, SDL_APP_FAILURE}) {
        updateResult = result;
        const int before = renders;
        const int beforePresent = presents;
        require(frostwind::Application::onIterate(state) == result, "Update result was not propagated.");
        require(renders == before && presents == beforePresent, "Terminating update rendered/presented.");
    }
    updateResult = SDL_APP_CONTINUE;
    throwUpdate = true;
    const int beforeThrow = renders;
    const int beforeThrowPresent = presents;
    require(frostwind::Application::onIterate(state) == SDL_APP_FAILURE, "Update exception escaped callback.");
    require(renders == beforeThrow && presents == beforeThrowPresent, "Throwing update rendered/presented.");
    throwUpdate = false;
    frostwind::Application::onQuit(state, SDL_APP_SUCCESS);
    require(destroyed, "Application was not destroyed.");
    for (auto fps : {30u, 60u, 120u}) {
        state = start(fps);
        frame(state);
        near(delta, 0);
        float travel = 0;
        const Uint64 interval = (SDL_NS_PER_SECOND + fps - 1) / fps;
        for (unsigned int i = 0; i < fps; ++i) {
            frame(state);
            travel += delta;
            require(waited == interval - renderWork - swapWork, "Unexpected frame pacing.");
        }
        near(travel, 1);
        frostwind::Application::onQuit(state, SDL_APP_SUCCESS);
    }
    for (bool throws : {false, true}) {
        failInitialize = !throws;
        throwInitialize = throws;
        destroyed = false;
        state = reinterpret_cast<void*>(1);
        require(frostwind::Application::onInit(&state, factory) == SDL_APP_FAILURE && !state && destroyed,
            "Initialization failure did not clean ownership/appstate.");
    }
}
} // namespace

int main()
{
    try {
        checks();
        std::puts("Update timing, callback results, inactivity, input coordination, and pacing checks passed.");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
