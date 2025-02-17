#pragma once

#include <prim/seadNamable.h>

namespace agl {

class ShaderLocation
{
public:
    ShaderLocation()
        : vs(-1)
        , ps(-1)
        , gs(-1)
    {
    }

    s16 vs;
    s16 ps;
    s16 gs;
};
static_assert(sizeof(ShaderLocation) == 6, "agl::ShaderLocation size mismatch");

class SamplerLocation : public sead::INamable, ShaderLocation
{
public:
    SamplerLocation()
        : sead::INamable("Undefined")
        , ShaderLocation()
    {
    }
};
static_assert(sizeof(SamplerLocation) == 0x10, "agl::SamplerLocation size mismatch");

}
