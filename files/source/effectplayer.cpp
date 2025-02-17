#include "actor/player.h"
#include "nw/eft.h"
#include "drawmgr.h"
#include "playermgr.h"
#include "ptclmgr.h"

#include "log.h"

/*
    TODO:
    * Make it more customizable.
    * Better digits rendering (make sead::PrimitiveRenderer work).
    * Better handling of inputs (slightly).
    * Better centering of emitter set number.

    Controls (Player 1 only):
    * Down: Next effect (emitter set)
    * Up: Previous effect (emitter set)
*/

class EffectPlayer : public Actor
{
public:
    EffectPlayer(const ActorBuildInfo* buildInfo);
    static Base* build(const ActorBuildInfo* buildInfo);

    u32 onCreate() override;
    u32 onExecute() override;
    u32 onDraw() override;

    forceinline void drawLine(const Vec2& position, const u32 rotation, const float lineLength, const float lineThickness, const u32 digit, const u32 modelIdx);
    forceinline void drawLine(const Vec2& point1, const Vec2& point2, const float lineThickness, const u32 digit, const u32 modelIdx);

    bool nextEffect(const PlayerInput& player1Input);
    bool prevEffect(const PlayerInput& player1Input);
    void killEffect();

    static const Vec2 s_p1;
    static const Vec2 s_p2;
    static const Vec2 s_p3;
    static const Vec2 s_p4;
    static const Vec2 s_p5;
    static const Vec2 s_p6;

    static const s32 holdRegisterDuration = 15; // 0.25 sec

    s32 currentEmitterSetID;
    nw::eft::Handle effectHandle;
    nw::math::MTX34 mtx;
    bool downHeld;
    bool upHeld;
    s32 downHeldCounter;
    s32 upHeldCounter;

    ModelWrapper* models[4][7]; // 4 digits, each can use up to 7 lines
};

const Vec2 EffectPlayer::s_p1(-8.0f,  16.0f);
const Vec2 EffectPlayer::s_p2( 8.0f,  16.0f);
const Vec2 EffectPlayer::s_p3( 8.0f,   0.0f);
const Vec2 EffectPlayer::s_p4( 8.0f, -16.0f);
const Vec2 EffectPlayer::s_p5(-8.0f, -16.0f);
const Vec2 EffectPlayer::s_p6(-8.0f,   0.0f);

const Profile effectPlayerProfile(&EffectPlayer::build, ProfileId::EffectPlayer, "EffectPlayer", nullptr, 0);
PROFILE_RESOURCES(ProfileId::EffectPlayer, "block_snake");

EffectPlayer::EffectPlayer(const ActorBuildInfo* buildInfo)
    : Actor(buildInfo)
    , effectHandle()
    , currentEmitterSetID(-1)
    , mtx(nw::math::MTX34::Identity())
    , downHeld(false)
    , upHeld(false)
    , downHeldCounter(0)
    , upHeldCounter(0)
{
}

Base* EffectPlayer::build(const ActorBuildInfo* buildInfo)
{
    return new EffectPlayer(buildInfo);
}

u32 EffectPlayer::onCreate()
{
    currentEmitterSetID = -1;
    downHeld = false;
    upHeld = false;
    downHeldCounter = 0;
    upHeldCounter = 0;

    nw::math::VEC3 scale;
    scale.x = this->scale.x;
    scale.y = this->scale.y;
    scale.z = this->scale.z;

    nw::math::VEC3 rotate;
    if (settings1 & 1)
    {
        rotate.x = sead::Mathf::idx2rad(rotation.x + 0x20000000);
        rotate.y = sead::Mathf::idx2rad(rotation.y + 0x20000000);
    }
    else
    {
        rotate.x = sead::Mathf::idx2rad(rotation.x);
        rotate.y = sead::Mathf::idx2rad(rotation.y);
    }
    rotate.z = sead::Mathf::idx2rad(rotation.z);

    nw::math::VEC3 translate;
    translate.x = (position.x += 8.0f);
    translate.y = (position.y -= 8.0f);
    translate.z = position.z;

    nw::math::MTX34::MakeSRT(&mtx, &scale, &rotate, &translate);

    for (u32 i = 0; i < 4; i++)
        for (u32 j = 0; j < 7; j++)
            models[i][j] = ModelWrapper::create("block_snake", "block_snake");

    return 1;
}

u32 EffectPlayer::onExecute()
{
    s32 numEmitterSet = PtclMgr::instance()->system->resources[0]->resource->numEmitterSet;
    if (numEmitterSet < 1 || currentEmitterSetID >= numEmitterSet)
    {
failure:
        LOG("Failure")
        killEffect();
        isDeleted = true;
        return 1;
    }

    const Player* player1 = nullptr;
    if (PlayerMgr::instance()->playerFlags & 1)
        player1 = PlayerMgr::instance()->players[0];

    if (player1 == nullptr)
    {
        if (currentEmitterSetID == -1)
            return 1;

        else
            goto update;
    }

    if (nextEffect(player1->input))
    {
        currentEmitterSetID = (currentEmitterSetID + 1) % numEmitterSet;
        LOG("Proceeding to next: %d", currentEmitterSetID)
    }
    else if (prevEffect(player1->input))
    {
        currentEmitterSetID = (currentEmitterSetID - 1) % numEmitterSet;
        if (currentEmitterSetID < 0)
            currentEmitterSetID += numEmitterSet;
        LOG("Going to previous: %d", currentEmitterSetID)
    }
    else
    {
        if (currentEmitterSetID == -1)
            return 1;

        else
            goto update;
    }

    killEffect();

    if (!PtclMgr::instance()->system->CreateEmitterSetID(&effectHandle, nw::math::MTX34::Identity(), currentEmitterSetID, 0, PtclMgr::instance()->getEmitterSetGroupID(currentEmitterSetID)))
        goto failure;

    LOG("Create succeeded")

update:
    nw::eft::EmitterSet* emitterSet = effectHandle.emitterSet;
    if (emitterSet == nullptr || effectHandle.createID != emitterSet->createID)
        goto failure;

    emitterSet->SetMtx(mtx);

    return 1;
}

void EffectPlayer::drawLine(const Vec2& position, const u32 rotation, const float lineLength, const float lineThickness, const u32 digit, const u32 modelIdx)
{
    f32 sinRZ, cosRZ;
    sead::Mathf::sinCosIdx(&sinRZ, &cosRZ, rotation);

    Vec3 scale(lineLength / 16, lineThickness / 16, 1.f / 16);

    //Vec3u rot(0, 0, rotation);
    //Vec3  pos(position.x + (lineLength * cosRZ) / 2,
    //          position.y + (lineLength * sinRZ) / 2,
    //          4000);
    //Mtx34 mtxRT; mtxRT.makeRTIdx(rot, pos);

    Mtx34 mtxRT(cosRZ, -sinRZ, 0, position.x + (lineLength * cosRZ) / 2,
                sinRZ,  cosRZ, 0, position.y + (lineLength * sinRZ) / 2,
                0, 0, 1, 4000);

    ModelWrapper* model = models[digit][modelIdx];
    model->setScale(scale);
    model->setMtx(mtxRT);
    model->updateModel();
    DrawMgr::instance->drawModel(model);
}

void EffectPlayer::drawLine(const Vec2& point1, const Vec2& point2, const float lineThickness, const u32 digit, const u32 modelIdx)
{
    const Vec2& leftPoint = (point1.x < point2.x) ? point1 : point2;
    const Vec2& rightPoint = (&leftPoint == &point1) ? point2 : point1;

    const f32 diffX = rightPoint.x - leftPoint.x;
    const f32 diffY = rightPoint.y - leftPoint.y;

    drawLine(leftPoint,
             sead::Mathf::atan2Idx(diffY, diffX),
             sead::Mathf::sqrt(diffX*diffX + diffY*diffY),
             lineThickness, digit, modelIdx);
}

u32 EffectPlayer::onDraw()
{
    if (currentEmitterSetID < 0)
        return 1;

    u32 num = currentEmitterSetID;
    u32 numDigits = 0;

    {
        u32 tmp = num;
        do
        {
            numDigits++;
            tmp /= 10;
        } while (tmp != 0);
    }

    if (numDigits > 4)
        return 1;

    position.y -= 3.5f * 16.0f;

    const f32 digitWidth = 1.25f * 16.0f * scale.x;
    const f32 xShift     = (numDigits - 1) * digitWidth / numDigits;

    Vec2 p1(position.x + s_p1.x * scale.x + xShift, position.y + s_p1.y * scale.y);
    Vec2 p2(position.x + s_p2.x * scale.x + xShift, position.y + s_p2.y * scale.y);
    Vec2 p3(position.x + s_p3.x * scale.x + xShift, position.y + s_p3.y * scale.y);
    Vec2 p4(position.x + s_p4.x * scale.x + xShift, position.y + s_p4.y * scale.y);
    Vec2 p5(position.x + s_p5.x * scale.x + xShift, position.y + s_p5.y * scale.y);
    Vec2 p6(position.x + s_p6.x * scale.x + xShift, position.y + s_p6.y * scale.y);

    for (u32 i = 0; i < numDigits; i++)
    {
        const u32 digit = num % 10;
        num /= 10;

        const u32 A = (digit >> 3 & 1);
        const u32 B = (digit >> 2 & 1);
        const u32 C = (digit >> 1 & 1);
        const u32 D = (digit >> 0 & 1);

        const u32 a = (~B & 1) & (~D & 1) | B & D | C | A;
        const u32 b = (~C & 1) & (~D & 1) | C & D | (~B & 1) | A;
        const u32 c = (~C & 1) | D | B | A;
        const u32 d = B & (~C & 1) & D | (~B & 1) & (~D & 1) | (~B & 1) & C | C & (~D & 1) | A;
        const u32 e = (~B & 1) & (~D & 1) | C & (~D & 1) | A & (~D & 1);
        const u32 f = (~C & 1) & (~D & 1) | B & (~C & 1) | B & (~D & 1) | A;
        const u32 g = (~B & 1) & C | C & (~D & 1) | B & (~C & 1) | B & (~D & 1) | A;

        if (a) drawLine(p1, p2, scale.y, i, 0);
        if (b) drawLine(p2, p3, scale.x, i, 1);
        if (c) drawLine(p3, p4, scale.x, i, 2);
        if (d) drawLine(p4, p5, scale.y, i, 3);
        if (e) drawLine(p5, p6, scale.x, i, 4);
        if (f) drawLine(p6, p1, scale.x, i, 5);
        if (g) drawLine(p3, p6, scale.y, i, 6);

        p1.x -= digitWidth;
        p2.x -= digitWidth;
        p3.x -= digitWidth;
        p4.x -= digitWidth;
        p5.x -= digitWidth;
        p6.x -= digitWidth;
    }

    position.y += 3.5f * 16.0f;

    return 1;
}

bool EffectPlayer::nextEffect(const PlayerInput& player1Input)
{
    if (!downHeld)
    {
        if (player1Input.isDownPressed())
        {
            downHeld = true;
            downHeldCounter = 0;
        }
    }
    else
    {
        if (player1Input.isDownHeld())
        {
            if (player1Input.isOnlyDownHeld() && ++downHeldCounter >= holdRegisterDuration)
            {
                downHeldCounter = 0;
                return true;
            }
        }
        else
        {
            downHeld = false;
            if (downHeldCounter > 0)
                return true;
        }
    }

    return false;
}

bool EffectPlayer::prevEffect(const PlayerInput& player1Input)
{
    if (!upHeld)
    {
        if (player1Input.isUpPressed())
        {
            upHeld = true;
            upHeldCounter = 0;
        }
    }
    else
    {
        if (player1Input.isUpHeld())
        {
            if (player1Input.isOnlyUpHeld() && ++upHeldCounter >= holdRegisterDuration)
            {
                upHeldCounter = 0;
                return true;
            }
        }
        else
        {
            upHeld = false;
            if (upHeldCounter > 0)
                return true;
        }
    }

    return false;
}

void EffectPlayer::killEffect()
{
    nw::eft::EmitterSet* emitterSet = effectHandle.emitterSet;
    if (emitterSet != nullptr && effectHandle.createID == emitterSet->createID && emitterSet->numEmitter >= 1)
        emitterSet->doFade = 1;

    effectHandle.emitterSet = nullptr;
}
