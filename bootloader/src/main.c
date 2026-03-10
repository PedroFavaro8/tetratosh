/**
 * TETRATOSH - Main Entry Point
 * 
 * Bootloader Entry for UEFI Systems
 */

#include "tetratosh.h"
#include "cpu_detect.h"
#include "pci_scan.h"
#include "acpi_parse.h"
#include "kext_selector.h"
#include "config_gen.h"
#include "efi_builder.h"
#include "opencore.h"
#include "ui.h"
#include "logger.h"

// Global variables
SYSTEM_INFO gSystemInfo;
TETRATOSH_CONFIG gConfig;
KEXT_LIST gSelectedKexts;

// Entry point
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
) {
    EFI_STATUS Status;
    
    // Initialize
    gST = SystemTable;
    gImageHandle = ImageHandle;
    
    // Initialize logging
    LogInit();
    
    // Show banner
    ShowBanner();
    
    // Run main routine
    Status = TetratoshMain();
    
    if (Status != EFI_SUCCESS) {
        LogError("Fatal error: 0x%X", Status);
        ShowErrorScreen(Status);
    }
    
    return Status;
}

EFI_STATUS TetratoshMain(VOID) {
    EFI_STATUS Status;
    UINTN Phase = 0;
    UINTN TotalPhases = 6;
    
    // Initialize configuration
    Log("Initializing configuration...");
    Status = InitializeConfig();
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    
    // Phase 1: Hardware Detection
    Phase = 1;
    ShowProgress("Detecting Hardware", Phase, TotalPhases);
    Log("=== Phase 1: Hardware Detection ===");
    
    Status = DetectHardware(&gSystemInfo);
    if (Status != EFI_SUCCESS) {
        LogError("Hardware detection failed!");
        return Status;
    }
    
    ShowHardwareInfo(&gSystemInfo);
    
    // Phase 2: Select Kexts
    Phase = 2;
    ShowProgress("Selecting Kexts", Phase, TotalPhases);
    Log("=== Phase 2: Kext Selection ===");
    
    Status = SelectKexts(&gSystemInfo, &gSelectedKexts);
    if (Status != EFI_SUCCESS) {
        LogError("Kext selection failed!");
        return Status;
    }
    
    ShowKextList(&gSelectedKexts);
    
    // Phase 3: Generate Config
    Phase = 3;
    ShowProgress("Generating Config", Phase, TotalPhases);
    Log("=== Phase 3: Config Generation ===");
    
    Status = GenerateConfig(&gSystemInfo, &gSelectedKexts);
    if (Status != EFI_SUCCESS) {
        LogError("Config generation failed!");
        return Status;
    }
    
    Log("Config generated successfully");
    
    // Phase 4: Build EFI
    Phase = 4;
    ShowProgress("Building EFI", Phase, TotalPhases);
    Log("=== Phase 4: EFI Building ===");
    
    Status = BuildEFI(&gSystemInfo, &gSelectedKexts);
    if (Status != EFI_SUCCESS) {
        LogError("EFI build failed!");
        return Status;
    }
    
    Log("EFI built successfully");
    
    // Phase 5: Verify EFI
    Phase = 5;
    ShowProgress("Verifying EFI", Phase, TotalPhases);
    Log("=== Phase 5: EFI Verification ===");
    
    Status = VerifyEFI(&gSystemInfo);
    if (Status != EFI_SUCCESS) {
        LogWarn("EFI verification had warnings");
    }
    
    // Phase 6: Launch OpenCore
    Phase = 6;
    ShowProgress("Launching OpenCore", Phase, TotalPhases);
    Log("=== Phase 6: Launching OpenCore ===");
    
    Status = LaunchOpenCore();
    if (Status != EFI_SUCCESS) {
        LogError("OpenCore launch failed!");
        return Status;
    }
    
    Log("=== Tetratosh Complete ===");
    
    return EFI_SUCCESS;
}

EFI_STATUS InitializeConfig(VOID) {
    // Default configuration
    gConfig.TargetOS = MACOS_AUTO;
    gConfig.VerboseMode = TRUE;
    gConfig.SafeMode = FALSE;
    gConfig.LegacyMode = FALSE;
    
    // Set output path
    StrCpy(gConfig.OutputPath, L"\\EFI\\Tetratosh");
    StrCpy(gConfig.OpenCorePath, L"\\EFI\\OC\\OpenCore.efi");
    
    // Check boot arguments for overrides
    // (Would parse boot arguments here)
    
    return EFI_SUCCESS;
}
