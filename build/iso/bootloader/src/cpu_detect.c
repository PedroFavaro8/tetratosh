/**
 * TETRATOSH - CPU Detection Module
 * 
 * Supports: Intel (Pentium 4 to 14th Gen) and AMD (K10 to Zen 4)
 */

#include "tetratosh.h"

/**
 * Get CPU vendor string from CPUID
 */
VOID GetCPUVendor(CHAR8 *VendorStr) {
    UINT32 Ebx, Ecx, Edx;
    
    // CPUID function 0
    AsmCpuid(0, NULL, &Ebx, &Ecx, &Edx);
    
    // Intel: "GenuineIntel"
    // AMD: "AuthenticAMD"
    // VIA: "CentaurHauls"
    
    VendorStr[0] = (Eb >> 0) & 0xFF;
    VendorStr[1] = (Eb >> 8) & 0xFF;
    VendorStr[2] = (Eb >> 16) & 0xFF;
    VendorStr[3] = (Eb >> 24) & 0xFF;
    
    VendorStr[4] = (Ec >> 0) & 0xFF;
    VendorStr[5] = (Ec >> 8) & 0xFF;
    VendorStr[6] = (Ec >> 16) & 0xFF;
    VendorStr[7] = (Ec >> 24) & 0xFF;
    
    VendorStr[8] = (Ed >> 0) & 0xFF;
    VendorStr[9] = (Ed >> 8) & 0xFF;
    VendorStr[10] = (Ed >> 16) & 0xFF;
    VendorStr[11] = (Ed >> 24) & 0xFF;
    
    VendorStr[12] = '\0';
}

/**
 * Get CPU Brand String
 */
VOID GetCPUBrand(CHAR8 *BrandStr) {
    UINT32 Eax, Ebx, Ecx, Edx;
    UINT32 i;
    
    // Brand string is in functions 0x80000002 - 0x80000004
    for (i = 0; i < 3; i++) {
        AsmCpuid(0x80000002 + i, &Eax, &Ebx, &Ecx, &Edx);
        
        // Each function returns 16 bytes of brand
        CopyMem(BrandStr + i * 16 + 0,  &Eax, 4);
        CopyMem(BrandStr + i * 16 + 4,  &Ebx, 4);
        CopyMem(BrandStr + i * 16 + 8,  &Ecx, 4);
        CopyMem(BrandStr + i * 16 + 12, &Edx, 4);
    }
    
    // Remove leading spaces
    while (*BrandStr == ' ') BrandStr++;
    
    // Null terminate
    BrandStr[48] = '\0';
}

/**
 * Detect CPU Features
 */
UINT32 GetCPUFlags(VOID) {
    UINT32 Eax, Ebx, Ecx, Edx;
    UINT32 Flags = 0;
    
    // CPUID function 1
    AsmCpuid(1, &Eax, &Ebx, &Ecx, &Edx);
    
    // Check features
    if (Edx & BIT23) Flags |= CPU_FLAG_MMX;
    if (Edx & BIT25) Flags |= CPU_FLAG_SSE;
    if (Edx & BIT26) Flags |= CPU_FLAG_SSE2;
    if (Ecx & BIT0)  Flags |= CPU_FLAG_SSE3;
    if (Ecx & BIT9)  Flags |= CPU_FLAG_SSSE3;
    if (Ecx & BIT19) Flags |= CPU_FLAG_SSE4_1;
    if (Ecx & BIT20) Flags |= CPU_FLAG_SSE4_2;
    if (Ecx & BIT28) Flags |= CPU_FLAG_AVX;
    if (Ecx & BIT12) Flags |= CPU_FLAG_FMA;
    
    // Extended features (function 7)
    AsmCpuid(7, 0, &Eax, &Ebx, &Ecx, &Edx);
    if (Ebx & BIT5) Flags |= CPU_FLAG_AVX2;
    if (Ebx & BIT8) Flags |= CPU_FLAG_AVX512;
    
    return Flags;
}

/**
 * Determine CPU Generation
 */
UINT32 GetCPUGeneration(UINT32 Family, UINT32 Model, UINT32 Vendor) {
    // Intel
    if (Vendor == 0x756E6547) {  // "GenuineIntel"
        // Family 0x0F = NetBurst (Pentium 4)
        if (Family == 0x0F) {
            return CPU_GEN_NETBURST;
        }
        
        // Family 0x06 = Core
        if (Family == 0x06) {
            // Extended Model
            UINT8 ExtModel = ((Model >> 4) & 0x0F);
            
            switch (Model & 0xFF) {
                case 0x0E: return CPU_GEN_CONROE;   // Core 2 Duo (65nm)
                case 0x0D: return CPU_GEN_MEROM;    // Core 2 Duo (65nm)
                case 0x17: return CPU_GEN_PENRYN;   // Core 2 (45nm)
                case 0x1D: return CPU_GEN_PENRYN;   // Core 2 Duo Mobile
                case 0x1E: return CPU_GEN_NEHALEM; // Core i7
                case 0x1F: return CPU_GEN_NEHALEM;
                case 0x2E: return CPU_GEN_NEHALEM_EX;
                case 0x25: return CPU_GEN_WESTMERE; // Core i7
                case 0x2C: return CPU_GEN_WESTMERE; // Core i7 Extreme
                case 0x2D: return CPU_GEN_WESTMERE_EX;
                case 0x2A: return CPU_GEN_SANDY;    // Sandy Bridge
                case 0x2D: return CPU_GEN_SANDY;
                case 0x3A: return CPU_GEN_IVY;      // Ivy Bridge
                case 0x3C: return CPU_GEN_HASWELL;  // Haswell
                case 0x3F: return CPU_GEN_HASWELL;
                case 0x3D: return CPU_GEN_BROADWELL; // Broadwell
                case 0x4F: return CPU_GEN_BROADWELL;
                case 0x4E: return CPU_GEN_SKYLAKE;  // Skylake
                case 0x4D: return CPU_GEN_SKYLAKE;
                case 0x5E: return CPU_GEN_SKYLAKE;
                case 0x55: return CPU_GEN_SKYLAKE_X;
                case 0x8E: return CPU_GEN_KABY;     // Kaby Lake
                case 0x8F: return CPU_GEN_KABY;
                case 0x9E: return CPU_GEN_COFFEE;   // Coffee Lake
                case 0x9F: return CPU_GEN_COFFEE;
                case 0xA5: return CPU_GEN_COFFEE;
                case 0xA6: return CPU_GEN_COFFEE;
                case 0xA7: return CPU_GEN_COFFEE;
                case 0x7E: return CPU_GEN_ICELAKE;   // Ice Lake
                case 0x8A: return CPU_GEN_ICELAKE;
                case 0x8D: return CPU_GEN_ICELAKE;
                case 0x9C: return CPU_GEN_TIGER;    // Tiger Lake
                case 0xA0: return CPU_GEN_ROCKET;   // Rocket Lake
                case 0xA7: return CPU_GEN_ROCKET;
                case 0x8C: return CPU_GEN_ALDER;    // Alder Lake
                case 0x8D: return CPU_GEN_ALDER;
                case 0xA4: return CPU_GEN_ALDER;
                case 0xA6: return CPU_GEN_ALDER;
                case 0xB4: return CPU_GEN_RAPTOR;   // Raptor Lake
                case 0xB5: return CPU_GEN_RAPTOR;
                case 0xB7: return CPU_GEN_RAPTOR;
                case 0xBF: return CPU_GEN_METEOR;   // Meteor Lake
            }
        }
    }
    
    // AMD
    if (Vendor == 0x68747541) {  // "AuthenticAMD"
        // Family
        UINT8 ExtFamily = ((Family >> 8) & 0xFF);
        
        // Family 0x0F = K8
        if (Family == 0x0F) {
            return CPU_GEN_K8;
        }
        
        // Family 0x10 = K10
        if (Family == 0x10) {
            return CPU_GEN_K10;
        }
        
        // Family 0x11 = Llano
        if (Family == 0x11) {
            return CPU_GEN_LLANO;
        }
        
        // Family 0x12 = Bulldozer
        if (Family == 0x12) {
            return CPU_GEN_BULLDOZER;
        }
        
        // Family 0x15 = Piledriver
        if (Family == 0x15) {
            return CPU_GEN_PILEDRI;
        }
        
        // Family 0x16 = Excavator
        if (Family == 0x16) {
            return CPU_GEN_EXCAVATOR;
        }
        
        // Family 0x17 = Zen
        if (Family == 0x17 || ExtFamily >= 0x17) {
            UINT8 ExtModel = ((Model >> 4) & 0x0F);
            
            if (ExtModel >= 0x30) return CPU_GEN_ZEN4;   // Zen 4
            if (ExtModel >= 0x20) return CPU_GEN_ZEN3;   // Zen 3
            if (ExtModel >= 0x10) return CPU_GEN_ZEN2;   // Zen 2
            if (ExtModel >= 0x08) return CPU_GEN_ZENP;   // Zen+
            return CPU_GEN_ZEN;                           // Zen
        }
        
        // Family 0x19 = Zen 3+
        if (Family == 0x19 || ExtFamily >= 0x19) {
            return CPU_GEN_ZEN3;  // Also covers Zen 4
        }
    }
    
    return CPU_GEN_UNKNOWN;
}

/**
 * Count CPU Cores
 */
VOID GetCPUCores(UINT8 *Cores, UINT8 *Threads) {
    UINT32 Eax, Ebx, Ecx, Edx;
    
    // Intel Hyper-Threading
    AsmCpuid(1, &Eax, &Ebx, &Ecx, &Edx);
    
    UINT8 NumThreads = ((Edx >> 16) & 0xFF);
    
    // Logical processors per core
    if (NumThreads > 1) {
        // Has Hyper-Threading
        AsmCpuid(4, 0, &Eax, NULL, NULL, NULL);
        UINT8 CoresPerPackage = ((Eax >> 26) & 0x3F) + 1;
        *Cores = CoresPerPackage;
        *Threads = NumThreads;
    } else {
        // No Hyper-Threading
        AsmCpuid(4, 0, &Eax, NULL, NULL, NULL);
        UINT8 CoresPerPackage = ((Eax >> 26) & 0x3F) + 1;
        *Cores = CoresPerPackage;
        *Threads = CoresPerPackage;
    }
}

/**
 * Main CPU Detection
 */
EFI_STATUS DetectCPU(CPU_INFO *CPU) {
    UINT32 Eax, Ebx, Ecx, Edx;
    
    // Get vendor
    GetCPUVendor(CPU->Brand);
    
    // Determine vendor
    if (StrCmp(CPU->Brand, "GenuineIntel") == 0) {
        CPU->Vendor = 0x756E6547;
    } else if (StrCmp(CPU->Brand, "AuthenticAMD") == 0) {
        CPU->Vendor = 0x68747541;
    } else {
        CPU->Vendor = 0;
    }
    
    // CPUID function 1
    AsmCpuid(1, &Eax, &Ebx, &Ecx, &Edx);
    
    CPU->Family = (Eax >> 8) & 0xF;
    CPU->Model = ((Eax >> 4) & 0xF) | ((Eax >> 12) & 0xF0);
    CPU->Stepping = Eax & 0xF;
    
    // Get brand string
    GetCPUBrand(CPU->Brand);
    
    // Get flags
    CPU->Flags = GetCPUFlags();
    
    // Determine generation
    CPU->Flags |= GetCPUGeneration(CPU->Family, CPU->Model, CPU->Vendor);
    
    // Get core count
    GetCPUCores(&CPU->NumCores, &CPU->NumThreads);
    
    // Get frequency (from MSR)
    UINT64 TscFreq;
    AsmReadMsr(0x10, &TscFreq);
    CPU->Frequency = (UINT32)(TscFreq / 1000000);
    
    Log("CPU Detected: %a", CPU->Brand);
    Log("  Family: 0x%X, Model: 0x%X, Stepping: %d", 
        CPU->Family, CPU->Model, CPU->Stepping);
    Log("  Cores: %d, Threads: %d", CPU->NumCores, CPU->NumThreads);
    Log("  Frequency: %d MHz", CPU->Frequency);
    
    return EFI_SUCCESS;
}
