#ifndef EFT_EMITTER_SET_H_
#define EFT_EMITTER_SET_H_

#include <nw/math/math_MTX34.h>
#include <nw/math/math_VEC2.h>
#include <nw/math/math_VEC3.h>
#include <nw/ut/ut_Color4f.h>
#include <nw/eft/eft_Enum.h>

namespace nw { namespace eft {

struct EmitterInstance;

class EmitterController
{
public:
    void SetFollowType(PtclFollowType followType);

    f32 emissionRatio;
    f32 emissionInterval;
    f32 life;
    u32 renderVisibilityFlags;
    EmitterInstance* emitter;
};
static_assert(sizeof(EmitterController) == 0x14, "EmitterController size mismatch");

class System;

class EmitterSet
{
public:
    EmitterSet()
        : numEmitter(0)
        , numEmitterAtCreate(0)
        , createID(0)
        , userData(0)
        , emissionPoints(NULL)
    {
        color.v    = (math::VEC4){ 1.0f, 1.0f, 1.0f, 1.0f };
        ptclRotate = (math::VEC3){ 0.0f, 0.0f, 0.0f };

        for (u32 i = 0; i < 16u; i++)
            emitters[i] = NULL;
    }

    void SetMtx(const math::MTX34& matrixSRT);
    void Kill();

    System* system;
    s32 numEmitter;
    s32 numEmitterAtCreate;
    u32 createID;
    EmitterInstance* emitters[16];
    EmitterController controllers[16];
    u32 resourceID;
    u32 emitterSetID;
    u32 userData;
    u32 groupID;
    u32 _unusedFlags;
    math::MTX34 matrixSRT;
    math::MTX34 matrixRT;
    math::VEC3 scaleForMatrix;
    math::VEC2 ptclScale;
    math::VEC2 ptclEmitScale;
    math::VEC2 ptclEffectiveScale;
    math::VEC3 emitterVolumeScale;
    ut::Color4f color;
    f32 allDirVel;
    f32 dirVel;
    f32 dirVelRandom;
    math::VEC3 addVelocity;
    math::VEC3 dir;
    s32 startFrame;
    math::VEC3 ptclRotate;
    s32 numEmissionPoints;
    math::VEC3* emissionPoints;
    EmitterSet* next;
    EmitterSet* prev;
    u8 doFade;
    u8 dirSet;
    u8 noCalc;
    u8 noDraw;
    u8 infiniteLifespan;
    u8 _unused;
    u8 _unusedPad[6];
};
static_assert(sizeof(EmitterSet) == 0x294, "EmitterSet size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_SET_H_
