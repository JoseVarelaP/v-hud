#pragma once
#define VHUD_API __declspec(dllexport)

#include "plugin.h"

class VHud {
public:
    VHud();

};

extern int& gGameState;
extern bool SAMP;
extern void CheckForMP();
