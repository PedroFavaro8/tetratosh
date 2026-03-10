/**
 * TETRATOSH - Kext Selector
 * 
 * Automatic kext selection based on detected hardware
 * Supports High Sierra to Sonoma
 * 
 * Patch selection logic:
 * - Always applies critical patches (TSC Sync, XHCI, PowerTimeout)
 * - Legacy CPUs get more patches (LegacyCommpage, RTC, etc.)
 * - AMD gets DisableIOMapper and special patches
 * - CPU-specific patches based on generation
 */

#include "tetratosh.h"

// Patch database - dynamically selected based on hardware
typedef struct {
    CHAR8 *PatchName;
    CHAR8 *TargetCPU[8];  // NULL-terminated array of compatible CPUs
    BOOLEAN IsCritical;
    BOOLEAN IsRecommended;
    BOOLEAN IsLegacyOnly;
    UINT32 Priority;
} PATCH_ENTRY;

// Complete patch database
PATCH_ENTRY gPatchDatabase[] = {
    // Critical patches - always needed
    {"CpuTscSync",       {"SandyBridge", "IvyBridge", "Haswell", "Broadwell", "Skylake", "KabyLake", "CoffeeLake", NULL}, TRUE,  FALSE, FALSE, 100},
    {"XhciPortLimit",    {"all", NULL},                                          TRUE,  FALSE, FALSE, 100},
    {"PowerTimeoutKernelPanic", {"all", NULL},                                    TRUE,  FALSE, FALSE, 100},
    {"AvoidRuntimeDefrag", {"all", NULL},                                        TRUE,  TRUE,  FALSE, 90},
    {"DevirtualiseMmio", {"all", NULL},                                          TRUE,  TRUE,  FALSE, 90},
    {"DisableWatchDog",  {"all", NULL},                                          TRUE,  TRUE,  FALSE, 90},
    {"ProvideConsoleGop", {"all", NULL},                                         TRUE,  TRUE,  FALSE, 90},
    {"RequestBootVarRouting", {"all", NULL},                                     TRUE,  TRUE,  FALSE, 90},
    
    // CPU-specific patches
    {"AppleCpuPmCfgLock", {"SandyBridge", "IvyBridge", "Haswell", "Nehalem", "Westmere", "Penryn", "Core2", "Conroe", "Merom", NULL}, FALSE, FALSE, FALSE, 50},
    {"AppleXcpmCfgLock",  {"SandyBridge", "IvyBridge", "Nehalem", "Westmere", "Penryn", "Core2", "Conroe", "Merom", NULL}, FALSE, FALSE, FALSE, 50},
    {"AppleXcpmExtraMsrs", {"Broadwell", "Skylake", "KabyLake", NULL},          FALSE, FALSE, FALSE, 50},
    {"IgnoreInvalidFlexRatio", {"Skylake", "KabyLake", "CoffeeLake", "CometLake", NULL}, TRUE, FALSE, FALSE, 80},
    
    // Legacy CPU patches
    {"LegacyCommpage",   {"Pentium4", "Core2", "Conroe", "Merom", "AMD-K8", "AMD-K10", NULL}, FALSE, FALSE, TRUE, 30},
    {"ExternalDiskIcons", {"SandyBridge", "IvyBridge", "AMD-K8", "AMD-K10", "AMD-Bulldozer", "Pentium4", "Core2", NULL}, FALSE, FALSE, TRUE, 20},
    {"ThirdPartyDrives", {"all", NULL},                                         FALSE, FALSE, TRUE, 20},
    {"DisableIOMapper",  {"AMD-K8", "AMD-K10", "AMD-Bulldozer", "AMD-Zen", "AMD-Zen2", "AMD-Zen3", "AMD-Zen4", NULL}, TRUE, FALSE, FALSE, 70},
    
    // USB patches
    {"USBInjectAll",     {"all", NULL},                                         FALSE, FALSE, TRUE, 15},
    
    // RTC patches
    {"DisableRtcChecksum", {"all", NULL},                                       FALSE, FALSE, FALSE, 10},
    {"LapicKernelPanic", {"HP", "SandyBridge", "IvyBridge", "Pentium4", "Core2", NULL}, FALSE, FALSE, FALSE, 25},
    
    // Recommended patches
    {"NormalizeHeaders", {"all", NULL},                                         FALSE, TRUE, FALSE, 40},
    {"PanicNoKextDump", {"all", NULL},                                          FALSE, TRUE, FALSE, 40},
    
    // AMD-specific
    {"ProcInfo",         {"AMD-K8", "AMD-K10", "AMD-Bulldozer", "AMD-Zen", "AMD-Zen2", "AMD-Zen3", "AMD-Zen4", NULL}, FALSE, FALSE, FALSE, 30},
    
    // TSC patches for legacy
    {"CpuTscSync",       {"Pentium4", "Core2", "Conroe", "Merom", "Penryn", NULL}, TRUE, FALSE, TRUE, 80},
    
    {NULL, {NULL}, FALSE, FALSE, FALSE, 0}  // End marker
};

// Legacy CPU detection - these need special handling
CHAR8 *gLegacyCPUs[] = {
    "Pentium4", "Core2", "Conroe", "Merom", "Penryn",
    "AMD-K8", "AMD-K10", NULL
};

// AMD CPUs
CHAR8 *gAMDs[] = {
    "AMD-K8", "AMD-K10", "AMD-Bulldozer", "AMD-Zen", "AMD-Zen2", "AMD-Zen3", "AMD-Zen4", NULL
};

// Kext database - embedded in bootloader
typedef struct {
    UINT16 VendorID;
    UINT16 DeviceID;
    UINT8 ClassCode;
    CHAR8 *KextName;
    UINT32 Flags;
} KEXT_DATABASE_ENTRY;

// Complete kext database for all hardware
KEXT_DATABASE_ENTRY gKextDatabase[] = {
    // ========== GPU KEXTS ==========
    // Intel GPUs
    {0x8086, 0x0102, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x0106, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x010A, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x0152, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x0156, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x015A, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x0162, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x0166, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x5912, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x5916, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x5917, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x591B, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x591D, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E90, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E91, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E92, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E93, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E98, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x3E9B, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x8A50, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x8A51, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x8A52, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x8A70, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x9A60, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x9A74, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x8086, 0x9A84, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    
    // NVIDIA GPUs
    {0x10DE, 0x0DC0, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_LEGACY},
    {0x10DE, 0x0DF8, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_LEGACY},
    {0x10DE, 0x1280, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x10DE, 0x1282, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x10DE, 0x1B00, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_NVIDIA_WEB},
    {0x10DE, 0x1B80, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_NVIDIA_WEB},
    {0x10DE, 0x1C03, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x10DE, 0x1F10, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_NVIDIA_WEB},
    {0x10DE, 0x2204, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_NVIDIA_WEB},
    {0x10DE, 0x2684, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_NVIDIA_WEB},
    
    // AMD GPUs
    {0x1002, 0x6739, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_LEGACY},
    {0x1002, 0x6740, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU | KEXT_FLAG_LEGACY},
    {0x1002, 0x67DF, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x6860, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x687F, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x6880, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x6898, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x68A0, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x7310, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x73A0, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x73AB, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    {0x1002, 0x744C, 0x03, "WhateverGreen.kext", KEXT_FLAG_GPU},
    
    // ========== NETWORK KEXTS ==========
    // Intel
    {0x8086, 0x100E, 0x02, "AppleIntelE1000e.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x100F, 0x02, "AppleIntelE1000e.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1015, 0x02, "AppleIntelE1000e.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x10D3, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x10D5, 0x02, "AppleIntelE1000e.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1130, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1131, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1132, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1133, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1137, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1138, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113A, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113B, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113C, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113D, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113E, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x113F, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1140, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1141, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1142, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1148, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1149, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1150, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x1151, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15A0, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15A1, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15A2, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15A3, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15B7, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15B8, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15C7, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15C8, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15D6, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15D7, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15D8, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15D9, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15E3, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15E4, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F0, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F1, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F2, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F3, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F4, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F5, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F6, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F7, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    {0x8086, 0x15F8, 0x02, "IntelMausi.kext", KEXT_FLAG_NETWORK},
    
    // Realtek
    {0x10EC, 0x8136, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8137, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8138, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8139, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8161, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8162, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8163, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8164, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8167, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8168, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8169, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816A, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816B, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816C, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816D, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816E, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x816F, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8170, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8171, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8172, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8173, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8174, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8175, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8176, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8177, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8178, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8179, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817A, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817B, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817C, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817D, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817E, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x817F, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8185, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8186, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8187, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x818B, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x818C, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x818D, 0x02, "RealtekRTL8111.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8125, 0x02, "RealtekRTL8125.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8129, 0x02, "RealtekRTL8125.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8135, 0x02, "RealtekRTL8125.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8152, 0x02, "RealtekRTL8152.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8153, 0x02, "RealtekRTL8152.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8156, 0x02, "RealtekRTL8125.kext", KEXT_FLAG_NETWORK},
    {0x10EC, 0x8161, 0x02, "RealtekRTL8125.kext", KEXT_FLAG_NETWORK},
    
    // Intel Wi-Fi
    {0x8086, 0x2723, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2724, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x34FE, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2725, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2726, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2727, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2728, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x2729, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x272B, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x272D, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    {0x8086, 0x272F, 0x0D, "AirportItlwm.kext", KEXT_FLAG_WIRELESS},
    
    // Broadcom Wi-Fi
    {0x14E4, 0x0576, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x43A0, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x43B1, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x4353, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x4354, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x4360, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x4361, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x43A3, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x43D0, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    {0x14E4, 0x43E0, 0x0D, "AirportBrcmFixup.kext", KEXT_FLAG_WIRELESS},
    
    // ========== AUDIO KEXTS ==========
    // All need AppleALC
    {0x10EC, 0x0880, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0883, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0885, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0887, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0889, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0892, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0897, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A00, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A03, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A05, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A08, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A0C, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A10, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A20, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A2D, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A3C, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A48, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A50, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A52, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A55, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A70, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A72, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A73, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A74, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A75, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A76, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A77, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x10EC, 0x0A78, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    
    // VIA
    {0x1106, 0x3288, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x3289, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328A, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328B, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328C, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328D, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328E, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x328F, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x1106, 0x3290, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    
    // Conexant
    {0x14F1, 0x5011, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x5012, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x5013, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x5018, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x5019, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x501A, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x509F, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A0, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A1, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A2, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A3, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A4, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A5, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A6, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A7, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A8, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50A9, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AA, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AB, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AC, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AD, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AE, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    {0x14F1, 0x50AF, 0x04, "AppleALC.kext", KEXT_FLAG_AUDIO},
    
    // ========== STORAGE KEXTS ==========
    // NVMe
    {0x8086, 0x2260, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x8086, 0x2261, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x8086, 0x2262, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x8086, 0x5001, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA800, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA801, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA802, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA804, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA805, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA806, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA807, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x144D, 0xA808, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5001, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5002, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5003, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5004, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5005, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5006, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x5007, 0x01, "NVMeFix.kext", KEXT_FLAG_STORAGE},
    
    // AHCI
    {0x8086, 0x2829, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x8086, 0x2825, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1022, 0x7801, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1002, 0x4390, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1002, 0x4391, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1002, 0x4392, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1002, 0x4393, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    {0x1002, 0x4394, 0x01, "AHCIPortInjector.kext", KEXT_FLAG_STORAGE},
    
    // USB
    {0x8086, 0x8CB1, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0x8CB2, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0x8CB3, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0x8D26, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0x8D2D, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0x8D7F, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0AF, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B0, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B1, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B2, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B3, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B4, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B5, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B6, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B7, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    {0x8086, 0xA0B8, 0x0C, "USBInjectAll.kext", KEXT_FLAG_USB},
    
    // ========== END MARKER ==========
    {0xFFFF, 0xFFFF, 0xFF, "", 0}
};

// Essential kexts - always needed
CHAR8 *gEssentialKexts[] = {
    "Lilu.kext",
    "VirtualSMC.kext",
    NULL
};

/**
 * Find kext for device
 */
CHAR8* FindKextForDevice(UINT16 VendorID, UINT16 DeviceID, UINT8 ClassCode) {
    UINTN i = 0;
    
    while (gKextDatabase[i].VendorID != 0xFFFF) {
        if (gKextDatabase[i].VendorID == VendorID &&
            gKextDatabase[i].DeviceID == DeviceID) {
            return gKextDatabase[i].KextName;
        }
        i++;
    }
    
    // Try class-based matching
    i = 0;
    while (gKextDatabase[i].VendorID != 0xFFFF) {
        if (gKextDatabase[i].ClassCode == ClassCode &&
            gKextDatabase[i].VendorID == 0xFFFF) {
            return gKextDatabase[i].KextName;
        }
        i++;
    }
    
    return NULL;
}

/**
 * Main kext selection
 */
EFI_STATUS SelectKexts(SYSTEM_INFO *SysInfo, KEXT_LIST *KextList) {
    UINTN i;
    BOOLEAN Added;
    
    KextList->Count = 0;
    
    // Add essential kexts first
    Log("Adding essential kexts...");
    for (i = 0; gEssentialKexts[i] != NULL; i++) {
        KEXT_INFO *Kext = &KextList->Kexts[KextList->Count];
        StrCpy(Kext->Name, gEssentialKexts[i]);
        Kext->Enabled = TRUE;
        Kext->IsEssential = TRUE;
        Kext->Priority = i;
        KextList->Count++;
        
        Log("  + %s (essential)", gEssentialKexts[i]);
    }
    
    // Add GPU kexts
    Log("Processing GPUs...");
    for (i = 0; i < SysInfo->GPUCount; i++) {
        GPU_INFO *GPU = &SysInfo->GPUs[i];
        
        // Find kext
        CHAR8 *KextName = FindKextForDevice(GPU->VendorID, GPU->DeviceID, GPU->ClassCode);
        
        if (KextName != NULL) {
            // Check if already added
            Added = FALSE;
            for (UINTN j = 0; j < KextList->Count; j++) {
                if (StrCmp(KextList->Kexts[j].Name, KextName) == 0) {
                    Added = TRUE;
                    break;
                }
            }
            
            if (!Added) {
                KEXT_INFO *Kext = &KextList->Kexts[KextList->Count];
                StrCpy(Kext->Name, KextName);
                Kext->Enabled = TRUE;
                Kext->IsEssential = FALSE;
                Kext->Priority = 100;
                KextList->Count++;
                
                Log("  + %s (GPU: %04x:%04x)", KextName, GPU->VendorID, GPU->DeviceID);
            }
        }
    }
    
    // Add Network kexts
    Log("Processing Network controllers...");
    for (i = 0; i < SysInfo->NetworkCount; i++) {
        NETWORK_INFO *Net = &SysInfo->Networks[i];
        
        CHAR8 *KextName = FindKextForDevice(Net->VendorID, Net->DeviceID, Net->ClassCode);
        
        if (KextName != NULL) {
            Added = FALSE;
            for (UINTN j = 0; j < KextList->Count; j++) {
                if (StrCmp(KextList->Kexts[j].Name, KextName) == 0) {
                    Added = TRUE;
                    break;
                }
            }
            
            if (!Added) {
                KEXT_INFO *Kext = &KextList->Kexts[KextList->Count];
                StrCpy(Kext->Name, KextName);
                Kext->Enabled = TRUE;
                Kext->IsEssential = FALSE;
                Kext->Priority = 200;
                KextList->Count++;
                
                Log("  + %s (Network: %04x:%04x)", KextName, Net->VendorID, Net->DeviceID);
            }
        }
    }
    
    // Add Audio kexts
    Log("Processing Audio codecs...");
    for (i = 0; i < SysInfo->AudioCount; i++) {
        AUDIO_INFO *Audio = &SysInfo->AudioCodecs[i];
        
        // Audio always uses AppleALC
        Added = FALSE;
        for (UINTN j = 0; j < KextList->Count; j++) {
            if (StrCmp(KextList->Kexts[j].Name, "AppleALC.kext") == 0) {
                Added = TRUE;
                break;
            }
        }
        
        if (!Added) {
            KEXT_INFO *Kext = &KextList->Kexts[KextList->Count];
            StrCpy(Kext->Name, "AppleALC.kext");
            Kext->Enabled = TRUE;
            Kext->IsEssential = FALSE;
            Kext->Priority = 150;
            KextList->Count++;
            
            Log("  + AppleALC.kext (Audio: %04x:%04x)", Audio->VendorID, Audio->DeviceID);
        }
    }
    
    // Add Storage kexts
    Log("Processing Storage controllers...");
    for (i = 0; i < SysInfo->StorageCount; i++) {
        STORAGE_INFO *Storage = &SysInfo->Storage[i];
        
        CHAR8 *KextName = FindKextForDevice(Storage->VendorID, Storage->DeviceID, Storage->ClassCode);
        
        if (KextName != NULL) {
            Added = FALSE;
            for (UINTN j = 0; j < KextList->Count; j++) {
                if (StrCmp(KextList->Kexts[j].Name, KextName) == 0) {
                    Added = TRUE;
                    break;
                }
            }
            
            if (!Added) {
                KEXT_INFO *Kext = &KextList->Kexts[KextList->Count];
                StrCpy(Kext->Name, KextName);
                Kext->Enabled = TRUE;
                Kext->IsEssential = FALSE;
                Kext->Priority = 300;
                KextList->Count++;
                
                Log("  + %s (Storage: %04x:%04x)", KextName, Storage->VendorID, Storage->DeviceID);
            }
        }
    }
    
    // Add Wireless kexts
    Log("Processing Wireless controllers...");
    for (i = 0; i < SysInfo->USBCount; i++) {
        // Would process USB wireless here
    }
    
    Log("Kext selection complete: %d kexts", KextList->Count);
    
    return EFI_SUCCESS;
}

/**
 * Check if CPU is in list
 */
static BOOLEAN IsCPUInList(CHAR8 *CPU, CHAR8 **List) {
    UINTN i = 0;
    while (List[i] != NULL) {
        if (StrCmp(CPU, List[i]) == 0) {
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

/**
 * Check if patch applies to current CPU
 */
static BOOLEAN PatchAppliesToCPU(PATCH_ENTRY *Patch, CHAR8 *CPUType) {
    UINTN i = 0;
    while (Patch->TargetCPU[i] != NULL) {
        if (StrCmp(Patch->TargetCPU[i], "all") == 0) {
            return TRUE;
        }
        if (StrCmp(Patch->TargetCPU[i], CPUType) == 0) {
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

/**
 * Determine patch quantity based on hardware
 * 
 * Returns recommended number of patches:
 * - Legacy CPUs (Pentium4, Core2): 8-12 patches
 * - Old Intel (Nehalem-Westmere): 6-8 patches
 * - Modern Intel (SandyBridge+): 4-6 patches
 * - AMD (all): 5-7 patches
 */
static UINTN CalculatePatchCount(CHAR8 *CPUType, CHAR8 *MacOSVersion) {
    // Legacy CPUs need more patches
    if (IsCPUInList(CPUType, gLegacyCPUs)) {
        return 10;  // Legacy: more patches for compatibility
    }
    
    // AMD needs moderate patches
    if (IsCPUInList(CPUType, gAMDs)) {
        return 6;
    }
    
    // Nehalem-Westmere
    if (StrCmp(CPUType, "Nehalem") == 0 || StrCmp(CPUType, "Westmere") == 0) {
        return 7;
    }
    
    // SandyBridge to KabyLake
    if (StrCmp(CPUType, "SandyBridge") == 0 || 
        StrCmp(CPUType, "IvyBridge") == 0 ||
        StrCmp(CPUType, "Haswell") == 0 ||
        StrCmp(CPUType, "Broadwell") == 0 ||
        StrCmp(CPUType, "Skylake") == 0 ||
        StrCmp(CPUType, "KabyLake") == 0) {
        return 5;
    }
    
    // Modern (CoffeeLake+)
    return 4;
}

/**
 * Select patches based on hardware and macOS version
 * 
 * Selection criteria:
 * 1. Critical patches always applied
 * 2. CPU-specific patches
 * 3. Legacy patches for old hardware
 * 4. macOS version compatibility
 */
EFI_STATUS SelectPatches(CHAR8 *CPUType, CHAR8 *MacOSVersion, PATCH_LIST *PatchList) {
    UINTN i;
    UINTN PatchCount = 0;
    UINTN MaxPatches;
    BOOLEAN AlreadyAdded;
    
    PatchList->Count = 0;
    
    // Calculate recommended patch count based on hardware
    MaxPatches = CalculatePatchCount(CPUType, MacOSVersion);
    
    Log("Selecting patches for CPU: %s, macOS: %s", CPUType, MacOSVersion);
    Log("Target patch count: %d", MaxPatches);
    
    // First pass: Add critical patches (always needed)
    Log("Applying critical patches...");
    for (i = 0; gPatchDatabase[i].PatchName != NULL; i++) {
        if (!gPatchDatabase[i].IsCritical) continue;
        if (!PatchAppliesToCPU(&gPatchDatabase[i], CPUType)) continue;
        
        // Check macOS version compatibility
        // (simplified - would check min_os/max_os)
        
        // Add patch
        PatchList->Patches[PatchList->Count].Enabled = TRUE;
        StrCpy(PatchList->Patches[PatchList->Count].Name, gPatchDatabase[i].PatchName);
        PatchList->Patches[PatchList->Count].Priority = gPatchDatabase[i].Priority;
        PatchList->Count++;
        PatchCount++;
        
        Log("  + [CRITICAL] %s", gPatchDatabase[i].PatchName);
    }
    
    // Second pass: Add recommended patches
    if (PatchCount < MaxPatches) {
        Log("Applying recommended patches...");
        for (i = 0; gPatchDatabase[i].PatchName != NULL && PatchCount < MaxPatches; i++) {
            if (!gPatchDatabase[i].IsRecommended) continue;
            if (!PatchAppliesToCPU(&gPatchDatabase[i], CPUType)) continue;
            
            // Check if already added
            AlreadyAdded = FALSE;
            for (UINTN j = 0; j < PatchList->Count; j++) {
                if (StrCmp(PatchList->Patches[j].Name, gPatchDatabase[i].PatchName) == 0) {
                    AlreadyAdded = TRUE;
                    break;
                }
            }
            
            if (!AlreadyAdded) {
                PatchList->Patches[PatchList->Count].Enabled = TRUE;
                StrCpy(PatchList->Patches[PatchList->Count].Name, gPatchDatabase[i].PatchName);
                PatchList->Patches[PatchList->Count].Priority = gPatchDatabase[i].Priority;
                PatchList->Count++;
                PatchCount++;
                
                Log("  + [RECOMMENDED] %s", gPatchDatabase[i].PatchName);
            }
        }
    }
    
    // Third pass: Add CPU-specific patches
    if (PatchCount < MaxPatches) {
        Log("Applying CPU-specific patches...");
        for (i = 0; gPatchDatabase[i].PatchName != NULL && PatchCount < MaxPatches; i++) {
            if (gPatchDatabase[i].IsCritical || gPatchDatabase[i].IsRecommended) continue;
            if (!PatchAppliesToCPU(&gPatchDatabase[i], CPUType)) continue;
            
            // Check if already added
            AlreadyAdded = FALSE;
            for (UINTN j = 0; j < PatchList->Count; j++) {
                if (StrCmp(PatchList->Patches[j].Name, gPatchDatabase[i].PatchName) == 0) {
                    AlreadyAdded = TRUE;
                    break;
                }
            }
            
            if (!AlreadyAdded) {
                PatchList->Patches[PatchList->Count].Enabled = TRUE;
                StrCpy(PatchList->Patches[PatchList->Count].Name, gPatchDatabase[i].PatchName);
                PatchList->Patches[PatchList->Count].Priority = gPatchDatabase[i].Priority;
                PatchList->Count++;
                PatchCount++;
                
                Log("  + %s", gPatchDatabase[i].PatchName);
            }
        }
    }
    
    // Fourth pass: Legacy patches for old hardware
    if (IsCPUInList(CPUType, gLegacyCPUs) && PatchCount < MaxPatches) {
        Log("Applying legacy patches...");
        for (i = 0; gPatchDatabase[i].PatchName != NULL && PatchCount < MaxPatches; i++) {
            if (!gPatchDatabase[i].IsLegacyOnly) continue;
            if (!PatchAppliesToCPU(&gPatchDatabase[i], CPUType)) continue;
            
            // Check if already added
            AlreadyAdded = FALSE;
            for (UINTN j = 0; j < PatchList->Count; j++) {
                if (StrCmp(PatchList->Patches[j].Name, gPatchDatabase[i].PatchName) == 0) {
                    AlreadyAdded = TRUE;
                    break;
                }
            }
            
            if (!AlreadyAdded) {
                PatchList->Patches[PatchList->Count].Enabled = TRUE;
                StrCpy(PatchList->Patches[PatchList->Count].Name, gPatchDatabase[i].PatchName);
                PatchList->Patches[PatchList->Count].Priority = gPatchDatabase[i].Priority;
                PatchList->Count++;
                PatchCount++;
                
                Log("  + [LEGACY] %s", gPatchDatabase[i].PatchName);
            }
        }
    }
    
    Log("Patch selection complete: %d patches applied", PatchList->Count);
    
    return EFI_SUCCESS;
}
