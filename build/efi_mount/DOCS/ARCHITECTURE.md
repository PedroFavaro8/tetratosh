# TETRATOSH - Arquitetura Completa do Sistema

## 1. Visão Geral do Sistema

```
+------------------------------------------------------------------+
|                        TETRATOSH                                 |
|   Bootloader Automatizado para Ambiente Hackintosh              |
+------------------------------------------------------------------+

FLUXO DE EXECUÇÃO:

┌─────────────────┐
│  UEFI Firmware  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Tetratosh     │  ←─────┐
│   Bootloader    │        │
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│ Hardware        │        │
│ Detection       │        │
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│ Kext Selector   │        │  LOOP
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│ Config Generator│        │
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│  EFI Builder    │        │
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│ OpenCore        │◄───────┘
│ Chainload       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    macOS        │
└─────────────────┘
```

## 2. Arquitetura de Módulos

### 2.1 Diagrama de Blocos

```
┌─────────────────────────────────────────────────────────────────────┐
│                         TETRATOSH                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │   BOOTLOADER │───▶│ HARDWARE     │───▶│   PCI        │          │
│  │              │    │ DETECTOR     │    │   SCANNER    │          │
│  └──────────────┘    └──────┬───────┘    └──────────────┘          │
│                            │                                       │
│                            ▼                                       │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │   ACPI       │◀───│              │───▶│   DEVICE     │          │
│  │   PARSER     │    │              │    │   DATABASE   │          │
│  └──────────────┘    └──────────────┘    └──────┬───────┘          │
│                                                 │                  │
│                                                 ▼                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐        │
│  │    KEXT      │◀───│              │───▶│    CONFIG    │        │
│  │   SELECTOR   │    │              │    │   GENERATOR  │        │
│  └──────────────┘    └──────────────┘    └──────┬───────┘        │
│                                                 │                  │
│                                                 ▼                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐        │
│  │   ISO        │◀───│    EFI       │◀───│   OPENCORE   │        │
│  │   BUILDER    │    │   BUILDER    │    │   LAUNCHER   │        │
│  └──────────────┘    └──────────────┘    └──────────────┘        │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────┐    │
│  │                    LOGGING SYSTEM                         │    │
│  └──────────────────────────────────────────────────────────┘    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 Interdependências

```
BOOTLOADER
    │
    ├──▶ HARDWARE_DETECTOR
    │       │
    │       ├──▶ PCI_SCANNER
    │       ├──▶ ACPI_PARSER
    │       └──▶ DEVICE_DATABASE
    │
    ├──▶ KEXT_SELECTOR
    │       │
    │       └──▶ DEVICE_DATABASE
    │
    ├──▶ CONFIG_GENERATOR
    │
    ├──▶ EFI_BUILDER
    │       │
    │       └──▶ OPENCORE_LAUNCHER
    │
    ├──▶ ISO_BUILDER
    │
    └──▶ LOGGING_SYSTEM
```

## 3. Estrutura de Dados

### 3.1 Device Info Structure

```c
struct DeviceInfo {
    char *name;              // Nome do dispositivo
    uint16_t vendor_id;     // ID do fabricante
    uint16_t device_id;      // ID do dispositivo
    uint8_t class_code;      // Classe PCI
    uint8_t subclass;        // Subclasse PCI
    uint8_t revision;        // Revisão
    char *required_kext;     // Kext necessário
    char *altername_kext;   // Kext alternativo
    bool is_builtin;         // É dispositivo nativo
};
```

### 3.2 Kext Info Structure

```c
struct KextInfo {
    char *name;              // Nome do kext
    char *filename;         // Nome do arquivo
    char *version;          // Versão
    char *min_os;           // Versão mínima do macOS
    char *max_os;           // Versão máxima do macOS
    char *url_download;     // URL para download
    uint32_t vendor_ids[8];  // Vendor IDs suportados
    uint32_t device_ids[16]; // Device IDs suportados
    char *cpu_arch;         // Arquitetura (x86_64, arm64)
    bool is_essential;      // Kext essencial (sempre necessário)
    char *description;      // Descrição
};
```

### 3.3 System Info Structure

```c
struct SystemInfo {
    char *cpu_brand;        // Marca da CPU
    uint32_t cpu_cores;     // Núcleos da CPU
    uint32_t cpu_threads;   // Threads da CPU
    char *cpu_arch;         // Arquitetura da CPU
    uint64_t ram_size;      // Tamanho da RAM em bytes
    
    // GPU
    DeviceInfo *gpus[8];
    uint32_t gpu_count;
    
    // Rede
    DeviceInfo *network_controllers[8];
    uint32_t network_count;
    
    // Áudio
    DeviceInfo *audio_codecs[4];
    uint32_t audio_count;
    
    // Storage
    DeviceInfo *storage_controllers[4];
    uint32_t storage_count;
    
    // Wireless
    DeviceInfo *wifi_controllers[4];
    uint32_t wifi_count;
    
    DeviceInfo *bluetooth_controllers[4];
    uint32_t bluetooth_count;
    
    char *chipset_vendor;   // Fabricante do chipset
    char *chipset_model;   // Modelo do chipset
    char *motherboard_vendor;
    char *motherboard_model;
};
```

## 4. Fluxo de Execução Detalhado

```
┌─────────────────────────────────────────────────────────────────────┐
│                    FLUXO DE EXECUÇÃO                                │
└─────────────────────────────────────────────────────────────────────┘

1. INICIALIZAÇÃO
   ├─ Bootloader UEFI inicia
   ├─ Inicializa serviços de logging
   ├─ Carrega configuração básica
   └─ Exibe tela de boas-vindas

2. DETECÇÃO DE HARDWARE
   ├─ CPU Detection
   │   ├─ Lê CPUID
   │   ├─ Identifica fabricante (Intel/AMD)
   │   ├─ Identifica modelo
   │   └─ Determina arquitetura
   │
   ├─ RAM Detection
   │   ├─ Lê informações de memória via SMBIOS
   │   └─ Calcula total de RAM
   │
   ├─ PCI Scanning
   │   ├─ Enumera todos os dispositivos PCI
   │   ├─ Para cada dispositivo:
   │   │   ├─ Lê Vendor ID
   │   │   ├─ Lê Device ID
   │   │   ├─ Lê Class Code
   │   │   └─ Lê Subclass
   │   └─ Armazena em lista
   │
   ├─ ACPI Parsing
   │   ├─ Lê DSDT
   │   ├─ Lê SSDTs
   │   ├─ Identifica dispositivos
   │   └─ Extrai informações de hardware
   │
   └─ GPU Detection
       ├─ Identifica GPUs via PCI
       ├─ Identifica GPUs integradas
       └─ Classifica como NVIDIA/AMD/Intel

3. SELEÇÃO DE KEXTS
   ├─ Para cada dispositivo detectado:
   │   ├─ Consulta Device Database
   │   ├─ Identifica kexts necessários
   │   └─ Adiciona à lista de kexts
   │
   ├─ Adiciona kexts essenciais:
   │   ├─ Lilu.kext (sempre necessário)
   │   ├─ VirtualSMC.kext (sempre necessário)
   │   ├─ WhateverGreen.kext (se GPU detectada)
   │   └─ AppleALC.kext (se áudio detectado)
   │
   └─ Verifica dependências entre kexts

4. GERAÇÃO DE CONFIG.PLIST
   ├─ Configura ACPI
   │   ├─ Adiciona patches necessários
   │   ├─ Configura SSDTs
   │   └─ Configura Quirks
   │
   ├─ Configura Boot Args
   │   ├─ -v (verbose)
   │   ├─ -liludetect
   │   └─ Outros argumentos
   │
   ├─ Configura DeviceProperties
   │   ├─ Configura framebuffer para GPU
   │   ├─ Configura propriedades de áudio
   │   └─ Configura propriedades de rede
   │
   ├─ Configura Kernel
   │   ├─ Adiciona kexts na ordem correta
   │   ├─ Configura patches
   │   └─ Configura Quirks
   │
   └─ Configura SMBIOS
       ├─ Seleciona modelo apropriado
       ├─ Configura informações de CPU
       └─ Configura memória

5. CONSTRUÇÃO DA EFI
   ├─ Cria estrutura de diretórios
   │   ├─ EFI/
   │   ├─ EFI/BOOT/
   │   ├─ EFI/OC/
   │   ├─ EFI/OC/ACPI/
   │   ├─ EFI/OC/Drivers/
   │   ├─ EFI/OC/Kexts/
   │   ├─ EFI/OC/Resources/
   │   └─ EFI/OC/Tools/
   │
   ├─ Copia arquivos
   │   ├─ Copia OpenCore.efi
   │   ├─ Copia Drivers necessários
   │   ├─ Copia Kexts selecionados
   │   ├─ Copia config.plist gerado
   │   └─ Copia ACPI patches
   │
   └─ Valida estrutura

6. CHAINLOAD DO OPENCORE
   ├─ Localiza OpenCore.efi
   ├─ Prepara parâmetros de boot
   ├─ Executa OpenCore via UEFI
   └─ Passa controle para OpenCore
```

## 5. Device Database - Estratégia

### 5.1 Estrutura do Banco de Dados

```
DEVICE_DATABASE/
├── devices.json          # Banco de dados principal
├── kexts.json           # Informações de kexts
├── smbios_compatibility.json  # Compatibilidade SMBIOS
└── acpi_patches.json    # Patches ACPI necessários
```

### 5.2 Formato JSON do Device Database

```json
{
  "devices": [
    {
      "name": "Intel I225-V Ethernet",
      "vendor_id": "8086",
      "device_id": "15F3",
      "class_code": "02",
      "subclass": "00",
      "kext": "IntelMausi.kext",
      "alt_kext": "NullEthernet.kext",
      "notes": "Requires Lilu and WhateverGreen"
    }
  ]
}
```

## 6. Algoritmo de Seleção de Kexts

```
ALGORITMO: SelectKexts

ENTRADA: SystemInfo (informações do hardware detectado)
SAÍDA: Lista de KextInfo (kexts selecionados)

FUNÇÃO SelectKexts(SystemInfo sys):
    lista_kexts = nova lista
    
    // 1. Adicionar kexts essenciais
    lista_kexts.adicionar(Lilu.kext)
    lista_kexts.adicionar(VirtualSMC.kext)
    
    // 2. Para cada GPU detectada
    PARA CADA gpu EM sys.gpus:
        SE gpu.vendor_id == "10DE":  // NVIDIA
            SE gpu.device_id EM [ "1B80", "1B83", "1B84", ... ]:
                lista_kexts.adicionar(NVMeDriver.kext)
            FIM SE
        FIM SE
        
        SE gpu.vendor_id == "1002":  // AMD
            lista_kexts.adicionar(WhateverGreen.kext)
        FIM SE
        
        SE gpu.vendor_id == "8086":  // Intel
            lista_kexts.adicionar(WhateverGreen.kext)
        FIM SE
    FIM PARA
    
    // 3. Para cada controlador de rede
    PARA CADA rede EM sys.network_controllers:
        kext = BuscarKextNoBanco(rede.vendor_id, rede.device_id)
        SE kext != NULO:
            lista_kexts.adicionar(kext)
        FIM SE
    FIM PARA
    
    // 4. Para cada codec de áudio
    PARA CADA audio EM sys.audio_codecs:
        kext = BuscarKextNoBanco(audio.vendor_id, audio.device_id)
        SE kext != NULO:
            lista_kexts.adicionar(kext)
        FIM SE
    FIM PARA
    
    // 5. Para cada controlador wireless
    PARA CADA wifi EM sys.wifi_controllers:
        kext = BuscarKextNoBanco(wifi.vendor_id, wifi.device_id)
        SE kext != NULO:
            lista_kexts.adicionar(kext)
        FIM SE
    FIM PARA
    
    // 6. Adicionar AppleALC se áudio detectado
    SE sys.audio_count > 0:
        lista_kexts.adicionar(AppleALC.kext)
    FIM SE
    
    // 7. Verificar ordem de carregamento
    lista_kexts.ordenar(PorPrioridade)
    
    RETORNAR lista_kexts
FIM FUNÇÃO
```

## 7. Algoritmo de Geração de config.plist

```
ALGORITMO: GenerateConfigPlist

ENTRADA: SystemInfo sys, ListaKexts kexts
SAÍDA: config.plist (XML)

FUNÇÃO GenerateConfigPlist(sys, kexts):
    config = novo documento XML
    
    // 1. Configuração ACPI
    config.ACPI.Add("DSDT", "DSDT.aml")
    config.ACPI.SSDTs = ["SSDT-PLUG.aml", "SSDT-EC.aml"]
    config.ACPI.Quirks.FadtEnableReset = true
    config.ACPI.Quirks.PatchAPIC = true
    
    // 2. Configuração de Boot Args
    config.Booter.Quirks = {
        "AvoidRuntimeDefrag": true,
        "DevirtualiseMmio": true,
        "DisableSingleUser": false,
        "DisableWatchDog": true,
        "EnableTsc32": true,
        ...
    }
    
    config.NVRAM.Add("boot-args", "-v liludetect=1 -liludbg")
    
    // 3. Configuração de DeviceProperties
    // GPU
    PARA CADA gpu EM sys.gpus:
        config.DeviceProperties.Add(gpu.pci_addr, {
            "framebuffer-patch-enable": 1,
            "framebuffer-con0-type": 0x08,
            ...
        })
    FIM PARA
    
    // Áudio
    PARA CADA audio EM sys.audio_codecs:
        config.DeviceProperties.Add(audio.pci_addr, {
            "layout-id": "0x0C",
            ...
        })
    FIM PARA
    
    // 4. Configuração de Kernel
    config.Kernel.Kexts = []
    PARA CADA kext EM kexts:
        config.Kernel.Kexts.adicionar({
            "BundlePath": kext.path,
            "ExecutablePath": kext.executable,
            "PlistPath": "Info.plist",
            "Enabled": true
        })
    FIM PARA
    
    config.Kernel.Quirks = {
        "AppleCpuPmCfgLock": false,
        "AppleXcpmCfgLock": false,
        "CpuIdOverride": null,
        ...
    }
    
    // 5. Configuração de SMBIOS
    smbios = SelecionarSMBIOS(sys)
    config.SMBIOS.ProductName = smbios.model
    config.SMBIOS.BoardProduct = smbios.board_id
    config.SMBIOS.SerialNumber = GerarSerial()
    config.SMBIOS.MLB = GerarMLB()
    config.SMBIOS.ROM = GerarMAC()
    
    // 6. Configuração de UEFI
    config.UEFI.Drivers = ["OpenRuntime.efi", "OpenCanopy.efi"]
    config.UEFI.Quirks = {
        "IgnoreInvalidFlexRatio": true,
        "ReleaseUsbOwnership": true,
        ...
    }
    
    RETORNAR config
FIM FUNÇÃO
```

## 8. Algoritmo de Chainload do OpenCore

```
ALGORITMO: ChainloadOpenCore

ENTRADA: Caminho para OpenCore.efi
SAÍDA: Nenhum (transfere controle)

FUNÇÃO ChainloadOpenCore(opencore_path):
    // 1. Obter handle do arquivo OpenCore
    status = gBS->LoadImage(
        FALSE,                      // BootPolicy
        gImageHandle,               // ParentImageHandle
        NULL,                       // DevicePath
        0,                          // SourceSize
        &opencore_image             // ImageHandle
    )
    SE status != EFI_SUCCESS:
        Log("Erro ao carregar OpenCore: " + status)
        RETORNAR status
    FIM SE
    
    // 2. Obter entry point do OpenCore
    status = gBS->HandleProtocol(
        opencore_image,
        &gEfiLoadedImageProtocolGuid,
        &loaded_image
    )
    
    // 3. Configurar parâmetros de boot
    // Configurar LoadOptions com argumentos necessários
    loaded_image->LoadOptions = NULL
    loaded_image->ParentHandle = gImageHandle
    
    // 4. Iniciar OpenCore
    status = gBS->StartImage(
        opencore_image,
        &exit_data_size,
        &exit_data
    )
    
    // Se chegamos aqui, o OpenCore retornou
    Log("OpenCore encerrou com código: " + status)
    
    RETORNAR status
FIM FUNÇÃO
```

## 9. Páginas de Memória UEFI

```
┌─────────────────────────────────────────────────────────────────────┐
│                  ESPAÇO DE ENDEREÇAMENTO UEFI                      │
└─────────────────────────────────────────────────────────────────────┘

0x00000000  ┌─────────────────────┐
            │     IVT & BIOS      │
            │     Data Area       │
0x00000400  ├─────────────────────┤
            │     EBDA            │
0x000A0000  ├─────────────────────┤
            │   Video RAM         │
0x000C0000  ├─────────────────────┤
            │   Reserved Area     │
0x000E0000  ├─────────────────────┤
            │   UEFI Runtime      │
            │   Services          │
0x00100000  ├─────────────────────┤
            │                     │
            │   Free Memory       │
            │   (Para Tetratosh)  │
            │                     │
            │   - Heap            │
            │   - Stack           │
            │   - Data            │
            │   - Code            │
            │                     │
            ├─────────────────────┤
            │   UEFI Boot Services│
0xFFFFFFFF  └─────────────────────┘
```

## 10. Formato da ISO

```
┌─────────────────────────────────────────────────────────────────────┐
│                      ESTRUTURA DA ISO                              │
└─────────────────────────────────────────────────────────────────────┘

TETRATOSH.ISO
│
├── EFI/
│   └── BOOT/
│       └── BOOTx64.efi      ← Tetratosh Bootloader
│
├── KEKST/
│   ├── REQUIRED/
│   │   ├── Lilu.kext/
│   │   ├── VirtualSMC.kext/
│   │   ├── WhateverGreen.kext/
│   │   └── AppleALC.kext/
│   │
│   └── COMMUNITY/
│       ├── Network/
│       │   ├── IntelMausi.kext/
│       │   ├── RealtekRTL8111.kext/
│       │   ├── AppleIGB.kext/
│       │   └── AirportItlwm.kext/
│       │
│       ├── Storage/
│       │   ├── NVMeFix.kext/
│       │   └── AHCI_3rdParty.kext/
│       │
│       └── Wireless/
│           ├── AirportItlwm.kext/
│           └── BlueToolFixup.kext/
│
├── OPENCORE/
│   ├── OpenCore.efi
│   ├── Drivers/
│   │   ├── OpenRuntime.efi
│   │   ├── OpenCanopy.efi
│   │   └── HfsPlus.efi
│   └── Resources/
│       └── (Ícones, etc)
│
├── TOOLS/
│   ├── macos_recovery.sh
│   └── efi_mounter.sh
│
├── DATABASE/
│   ├── devices.json
│   ├── kexts.json
│   └── smbios.json
│
└── BUILD/
    └── (Arquivos temporários de build)
```

## 11. Telas do Bootloader

```
┌─────────────────────────────────────────────────────────────────────┐
│                    TELA DE BOOT DO TETRATOSH                       │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                                                             │   │
│  │                   ██████  ██████  ██████                   │   │
│  │                   █      █      █ █                         │   │
│  │                   █████  █████  █████                       │   │
│  │                   █      █  █   █                           │   │
│  │                   ██████ █   █  ██████                     │   │
│  │                                                             │   │
│  │                 Automated Hackintosh Bootloader            │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  [Detecting Hardware...]                                           │
│                                                                     │
│  CPU: Intel Core i7-10700K                                         │
│  RAM: 32GB DDR4                                                    │
│  GPU: Intel UHD Graphics 630                                       │
│  GPU: NVIDIA GeForce RTX 3080                                      │
│  Ethernet: Intel I225-V                                             │
│  Audio: Realtek ALC1220                                            │
│                                                                     │
│  [Building EFI...]                                                  │
│                                                                     │
│  ✓ Selected: Lilu.kext                                             │
│  ✓ Selected: VirtualSMC.kext                                       │
│  ✓ Selected: WhateverGreen.kext                                    │
│  ✓ Selected: AppleALC.kext                                         │
│  ✓ Selected: IntelMausi.kext                                       │
│  ✓ Selected: NVMeFix.kext                                          │
│                                                                     │
│  [Launching OpenCore...]                                           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## 12. Códigos de Erro

```c
// Códigos de erro do Tetratosh
#define TETRATOSH_SUCCESS              0x00
#define TETRATOSH_ERR_HW_DETECT        0x01
#define TETRATOSH_ERR_PCI_SCAN         0x02
#define TETRATOSH_ERR_ACPI_PARSE       0x03
#define TETRATOSH_ERR_NO_KEXTS         0x04
#define TETRATOSH_ERR_CONFIG_GEN       0x05
#define TETRATOSH_ERR_EFI_BUILD       0x06
#define TETRATOSH_ERR_OPENCORE_LOAD   0x07
#define TETRATOSH_ERR_NO_MEMORY        0x08
#define TETRATOSH_ERR_FILESYSTEM      0x09
#define TETRATOSH_ERR_INVALID_PARAM    0x0A
```

## 13. Variáveis de Ambiente de Boot

```
Parâmetros passados para o Tetratosh via boot:

- tetratosh.quiet     : Modo silencioso (sem output)
- tetratosh.log       : Nível de logging (0-4)
- tetratosh.efi_path  : Caminho customizado para EFI
- tetratosh.no_gui    : Modo texto (sem interface)
- tetratosh.unsafe    : Desabilita verificações de segurança
```

## 14. Testes de Compatibilidade

### 14.1 CPUs Suportadas

- **Intel**: Skylake (6th) e posterior
  - Skylake, Kaby Lake, Coffee Lake, Comet Lake, Ice Lake, Rocket Lake, Alder Lake
  
- **AMD**: Ryzen e posterior
  - Zen, Zen+, Zen 2, Zen 3, Zen 4

### 14.2 GPUs Suportadas

- **Intel**: UHD 630, Iris Plus, Xe Graphics
- **AMD**: Polaris, Vega, Navi, RDNA, RDNA 2
- **NVIDIA**: Maxwell, Pascal, Turing, Ampere (com limitations)

### 14.3 Controladores de Rede

- Intel I211, I219, I225-V, I226-V
- Realtek RTL8111, RTL8125
- Broadcom

### 14.4 Controladores de Áudio

- Realtek ALC series
- Conexant
- IDT

---

**Versão do Documento**: 1.0  
**Última Atualização**: 2024  
**Autor**: TETRATOSH Team
