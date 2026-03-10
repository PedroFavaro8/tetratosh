# Algoritmo de Detecção de Hardware

## 1. Visão Geral

O módulo de detecção de hardware é responsável por identificar todos os componentes do sistema, incluindo CPU, GPU, memória RAM, controladores de armazenamento, rede, áudio e dispositivos sem fio.

## 2. Fluxo de Detecção

```
┌─────────────────────────────────────────────────────────────────────┐
│                    FLUXO DE DETECÇÃO DE HARDWARE                   │
└─────────────────────────────────────────────────────────────────────┘

    ┌────────────────┐
    │  INÍCIO        │
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │ CPU Detection  │ ──▶ CPUID / MSR
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │ RAM Detection  │ ──▶ SMBIOS / MemoryInfo
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  PCI Scan      │ ──▶ PCI Config Space
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  ACPI Parse    │ ──▶ DSDT / SSDT / MADT
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  GPU Detect    │ ──▶ PCI / APCI
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  Classify      │ ──▶ Categorizar dispositivos
    └────────┬───────┘
             │
             ▼
    ┌────────────────┐
    │  FIM           │
    └────────────────┘
```

## 3. Algoritmo Principal de Detecção

### Pseudocódigo

```
FUNÇÃO DetectAllHardware() -> SystemInfo:
    Log("=== Starting Hardware Detection ===")
    
    // Inicializa estrutura de sistema
    sistema = nova SystemInfo()
    
    // Etapa 1: Detectar CPU
    Log("Detecting CPU...")
    sistema.cpu = DetectCPU()
    Log("CPU: " + sistema.cpu.brand)
    
    // Etapa 2: Detectar RAM
    Log("Detecting RAM...")
    sistema.ram_size = DetectRAM()
    Log("RAM: " + FormatBytes(sistema.ram_size))
    
    // Etapa 3: Escanear PCI
    Log("Scanning PCI devices...")
    dispositivos_pci = ScanPCI()
    Log("Found " + Count(dispositivos_pci) + " PCI devices")
    
    // Etapa 4: Parse ACPI
    Log("Parsing ACPI tables...")
    acpi_devices = ParseACPI()
    
    // Etapa 5: Classificar dispositivos
    Log("Classifying devices...")
    sistema.gpus = ClassificarGPU(dispositivos_pci)
    sistema.network_controllers = ClassificarNetwork(dispositivos_pci)
    sistema.audio_codecs = ClassificarAudio(dispositivos_pci)
    sistema.storage_controllers = ClassificarStorage(dispositivos_pci)
    sistema.wifi_controllers = ClassificarWiFi(dispositivos_pci)
    sistema.bluetooth_controllers = ClassificarBluetooth(dispositivos_pci)
    
    // Etapa 6: Detectar Chipset
    Log("Detecting chipset...")
    sistema.chipset = DetectChipset(acpi_devices)
    
    Log("=== Hardware Detection Complete ===")
    
    RETORNAR sistema
FIM FUNÇÃO
```

## 4. Detecção de CPU

### 4.1 Algoritmo

```
FUNÇÃO DetectCPU() -> CPUInfo:
    cpu = nova CPUInfo()
    
    // Ler CPUID para Intel
    cpuid_eax = 0
    ASM("mov eax, 0; cpuid" -> cpuid_eax)
    
    SE cpuid_eax >= 1:
        // CPU signature
        cpuid_eax = 1
        ASM("mov eax, 1; cpuid" -> cpuid_result)
        
        cpu.stepping = cpuid_result[0:4]
        cpu.model = cpuid_result[4:8]
        cpu.family = cpuid_result[8:12]
        cpu.type = cpuid_result[12:14]
        cpu.ext_model = cpuid_result[16:20]
        cpu.ext_family = cpuid_result[20:28]
        
        // Brand string
        PARA i = 0 ATÉ 12:
            cpuid_ebx = i * 2
            ASM("mov eax, 0x8000000" + i + "; cpuid" -> brand[i])
        FIM PARA
        cpu.brand = FormatarString(brand)
        
        // Contagem de núcleos
        cpuid_eax = 4
        ASM("mov eax, 4; mov ecx, 0; cpuid" -> cpuid_result)
        cpu.cores_por_package = (cpuid_result >> 26) + 1
        
        // Threads
        cpuid_ebx = 1
        ASM("mov eax, 1; cpuid" -> cpuid_result)
        cpu.threads_por_core = (cpuid_result >> 16) & 0xFF
        cpu.total_threads = cpu.cores_por_package * cpu.threads_por_core
    FIM SE
    
    // Detectar arquitetura
    cpu.architecture = DetectarArquitetura(cpu)
    
    // Detectar vendor
    cpu.vendor = DetectarVendor(cpuid)
    
    // Detectar features
    cpu.features = DetectarFeatures(cpuid)
    
    RETORNAR cpu
FIM FUNÇÃO

FUNÇÃO DetectarArquitetura(cpu) -> string:
    SE cpu.vendor == "GenuineIntel":
        // Determinar geração Intel
        SE cpu.ext_model >= 9:
            SE cpu.family == 6:
                // Coffee Lake e posterior
                SE cpu.model == 0x9E: RETORNAR "KabyLake"
                SE cpu.model == 0x9D: RETORNAR "KabyLake"
                SE cpu.model == 0xA5: RETORNAR "CometLake"
                SE cpu.model == 0xA6: RETORNAR "CometLake"
                SE cpu.model == 0x8A: RETORNAR "IceLake"
                SE cpu.model == 0x8D: RETORNAR "IceLake"
                SE cpu.model == 0xA7: RETORNAR "RocketLake"
                SE cpu.model == 0xC7: RETORNAR "AlderLake"
                RETORNAR "Skylake"
            FIM SE
        FIM SE
        RETORNAR "Legacy"
    
    SENÃO SE cpu.vendor == "AuthenticAMD":
        // AMD Zen
        SE cpu.ext_model >= 0x30: RETORNAR "Zen4"
        SE cpu.ext_model >= 0x20: RETORNAR "Zen3"
        SE cpu.ext_model >= 0x10: RETORNAR "Zen2"
        SE cpu.ext_model >= 0x01: RETORNAR "Zen+"
        RETORNAR "Zen"
    FIM SE
    
    RETORNAR "Unknown"
FIM FUNÇÃO
```

## 5. Detecção de RAM

### 5.1 Algoritmo

```
FUNÇÃO DetectRAM() -> uint64:
    // Método 1: Via SMBIOS
    SE existe SMBIOS:
        memoria = LerSMBIOSType17()
        RETORNAR memoria.tamanho_total
    FIM SE
    
    // Método 2: Via E820
    total = 0
    PARA CADA entrada EM E820:
        SE entrada.tipo == 1:  // Usable
            total += entrada.tamanho
        FIM SE
    FIM PARA
    RETORNAR total
FIM FUNÇÃO
```

## 6. Scanner PCI

### 6.1 Algoritmo

```
FUNÇÃO ScanPCI() -> ListaDeviceInfo:
    dispositivos = nova lista()
    
    // ObtémMCFG (PCI Express)
    mcfg = EncontrarTabelaACPI("MCFG")
    
    // Para cada bus PCI
    PARA bus = 0 ATÉ 255:
        PARA device = 0 ATÉ 31:
            // Lê Vendor ID (offset 0x00)
            vendor_id = LerConfigSpacePCI(bus, device, 0, 0x00)
            
            // Se vendor_id é válido (não 0xFFFF)
            SE vendor_id != 0xFFFF:
                // Lê Device ID (offset 0x02)
                device_id = LerConfigSpacePCI(bus, device, 0, 0x02)
                
                // Lê Class Code (offset 0x0B)
                class_code = LerConfigSpacePCI(bus, device, 0, 0x0B)
                
                // Lê Subclass (offset 0x0A)
                subclass = LerConfigSpacePCI(bus, device, 0, 0x0A)
                
                // Lê Revision (offset 0x08)
                revision = LerConfigSpacePCI(bus, device, 0, 0x08)
                
                // Lê Header Type (offset 0x0E)
                header = LerConfigSpacePCI(bus, device, 0, 0x0E)
                
                // Cria estrutura de dispositivo
                dev = novo DeviceInfo()
                dev.vendor_id = vendor_id
                dev.device_id = device_id
                dev.class_code = class_code
                dev.subclass = subclass
                dev.revision = revision & 0xFF
                dev.bus = bus
                dev.device = device
                dev.function = 0
                
                // Adiciona à lista
                dispositivos.adicionar(dev)
                
                // Se é multifunction, verifica functions 1-7
                SE (header & 0x80) != 0:
                    PARA func = 1 ATÉ 7:
                        vendor_id = LerConfigSpacePCI(bus, device, func, 0x00)
                        SE vendor_id != 0xFFFF:
                            dev = novo DeviceInfo()
                            dev.vendor_id = vendor_id
                            dev.device_id = LerConfigSpacePCI(bus, device, func, 0x02)
                            dev.class_code = LerConfigSpacePCI(bus, device, func, 0x0B)
                            dev.subclass = LerConfigSpacePCI(bus, device, func, 0x0A)
                            dev.bus = bus
                            dev.device = device
                            dev.function = func
                            dispositivos.adicionar(dev)
                        FIM SE
                    FIM PARA
                FIM SE
            FIM SE
        FIM PARA
    FIM PARA
    
    RETORNAR dispositivos
FIM FUNÇÃO

// Leitura de Configuration Space PCI
FUNÇÃO LerConfigSpacePCI(bus, device, function, offset) -> uint16:
    // Endereço PCI
    address = (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC) | 0x80000000
    
    // Escreve no endereço de configuração
    OUTPD(0xCF8, address)
    
    // Lê dados
    data = INPD(0xCFC + (offset & 3))
    
    // Extrai palavra ou byte conforme offset
    SE (offset & 2) != 0:
        RETORNAR (data >> 16) & 0xFFFF
    SENÃO:
        RETORNAR data & 0xFFFF
    FIM SE
FIM FUNÇÃO
```

## 7. Parser ACPI

### 7.1 Algoritmo

```
FUNÇÃO ParseACPI() -> ACPIInfo:
    acpi = nova ACPIInfo()
    
    // Encontrar e parsear DSDT
    dsdt = EncontrarTabelaACPI("DSDT")
    SE dsdt != NULO:
        acpi.dsdt = ParseDSDT(dsdt)
        Log("DSDT parsed, " + Count(acpi.dsdt.devices) + " devices found")
    FIM SE
    
    // Encontrar e parsear SSDTs
    ssdt_list = EncontrarTabelasACPI("SSDT")
    PARA CADA ssdt EM ssdt_list:
        acpi.ssdts.adicionar(ParseSSDT(ssdt))
    FIM PARA
    Log("Parsed " + Count(acpi.ssdts) + " SSDTs")
    
    // Encontrar MADT (APIC)
    madt = EncontrarTabelaACPI("APIC")
    SE madt != NULO:
        acpi.madt = ParseMADT(madt)
        Log("MADT parsed, " + Count(acpi.madt.processors) + " processors")
    FIM SE
    
    // Encontrar FADT
    fadt = EncontrarTabelaACPI("FACP")
    SE fadt != NULO:
        acpi.fadt = ParseFADT(fadt)
    FIM SE
    
    RETORNAR acpi
FIM FUNÇÃO

FUNÇÃO ParseDSDT(dsdt) -> DSDTInfo:
    // DSDT contémaml Diff Object
    // Precisa encontrar todos os Device() objects
    
    info = nova DSDTInfo()
    
    //寻找到 DSDT Header
    header = LerBytes(dsdt, 0, sizeof(ACPI_TABLE_HEADER))
    
    // Validar assinatura
    SE header.signature != "DSDT":
        RETORNAR NULO
    FIM SE
    
    // Encontrar dispositivos
    // Percorre a tabela em busca de "Device" objects
    pos = header.length
    ENQUANTO pos < dsdt.tamanho:
        objeto = LerProximoObjeto(dsdt, pos)
        
        SE objeto.tipo == NAMETYPE_OBJECT:
            SE objeto.nome == "PCI0" OU objeto.nome == "_SB":
                // Root PCI host bridge
                info.root_bridge = objeto
            FIM SE
        FIM SE
        
        SE objeto.tipo == SCOPE_OP:
            // Scope contém devices
            info.scopes.adicionar(objeto)
        FIM SE
        
        pos = objeto.proximo_offset
    FIM ENQUANTO
    
    RETORNAR info
FIM FUNÇÃO
```

## 8. Classificação de Dispositivos

### 8.1 Tabela de Class Codes PCI

```
CÓDIGOS DE CLASSE PCI:

Class 0x00: Unclassified
  - 0x00: Non-VGA
  - 0x01: VGA
  
Class 0x01: Mass Storage Controller
  - 0x01: IDE
  - 0x02: Floppy
  - 0x03: IPI
  - 0x04: RAID
  - 0x05: ATA
  - 0x06: SATA
  - 0x07: SAS
  - 0x08: NVMe
  
Class 0x02: Network Controller
  - 0x00: Ethernet
  - 0x01: Token Ring
  - 0x02: FDDI
  - 0x03: ATM
  - 0x04: ISDN
  - 0x05: WorldFip
  - 0x10: WiFi
  - 0x20: Cell
  
Class 0x03: Display Controller
  - 0x00: VGA
  - 0x01: XGA
  - 0x02: 3D
  - 0x80: Other
  
Class 0x04: Multimedia Controller
  - 0x01: Video
  - 0x02: Audio
  - 0x03: Telephony
  
Class 0x05: Memory Controller
  - 0x00: RAM
  - 0x01: Flash
  - 0x80: Other
  
Class 0x06: Bridge Device
  - 0x00: Host/PCI
  - 0x01: PCI/ISA
  - 0x02: PCI/EISA
  - 0x03: PCI/MCA
  - 0x04: PCI/PCI
  - 0x05: PCI/PCMCIA
  - 0x06: PCI/NuBus
  - 0x07: PCI/CardBus
  
Class 0x07: Simple Communication Controller
  - 0x00: Serial
  - 0x01: Parallel
  - 0x02: Multiport
  - 0x03: Modem
  - 0x04: GPIB
  - 0x05: Smart Card
  
Class 0x08: Base System Peripherals
  - 0x00: PIC
  - 0x01: DMA
  - 0x02: Timer
  - 0x03: RTC
  - 0x04: Hot Plug
  - 0x05: SD Host
  - 0x06: IOMMU
  
Class 0x09: Input Device
  - 0x00: Keyboard
  - 0x01: Digitizer
  - 0x02: Mouse
  - 0x03: Scanner
  - 0x10: Gameport
  
Class 0x0A: Docking Station
  - 0x00: Generic
  - 0x80: Other
  
Class 0x0B: Processor
  - 0x00: 386
  - 0x01: 486
  - 0x02: Pentium
  - 0x10: Alpha
  - 0x20: PowerPC
  - 0x30: MIPS
  - 0x40: RISC-V
  
Class 0x0C: Serial Bus Controller
  - 0x00: FireWire
  - 0x01: ACCESS.bus
  - 0x02: SSA
  - 0x03: USB
  - 0x04: Fibre Channel
  - 0x05: SMBus
  - 0x06: Infiniband
  - 0x07: IPMI
  - 0x08: SERCOS
  - 0x09: CAN
  
Class 0x0D: Wireless Controller
  - 0x01: iRDA
  - 0x10: Bluetooth
  - 0x20: Broadband
  - 0x80: Other
```

### 8.2 Algoritmo de Classificação

```
FUNÇÃO ClassificarGPU(dispositivos) -> ListaGPU:
    gpus = nova lista()
    
    PARA CADA dev EM dispositivos:
        // Class Code 0x03 = Display Controller
        SE dev.class_code == 0x03:
            gpu = novo GPUInfo()
            gpu.pci_address = dev.bus + ":" + dev.device + ":" + dev.function
            gpu.vendor_id = dev.vendor_id
            gpu.device_id = dev.device_id
            
            // Identificar vendor
            SE dev.vendor_id == 0x8086:
                gpu.vendor = "Intel"
                gpu.name = IdentificarGPUIntel(dev.device_id)
            SENÃO SE dev.vendor_id == 0x10DE:
                gpu.vendor = "NVIDIA"
                gpu.name = IdentificarGPUNVIDIA(dev.device_id)
            SENÃO SE dev.vendor_id == 0x1002:
                gpu.vendor = "AMD"
                gpu.name = IdentificarGPUAMD(dev.device_id)
            FIM SE
            
            gpus.adicionar(gpu)
            Log("GPU detected: " + gpu.vendor + " " + gpu.name)
        FIM SE
    FIM PARA
    
    RETORNAR gpus
FIM FUNÇÃO

FUNÇÃO ClassificarNetwork(dispositivos) -> ListaNetwork:
    redes = nova lista()
    
    PARA CADA dev EM dispositivos:
        // Class Code 0x02 = Network Controller
        SE dev.class_code == 0x02:
            rede = novo NetworkInfo()
            rede.vendor_id = dev.vendor_id
            rede.device_id = dev.device_id
            rede.pci_address = FormatAddress(dev)
            
            // Identificar vendor
            SE dev.vendor_id == 0x8086:
                rede.vendor = "Intel"
                rede.name = IdentificarNetworkIntel(dev.device_id)
            SENÃO SE dev.vendor_id == 0x10EC:
                rede.vendor = "Realtek"
                rede.name = IdentificarNetworkRealtek(dev.device_id)
            SENÃO SE dev.vendor_id == 0x14E4:
                rede.vendor = "Broadcom"
                rede.name = IdentificarNetworkBroadcom(dev.device_id)
            FIM SE
            
            redes.adicionar(rede)
            Log("Network: " + rede.vendor + " " + rede.name)
        FIM SE
    FIM PARA
    
    RETORNAR redes
FIM FUNÇÃO

FUNÇÃO ClassificarAudio(dispositivos) -> ListaAudio:
    audios = nova lista()
    
    PARA CADA dev EM dispositivos:
        // Multimedia Controller (0x04) - Audio (0x03)
        SE dev.class_code == 0x04 E dev.subclass == 0x03:
            audio = novo AudioInfo()
            audio.vendor_id = dev.vendor_id
            audio.device_id = dev.device_id
            audio.pci_address = FormatAddress(dev)
            
            // Identificar vendor
            SE dev.vendor_id == 0x10EC:
                audio.vendor = "Realtek"
                audio.name = IdentificarAudioRealtek(dev.device_id)
            SENÃO SE dev.vendor_id == 0x1106:
                audio.vendor = "VIA"
            SENÃO SE dev.vendor_id == 0x1002:
                audio.vendor = "AMD"
            FIM SE
            
            audios.adicionar(audio)
            Log("Audio: " + audio.vendor + " " + audio.name)
        FIM SE
    FIM PARA
    
    RETORNAR audios
FIM FUNÇÃO

FUNÇÃO ClassificarStorage(dispositivos) -> ListaStorage:
    storages = nova lista()
    
    PARA CADA dev EM dispositivos:
        // Class Code 0x01 = Mass Storage
        SE dev.class_code == 0x01:
            storage = novo StorageInfo()
            storage.vendor_id = dev.vendor_id
            storage.device_id = dev.device_id
            storage.subclass = dev.subclass
            storage.pci_address = FormatAddress(dev)
            
            SE dev.subclass == 0x06:  // SATA
                storage.type = "SATA"
            SENÃO SE dev.subclass == 0x08:  // NVMe
                storage.type = "NVMe"
            SENÃO SE dev.subclass == 0x05:  // ATA
                storage.type = "ATA"
            SENÃO SE dev.subclass == 0x04:  // RAID
                storage.type = "RAID"
            FIM SE
            
            storages.adicionar(storage)
            Log("Storage: " + storage.type)
        FIM SE
    FIM PARA
    
    RETORNAR storages
FIM FUNÇÃO

FUNÇÃO ClassificarWiFi(dispositivos) -> ListaWiFi:
    wifis = nova lista()
    
    PARA CADA dev EM dispositivos:
        // Wireless Controller (0x0D)
        SE dev.class_code == 0x0D:
            SE dev.subclass == 0x10:  // Bluetooth
                // Bluetooth (não é WiFi)
            SENÃO:
                wifi = novo WiFiInfo()
                wifi.vendor_id = dev.vendor_id
                wifi.device_id = dev.device_id
                wifi.pci_address = FormatAddress(dev)
                
                SE dev.vendor_id == 0x8086:
                    wifi.vendor = "Intel"
                    wifi.name = "Intel Wi-Fi"
                SENÃO SE dev.vendor_id == 0x14E4:
                    wifi.vendor = "Broadcom"
                    wifi.name = "Broadcom Wi-Fi"
                SENÃO SE dev.vendor_id == 0x10EC:
                    wifi.vendor = "Realtek"
                    wifi.name = "Realtek Wi-Fi"
                FIM SE
                
                wifis.adicionar(wifi)
                Log("WiFi: " + wifi.vendor + " " + wifi.name)
            FIM SE
        FIM SE
    FIM PARA
    
    RETORNAR wifis
FIM FUNÇÃO
```

## 9. Detecção de Chipset

```
FUNÇÃO DetectChipset(acpi) -> ChipsetInfo:
    chipset = novo ChipsetInfo()
    
    // Método 1: Via DSDT
    SE acpi.dsdt != NULO:
        // Procurar por _SB ou PCI0
        root = acpi.dsdt.root_bridge
        
        // Verificar dispositivo PCH
        pch = EncontrarDispositivo(root, "PCH")
        SE pch != NULO:
            chipset.pch = ParsePCH(pch)
        FIM SE
    FIM SE
    
    // Método 2: Via Device IDs
    // Identificar Intel PCH pelo device ID
    chipset.is_intel = VerificarPCHIntel(pci_devices)
    chipset.is_amd = VerificarAMD chipset(pci_devices)
    
    RETORNAR chipset
FIM FUNÇÃO
```

## 10. Logging de Detecção

```
┌─────────────────────────────────────────────────────────────────────┐
│                    EXEMPLO DE LOG DE DETECÇÃO                      │
└─────────────────────────────────────────────────────────────────────┘

[DETECT] Starting hardware detection...
[DETECT] CPU: Detecting CPU...
[DETECT] CPU: Intel(R) Core(TM) i7-10700K CPU @ 3.80GHz
[DETECT] CPU: Architecture: CometLake
[DETECT] CPU: Cores: 8, Threads: 16
[DETECT] RAM: Detecting memory...
[DETECT] RAM: Total: 32 GB (DDR4)
[DETECT] PCI: Scanning PCI bus...
[DETECT] PCI: Found 24 devices
[DETECT] CLASS: GPU: Intel Corporation UHD Graphics 630
[DETECT] CLASS: GPU: NVIDIA Corporation GA102 [GeForce RTX 3080]
[DETECT] CLASS: NET: Intel Corporation Ethernet Controller I225-V
[DETECT] CLASS: NET: Realtek Semiconductor RTL8125
[DETECT] CLASS: AUDIO: Realtek ALC1220
[DETECT] CLASS: STOR: Intel Corporation SATA Controller
[DETECT] CLASS: STOR: Intel Corporation NVMe Controller
[DETECT] CLASS: USB: Intel Corporation USB Controller
[DETECT] ACPI: Parsing ACPI tables...
[DETECT] ACPI: DSDT found, 156 devices
[DETECT] ACPI: SSDTs found: 12
[DETECT] ACPI: MADT: 16 processors
[DETECT] CHIPSET: Intel Z490
[DETECT] Hardware detection complete!
```

## 11. Tabela de Identificação de Dispositivos

### 11.1 GPUs Intel

```
Device ID    Nome
---------------------------------
0x5912       HD Graphics 630
0x591B       HD Graphics P630
0x591D       UHD Graphics 620
0x591E       UHD Graphics 615
0x591F       UHD Graphics 600
0x3E90       UHD Graphics 610
0x3E91       UHD Graphics 615
0x3E92       UHD Graphics 617
0x3E93       UHD Graphics P630
0x9BC4       Iris Plus Graphics 940
0x9BC5       Iris Plus Graphics 950
0x8A70       Iris Plus Graphics G7
0x8A71       Iris Plus Graphics G7
0x8A52       Intel Xe Graphics
0x8A56       Intel Xe Graphics MAX
```

### 11.2 GPUs NVIDIA

```
Device ID    Nome
---------------------------------
0x1B80       GeForce GTX 1080
0x1B83       GeForce GTX 1080 Ti
0x1B84       GeForce GTX 1070
0x1B86       GeForce GTX 1080 SLI
0x1F10      RTX 2080 Ti
0x1F11      RTX 2080
0x1F12      RTX 2070
0x1F13      RTX 2080 Super
0x1F14      RTX 2070 Super
0x1F15      RTX 2060 Super
0x1F16      RTX 2060
0x2204      RTX 4090
0x2205      RTX 4080
0x2206      RTX 4070 Ti
0x2207      RTX 4070
```

### 11.3 GPUs AMD

```
Device ID    Nome
---------------------------------
0x687F       Radeon RX Vega 64
0x6863       Radeon RX Vega 56
0x6879       Radeon VII
0x67DF      Radeon RX 5700 XT
0x67C7      Radeon RX 5700
0x67EF      Radeon RX 5600 XT
0x73DF      Radeon RX 6900 XT
0x73BF      Radeon RX 6800 XT
0x73AF      Radeon RX 6800
```

### 11.4 Controladores de Rede Intel

```
Device ID    Nome
---------------------------------
0x15B7       Intel I219-V
0x15B8       Intel I219-LM
0x15D7       Intel I219-V (CFL)
0x15E3       Intel I219-V (CNL)
0x15BC       Intel I219-LM (SKL)
0x156F       Intel I219-SF (BDW)
0x15A2       Intel I219-LM (HSW)
0x15D9       Intel I226-V (Tiger Lake)
0x15F3       Intel I225-V (Comet Lake)
0x15F4       Intel I225-V (Rocket Lake)
```

### 11.5 Controladores de Rede Realtek

```
Device ID    Nome
---------------------------------
0x8168       RTL8111/8168/8411
0x8125       RTL8125 2.5GbE
0x8136       RTL8105E/RTL8105E
0x8152       RTL8152/RTL8153
0x8156       RTL8156 (2.5G)
```

---

**Versão**: 1.0
