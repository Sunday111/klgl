#pragma once

struct FractalSettings;

class FractalRenderer
{
public:
    virtual ~FractalRenderer() noexcept = default;

    virtual void Render(FractalSettings&) = 0;
    virtual void ApplySettings(FractalSettings&) = 0;
};
