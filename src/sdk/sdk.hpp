#pragma once

#include <windows.h>
#include <cstdint>
#include <cmath>

typedef uint32_t addr_t;

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vec3 operator-() const {
        return { -x, -y, -z };
    }

    Vec3 operator-(const Vec3& o) const {
        return { x - o.x, y - o.y, z - o.z };
    }


    Vec3 operator+(const Vec3& o) const {
        return { x + o.x, y + o.y, z + o.z };
    }

    Vec3 operator*(float s) const {
        return { x * s, y * s, z * s };
    }

    float Dist(const Vec3& o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    float Dist2D(const Vec3& o) const {
        float dx = x - o.x, dy = y - o.y;
        return sqrtf(dx * dx + dy * dy);
    }

    bool IsZero() const {
        return fabsf(x) < 0.01f && fabsf(y) < 0.01f && fabsf(z) < 0.01f;
    }

    bool IsValid() const {
        if (std::isnan(x) || std::isnan(y) || std::isnan(z)) return false;
        if (std::isinf(x) || std::isinf(y) || std::isinf(z)) return false;
        return fabsf(x) < 16384.0f && fabsf(y) < 16384.0f && fabsf(z) < 16384.0f;
    }
};

struct hud_player_info_t {
    char *name;
    short ping;
    unsigned char thisplayer;
    unsigned char spectator;
    unsigned char packetloss;
    char *model;
    short topcolor;
    short bottomcolor;
};

// GoldSrc ref_params_t passed into V_CalcRefdef
struct ref_params_t {
    float vieworg[3];       // +0x00
    float viewangles[3];    // +0x0C
    float forward[3];       // +0x18
    float right[3];         // +0x24
    float up[3];            // +0x30
    float frametime;        // +0x3C
    float time;             // +0x40
    int   intermission;     // +0x44
    int   paused;           // +0x48
    int   spectator;        // +0x4C
    int   onground;         // +0x50
    int   waterlevel;       // +0x54
    float simvel[3];        // +0x58
    float simorg[3];        // +0x64
    float viewheight[3];    // +0x70
    float idealpitch;       // +0x7C
    float cl_viewangles[3]; // +0x80
    int   health;           // +0x8C
    float crosshairangle[3];// +0x90
    float viewsize;         // +0x9C
    float punchangle[3];    // +0xA0
    int   maxclients;       // +0xAC
    int   viewentity;       // +0xB0
    int   playermodel;      // +0xB4
    void* cmd;              // +0xB8
    void* movevars;         // +0xBC
    int   viewport[4];      // +0xC0
    int   nextView;         // +0xD0
    int   onlyClientDraw;   // +0xD4
};

#define STUDIO_MAGIC 0x54534449 // "IDST"
#define STUDIO_VERSION 10

#define HITGROUP_GENERIC  0
#define HITGROUP_HEAD     1
#define HITGROUP_CHEST    2
#define HITGROUP_STOMACH  3
#define HITGROUP_LEFTARM  4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG  6
#define HITGROUP_RIGHTLEG 7

struct studiohdr_t {
    int   id;
    int   version;
    char  name[64];
    int   length;
    float eyeposition[3];
    float min[3];
    float max[3];
    float bbmin[3];
    float bbmax[3];
    int   flags;
    int   numbones;
    int   boneindex;
    int   numbonecontrollers;
    int   bonecontrollerindex;
    int   numhitboxes;
    int   hitboxindex;
    int   numseq;
    int   seqindex;
    int   numseqgroups;
    int   seqgroupindex;
    int   numtextures;
    int   textureindex;
    int   texturedataindex;
    int   numskinref;
    int   numskinfamilies;
    int   skinindex;
    int   numbodyparts;
    int   bodypartindex;
    int   numattachments;
    int   attachmentindex;
};

struct mstudiobone_t {
    char  name[32];
    int   parent;
    int   flags;
    int   bonecontroller[6];
    float value[6];
    float scale[6];
};

struct mstudiohitbox_t {
    int   bone;
    int   group;
    float bbmin[3];
    float bbmax[3];
};

struct PlayerData {
    int    index;
    addr_t entAddr;
    addr_t modelAddr;
    addr_t studioHdrAddr;
    Vec3   origin;
    Vec3   angles;
    Vec3   headPos;       // True World-Space Head Hitbox Center
    Vec3   neckPos;       // Upper Cervical Neck Base
    Vec3   upperSpinePos; // Upper Torso / Clavicle Junction
    Vec3   chestPos;      // True World-Space Chest Hitbox Center
    Vec3   stomachPos;    // True World-Space Stomach / Abdominal Center
    Vec3   pelvisPos;     // Pelvis / Hip Root
    Vec3   feetPos;
    Vec3   topPos;
    Vec3   eyePos;

    // Full Anatomical Skeletal Joint World Positions
    Vec3   lClaviclePos, rClaviclePos;
    Vec3   lShoulderPos, rShoulderPos;
    Vec3   lElbowPos,    rElbowPos;
    Vec3   lHandPos,     rHandPos;
    Vec3   lHipPos,      rHipPos;
    Vec3   lKneePos,     rKneePos;
    Vec3   lAnklePos,    rAnklePos;
    Vec3   lFootPos,     rFootPos;
    Vec3   lToePos,      rToePos;


    addr_t originOffset;
    int    health;
    int    team; // 1 = T, 2 = CT, 0 = Unknown
    bool   alive;
    bool   isLocal;
    bool   isDucking;
    bool   isDefusing;
    bool   hasC4;
    bool   hasStudioHitbox;
    int    headHitboxIndex;
    Vec3   headHitboxMin;
    Vec3   headHitboxMax;
    float  distanceMeters;
    char   name[32];
    char   modelName[64];
    char   weaponName[32];
};

struct WorldEntityData {
    int index;
    addr_t entAddr;
    Vec3 origin;
    char modelName[64];
    char displayName[32];
    bool isC4;
    bool isPlantedC4;
    bool isGrenade;
    bool active;
    uint64_t lastSeenFrame;
    float distanceMeters;
};

enum MenuTab {
    TAB_AIMBOT    = 0,
    TAB_VISUALS   = 1,
    TAB_RADAR_HUD = 2,
    TAB_MISC      = 3,
    TAB_THEMES    = 4,
    TAB_CONFIG    = 5,
    TAB_COUNT     = 6
};

enum EspBoxStyle {
    BOX_OFF    = 0,
    BOX_2D     = 1,
    BOX_CORNER = 2,
    BOX_COUNT  = 3
};

enum EspHealthStyle {
    HP_OFF      = 0,
    HP_SOLID    = 1,
    HP_GRADIENT = 2,
    HP_COUNT    = 3
};

enum ThemePreset {
    THEME_CYBER_CYAN   = 0,
    THEME_NEON_PURPLE  = 1,
    THEME_MATRIX_GREEN = 2,
    THEME_CRIMSON_RED  = 3,
    THEME_GOLDEN_AMBER = 4,
    THEME_COUNT        = 5
};

struct PanelState {
    float x, y, w, h;
    bool  pinned;
    bool  collapsed;
};

struct MenuState {
    bool visible;
    int  activeTab;       // 0=Aimbot, 1=Visuals, 2=Radar/Misc, 3=Config/Themes

    // Multi-Panel Tab Windows (Combat, Render, Movement/Radar, Themes/Config)
    PanelState panels[4];

    // Floating Widget Windows
    float specX, specY;
    float kbX, kbY;
    float bombX, bombY;

    // Tab 0: Aimbot
    bool  aimEnable;
    int   aimKey;         // 0=Mouse1/2, 1=Mouse2, 2=Shift, 3=Alt, 4=Ctrl, 5=Auto (Always)
    int   aimBone;        // 0=Head, 1=Neck, 2=Chest, 3=Pelvis
    float aimSmooth;      // 1.0f to 25.0f
    float aimFov;         // 1.0f to 45.0f
    bool  aimEnemyOnly;   // Target Enemies Only
    bool  aimVisCheck;    // Line-of-Sight Visibility Check
    bool  aimRcs;         // Recoil Control System
    bool  aimTrigger;     // Auto Fire (Triggerbot)

    // Tab 1: Visuals
    int   espBox;         // 0=Off, 1=2D Box, 2=Corner Box
    int   espHealth;      // 0=Off, 1=Solid, 2=Gradient
    bool  espInfo;        // Name, Weapon, Distance
    bool  snaplines;      // Snaplines to screen bottom
    bool  headMarker;     // Bone apex marker
    bool  boxChams;       // Alpha glow fill
    float chamsAlpha;     // 0.05f to 0.80f
    bool  c4Tracker;      // World C4 & Grenades
    bool  skeletonEsp;    // Full Skeletal Joint Bone ESP [NEW]
    bool  offscreenEsp;   // Offscreen Directional Threat Arrows [NEW]
    bool  hpText;         // Numeric Health Value Pill [NEW]

    // Tab 2: Radar & Floating HUDs
    bool  radar2D;        // 2D Tactical Radar
    float radarRange;     // 500.0f to 4000.0f
    bool  radarSweep;     // Radar Dynamic Sweep Animation [NEW]
    bool  watermark;      // Sleek top-right watermark
    bool  spectatorList;  // Floating Spectator Observer List
    bool  keybindList;    // Floating Keybinds Status HUD
    bool  bombTimer;      // Live C4 Bomb Timer HUD
    bool  diagHud;        // Detailed developer telemetry

    // Tab 3: Misc & Movement
    bool  bhop;
    bool  crosshair;
    bool  sniperCrosshair;// Always-on Crosshair for AWP / Scout [NEW]
    bool  recoilCrosshair;// Follow-Recoil Bullet Landing Point [NEW]
    bool  fovCircle;
    bool  enemyOnly;
    float fovRadius;

    // Tab 4: Themes & Color Palette
    int   themeIndex;     // Preset theme (0..4)
    float accentR, accentG, accentB;

    int   selected;
};

struct pmplane_t {
    float normal[3];
    float dist;
};

struct pmtrace_t {
    int       allsolid;   // if true, plane is not valid
    int       startsolid; // if true, initial point in solid area
    int       inopen;     // open area
    int       inwater;    // water area
    float     fraction;   // time completed, 1.0 = did not hit any obstacle
    float     endpos[3];  // final position reached by trace
    pmplane_t plane;      // surface normal plane
    int       ent;        // entity hit index
    int       hitgroup;   // hit group (1=head, 2=chest, 3=stomach, etc.)
};

// Exact GoldSrc Function Pointers in cl_enginefuncs_s
typedef void  (*pfnClientCmd_t)(const char *szCmdString); // Index 20 (offset 0x50)
typedef int   (*pfnGetPlayerInfo_t)(int ent_num, hud_player_info_t *pinfo); // Index 21 (offset 0x54)
typedef void  (*pfnGetViewAngles_t)(float *angles);        // Index 32 (offset 0x80)
typedef void  (*pfnSetViewAngles_t)(float *angles);        // Index 33 (offset 0x84)
typedef void* (*pfnGetLocalPlayer_t)(void);      // Index 49 (offset 0xC4)
typedef void* (*pfnGetEntityByIndex_t)(int idx); // Index 51 (offset 0xCC)
typedef void  (*pfnTraceLine_t)(const float *start, const float *end, int flags, int usehull, int ignore_ent, pmtrace_t *ptr); // Index 56 (offset 0xE0)
typedef int   (*pfnHUD_GetPlayerTeam_t)(int ent_num); // client.dll::HUD_GetPlayerTeam export
typedef void  (*V_CalcRefdefFn)(ref_params_t* pparams);
typedef int   (*HUD_AddEntityFn)(int type, void* ent, const char* modelname);


