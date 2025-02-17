#ifndef EFT_EMITTER_H_
#define EFT_EMITTER_H_

#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_ResData.h>

namespace nw { namespace eft {

class EmitterCalc;
class EmitterController;
struct EmitterDynamicUniformBlock;
class EmitterSet;
struct EmitterStaticUniformBlock;
struct KeyFrameAnimArray;
class ParticleShader;
class Primitive;
struct PtclAttributeBuffer;
class StripeVertexBuffer;

struct EmitterInstance
{
    void Init(const SimpleEmitterData* data);
    inline void UpdateEmitterStaticUniformBlock(EmitterStaticUniformBlock* uniformBlock, const SimpleEmitterData* data);
    inline void UpdateChildStaticUniformBlock(EmitterStaticUniformBlock* uniformBlock, const ChildData* data);
    void UpdateResInfo();

    const ComplexEmitterData* GetComplexEmitterData() const
    {
        if (data->type != EmitterType_Complex)
            return NULL;

        return static_cast<const ComplexEmitterData*>(data);
    }

    bool HasChild() const
    {
        return data->type != EmitterType_Simple && (static_cast<const ComplexEmitterData*>(data)->childFlags & 1);
    }

    const ChildData* GetChildData() const
    {
        if (!HasChild())
            return NULL;

        return reinterpret_cast<const ChildData*>(static_cast<const ComplexEmitterData*>(data) + 1);
    }

    f32 counter;
    f32 counter2;
    f32 emitCounter;
    f32 preCalcCounter;
    f32 emitLostTime;
    u32 numParticles;
    u32 numChildParticles;
    u32 groupID;
    EmitterSet* emitterSet;
    EmitterController* controller;
    u32 emitterSetCreateID;
    math::MTX34 matrixRT;
    math::MTX34 matrixSRT;
    PtclRandom random;
    s32 emissionInterval;
    f32 fadeAlpha;
    f32 emissionSpeed;
    math::VEC3 prevPos;   // Used for equidistant emission
    bool prevPosSet;      // ^^
    f32 emitLostDistance; // ^^
    math::VEC3 scaleRandom;
    math::VEC3 rotateRandom;
    math::VEC3 translateRandom;
    PtclFollowType ptclFollowType;
    EmitterInstance* prev;
    EmitterInstance* next;
    EmitterCalc* calc;
    const SimpleEmitterData* data;
    PtclInstance* particleHead;
    PtclInstance* childParticleHead;
    PtclInstance* particleTail;
    PtclInstance* childParticleTail;
    ParticleShader* shader[ShaderType_Max];
    ParticleShader* childShader[ShaderType_Max];
    Primitive* primitive;
    Primitive* childPrimitive;
    KeyFrameAnimArray* animArray;
    f32 anim[25];
    math::MTX34 animMatrixRT;
    math::MTX34 animMatrixSRT;
    f32 emitLostRate;
    bool isEmitted;
    bool isCalculated;
    u8 _unused;
    u32 particleBehaviorFlg;
    u32 shaderAvailableAttribFlg;
    u32 childShaderAvailableAttribFlg;
    u32 numDrawParticle;
    u32 numDrawChildParticle;
    u32 numDrawStripe;
    PtclAttributeBuffer* ptclAttributeBuffer;
    PtclAttributeBuffer* childPtclAttributeBuffer;
    StripeVertexBuffer* stripeVertexBuffer;
    EmitterStaticUniformBlock* emitterStaticUniformBlock;
    EmitterStaticUniformBlock* childEmitterStaticUniformBlock;
    EmitterDynamicUniformBlock* emitterDynamicUniformBlock;
    EmitterDynamicUniformBlock* childEmitterDynamicUniformBlock;
};
static_assert(sizeof(EmitterInstance) == 0x220, "EmitterInstance size mismatch");

class System;

class EmitterCalc
{
public:
    EmitterCalc(System* system)
    {
        mSys = system;
    }

    virtual ~EmitterCalc() { } // deleted

    virtual void CalcEmitter(EmitterInstance* emitter) = 0;
    virtual PtclType GetPtclType() const = 0;
    virtual u32 CalcParticle(EmitterInstance* emitter, CpuCore core, bool noCalcBehavior, bool noMakePtclAttributeBuffer) = 0;
    virtual u32 CalcChildParticle(EmitterInstance* emitter, CpuCore core, bool noCalcBehavior, bool noMakePtclAttributeBuffer) = 0;

    static void RemoveParticle(EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core);
    static inline void AddChildPtclToList(EmitterInstance* emitter, PtclInstance* childPtcl);
    static inline void AddPtclToList(EmitterInstance* emitter, PtclInstance* ptcl);
    static void EmitCommon(EmitterInstance* emitter, PtclInstance* ptcl);

    static const void* _ptclField_Random(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);
    static const void* _ptclField_Magnet(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);
    static const void* _ptclField_Spin(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);
    static const void* _ptclField_Collision(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);
    static const void* _ptclField_Convergence(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);
    static const void* _ptclField_PosAdd(EmitterInstance* emitter, PtclInstance* ptcl, const void* fieldData);

    static void InitializeFluctuationTable(Heap* heap);
    static void CalcFluctuation(EmitterInstance* emitter, PtclInstance* ptcl);

    static u32 CalcSimpleParticleBehavior(EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core);
    static u32 CalcComplexParticleBehavior(EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core);
    static u32 CalcChildParticleBehavior(EmitterInstance* emitter, PtclInstance* ptcl);
    static void MakeParticleAttributeBuffer(PtclAttributeBuffer* ptclAttributeBuffer, PtclInstance* ptcl, u32 shaderAvailableAttribFlg, f32 cameraOffset);

    void ApplyAnim(EmitterInstance* emitter);
    void UpdateEmitterInfoByEmit(EmitterInstance* emitter);

    static PtclInstance* CalcEmitPoint(EmitterInstance* emitter);
    static PtclInstance* CalcEmitCircle(EmitterInstance* emitter);
    static PtclInstance* CalcEmitCircleSameDivide(EmitterInstance* emitter);
    static PtclInstance* CalcEmitFillCircle(EmitterInstance* emitter);
    static PtclInstance* CalcEmitSphere(EmitterInstance* emitter);
    static PtclInstance* CalcEmitSphereSameDivide(EmitterInstance* emitter);
    static PtclInstance* CalcEmitSphereSameDivide64(EmitterInstance* emitter);
    static PtclInstance* CalcEmitFillSphere(EmitterInstance* emitter);
    static PtclInstance* CalcEmitCylinder(EmitterInstance* emitter);
    static PtclInstance* CalcEmitFillCylinder(EmitterInstance* emitter);
    static PtclInstance* CalcEmitBox(EmitterInstance* emitter);
    static PtclInstance* CalcEmitFillBox(EmitterInstance* emitter);
    static PtclInstance* CalcEmitLine(EmitterInstance* emitter);
    static PtclInstance* CalcEmitLineSameDivide(EmitterInstance* emitter);
    static PtclInstance* CalcEmitRectangle(EmitterInstance* emitter);

    static f32* sFluctuationTbl;
    static System* mSys;

    typedef PtclInstance* (*EmitFunction)(EmitterInstance* emitter);
    static EmitFunction mEmitFunctions[];
};
static_assert(sizeof(EmitterCalc) == 4, "EmitterCalc size mismatch");

void EmitterCalc::AddChildPtclToList(EmitterInstance* emitter, PtclInstance* childPtcl)
{
    if (emitter->childParticleHead == NULL)
    {
        emitter->childParticleHead = childPtcl;
        childPtcl->next = NULL;
        childPtcl->prev = NULL;
    }
    else
    {
        emitter->childParticleHead->prev = childPtcl;
        childPtcl->next = emitter->childParticleHead;
        emitter->childParticleHead = childPtcl;
        childPtcl->prev = NULL;
    }

    if (emitter->childParticleTail == NULL)
        emitter->childParticleTail = childPtcl;

    emitter->numChildParticles++;
}

void EmitterCalc::AddPtclToList(EmitterInstance* emitter, PtclInstance* ptcl)
{
    if (emitter->particleHead == NULL)
    {
        emitter->particleHead = ptcl;
        ptcl->next = NULL;
        ptcl->prev = NULL;
    }
    else
    {
        emitter->particleHead->prev = ptcl;
        ptcl->next = emitter->particleHead;
        emitter->particleHead = ptcl;
        ptcl->prev = NULL;
    }

    if (emitter->particleTail == NULL)
        emitter->particleTail = ptcl;

    emitter->numParticles++;
}

} } // namespace nw::eft

#endif // EFT_EMITTER_H_
