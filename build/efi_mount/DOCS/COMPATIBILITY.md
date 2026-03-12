# TETRATOSH - Maximum Compatibility Guide

## CPU Support

### Intel (Full Support)
| Generation | Year | Status | Kexts Required |
|------------|------|--------|----------------|
| Nehalem | 2008-2011 | ✓ Supported | FakeSMC, NullCPUPowerManagement |
| Westmere | 2010-2011 | ✓ Supported | FakeSMC |
| Sandy Bridge | 2011-2012 | ✓ Supported | AppleALC, WhateverGreen |
| Ivy Bridge | 2012-2013 | ✓ Supported | AppleALC, WhateverGreen |
| Haswell | 2013-2014 | ✓ Full Support | WhateverGreen, AppleALC |
| Broadwell | 2014-2015 | ✓ Full Support | WhateverGreen, AppleALC |
| Skylake | 2015-2016 | ✓ Full Support | WhateverGreen, AppleALC |
| Kaby Lake | 2016-2017 | ✓ Full Support | WhateverGreen, AppleALC |
| Coffee Lake | 2017-2019 | ✓ Full Support | WhateverGreen, AppleALC |
| Comet Lake | 2019-2020 | ✓ Full Support | WhateverGreen, AppleALC |
| Ice Lake | 2019-2020 | ✓ Full Support | WhateverGreen, AppleALC |
| Rocket Lake | 2020-2021 | ✓ Full Support | WhateverGreen, AppleALC |
| Alder Lake | 2021-2024 | ✓ Full Support | WhateverGreen, AppleALC |
| Raptor Lake | 2022-2024 | ✓ Full Support | WhateverGreen, AppleALC |

### AMD (Full Support)
| Family | Year | Status | Kexts Required |
|--------|------|--------|----------------|
| Bulldozer | 2011-2012 | ✓ Supported | NullCPUPowerManagement |
| Piledriver | 2012-2014 | ✓ Supported | NullCPUPowerManagement |
| Excavator | 2014-2016 | ✓ Supported | NullCPUPowerManagement |
| Zen | 2017-2019 | ✓ Full Support | CPUFriend |
| Zen+ | 2018-2019 | ✓ Full Support | CPUFriend |
| Zen2 | 2019-2020 | ✓ Full Support | CPUFriend, WhateverGreen |
| Zen3 | 2020-2022 | ✓ Full Support | CPUFriend, WhateverGreen |
| Zen4 | 2022-2024 | ✓ Full Support | CPUFriend, WhateverGreen |

## GPU Support

### Intel Integrated Graphics
| GPU | Status | Platform ID |
|-----|--------|-------------|
| HD Graphics (Sandy) | ✓ | 0x01010000 |
| HD Graphics 3000 (Sandy) | ✓ | 0x01030000 |
| HD Graphics 4000 (Ivy) | ✓ | 0x01660000 |
| HD Graphics 4600 (Haswell) | ✓ | 0x04120000 |
| Iris Pro 5200 (Broadwell) | ✓ | 0x16220000 |
| HD Graphics 530 (Skylake) | ✓ | 0x19160000 |
| HD Graphics 630 (Kaby) | ✓ | 0x59160000 |
| UHD Graphics 630 (Coffee) | ✓ | 0x3E9B0000 |
| Iris Plus (Ice Lake) | ✓ | 0x8A500000 |
| Iris Xe (Tiger Lake) | ✓ | 0x9A740000 |
| Iris Xe (Alder Lake) | ✓ | 0x9A840000 |

### NVIDIA GPUs
| Series | Status | Notes |
|--------|--------|-------|
| GeForce 7000-9000 | Legacy | No acceleration |
| GeForce GT 600-900 | Legacy | No acceleration |
| GeForce 1000-2000 (Maxwell/Pascal) | Web Driver | Requires NVIDIA Web Driver |
| GeForce 3000 (Turing) | Web Driver | Requires NVIDIA Web Driver |
| GeForce 4000 (Ada) | Web Driver | Requires NVIDIA Web Driver |

### AMD GPUs
| Series | Status | Notes |
|--------|--------|-------|
| Radeon HD 5000-7000 | Legacy | No acceleration |
| Radeon R200-R300 | ✓ Native | WhateverGreen |
| Radeon RX 400-500 (Polaris) | ✓ Native | WhateverGreen |
| Radeon RX Vega | ✓ Native | WhateverGreen |
| Radeon RX 5000 (Navi) | ✓ Native | WhateverGreen |
| Radeon RX 6000 (RDNA2) | ✓ Native | WhateverGreen |
| Radeon RX 7000 (RDNA3) | ✓ Native | WhateverGreen |

## Network Support

### Intel Ethernet
| Controller | Kext |
|------------|------|
| 82540-82574 | AppleIntelE1000e |
| 82575-82579 | AppleIntelE1000e |
| I219-V/LM | IntelMausi |
| I225-V | IntelMausi |
| I226-V | IntelMausi |

### Realtek Ethernet
| Controller | Kext |
|------------|------|
| RTL8101-8107 | RealtekRTL8111 |
| RTL8111-8168 | RealtekRTL8111 |
| RTL8125 | RealtekRTL8125 |
| RTL8152/8153 | RealtekRTL8152 |

### Wireless
| Controller | Kext |
|------------|------|
| Intel Wi-Fi (AC) | AirportItlwm |
| Intel Wi-Fi (AX) | AirportItlwm |
| Broadcom Wi-Fi | AirportBrcmFixup |
| Realtek Wi-Fi | AirportItlwm |

## Audio Support

### Realtek ALC
| Codec | Layout ID | Kext |
|-------|-----------|------|
| ALC887 | 0x11 | AppleALC |
| ALC892 | 0x04 | AppleALC |
| ALC1220 | 0x07 | AppleALC |
| ALC1220A | 0x0C | AppleALC |
| ALC285 | 0x17 | AppleALC |
| ALC269 | 0x03 | AppleALC |
| ALC293 | 0x15 | AppleALC |

### Other Codecs
| Codec | Kext |
|-------|------|
| Conexant | AppleALC |
| VIA VT2021 | AppleALC |
| IDT | AppleALC |
| Legacy | VoodooHDA |

## Storage Support

| Controller | Support | Kext |
|------------|---------|------|
| AHCI (Intel) | Native | - |
| AHCI (AMD) | Native | - |
| NVMe (Intel) | Native | NVMeFix |
| NVMe (Samsung) | Native | NVMeFix |
| NVMe (AMD) | Native | NVMeFix |
| RAID | Native | - |

## USB Support

All USB controllers are supported via USBInjectAll and XHCI-unsupported when needed.

## Input Devices

### Keyboard/Mouse
- PS/2: VoodooPS2Controller
- USB: Native
- I2C: VoodooI2C

### Trackpad
- PS/2: VoodooPS2Controller
- I2C: VoodooI2C + VoodooI2CHID

## macOS Version Support

| CPU Generation | Min macOS | Max macOS |
|----------------|-----------|-----------|
| Nehalem | 10.6 | 10.13 |
| Westmere | 10.6 | 10.13 |
| Sandy Bridge | 10.7 | 10.15 |
| Ivy Bridge | 10.8 | 12.x |
| Haswell | 10.9 | 14.x |
| Broadwell | 10.10 | 14.x |
| Skylake | 10.11 | 14.x |
| Kaby Lake | 10.12 | 14.x |
| Coffee Lake | 10.13 | 15.x |
| Comet Lake | 10.15 | 15.x |
| Ice Lake | 10.15 | 15.x |
| AMD Zen | 10.13 | 15.x |
| AMD Zen2 | 10.15 | 15.x |
| AMD Zen3 | 11.0 | 15.x |
| AMD Zen4 | 13.0 | 15.x |

## Offline Mode

The system is designed to work completely offline:

1. **All kexts pre-downloaded**: Every community kext included
2. **All ACPI patches included**: SSDTs and patches for all CPUs
3. **Device database embedded**: Complete hardware compatibility list
4. **No internet required at boot**: Everything bundled in ISO

## Build Output

After running build scripts:
- `kexts/` - All community kexts
- `acpi/` - All ACPI files
- `device_database/` - Hardware database
- `acpi_database/` - ACPI patches

Total size: ~500MB - 1GB (depending on kext versions)
