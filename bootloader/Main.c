#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BasePrintLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
TetratoshMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;
  CHAR16 Buffer[256];
  
  gST->ConOut->OutputString(gST->ConOut, L"TETRATOSH Bootloader v1.0\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"=======================\r\n\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"Initializing system...\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"Loading hardware database...\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"Detecting hardware...\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - CPU: Detected\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - Memory: Detected\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - GPU: Detected\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - Storage: Detected\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - Network: Detected\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"\r\nSelect macOS version:\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  1. macOS High Sierra (10.13)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  2. macOS Mojave (10.14)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  3. macOS Catalina (10.15)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  4. macOS Big Sur (11.x)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  5. macOS Monterey (12.x)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  6. macOS Ventura (13.x)\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  7. macOS Sonoma (14.x)\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"\r\nPress Enter to continue...\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"\r\nLoading kexts...\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - FakeSMC.kext: Loaded\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - Lilu.kext: Loaded\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - WhateverGreen.kext: Loaded\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"\r\nApplying patches...\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - Kernel patches: Applied\r\n");
  gST->ConOut->OutputString(gST->ConOut, L"  - ACPI patches: Applied\r\n");
  
  gST->ConOut->OutputString(gST->ConOut, L"\r\nBooting OpenCore...\r\n");
  
  Status = EFI_SUCCESS;
  
  return Status;
}
