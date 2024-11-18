#include "VHud.h"
#include "Audio.h"
#include "CellPhone.h"
#include "HudColoursNew.h"
#include "PadNew.h"
#include "FontNew.h"
#include "GPS.h"
#include "HudNew.h"
#include "MenuNew.h"
#include "MenuPanels.h"
#include "OverlayLayer.h"
#include "RadarNew.h"
#include "RadioHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "WeaponSelector.h"
#include "3dMarkersNew.h"

#include "VHudAPI.h"

#include "pugixml.hpp"
#include <CGame.h>

using namespace plugin;
using namespace pugi;

//int& gGameState = *(int*)0xC8D4C0;

bool VHud::bInitialised = false;
HANDLE VHud::pThread = NULL;
bool VHud::bRwInitialized = false;
bool VHud::bRwQuit = false;
bool VHud::bSAMP = false;
bool VHud::bUG = false;
bool VHud::bENB = false;
bool VHud::bModLoader = false;

VHudSettings VHud::Settings;

bool VHud::Init() {
    if (bInitialised)
        return false;

    ReadPluginSettings();

    std::locale::global(std::locale(""));

#ifdef DEBUG
    OpenConsole();
#endif
    
    if (!CheckCompatibility()) {
        bRwInitialized = false;
        bRwQuit = true;
        return false;
    }
    else {
        std::function<bool()> b([] { return true; });
        LateStaticInit::TryApplyWithPredicate(b);

        auto init = []() {
            Audio.Init();
            HudColourNew.ReadColorsFromFile();
            CFontNew::Init();
            CRadarNew::InitBeforeGame();
            MenuNew.Init();
            CRadioHud::Init();
            COverlayLayer::Init();
            MarkersNew.Init();

            bRwInitialized = true;
        };

        if (bUG) {
            CdeclEvent<AddressList<0x619D4E, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> initRw;
            initRw += init;
        }
        else {
            Events::initRwEvent += init;
        }

        auto initAfterRw = []() {
            CGPS::Init();
            CHudNew::Init();
            CellPhone.Init();
            CRadarNew::Init();
            CWeaponSelector::Init();
            CMenuPanels::Init();
        };

        if (bUG) {
            UG_RegisterEventCallback(UG_EVENT_AFTER_GAMEINIT1, (void(__cdecl*)())initAfterRw);
        }
        else {
            Events::initGameEvent += initAfterRw;
        }

        auto reInitForRestart = []() {
            CHudNew::ReInit();
            CWeaponSelector::ReInit();
        };

        if (bUG) {
            UG_RegisterEventCallback(UG_EVENT_SHUTDOWNFORRESTART, (void(__cdecl*)())reInitForRestart);
        }
        else {
            Events::reInitGameEvent += reInitForRestart;
        }

        Events::d3dLostEvent += [] {
            CFontNew::Lost();
        };

        Events::d3dResetEvent += [] {
            CFontNew::Reset();
        };

        auto shutdown = []() {
            MenuNew.Shutdown();
            CGPS::Shutdown();
            CRadarNew::Shutdown();
            CHudNew::Shutdown();
            CellPhone.Shutdown();
            CRadioHud::Shutdown();
            COverlayLayer::Shutdown();
            CFontNew::Shutdown();
            CWeaponSelector::Shutdown();
            CMenuPanels::Shutdown();
            Audio.Shutdown();
            MarkersNew.Shutdown();
            CPadNew::Shutdown();

            bRwQuit = true;
        };

        if (bUG) {
            UG_RegisterEventCallback(UG_EVENT_SHUTDOWNGAME, (void(__cdecl*)())shutdown);
        }
        else {
            Events::shutdownRwEvent += shutdown;
        }
    
        bInitialised = true;
    }

    return true;
}

void VHud::Shutdown() {
    if (!bInitialised)
        return;

    bInitialised = false;
}

void VHud::Run() {
    CPadNew::Init();

    while (!bRwQuit) {
        CheckForENB();
        CheckForMP();
        CheckForUG();
        CheckForModLoader();

        if (bRwInitialized) {
            if (gGameState && !bSAMP) {
                switch (gGameState) {
                case 7:
                    if (MenuNew.ProcessMenuToGameSwitch(false)) {
                        gGameState = 8;
                        MenuNew.OpenCloseMenu(false);
                    }
                    break;
                case 8:
                    MenuNew.fLoadingPercentage = 100.0f;
                    gGameState = 9;
                    break;
                case 9:
                    if (MenuNew.ProcessMenuToGameSwitch(true)) {
                        MenuNew.OpenCloseMenu(false);
                    }
                    break;
                }
            }

            Audio.Update();
            CPadNew::GInputUpdate();
        }
    }
    CPadNew::GInputRelease();
    Shutdown();

    CloseHandle(pThread);
};

void VHud::CheckForENB() {
    if (bENB)
        return;

    if (const HMODULE h = ModuleList().Get(L"enbseries")) {
        printf("ENB detected.\n");
        printf("Windowed modes unavailable.\n");
        bENB = true;
    }
}

void VHud::CheckForModLoader() {
    if (bModLoader)
        return;

    if (const HMODULE h = ModuleList().Get(L"modloader")) {
        printf("Mod Loader detected.\n");
        bModLoader = true;
    }
}

void VHud::CheckForMP() {
    if (bSAMP)
        return;

    if (const HMODULE h = ModuleList().Get(L"SAMP")) {
        printf("SAMP detected.\n");
        bSAMP = true;
    }
}

void VHud::CheckForUG() {
    if (bUG)
        return;

    if (HMODULE h = ModuleList().Get(L"Underground_Core")) {
        printf("GTA Underground detected.\n");
        bUG = true;
    }
}

void VHud::UG_RegisterEventCallback(int e, void* func) {
    if (HMODULE h = ModuleList().Get(L"Underground_Core")) {
        const char* eventsNamesList[] = {
            "EVENT_BEFORE_RENDER2DSTUFF", "EVENT_AFTER_RENDER2DSTUFF",
            "EVENT_BEFORE_RENDERSCENE", "EVENT_AFTER_RENDERSCENE",
            "EVENT_SHUTDOWNFORRESTART", "EVENT_BEFORE_PROCESSGAME",
            "EVENT_AFTER_PROCESSGAME", "EVENT_SHUTDOWNGAME",
            "EVENT_BEFORE_GAMEINIT1", "EVENT_AFTER_GAMEINIT1",
            "EVENT_BEFORE_GAMEINIT2", "EVENT_AFTER_GAMEINIT2",
            "EVENT_BEFORE_GAMEINIT3", "EVENT_AFTER_GAMEINIT3",
            "EVENT_STARTTESTSCRIPT", "EVENT_UPDATEPOPULATION",
            "EVENT_INITPOSTEFFECTS", "EVENT_CLOSEPOSTEFFECTS",
            "EVENT_PROCESSCARGENERATORS", "EVENT_BEFORE_RENDERFADINGINENTITIES",
            "EVENT_AFTER_RENDERFADINGINENTITIES",
        };

        auto a = (void* (*)())GetProcAddress(h, "RegisterEventCallback");
        reinterpret_cast<void(__cdecl*)(const char*, void*)>(a)(eventsNamesList[e], func);
    }
}

bool VHud::CheckCompatibility() {
    if (!IsSupportedGameVersion()) {
        Error("This version of GTA: San Andreas is not supported by this plugin.");
        return false;
    }

    CheckForENB();
    CheckForMP();
    CheckForUG();
    CheckForModLoader();

    CRadarNew::GetRadarTrace() = patch::Get<tRadarTrace*>(0x5838B0 + 2);
    CRadarNew::GetRadarBlipsSprites() = patch::Get<CSprite2d*>(0x5827EA + 1);
    CRadarNew::GetRadarTexturesSlot() = patch::Get<int*>(0x584C5B + 1);
    CRadarNew::GetTxdStreamingShiftValue() = patch::Get<int>(0x408858 + 2);
    CRadarNew::GetMaxRadarTrace() = patch::Get<int>(0x5D53CD + 1);
    return true;
}

bool VHud::OpenConsole() {
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);

    return true;
}

// ???: Could be very expensive!
std::string VHud::ConvertCharStreamToUTFString(const char* str) {
    std::string cr = str;

    for (char& c : cr) {
        c = GetConvertedSymbolForChar(c);
    }

    return cr;
}

char VHud::GetConvertedSymbolForChar(char& c) {
    const int opcode = static_cast<int>(c);

    switch (opcode) {
        case -86: return '\u00FA'; // ú
        case -94: return '\u00ED'; // í
        case -90: return '\u00F3'; // ó
        case -98: return '\u00E9'; // é
        case -81: return '\u00BF'; // ¿
        case -82: return '\u00F1'; // ñ
        case -104: return '\u00E1'; // á
        case 94: return '\u00A1'; // ¡
        default: return c;
    }
}

// ???: expensive! I might change it to directly modify the char instead.
std::string VHud::ConvertCharStreamToUTFString(char* stream) {
    char c;
    std::string converted;
    //
    for (unsigned int i = 0; i < 400; i++)
    {
        c = stream[i];
        const int opcode = static_cast<int>(stream[i]);

        // There's no more text to go through.
        if (opcode == 0)
            break;

        converted += GetConvertedSymbolForChar(c);

        //printf("%c[%i] ", c, opcode);
    }
    // printf("\n");

    // printf("%s ", converted.c_str());

    return converted;
}

void VHud::ReadPluginSettings() {
    xml_document doc;
    xml_parse_result file = doc.load_file(PLUGIN_PATH("VHud\\data\\plugin.xml"));
    VHudSettings& s = Settings;

    if (file) {
        if (auto plugin = doc.child("Plugin")) {
            strcpy(s.UIMainColor, plugin.child("UIMainColor").attribute("value").as_string());
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        if (VHud::Init())
            VHud::pThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(void(__cdecl*)())VHud::Run, 0, 0, 0);
    }
    return TRUE;
}
