/**
 * TETRATOSH - Boot Menu UI
 * 
 * Menu de seleção de macOS no boot
 */

#include "tetratosh.h"

/**
 * macOS Versions available
 */
typedef struct {
    CHAR8 Name[32];
    CHAR8 Version[16];
    UINT8 Major;
    UINT8 Minor;
    UINT32 MinYear;
    UINT32 MaxYear;
    BOOLEAN IsSupported;
    CHAR8 *SMBIOS;
    CHAR8 *Description;
} MACOS_ENTRY;

// macOS versions supported
MACOS_ENTRY gMacOSVersions[] = {
    {"Sonoma", "14.x", 14, 0, 2019, 2024, TRUE, "MacBookPro18,1", "macOS 14 (Requires newer CPUs)"},
    {"Ventura", "13.x", 13, 0, 2018, 2024, TRUE, "MacBookPro18,1", "macOS 13 (Broadcom/NVIDIA recommended)"},
    {"Monterey", "12.x", 12, 0, 2017, 2024, TRUE, "MacBookPro18,1", "macOS 12 (Best compatibility)"},
    {"Big Sur", "11.x", 11, 0, 2015, 2023, TRUE, "iMac20,1", "macOS 11 (Legacy but stable)"},
    {"Catalina", "10.15", 10, 15, 2013, 2022, TRUE, "iMac19,1", "macOS 10.15 (Very stable)"},
    {"Mojave", "10.14", 10, 14, 2012, 2021, TRUE, "iMac18,3", "macOS 10.14 (Legacy)"},
    {"High Sierra", "10.13", 10, 13, 2007, 2019, TRUE, "iMac18,1", "macOS 10.13 (Legacy hardware)"},
    {"", "", 0, 0, 0, 0, FALSE, "", ""} // End marker
};

/**
 * User selection
 */
typedef struct {
    UINT8 SelectedIndex;
    MACOS_ENTRY *SelectedOS;
    BOOLEAN Verbose;
    BOOLEAN SafeMode;
    BOOLEAN DebugMode;
    UINTN Timeout; // seconds
} BOOT_OPTIONS;

BOOT_OPTIONS gBootOptions;

/**
 * Draw a box on screen
 */
VOID DrawBox(UINTN X, UINTN Y, UINTN W, UINTN H, CHAR16 *Title) {
    UINTN i;
    
    // Top border
    PrintAt(X, Y, L"┌");
    for (i = 1; i < W - 1; i++) PrintAt(X + i, Y, L"─");
    PrintAt(X + W - 1, Y, L"┐");
    
    // Sides
    for (i = 1; i < H - 1; i++) {
        PrintAt(X, Y + i, L"│");
        PrintAt(X + W - 1, Y + i, L"│");
    }
    
    // Bottom border
    PrintAt(X, Y + H - 1, L"└");
    for (i = 1; i < W - 1; i++) PrintAt(X + i, Y + H - 1, L"─");
    PrintAt(X + W - 1, Y + H - 1, L"┘");
    
    // Title
    if (Title != NULL) {
        UINTN TitleLen = StrLen(Title);
        UINTN TitleX = X + (W - TitleLen) / 2;
        PrintAt(TitleX, Y, L" %s ", Title);
    }
}

/**
 * Show boot menu
 */
UINTN ShowBootMenu(VOID) {
    UINTN Selected = 0;
    UINTN Count = 0;
    UINTN MaxVisible = 12;
    UINTN StartY = 8;
    UINTN X = 15;
    UINTN Y = StartY;
    UINTN W = 50;
    UINTN H = 18;
    EFI_INPUT_KEY Key;
    BOOLEAN Done = FALSE;
    
    // Count available OS
    while (gMacOSVersions[Count].IsSupported) Count++;
    
    // Clear screen
    gST->ConOut->ClearScreen(gST->ConOut);
    
    while (!Done) {
        // Draw main box
        DrawBox(0, 0, 80, 25, L" TETRATOSH - Select macOS Version ");
        
        // Draw info
        PrintAt(2, 2, L"TETRATOSH v%s - Automated Hackintosh Bootloader", TETRATOSH_VERSION);
        PrintAt(2, 3, L"===============================================================================");
        
        // Instructions
        PrintAt(2, 22, L"[↑/↓] Select  [Enter] Boot  [V] Verbose  [S] Safe Mode");
        
        // Draw menu items
        Y = StartY;
        for (UINTN i = 0; i < Count; i++) {
            if (i == Selected) {
                PrintAt(X + 2, Y, L" ▶ %a - %a", 
                    gMacOSVersions[i].Name, 
                    gMacOSVersions[i].Description);
            } else {
                PrintAt(X + 2, Y, L"   %a - %a", 
                    gMacOSVersions[i].Name, 
                    gMacOSVersions[i].Description);
            }
            Y++;
        }
        
        // Draw details box
        DrawBox(2, 22, 76, 3, NULL);
        PrintAt(4, 23, L"Selected: %a %a | Compatible with detected hardware", 
            gMacOSVersions[Selected].Name,
            gMacOSVersions[Selected].Version);
        
        // Wait for key
        gST->BootServices->WaitForEvent(1, &gST->ConIn->WaitForKey, NULL);
        gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        
        switch (Key.UnicodeChar) {
            case 0x1B: // Escape
                // Arrow keys
                break;
            case 0x00:
                if (Key.ScanCode == 0x01) { // Up
                    if (Selected > 0) Selected--;
                } else if (Key.ScanCode == 0x02) { // Down
                    if (Selected < Count - 1) Selected++;
                }
                break;
            case 'v':
            case 'V':
                gBootOptions.Verbose = !gBootOptions.Verbose;
                break;
            case 's':
            case 'S':
                gBootOptions.SafeMode = !gBootOptions.SafeMode;
                break;
            case 0x0D: // Enter
            case 0x0A:
                Done = TRUE;
                break;
        }
    }
    
    return Selected;
}

/**
 * Get quirks for selected macOS version
 */
VOID GetQuirksForVersion(MACOS_ENTRY *OS, KERNEL_QUIRKS *Quirks) {
    // Reset quirks
    ZeroMem(Quirks, sizeof(KERNEL_QUIRKS));
    
    // Based on macOS version
    switch (OS->Major) {
        case 14: // Sonoma
        case 13: // Ventura
        case 12: // Monterey
            Quirks->CpuTscSync = TRUE;
            Quirks->XhciPortLimit = TRUE;
            Quirks->AppleCpuPmCfgLock = FALSE;
            Quirks->AppleXcpmCfgLock = FALSE;
            break;
            
        case 11: // Big Sur
        case 10: // Catalina/Mojave/High Sierra
            Quirks->CpuTscSync = TRUE;
            Quirks->XhciPortLimit = TRUE;
            Quirks->AppleCpuPmCfgLock = FALSE;
            Quirks->AppleXcpmCfgLock = FALSE;
            Quirks->ThirdPartyDrives = TRUE;
            break;
            
        default:
            Quirks->XhciPortLimit = TRUE;
            break;
    }
}

/**
 * Get SMBIOS for selected OS
 */
CHAR8* GetSMBIOSForOS(MACOS_ENTRY *OS, CPU_INFO *CPU) {
    // Based on CPU generation
    if (CPU->Vendor == 0x68747541) { // AMD
        return "MacPro7,1";
    }
    
    // Intel
    switch (OS->Major) {
        case 14: // Sonoma
            if (CPU->Flags & CPU_FLAG_ALDER) return "MacBookPro18,1";
            return "iMac22,1";
            
        case 13: // Ventura
            if (CPU->Flags & CPU_FLAG_ALDER) return "MacBookPro18,1";
            return "iMac21,1";
            
        case 12: // Monterey
            if (CPU->Flags & CPU_FLAG_ALDER) return "MacBookPro18,1";
            return "iMac20,1";
            
        case 11: // Big Sur
            return "iMac20,1";
            
        case 10:
            if (OS->Minor == 15) return "iMac19,1";
            if (OS->Minor == 14) return "iMac18,3";
            return "iMac18,1"; // High Sierra
            
        default:
            return "iMac18,1";
    }
}

/**
 * Get boot args for selected OS
 */
VOID GetBootArgsForOS(MACOS_ENTRY *OS, CHAR8 *Args, UINTN Size) {
    // Base args
    StrCpy(Args, "-v");
    
    if (gBootOptions.Verbose) {
        // Already verbose
    }
    
    if (gBootOptions.SafeMode) {
        StrCat(Args, " -x");
    }
    
    // Version specific args
    switch (OS->Major) {
        case 14: // Sonoma
            StrCat(Args, " liludetect=1 keepsyms=1");
            break;
        case 13: // Ventura
            StrCat(Args, " liludetect=1 keepsyms=1");
            break;
        case 12: // Monterey
            StrCat(Args, " liludetect=1 keepsyms=1");
            break;
        case 11: // Big Sur
            StrCat(Args, " liludetect=1");
            break;
        case 10:
            if (OS->Minor >= 14) { // Mojave
                StrCat(Args, " -liludetect");
            }
            break;
    }
    
    // TSC sync for some CPUs
    if (gSystemInfo.CPU.Flags & CPU_FLAG_SANDY ||
        gSystemInfo.CPU.Flags & CPU_FLAG_IVY) {
        StrCat(Args, " -tscsync");
    }
}
