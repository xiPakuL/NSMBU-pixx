#pragma once

#include <prim/seadSafeString.h>

#include "preprocessor.h"
#include "profileid.h"

class ActorInfo;
struct ActorBuildInfo;
class Base;
struct ProfileResources;

class Profile
{
public:
    Profile(Base* (*buildFunc)(const ActorBuildInfo*), u32 id, const sead::SafeString& name, const ActorInfo* actorInfo, u32 flags);

    static Profile* get(u32 id);
    static s16 getPriority(u32 id);
    static u8 getHasResources(u32 id);
    static u8 getResourceCount(u32 id);
    static const sead::SafeString* getResourceList(u32 id);

    static u32 getNumProfiles();

    static u32 spriteToProfileList[];

    Base* (*buildFunc)(const ActorBuildInfo*);  // 0
    u32 id;                                     // 4
    const ActorInfo* actorInfo;                 // 8
    u8 hasResourcesLoaded;                      // C
    u32 flags;                                  // 10

private:
    static const u32 NUM_PROFILES_ORIGINAL = ProfileId::NumOriginal;
    static const u32 NUM_PROFILES          = ProfileId::Num;
    static const u32 NUM_PROFILES_CUSTOM   = NUM_PROFILES - NUM_PROFILES_ORIGINAL;

    static Profile* profilesOriginal[NUM_PROFILES_ORIGINAL];
    static s16 prioritiesOriginal[NUM_PROFILES_ORIGINAL];
    static u8 hasResourcesOriginal[NUM_PROFILES_ORIGINAL];
    static u8 resourceCountOriginal[NUM_PROFILES_ORIGINAL];
    static const sead::SafeString* resourceListsOriginal[NUM_PROFILES_ORIGINAL];

    static Profile* profilesCustom[NUM_PROFILES_CUSTOM];
    static s16 prioritiesCustom[NUM_PROFILES_CUSTOM];
    static u8 hasResourcesCustom[NUM_PROFILES_CUSTOM];
    static u8 resourceCountCustom[NUM_PROFILES_CUSTOM];
    static const sead::SafeString* resourceListsCustom[NUM_PROFILES_CUSTOM];

    friend struct ProfileResources;
};


struct ProfileResources
{
    ProfileResources(u32 id, u8 count, const sead::SafeString resources[])
    {
        if (id < Profile::NUM_PROFILES_ORIGINAL)
        {
            Profile::hasResourcesOriginal[id] = count > 0;
            Profile::resourceCountOriginal[id] = count;
            Profile::resourceListsOriginal[id] = resources;
        }
        else if (id < Profile::NUM_PROFILES)
        {
            const u32 customId = id - Profile::NUM_PROFILES_ORIGINAL;
            Profile::hasResourcesCustom[customId] = count > 0;
            Profile::resourceCountCustom[customId] = count;
            Profile::resourceListsCustom[customId] = resources;
        }
        else
        {
            Profile::hasResourcesOriginal[0] = count > 0;
            Profile::resourceCountOriginal[0] = count;
            Profile::resourceListsOriginal[0] = resources;
        }
    }
};

#define PROFILE_RESOURCES_IDENT(ident, id, ...)                                                                                                           \
    static const u32 PP_CONCAT(profileResourceCount, ident) = PP_NARG(__VA_ARGS__);                                                                       \
    static_assert(PP_CONCAT(profileResourceCount, ident) <= 0xFF, "Cannot have more than 255 resources!");                                                \
    static const sead::SafeString PP_CONCAT(profileResourceFiles, ident)[] = { __VA_ARGS__ };                                                             \
    static const ProfileResources PP_CONCAT(profileResources, ident)(id, PP_CONCAT(profileResourceCount, ident), PP_CONCAT(profileResourceFiles, ident))

#define PROFILE_RESOURCES(id, ...)   \
    PROFILE_RESOURCES_IDENT(__LINE__, id, __VA_ARGS__)
