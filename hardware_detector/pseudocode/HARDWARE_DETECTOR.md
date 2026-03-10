# Módulo: Hardware Detector - Pseudocódigo

## 1. Interface do Módulo

```c
// hardware_detector.h

#ifndef HARDWARE_DETECTOR_H
#define HARDWARE_DETECTOR_H

#include "pci_scanner.h"
#include "acpi_parser.h"
#include "device_database.h"

typedef struct {
    // CPU Info
    CHAR8 CPUVendor[16];
    CHAR8 CPUBrand[64];
    UINT32 CPUFamily;
    UINT32 CPUModel;
    UINT32 CPUStepping;
    UINT32 CPUCores;
    UINT32 CPUThreads;
    CHAR8 CPUArchitecture[16];  // "Skylake", "KabyLake", "Zen", etc.
    
    // Memory Info
    UINT64 RAMTotal;
    UINT32 RAMSlots;
    UINT64 RAMModules[8];
    
    // GPU Info
    UINT32 GPUCount;
    GPUInfo GPUs[8];
    
    // Network Info
    UINT32 NetworkCount;
    DeviceInfo NetworkControllers[8];
    
    // Audio Info
    UINT32 AudioCount;
    DeviceInfo AudioCodecs[4];
    
    // Storage Info
    UINT32 StorageCount;
    DeviceInfo StorageControllers[8];
    
    // Wireless Info
    UINT32 WirelessCount;
    DeviceInfo WirelessControllers[4];
    
    // Chipset Info
    CHAR8 ChipsetVendor[16];
    CHAR8 ChipsetModel[32];
    
    // Motherboard Info
    CHAR8 MotherboardVendor[32];
    CHAR8 MotherboardModel[32];
    CHAR8 BIOSVendor[16];
    CHAR8 BIOSVersion[16];
    
} SystemInfo;

EFI_STATUS DetectHardware(SystemInfo *Info);
VOID FreeSystemInfo(SystemInfo *Info);

#endif
```

## 2. Implementação Principal

```c
// hardware_detector.c

EFI_STATUS DetectHardware(SystemInfo *Info) {
    EFI_STATUS Status;
    
    Log("Starting hardware detection...");
    
    // Inicializar estrutura
    ZeroMem(Info, sizeof(SystemInfo));
    
    // 1. Detectar CPU
    Log("Detecting CPU...");
    Status = DetectCPU(Info);
    SE Status != EFI_SUCCESS:
        LogError("CPU detection failed");
        RETORNAR Status;
    FIM SE
    Log("CPU: %a", Info->CPUBrand);
    
    // 2. Detectar Memória
    Log("Detecting memory...");
    Status = DetectRAM(Info);
    SE Status != EFI_SUCCESS:
        LogError("RAM detection failed");
    FIM SE
    Log("RAM: %lu MB", Info->RAMTotal / (1024 * 1024));
    
    // 3. Escanear PCI
    Log("Scanning PCI bus...");
    Status = PCIScan(Info);
    SE Status != EFI_SUCCESS:
        LogError("PCI scan failed");
        RETORNAR Status;
    FIM SE
    Log("Found %d PCI devices", Info->PCIDeviceCount);
    
    // 4. Parse ACPI
    Log("Parsing ACPI tables...");
    Status = ACPIParse(Info);
    SE Status != EFI_SUCCESS:
        LogWarn("ACPI parsing had issues");
    FIM SE
    
    // 5. Classificar Dispositivos
    Log("Classifying devices...");
    ClassifyDevices(Info);
    
    // 6. Detectar Chipset
    Log("Detecting chipset...");
    DetectChipset(Info);
    
    Log("Hardware detection complete!");
    
    RETORNAR EFI_SUCCESS;
}
```

## 3. Detecção de CPU

```c
// cpu_detector.c

EFI_STATUS DetectCPU(SystemInfo *Info) {
    UINT32 Eax, Ebx, Ecx, Edx;
    
    // CPUID função 0 - Get vendor string
    AsmCpuid(0, &Eax, &Ebx, &Ecx, &Edx);
    
    // Determinar vendor
    SE Ebx == 'Genu' && Ecx == 'ineI' && Edx == 'ntel':
        CopyMem(Info->CPUVendor, "GenuineIntel", 12);
    SENÃO SE Ebx == 'Auth' && Ecx == 'entic' && Edx == 'AMD':
        CopyMem(Info->CPUVendor, "AuthenticAMD", 12);
    SENÃO:
        CopyMem(Info->CPUVendor, "Unknown", 8);
    FIM SE
    
    // CPUID função 1 - Get CPU info
    AsmCpuid(1, &Eax, &Ebx, &Ecx, &Edx);
    
    Info->CPUFamily = (Eax >> 8) & 0xF;
    Info->CPUModel = ((Eax >> 4) & 0xF) | ((Eax >> 12) & 0xF0);
    Info->CPUStepping = Eax & 0xF;
    
    // Brand string - funções 0x80000002 a 0x80000004
    CHAR8 Brand[48];
    ZeroMem(Brand, 48);
    
    PARA i = 0 ATÉ 3:
        AsmCpuid(0x80000002 + i, &Eax, &Ebx, &Ecx, &Edx);
        CopyMem(Brand + i*16 + 0,  &Eax, 4);
        CopyMem(Brand + i*16 + 4,  &Ebx, 4);
        CopyMem(Brand + i*16 + 8,  &Ecx, 4);
        CopyMem(Brand + i*16 + 12, &Edx, 4);
    FIM PARA
    
    // Limpar espaços
    TrimSpaces(Brand);
    CopyMem(Info->CPUBrand, Brand, 64);
    
    // Determinar arquitetura
    DetermineCPUArchitecture(Info);
    
    // Contar núcleos e threads
    CountCPUCores(Info);
    
    RETORNAR EFI_SUCCESS;
}

VOID DetermineCPUArchitecture(SystemInfo *Info) {
    // Intel
    SE AsciiStrCmp(Info->CPUVendor, "GenuineIntel") == 0:
        // Determinar geração pelo model/stepping
        SE Info->CPUModel == 0x9E:  // Kaby Lake
            AsciiStrCpy(Info->CPUArchitecture, "KabyLake");
        SENÃO SE Info->CPUModel == 0x9E:  // Coffee Lake
            AsciiStrCpy(Info->CPUArchitecture, "CoffeeLake");
        SENÃO SE Info->CPUModel == 0xA5:  // Comet Lake
            AsciiStrCpy(Info->CPUArchitecture, "CometLake");
        SENÃO SE Info->CPUModel == 0x8A:  // Ice Lake
            AsciiStrCpy(Info->CPUArchitecture, "IceLake");
        SENÃO SE Info->CPUModel == 0xA7:  // Rocket Lake
            AsciiStrCpy(Info->CPUArchitecture, "RocketLake");
        SENÃO SE Info->CPUModel == 0xA6:  // Alder Lake
            AsciiStrCpy(Info->CPUArchitecture, "AlderLake");
        SENÃO:
            AsciiStrCpy(Info->CPUArchitecture, "Unknown");
        FIM SE
    FIM SE
    
    // AMD
    SENÃO SE AsciiStrCmp(Info->CPUVendor, "AuthenticAMD") == 0:
        // AMD Zen family
        UINT32 ExtModel = (Info->CPUModel >> 4) & 0xF;
        
        SE ExtModel >= 0x30:
            AsciiStrCpy(Info->CPUArchitecture, "Zen4");
        SENÃO SE ExtModel >= 0x20:
            AsciiStrCpy(Info->CPUArchitecture, "Zen3");
        SENÃO SE ExtModel >= 0x10:
            AsciiStrCpy(Info->CPUArchitecture, "Zen2");
        SENÃO SE ExtModel >= 0x08:
            AsciiStrCpy(Info->CPUArchitecture, "Zen+");
        SENÃO:
            AsciiStrCpy(Info->CPUArchitecture, "Zen");
        FIM SE
    FIM SE
}

VOID CountCPUCores(SystemInfo *Info) {
    UINT32 Eax, Ebx, Ecx, Edx;
    
    // CPUID função 4 - Intel topology
    AsmCpuid(4, 0, &Eax, &Ebx, &Ecx, &Edx);
    
    UINT32 CoresPerPackage = ((Eax >> 26) & 0x3F) + 1;
    
    // CPUID função 1 - Thread count
    AsmCpuid(1, &Eax, &Ebx, &Ecx, &Edx);
    
    UINT32 ThreadsPerCore = ((Edx >> 16) & 0xFF);
    
    Info->CPUCores = CoresPerPackage;
    Info->CPUThreads = CoresPerPackage * ThreadsPerCore;
}
```

## 4. Detecção de Memória

```c
// memory_detector.c

EFI_STATUS DetectRAM(SystemInfo *Info) {
    EFI_STATUS Status;
    
    // Método 1: Via SMBIOS
    Status = DetectRAMFromSMBIOS(Info);
    SE Status == EFI_SUCCESS:
        RETORNAR EFI_SUCCESS;
    FIM SE
    
    // Método 2: Via E820
    Status = DetectRAMFromE820(Info);
    SE Status == EFI_SUCCESS:
        RETORNAR EFI_SUCCESS;
    FIM SE
    
    // Método 3: Via UEFI Memory Map
    DetectRAMFromMemoryMap(Info);
    
    RETORNAR EFI_SUCCESS;
}

EFI_STATUS DetectRAMFromSMBIOS(SystemInfo *Info) {
    EFI_SMBIOS_PROTOCOL *Smbios;
    EFI_SMBIOS_HANDLE Handle;
    EFI_SMBIOS_TABLE_HEADER *Entry;
    EFI_STATUS Status;
    
    // Localizar SMBIOS
    Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, &Smbios);
    SE Status != EFI_SUCCESS:
        RETORNAR Status;
    FIM SE
    
    // Buscar Type 17 (Memory Device)
    Handle = SMBIOS_HANDLE_NULL;
    
    ENQUANTO TRUE:
        Status = Smbios->GetNext(Smbios, &Handle, NULL, &Entry, NULL);
        
        SE Status != EFI_SUCCESS:
            BREAK;
        FIM SE
        
        SE Entry->Type == 17:  // Memory Device
            SMBIOS_TABLE_TYPE17 *Mem = (SMBIOS_TABLE_TYPE17 *)Entry;
            
            // Calcular tamanho
            UINT64 Size = Mem->Size;
            SE Size == 0xFFFF:
                // Tamanho não especificado
                CONTINUE;
            FIM SE
            
            SE (Mem->Size & 0x8000):
                // Tamanho em KB
                Size = (Size & 0x7FFF) * 1024;
            SENÃO:
                // Tamanho em MB
                Size = Size * 1024 * 1024;
            FIM SE
            
            Info->RAMTotal += Size;
            Info->RAMSlots++;
        FIM SE
    FIM ENQUANTO
    
    RETORNAR EFI_SUCCESS;
}

EFI_STATUS DetectRAMFromE820(SystemInfo *Info) {
    EFI_E820_ENTRY_EFI *E820;
    UINTN Count;
    UINTN i;
    UINT64 Total = 0;
    
    // Chamar GetMemoryMap
    gBS->GetMemoryMap(&Count, &E820, NULL, NULL, NULL);
    
    // Filtrar only usable memory (type 1)
    PARA i = 0 ATÉ Count:
        SE E820[i].Type == 1:  // EfiACPIReclaimMemory
            Total += E820[i].Length;
        FIM SE
    FIM PARA
    
    Info->RAMTotal = Total;
    RETORNAR EFI_SUCCESS;
}
```

## 5. Classificação de Dispositivos

```c
// device_classifier.c

VOID ClassifyDevices(SystemInfo *Info) {
    PCI_DEVICE *Devices;
    UINTN DeviceCount;
    UINTN i;
    
    Devices = GetPCIDeviceList();
    DeviceCount = GetPCIDeviceCount();
    
    Info->GPUCount = 0;
    Info->NetworkCount = 0;
    Info->AudioCount = 0;
    Info->StorageCount = 0;
    Info->WirelessCount = 0;
    
    PARA i = 0 ATÉ DeviceCount:
        PCI_DEVICE *Dev = &Devices[i];
        
        // Class Code 0x03 - Display Controller (GPU)
        SE Dev->ClassCode == 0x03:
            ClassifyGPU(Info, Dev);
        FIM SE
        
        // Class Code 0x02 - Network Controller
        SE Dev->ClassCode == 0x02:
            ClassifyNetwork(Info, Dev);
        FIM SE
        
        // Class Code 0x04 - Multimedia (Audio)
        SE Dev->ClassCode == 0x04 && Dev->Subclass == 0x03:
            ClassifyAudio(Info, Dev);
        FIM SE
        
        // Class Code 0x01 - Mass Storage
        SE Dev->ClassCode == 0x01:
            ClassifyStorage(Info, Dev);
        FIM SE
        
        // Class Code 0x0D - Wireless
        SE Dev->ClassCode == 0x0D:
            ClassifyWireless(Info, Dev);
        FIM SE
    FIM PARA
}

VOID ClassifyGPU(SystemInfo *Info, PCI_DEVICE *Dev) {
    SE Info->GPUCount >= 8:
        RETORNAR;
    FIM SE
    
    GPUInfo *GPU = &Info->GPUs[Info->GPUCount];
    
    GPU->VendorID = Dev->VendorID;
    GPU->DeviceID = Dev->DeviceID;
    GPU->Revision = Dev->Revision;
    
    // Identificar vendor
    SE Dev->VendorID == 0x8086:
        AsciiStrCpy(GPU->Vendor, "Intel");
        IdentifyIntelGPU(GPU);
    SENÃO SE Dev->VendorID == 0x10DE:
        AsciiStrCpy(GPU->Vendor, "NVIDIA");
        IdentifyNvidiaGPU(GPU);
    SENÃO SE Dev->VendorID == 0x1002:
        AsciiStrCpy(GPU->Vendor, "AMD");
        IdentifyAMDGPU(GPU);
    FIM SE
    
    Info->GPUCount++;
    
    Log("GPU detected: %a %a", GPU->Vendor, GPU->Name);
}
```
