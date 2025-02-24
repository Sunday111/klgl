#pragma once

#include <filesystem>
#include <memory>
#include <optional>

namespace klgl::events
{
class EventManager;
}

namespace klgl
{

class Window;

class Application
{
    struct State;

public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    virtual ~Application();

    virtual void Initialize();
    virtual void Run();
    virtual void PreTick();
    virtual void Tick();
    virtual void PostTick();
    virtual void MainLoop();
    virtual void InitializeReflectionTypes();
    [[nodiscard]] virtual bool WantsToClose() const;

    virtual std::tuple<int, int> GetOpenGLVersion() const { return {3, 3}; }

    Window& GetWindow();
    const Window& GetWindow() const;

    const std::filesystem::path& GetExecutableDir() const;
    virtual std::filesystem::path GetContentDir() const;
    virtual std::filesystem::path GetShaderDir() const;

    events::EventManager& GetEventManager();

    // Current time. Relative to app start
    float GetTimeSeconds() const;

    // Time (in seconds) when the current fame started. Relative to app start
    float GetCurrentFrameStartTime() const;

    // How many ticks app does per second (on average among last 128 ticks)
    float GetFramerate() const;

    // Duration of the previous tick (in seconds)
    float GetLastFrameDurationSeconds() const;

    void SetTargetFramerate(std::optional<float> framerate);

private:
    std::unique_ptr<State> state_;
};

}  // namespace klgl
