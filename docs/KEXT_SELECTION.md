# Algoritmo de Seleção de Kexts

## 1. Visão Geral

O módulo de seleção de kexts é responsável por identificar automaticamente quais kexts são necessários para o hardware detectado e organizar a ordem de carregamento.

## 2. Fluxo de Seleção

```
┌─────────────────────────────────────────────────────────────────────┐
│                    FLUXO DE SELEÇÃO DE KEXTS                       │
└─────────────────────────────────────────────────────────────────────┘

    ┌────────────────┐
    │  INÍCIO        │
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │ Carregar       │
    │ Database        │
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  Kexts         │◀────────────────────┐
    │  Essenciais    │                      │
    └────────┬───────┘                      │
             │                               │
             ▼                               │
    ┌────────────────┐                       │
    │ Detectar GPU   │                       │
    └────────┬───────┘                       │
             │                               │
     ┌───────┴───────┐                       │
     │               │                       │
     ▼               ▼                       │
┌────────┐    ┌────────┐                    │
│ NVIDIA │    │  AMD   │───────┐            │
└────────┘    └────────┘       │            │
     │               │        │            │
     ▼               ▼        ▼            │
┌────────┐    ┌────────┐ ┌────────┐        │
│GPU用Kext│   │ Whatever│ │Whatever│        │
└────────┘    └────────┘ └────────┘        │
                                     │      │
                                     ▼      ▼
                            ┌────────────────┐
                            │  Kexts de Rede │
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │  Kexts de Áudio│
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │  Kexts Wireless│
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │  Kexts Storage │
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │ Ordenar por    │
                            │ Prioridade     │
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │ Validar        │
                            │ Dependências   │
                            └────────┬───────┘
                                     │
                                     ▼
                            ┌────────────────┐
                            │    FIM         │
                            └────────────────┘
```

## 3. Estrutura de Dados

### 3.1 KextInfo

```c
struct KextInfo {
    char *name;                 // Nome: "Lilu.kext"
    char *bundle_id;           // Bundle ID: "com.apple.hwp.lilu"
    char *path;                // Caminho no sistema de arquivos
    char *version;             // Versão: "1.6.7"
    char *min_macos;           // Versão mínima do macOS
    char *max_macos;           // Versão máxima do macOS
    char *cpu_arch;           // Arquitetura: "x86_64", "arm64", "both"
    bool is_essential;         // É kext essencial
    bool is_loaded_first;     // Deve ser carregado primeiro
    char *required_by[8];     // Kexts que dependem deste
    char *conflicts[4];       // Kexts conflitantes
    char *description;        // Descrição
    KextType type;            // Tipo do kext
};

enum KextType {
    KEXT_TYPE_ESSENTIAL,      // Essencial (Lilu, VirtualSMC)
    KEXT_TYPE_GPU,            // GPU (WhateverGreen)
    KEXT_TYPE_AUDIO,          // Áudio (AppleALC)
    KEXT_TYPE_NETWORK,        // Rede (IntelMausi, etc)
    KEXT_TYPE_STORAGE,        // Armazenamento
    KEXT_TYPE_WIRELESS,       // Wireless
    KEXT_TYPE_INPUT,          // Input (VoodooPS2)
    KEXT_TYPE_OTHER           // Outro
};
```

### 3.2 DeviceToKext Mapping

```c
struct DeviceKextMapping {
    uint16_t vendor_id;
    uint16_t device_id;
    char *primary_kext;       // Kext primário
    char *alternative_kext;   // Kext alternativo
    bool requires_lilu;       // Depende do Lilu
    bool requires_whatevergreen; // Depende do WhateverGreen
    char *notes;              // Observações
};
```

## 4. Algoritmo Principal

### Pseudocódigo

```
FUNÇÃO SelectKexts(system_info) -> ListaKexts:
    Log("=== Starting Kext Selection ===")
    
    kexts = nova lista()
    
    // 1. Carregar database de kexts
    db = CarregarKextDatabase()
    
    // 2. Adicionar kexts essenciais
    Log("Adding essential kexts...")
    kexts.adicionar(db.buscar("Lilu"))
    kexts.adicionar(db.buscar("VirtualSMC"))
    
    // 3. Processar GPUs
    Log("Processing GPUs...")
    kexts_gpu = ProcessarGPUs(system_info.gpus, db)
    kexts.adicionar_todos(kexts_gpu)
    
    // 4. Processar Controladores de Rede
    Log("Processing network controllers...")
    kexts_rede = ProcessarRede(system_info.network_controllers, db)
    kexts.adicionar_todos(kexts_rede)
    
    // 5. Processar Áudio
    Log("Processing audio...")
    kexts_audio = ProcessarAudio(system_info.audio_codecs, db)
    kexts.adicionar_todos(kexts_audio)
    
    // 6. Processar Wireless
    Log("Processing wireless...")
    kexts_wireless = ProcessarWireless(system_info.wifi_controllers, db)
    kexts.adicionar_todos(kexts_wireless)
    
    // 7. Processar Armazenamento
    Log("Processing storage...")
    kexts_storage = ProcessarStorage(system_info.storage_controllers, db)
    kexts.adicionar_todos(kexts_storage)
    
    // 8. Verificar dependências
    Log("Resolving dependencies...")
    kexts = ResolverDependencias(kexts, db)
    
    // 9. Ordenar por prioridade
    Log("Sorting kexts...")
    kexts.ordenar(PorOrdemCarregamento)
    
    // 10. Remover duplicatas
    kexts.remover_duplicados()
    
    Log("=== Kext Selection Complete: " + kexts.contar() + " kexts ===")
    
    RETORNAR kexts
FIM FUNÇÃO
```

## 5. Processamento por Categoria

### 5.1 GPUs

```
FUNÇÃO ProcessarGPUs(gpus, db) -> ListaKexts:
    resultado = nova lista()
    
    // WhateverGreen é necessário para todas as GPUs não-nativas
    qualquer_gpu = FALSE
    
    PARA CADA gpu EM gpus:
        // GPUs integradas Intel geralmente funcionam nativamente
        SE gpu.vendor_id == 0x8086:
            // Intel GPU integrada
            SE gpu.device_id EM [0x5912, 0x591B, 0x591D, ...]: // Gen9+
                // Funciona com driver nativo
                qualquer_gpu = TRUE
                Log("Intel GPU: using native driver")
            FIM SE
        FIM SE
        
        // NVIDIA
        SE gpu.vendor_id == 0x10DE:
            // NVIDIA requer WhateverGreen para aceleração
            // e frequentemente requer web drivers
            hw = db.buscar_nvidia(gpu.device_id)
            
            SE hw != NULO:
                SE hw.webdriver_requerido:
                    resultado.adicionar(db.buscar("WhateverGreen"))
                    Log("NVIDIA GPU: WhateverGreen added (web driver needed)")
                FIM SE
            FIM SE
        FIM SE
        
        // AMD
        SE gpu.vendor_id == 0x1002:
            // AMD funciona melhor com WhateverGreen
            hw = db.buscar_amd(gpu.device_id)
            
            SE hw != NULO:
                // GPUs AMD Polaris+ funcionam bem
                resultado.adicionar(db.buscar("WhateverGreen"))
                Log("AMD GPU: WhateverGreen added")
            FIM SE
        FIM SE
        
        // GPU externa/dedicada (Qualquer vendor)
        SE gpu.device_type == "DISCRETE":
            resultado.adicionar(db.buscar("WhateverGreen"))
        FIM SE
    FIM PARA
    
    RETORNAR resultado
FIM FUNÇÃO
```

### 5.2 Rede

```
FUNÇÃO ProcessarRede(rede_controllers, db) -> ListaKexts:
    resultado = nova lista()
    
    PARA CADA rede EM rede_controllers:
        mapping = db.buscar_device(rede.vendor_id, rede.device_id)
        
        SE mapping != NULO:
            // Encontrou kext no banco
            kext = db.buscar(mapping.primary_kext)
            resultado.adicionar(kext)
            
            // Verificar se precisa de Lilu
            SE mapping.requires_lilu:
                // Lilu já foi adicionado como essencial
                Log("Network: " + rede.name + " -> " + kext.name + " (needs Lilu)")
            FIM SE
        SENÃO:
            // Tentar encontrar por vendor
            kext = EncontrarKextPorVendor(rede.vendor_id, db)
            
            SE kext != NULO:
                resultado.adicionar(kext)
                Log("Network: " + rede.name + " -> " + kext.name)
            SENÃO:
                // Kext não encontrado, mas não é crítico
                Log("Warning: No kext found for " + rede.name)
            FIM SE
        FIM SE
    FIM PARA
    
    // Adicionar NullEthernet como fallback se nenhuma rede detectada
    // ou como segundo controller
    SE resultado.contar() == 0:
        resultado.adicionar(db.buscar("NullEthernet.kext"))
    FIM SE
    
    RETORNAR resultado
FIM FUNÇÃO
```

### 5.3 Áudio

```
FUNÇÃO ProcessarAudio(audio_codecs, db) -> ListaKexts:
    resultado = nova lista()
    
    // AppleALC funciona para a maioria dos codecs
    applealc_adicionado = FALSE
    
    PARA CADA audio EM audio_codecs:
        mapping = db.buscar_audio(audio.vendor_id, audio.device_id)
        
        SE mapping != NULO:
            // Layout ID específico para este codec
            layout_id = mapping.layout_id
            
            // AppleALC é o kext principal
            SE NOT applealc_adicionado:
                kext = db.buscar("AppleALC.kext")
                resultado.adicionar(kext)
                applealc_adicionado = TRUE
            FIM SE
            
            // Adicionar PinConfiguration se necessário
            SE mapping.requires_pinconfig:
                Log("Audio: " + audio.name + " requires custom pin config")
            FIM SE
            
            Log("Audio: " + audio.name + " -> AppleALC (layout-id: 0x" + layout_id + ")")
        SENÃO:
            // Tentar detectar automaticamente
            layout_id = DetectarLayoutID(audio)
            
            SE NOT applealc_adicionado:
                kext = db.buscar("AppleALC.kext")
                resultado.adicionar(kext)
                applealc_adicionado = TRUE
            FIM SE
            
            Log("Audio: " + audio.name + " -> AppleALC (auto-detected)")
        FIM SE
    FIM PARA
    
    // Se nenhum áudio detectado, adicionar layout simples
    SE resultado.contar() == 0:
        // Adicionar AppleALC com layout genérico
        kext = db.buscar("AppleALC.kext")
        resultado.adicionar(kext)
        Log("Audio: Using default AppleALC")
    FIM SE
    
    RETORNAR resultado
FIM FUNÇÃO
```

### 5.4 Wireless

```
FUNÇÃO ProcessarWireless(wifi_controllers, db) -> ListaKexts:
    resultado = nova lista()
    
    PARA CADA wifi EM wifi_controllers:
        // Wireless Broadcom geralmente funciona com driver nativo
        SE wifi.vendor_id == 0x14E4:
            // Broadcom
            hw = db.buscar_broadcom(wifi.device_id)
            
            SE hw != NULO E hw.native_suportado:
                // Funciona com AirPort (driver nativo)
                Log("WiFi: " + wifi.name + " - native driver available")
            SENÃO:
                // Precis de kext externo
                resultado.adicionar(db.buscar("AirportBrcmFixup.kext"))
                Log("WiFi: " + wifi.name + " -> AirportBrcmFixup")
            FIM SE
        FIM SE
        
        // Intel
        SE wifi.vendor_id == 0x8086:
            // Intel WiFi requer itlwm
            hw = db.buscar_intel_wifi(wifi.device_id)
            
            SE hw != NULO:
                resultado.adicionar(db.buscar("AirportItlwm.kext"))
                resultado.adicionar(db.buscar("IntelBluetoothFirmware.kext"))
                Log("WiFi: " + wifi.name + " -> AirportItlwm")
            FIM SE
        FIM SE
        
        // Realtek
        SE wifi.vendor_id == 0x10EC:
            resultado.adicionar(db.buscar("AirportItlwm.kext"))
            Log("WiFi: " + wifi.name + " -> AirportItlwm (experimental)")
        FIM SE
    FIM PARA
    
    // Adicionar Bluetooth fix se necessário
    SE resultado.contar() > 0:
        resultado.adicionar(db.buscar("BlueToolFixup.kext"))
    FIM SE
    
    RETORNAR resultado
FIM FUNÇÃO
```

### 5.5 Armazenamento

```
FUNÇÃO ProcessarStorage(storage_controllers, db) -> ListaKexts:
    resultado = nova lista()
    
    nvme_detectado = FALSE
    
    PARA CADA storage EM storage_controllers:
        SE storage.type == "NVMe":
            SE NOT nvme_detectado:
                // NVMeFix para correção de bugs
                resultado.adicionar(db.buscar("NVMeFix.kext"))
                nvme_detectado = TRUE
                Log("Storage: NVMe detected -> NVMeFix")
            FIM SE
        FIM SE
        
        SE storage.type == "SATA":
            // SATA geralmente funciona nativamente
            // Mas pode precisar de patches para AHCI
            hw = db.buscar_sata(storage.vendor_id, storage.device_id)
            
            SE hw != NULO E hw.precisa_kext:
                resultado.adicionar(db.buscar("AHCIPortInjector.kext"))
                Log("Storage: SATA -> AHCIPortInjector")
            FIM SE
        FIM SE
        
        SE storage.type == "RAID":
            // RAID pode precisar de kext especial
            resultado.adicionar(db.buscar("RAIDFix.kext"))
            Log("Storage: RAID detected -> RAIDFix")
        FIM SE
    FIM PARA
    
    RETORNAR resultado
FIM FUNÇÃO
```

## 6. Resolução de Dependências

```
FUNÇÃO ResolverDependencias(kexts, db) -> ListaKexts:
    resultado = nova lista()
    adicionados = novo set()
    
    // Garantir que todas as dependências estão presentes
    PARA CADA kext EM kexts:
        // Verificar dependências
        PARA CADA dep EM kext.required_by:
            dep_kext = db.buscar(dep)
            
            SE dep_kext != NULO E NOT adicionados.contem(dep_kext.name):
                resultado.adicionar(dep_kext)
                adicionados.adicionar(dep_kext.name)
            FIM SE
        FIM PARA
        
        // Adicionar o próprio kext
        SE NOT adicionados.contem(kext.name):
            resultado.adicionar(kext)
            adicionados.adicionar(kext.name)
        FIM SE
    FIM PARA
    
    RETORNAR resultado
FIM FUNÇÃO
```

## 7. Ordenação por Prioridade

```
FUNÇÃO OrdenarKexts(kexts) -> ListaKexts:
    // Ordem correta de carregamento:
    // 1. Lilu (sempre primeiro)
    // 2. VirtualSMC / FakeSMC
    // 3. WhateverGreen (GPU)
    // 4. Kexts de hardware específico
    // 5. AppleALC (áudio)
    // 6. Kexts de rede
    // 7. Kexts wireless
    // 8. Kexts de input
    // 9. Outros
    
    prioridade = {
        "Lilu.kext": 1,
        "VirtualSMC.kext": 2,
        "FakeSMC.kext": 2,
        "WhateverGreen.kext": 10,
        "AppleALC.kext": 20,
        "IntelMausi.kext": 30,
        "RealtekRTL8111.kext": 30,
        "AirportItlwm.kext": 40,
        "AirportBrcmFixup.kext": 40,
        "NVMeFix.kext": 50,
        "VoodooPS2Controller.kext": 60,
        "USBInjectAll.kext": 70,
    }
    
    kexts.ordenar(por(prioridade[kext.name] OU 100))
    
    RETORNAR kexts
FIM FUNÇÃO
```

## 8. Database de Kexts

### 8.1 Estrutura JSON

```json
{
  "kexts": [
    {
      "name": "Lilu.kext",
      "bundle_id": "com.apple.hwp.lilu",
      "type": "ESSENTIAL",
      "version": "1.7.0",
      "description": "Arbitrary kext and process patching",
      "required_by": [],
      "is_essential": true
    },
    {
      "name": "VirtualSMC.kext",
      "bundle_id": "com.apple.smc.virtualsmc",
      "type": "ESSENTIAL",
      "version": "1.3.2",
      "description": "Advanced Apple SMC emulator",
      "required_by": [],
      "is_essential": true
    },
    {
      "name": "WhateverGreen.kext",
      "bundle_id": "com.apple.whatevrgreen",
      "type": "GPU",
      "version": "1.6.9",
      "description": "GPU patches for various GPUs",
      "required_by": ["Lilu.kext"],
      "is_essential": false
    },
    {
      "name": "AppleALC.kext",
      "bundle_id": "com.apple.applealcaudio",
      "type": "AUDIO",
      "version": "1.9.2",
      "description": "Native AppleALC audio patcher",
      "required_by": ["Lilu.kext"],
      "is_essential": false
    },
    {
      "name": "IntelMausi.kext",
      "bundle_id": "com.insanelymac.intelmausi",
      "type": "NETWORK",
      "version": "1.0.8",
      "description": "Intel LAN driver",
      "required_by": [],
      "is_essential": false
    }
  ],
  
  "device_mappings": [
    {
      "vendor_id": "8086",
      "device_id": "15F3",
      "kext": "IntelMausi.kext",
      "notes": "Intel I225-V"
    },
    {
      "vendor_id": "10EC",
      "device_id": "8168",
      "kext": "RealtekRTL8111.kext",
      "notes": "Realtek 8111"
    },
    {
      "vendor_id": "14E4",
      "device_id": "43A0",
      "kext": "AirportBrcmFixup.kext",
      "notes": "Broadcom WiFi"
    }
  ],
  
  "audio_layouts": [
    {
      "vendor_id": "10EC",
      "device_id": "1220",
      "layout_id": "7",
      "notes": "Realtek ALC1220"
    },
    {
      "vendor_id": "10EC",
      "device_id": "0887",
      "layout_id": "11",
      "notes": "Realtek ALC887"
    }
  ]
}
```

## 9. Tabela de Kexts por Hardware

### 9.1 Rede

| Vendor | Device | Kext | Notas |
|--------|--------|------|-------|
| Intel | I219-V/LM | IntelMausi.kext | |
| Intel | I225-V | IntelMausi.kext | Lilu |
| Intel | I226-V | IntelMausi.kext | Lilu |
| Realtek | RTL8111 | RealtekRTL8111.kext | Lilu |
| Realtek | RTL8125 | RealtekRTL8125.kext | Lilu |
| Realtek | RTL8152 | RealtekRTL8152.kext | |
| Broadcom | BCM57xx | AppleBCM5701.kext | |
| Broadcom | NetXtreme | AppleBCM5701.kext | |

### 9.2 Wireless

| Vendor | Device | Kext | Notas |
|--------|--------|------|-------|
| Intel | WiFi | AirportItlwm.kext | itlwm + IntelBT |
| Broadcom | WiFi | AirportBrcmFixup.kext | Native + patch |
| Realtek | WiFi | AirportItlwm.kext | Experimental |

### 9.3 Áudio

| Vendor | Device | Layout ID | Kext |
|--------|--------|-----------|------|
| Realtek | ALC1220 | 0x07 | AppleALC.kext |
| Realtek | ALC1220A | 0x0C | AppleALC.kext |
| Realtek | ALC887 | 0x11 | AppleALC.kext |
| Realtek | ALC892 | 0x04 | AppleALC.kext |
| Realtek | ALC269 | 0x03 | AppleALC.kext |
| Conexant | CX20751/2 | 0x28 | AppleALC.kext |
| VIA | VT2021 | 0x09 | AppleALC.kext |

## 10. Exemplos de Log

```
┌─────────────────────────────────────────────────────────────────────┐
│                    EXEMPLO DE LOG DE SELEÇÃO                        │
└─────────────────────────────────────────────────────────────────────┘

[KEXT] Starting Kext Selection...
[KEXT] Loading kext database...
[KEXT] Adding essential kexts:
[KEXT]   + Lilu.kext
[KEXT]   + VirtualSMC.kext
[KEXT] Processing GPUs...
[KEXT]   GPU 0: Intel UHD Graphics 630 (0x5912)
[KEXT]   - No external kext needed (native support)
[KEXT]   GPU 1: NVIDIA GeForce RTX 3080 (0x2204)
[KEXT]   + WhateverGreen.kext (NVIDIA needs WEG)
[KEXT]   GPU: WhateverGreen.kext added
[KEXT] Processing network controllers...
[KEXT]   Network: Intel I225-V (8086:15F3)
[KEXT]   + IntelMausi.kext
[KEXT]   Network: Realtek RTL8125 (10EC:8125)
[KEXT]   + RealtekRTL8125.kext
[KEXT] Processing audio...
[KEXT]   Audio: Realtek ALC1220 (10EC:1220)
[KEXT]   + AppleALC.kext (layout-id: 0x07)
[KEXT] Processing wireless...
[KEXT]   WiFi: Intel Wi-Fi 6 AX201 (8086:34FE)
[KEXT]   + AirportItlwm.kext
[KEXT]   + IntelBluetoothFirmware.kext
[KEXT] Processing storage...
[KEXT]   Storage: NVMe (2263:5001)
[KEXT]   + NVMeFix.kext
[KEXT]   Storage: SATA (8086:2829)
[KEXT]   - Native driver available
[KEXT] Resolving dependencies...
[KEXT]   All dependencies resolved
[KEXT] Sorting kexts by priority...
[KEXT] 
[KEXT] Final kext list (in load order):
[KEXT]   1. Lilu.kext
[KEXT]   2. VirtualSMC.kext
[KEXT]   3. WhateverGreen.kext
[KEXT]   4. AppleALC.kext
[KEXT]   5. IntelMausi.kext
[KEXT]   6. RealtekRTL8125.kext
[KEXT]   7. AirportItlwm.kext
[KEXT]   8. IntelBluetoothFirmware.kext
[KEXT]   9. NVMeFix.kext
[KEXT] 
[KEXT] === Kext Selection Complete: 9 kexts selected ===
```

---

**Versão**: 1.0
