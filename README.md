# TETRATOSH

# credits for opencore project because i use opencore on the chainload

# anyone if want: please help to construct and build the project opening a pull request

Automated Hackintosh Bootloader - A fully automatic bootloader that prepares a Hackintosh environment before launching OpenCore.

## Build Status


## ISO Structure

```
Tetratosh.iso
├── EFI/
│   ├── BOOT/
│   │   └── BOOTX64.efi         # UEFI Shell
│   ├── Tetratosh/
│   │   ├── Config/config.plist # OpenCore config
│   │   ├── ACPI/               # ACPI patches database
│   │   ├── Kexts/              # Kexts directory
│   │   └── *.json              # Device databases
│   └── OVMF.fd                 # UEFI Firmware
├── installers/                  # macOS installers (add .dmg here)
│   ├── HighSierra/
│   ├── Mojave/
│   ├── Catalina/
│   ├── BigSur/
│   ├── Monterey/
│   ├── Ventura/
│   └── Sonoma/
└── startup.nsh                 # Startup script
```

## Como Usar

1. **Gravar ISO em USB**:
   ```bash
   sudo dd if=Tetratosh.iso of=/dev/sdX bs=1M
   ```

2. **Adicionar instaladores macOS**:
   - Coloque seus `.dmg` nas pastas `installers/`
   - Ex: `installers/HighSierra/Install_macOS_High_Sierra.dmg`

3. **Bootar**:
   - Configure o BIOS para UEFI boot
   - Boot do USB

## Para Desenvolvedores

### Compilar bootloader (requer EDK2):
```bash
# Setup EDK2
./scripts/setup_edk2.sh

# Compilar
./scripts/build_tetratosh.sh
```

### Estrutura do Projeto

```
TETRATOSH/
├── bootloader/           # Código fonte UEFI
│   ├── src/            # Módulos C
│   └── include/        # Headers
├── device_database/     # Banco de dados de hardware
├── acpi_database/      # Patches ACPI
├── installers/         # Instaladores macOS
├── scripts/            # Scripts de build
├── docs/              # Documentação
└── Tetratosh.iso     # ISO pronta
```

## Características

- **100% Offline**: Não precisa de internet no boot
- **Hardware Detection**: CPU, GPU, RAM, Rede, Áudio
- **Kexts Automáticos**: Seleciona kexts baseado no hardware
- **Legacy Support**: Suporte a hardware antigo (Pentium 4, Core 2, etc)

## Credits

- OpenCore Team
- Acidanthera (Lilu, WhateverGreen, AppleALC , etc)
- EDK2 Project
