#pragma once
#include "ue.h"

constexpr bool Console = false;

enum StarfallURLSet {
    Default,
    Custom,
    Hybrid,
    Dev,
    All
};

constexpr StarfallURLSet URLSet = Default;

static const char* Backend = "http://127.0.0.1:3551";

constexpr bool bHasPushWidget = false;
constexpr bool UseBackendParam = false;
constexpr bool ManualMapping = false;
constexpr bool FixMemLeak = true;
