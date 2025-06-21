#pragma once

class FractalApp;

class FractalRenderer
{
public:
    virtual ~FractalRenderer() noexcept = default;

    virtual void Render(FractalApp&) = 0;
    virtual void ApplySettings(FractalApp&) = 0;
};
