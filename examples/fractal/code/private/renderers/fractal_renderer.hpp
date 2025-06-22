#pragma once

class FractalSettings;

class FractalRenderer
{
public:
    virtual ~FractalRenderer() noexcept = default;

    virtual void Render(const FractalSettings&) = 0;
    virtual void ApplySettings(const FractalSettings&) = 0;
};
