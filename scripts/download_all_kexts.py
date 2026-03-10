#!/usr/bin/env python3
"""
TETRATOSH - COMPLETE Kext Downloader
Downloads ALL community kexts for MAXIMUM compatibility
Including legacy kexts for Haswell, Ivy Bridge, Sandy Bridge, etc.

Usage:
    python3 download_all_kexts.py [--output-dir DIR] [--dry-run]

Author: TETRATOSH Team
"""

import os
import sys
import json
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Optional
from urllib.request import urlretrieve, urlopen
from urllib.error import URLError, HTTPError
import ssl
import re
import time


# COMPLETE DATABASE - ALL COMMUNITY KEXTS
KEXTS_DB = {
    # =====================================================
    # ESSENTIAL KEXTS (Always Required)
    # =====================================================
    "Essential": {
        "Lilu": {
            "repo": "acidanthera/Lilu",
            "pattern": r"Lilu[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Arbitrary kext and process patching - REQUIRED"
        },
        "VirtualSMC": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"VirtualSMC[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Advanced Apple SMC emulator - REQUIRED"
        },
    },
    
    # =====================================================
    # GPU KEXTS
    # =====================================================
    "GPU-WhateverGreen": {
        "WhateverGreen": {
            "repo": "acidanthera/WhateverGreen",
            "pattern": r"WhateverGreen[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "GPU patches - Intel/NVIDIA/AMD"
        },
    },
    
    "GPU-NVIDIA": {
        "NVWebDriverFixup": {
            "repo": "acidanthera/NVWebDriverFixup",
            "pattern": r"NVWebDriverFixup[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "NVIDIA Web Driver support"
        },
    },
    
    "GPU-Legacy": {
        "FakePCIID": {
            "repo": "RehabMan/OS-X-Fake-PCI-ID",
            "pattern": r"FakePCIID[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Fake PCI ID for legacy GPUs"
        },
        "FakePCIID_Intel_HD4000": {
            "repo": "RehabMan/OS-X-Fake-PCI-ID",
            "pattern": r"FakePCIID[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel HD4000 support"
        },
        "FakePCIID_Intel_HD5000": {
            "repo": "RehabMan/OS-X-Fake-PCI-ID",
            "pattern": r"FakePCIID[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel HD5000 support"
        },
    },
    
    # =====================================================
    # AUDIO KEXTS
    # =====================================================
    "Audio-AppleALC": {
        "AppleALC": {
            "repo": "acidanthera/AppleALC",
            "pattern": r"AppleALC[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Native AppleALC audio patcher"
        },
    },
    
    "Audio-Legacy": {
        "VoodooHDA": {
            "repo": "RehabMan/OS-X-VoodooHDA",
            "pattern": r"VoodooHDA[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Legacy VoodooHDA - for unsupported codecs"
        },
        "AppleHDA": {
            "repo": "RehabMan/OS-X-AppleHDA",
            "pattern": r"AppleHDA[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "AppleHDA patcher for legacy"
        },
        "AppleHDA1220": {
            "repo": "RehabMan/OS-X-AppleHDA",
            "pattern": r"AppleHDA[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "ALC1220 patch"
        },
        "AppleHDA889": {
            "repo": "RehabMan/OS-X-AppleHDA",
            "pattern": r"AppleHDA[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "ALC889 patch"
        },
    },
    
    # =====================================================
    # NETWORK KEXTS - INTEL
    # =====================================================
    "Network-Intel": {
        "IntelMausi": {
            "repo": "Mieze/IntelMausiEthernet",
            "pattern": r"IntelMausi[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel I219/I225/I226 Ethernet"
        },
        "IntelGBMausi": {
            "repo": "Mieze/IntelMausiEthernet",
            "pattern": r"IntelGBMausi[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Gigabit Ethernet"
        },
        "IntelCopperMausi": {
            "repo": "Mieze/IntelMausiEthernet",
            "pattern": r"IntelMausi[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Copper M"
        },
    },
    
    "Network-Intel-Legacy": {
        "AppleIntelE1000e": {
            "repo": "RehabMan/OS-X-Intel-Ethernet",
            "pattern": r"AppleIntelE1000e[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel E1000 - legacy LAN"
        },
        "AppleIntelE1000e_2": {
            "repo": "RehabMan/OS-X-Intel-Ethernet",
            "pattern": r"AppleIntelE1000e[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel E1000e v2"
        },
        "Intel82579LM": {
            "repo": "RehabMan/OS-X-Intel-82574L",
            "pattern": r"Intel[_-]?82579[_-]?LM[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel 82579LM"
        },
        "Intel82566MM": {
            "repo": "RehabMan/OS-X-Intel-82566MM",
            "pattern": r"Intel82566MM[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel 82566MM"
        },
    },
    
    # =====================================================
    # NETWORK KEXTS - REALTEK
    # =====================================================
    "Network-Realtek": {
        "RealtekRTL8111": {
            "repo": "Mieze/RTL8111_driver_for_Mac_OS",
            "pattern": r"RTL8111[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek 8111/8168"
        },
        "RealtekRTL8125": {
            "repo": "Mieze/RTL8125_driver_for_Mac_OS",
            "pattern": r"RTL8125[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek 8125 2.5GbE"
        },
        "RealtekRTL8152": {
            "repo": "Mieze/RTL8152_driver_for_Mac_OS",
            "pattern": r"RTL8152[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek RTL8152/8153 USB"
        },
    },
    
    "Network-Realtek-Legacy": {
        "AppleRTL8169": {
            "repo": "RehabMan/OS-X-Realtek-Network",
            "pattern": r"AppleRTL8169[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek 8169"
        },
    },
    
    # =====================================================
    # NETWORK KEXTS - BROADCOM
    # =====================================================
    "Network-Broadcom": {
        "AppleBCM5701": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"AppleBCM5701[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom BCM57xx"
        },
        "BrcmPatchRAM": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BrcmPatchRAM[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom RAM patcher"
        },
        "BrcmFirmwareData": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BrcmFirmwareData[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom firmware"
        },
    },
    
    # =====================================================
    # WIRELESS KEXTS - INTEL
    # =====================================================
    "Wireless-Intel": {
        "AirportItlwm": {
            "repo": "OpenIntelWireless/itlwm",
            "pattern": r"AirportItlwm[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Wi-Fi"
        },
        "IntelBluetoothFirmware": {
            "repo": "OpenIntelWireless/IntelBluetoothFirmware",
            "pattern": r"IntelBluetoothFirmware[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Bluetooth"
        },
        "itlwm": {
            "repo": "OpenIntelWireless/itlwm",
            "pattern": r"itlwm[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Wi-Fi legacy"
        },
        "IntelWiFiMVM": {
            "repo": "OpenIntelWireless/IntelWiFiMVM",
            "pattern": r"IntelWiFiMVM[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel WiFi MVM"
        },
    },
    
    # =====================================================
    # WIRELESS KEXTS - BROADCOM
    # =====================================================
    "Wireless-Broadcom": {
        "AirportBrcmFixup": {
            "repo": "Acidanthera/AirportBrcmFixup",
            "pattern": r"AirportBrcmFixup[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom Wi-Fi patcher"
        },
    },
    
    "Wireless-Broadcom-Legacy": {
        "AirportBrcm4331": {
            "repo": "RehabMan/OS-X-AirPort-Brcm4331",
            "pattern": r"Airport-Brcm4331[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom BCM4331"
        },
        "AirportBrcm43224": {
            "repo": "RehabMan/OS-X-AirPort-Brcm43224",
            "pattern": r"Airport-Brcm43224[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom BCM43224"
        },
    },
    
    # =====================================================
    # WIRELESS KEXTS - REALTEK
    # =====================================================
    "Wireless-Realtek": {
        "AirportRealtek": {
            "repo": "OpenIntelWireless/itlwm",
            "pattern": r"itlwm[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek Wi-Fi support"
        },
    },
    
    # =====================================================
    # BLUETOOTH KEXTS
    # =====================================================
    "Bluetooth": {
        "BlueToolFixup": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BlueToolFixup[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Bluetooth fixup"
        },
    },
    
    "Bluetooth-Legacy": {
        "BrcmBluetooth": {
            "repo": "RehabMan/OS-X-BrcmBluetooth",
            "pattern": r"BrcmBluetooth[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Legacy Broadcom Bluetooth"
        },
    },
    
    # =====================================================
    # STORAGE KEXTS
    # =====================================================
    "Storage": {
        "NVMeFix": {
            "repo": "Acidanthera/NVMeFix",
            "pattern": r"NVMeFix[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "NVMe drive fix"
        },
        "AHCIPortInjector": {
            "repo": "Acidanthera/AHCI_3rdParty_EFI",
            "pattern": r"AHCIPortInjector[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "AHCI port injector"
        },
    },
    
    "Storage-Legacy": {
        "AHCI_3rdParty_EFI": {
            "repo": "Acidanthera/AHCI_3rdParty_EFI",
            "pattern": r"AHCI[_-]?3rd[_-]?Party[_-]?EFI[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "3rd Party AHCI"
        },
        "AppleAHCIPortInjector": {
            "repo": "RehabMan/OS-X-AHCIPortInjector",
            "pattern": r"AppleAHCIPortInjector[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Legacy AHCI injector"
        },
    },
    
    # =====================================================
    # USB KEXTS
    # =====================================================
    "USB": {
        "USBInjectAll": {
            "repo": "RehabMan/OS-X-USB-Inject-All",
            "pattern": r"USBInjectAll[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "USB inject all"
        },
        "XHCI-unsupported": {
            "repo": "RehabMan/OS-X-USB-Inject-All",
            "pattern": r"XHCI[_-]unsupported[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Unsupported XHCI"
        },
    },
    
    "USB-Legacy": {
        "USBLegacy": {
            "repo": "RehabMan/OS-X-USB-Legacy",
            "pattern": r"USB[_-]?Legacy[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Legacy USB support"
        },
    },
    
    # =====================================================
    # INPUT KEXTS - PS/2
    # =====================================================
    "Input-PS2": {
        "VoodooPS2Controller": {
            "repo": "RehabMan/OS-X-Voodoo-PS2-Controller",
            "pattern": r"VoodooPS2[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "PS/2 keyboard and mouse"
        },
        "VoodooPS2Keyboard": {
            "repo": "RehabMan/OS-X-Voodoo-PS2-Controller",
            "pattern": r"VoodooPS2[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "PS/2 Keyboard"
        },
        "VoodooPS2Mouse": {
            "repo": "RehabMan/OS-X-Voodoo-PS2-Controller",
            "pattern": r"VoodooPS2[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "PS/2 Mouse"
        },
        "VoodooPS2Trackpad": {
            "repo": "RehabMan/OS-X-Voodoo-PS2-Controller",
            "pattern": r"VoodooPS2[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "PS/2 Trackpad"
        },
    },
    
    # =====================================================
    # INPUT KEXTS - I2C
    # =====================================================
    "Input-I2C": {
        "VoodooI2C": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2C[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "I2C trackpad/touchscreen"
        },
        "VoodooI2CHID": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2CHID[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "I2C HID driver"
        },
        "VoodooI2CELAN": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2C[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "ELAN I2C"
        },
        "VoodooI2CSynaptics": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2C[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Synaptics I2C"
        },
    },
    
    # =====================================================
    # INPUT KEXTS - USB
    # =====================================================
    "Input-USB": {
        "VoodooUSB": {
            "repo": "RehabMan/OS-X-VoodooUSB",
            "pattern": r"VoodooUSB[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "USB input support"
        },
    },
    
    # =====================================================
    # SMC PLUGINS
    # =====================================================
    "SMC-Plugins": {
        "SMCProcessor": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCProcessor[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "SMC processor plugin"
        },
        "SMCSuperIO": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCSuperIO[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "SMC super I/O plugin"
        },
    },
    
    "SMC-Legacy": {
        "FakeSMC": {
            "repo": "RehabMan/OS-X-FakeSMC",
            "pattern": r"FakeSMC[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Legacy FakeSMC"
        },
        "SMCAMD": {
            "repo": "RehabMan/OS-X-SMCAMD",
            "pattern": r"SMCAMD[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "AMD SMC"
        },
    },
    
    # =====================================================
    # UTILITY KEXTS
    # =====================================================
    "Utility": {
        "HibernationFixup": {
            "repo": "acidanthera/HibernationFixup",
            "pattern": r"HibernationFixup[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Hibernation fix"
        },
        "CPUFriend": {
            "repo": "acidanthera/CPUFriend",
            "pattern": r"CPUFriend[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "CPU power management"
        },
        "CPUFriendDataProvider": {
            "repo": "acidanthera/CPUFriendDataProvider",
            "pattern": r"CPUFriendDataProvider[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "CPUFriend data"
        },
        "FeatureUnlock": {
            "repo": "acidanthera/FeatureUnlock",
            "pattern": r"FeatureUnlock[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Unlock macOS features"
        },
        "RTCMemoryFixup": {
            "repo": "acidanthera/RTCMemoryFixup",
            "pattern": r"RTCMemoryFixup[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "RTC memory fix"
        },
    },
    
    "Utility-Legacy": {
        "NullCPUPowerManagement": {
            "repo": "RehabMan/OS-X-NULL-CPU-Power-Management",
            "pattern": r"NullCPUPowerManagement[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Null CPU power management"
        },
        "EFICheckDisabler": {
            "repo": "RehabMan/OS-X-EfiCheckDisabler",
            "pattern": r"EFICheckDisabler[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "EFI check disabler"
        },
        "KextsToPatch": {
            "repo": "RehabMan/OS-X-kexts-patch",
            "pattern": r"kexts[_-]?to[_-]?patch[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Kexts to patch"
        },
    },
    
    # =====================================================
    # SENSOR KEXTS
    # =====================================================
    "Sensors": {
        "SMCDellSensors": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCDellSensors[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Dell SMC sensors"
        },
        "SMCAmi": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCAmi[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "AMI SMC sensors"
        },
        "SMCAsus": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCAsus[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "ASUS SMC sensors"
        },
    },
    
    # =====================================================
    # VIDEO PROBE KEXTS
    # =====================================================
    "Video": {
        "IOProbe": {
            "repo": "acidanthera/IOProbe",
            "pattern": r"IOProbe[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Video probe"
        },
    },
    
    # =====================================================
    # CRYPTO KEXTS
    # =====================================================
    "Crypto": {
        "TPM": {
            "repo": "acidanthera/TPM",
            "pattern": r"TPM[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "TPM support"
        },
    },
    
    # =====================================================
    # DISPLAY SWITCHER
    # =====================================================
    "Display": {
        "DisplaySwitch": {
            "repo": "acidanthera/DisplaySwitch",
            "pattern": r"DisplaySwitch[_-]v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Display switcher"
        },
    },
    
    # =====================================================
    # DEBUG KEXTS
    # =====================================================
    "Debug": {
        "LiluDebug": {
            "repo": "acidanthera/Lilu",
            "pattern": r"Lilu[_-]v?(\d+\.\d+\.\d+)[_-]debug\.zip",
            "required": False,
            "description": "Lilu debug version"
        },
    },
}


class CompleteKextDownloader:
    """Downloads ALL community kexts for maximum compatibility."""
    
    def __init__(self, output_dir: str = "kexts", dry_run: bool = False, verbose: bool = False):
        self.output_dir = Path(output_dir)
        self.dry_run = dry_run
        self.verbose = verbose
        self.ctx = ssl.create_default_context()
        self.ctx.check_hostname = False
        self.ctx.verify_mode = ssl.CERT_NONE
        self.downloaded = []
        self.failed = []
        
    def log(self, message: str, level: str = "INFO"):
        prefix = {
            "INFO": "[KEXT]",
            "WARN": "[KEXT WARN]",
            "ERROR": "[KEXT ERROR]",
            "OK": "[KEXT OK]"
        }
        print(f"{prefix.get(level, '[KEXT]')} {message}")
    
    def create_directories(self):
        """Create complete directory structure."""
        categories = [
            "essential",
            "gpu",
            "audio",
            "network/intel",
            "network/intel/legacy",
            "network/realtek",
            "network/realtek/legacy",
            "network/broadcom",
            "wireless/intel",
            "wireless/broadcom",
            "wireless/broadcom/legacy",
            "wireless/realtek",
            "bluetooth",
            "bluetooth/legacy",
            "storage",
            "storage/legacy",
            "usb",
            "usb/legacy",
            "input/ps2",
            "input/i2c",
            "input/usb",
            "smc",
            "smc/legacy",
            "sensors",
            "utility",
            "utility/legacy",
            "video",
            "crypto",
            "display",
            "debug"
        ]
        
        for category in categories:
            (self.output_dir / category).mkdir(parents=True, exist_ok=True)
        
        (self.output_dir / "community").mkdir(parents=True, exist_ok=True)
        
    def get_latest_release(self, repo: str) -> Optional[Dict]:
        """Get latest release from GitHub."""
        url = f"https://api.github.com/repos/{repo}/releases/latest"
        
        try:
            request = urlopen(url, context=self.ctx, timeout=30)
            data = json.loads(request.read().decode('utf-8'))
            return data
        except Exception as e:
            self.log(f"Failed to fetch {repo}: {e}", "ERROR")
            return None
    
    def find_asset_url(self, release_data: Dict, pattern: str) -> Optional[str]:
        """Find asset matching pattern."""
        if not release_data or "assets" not in release_data:
            return None
            
        regex = re.compile(pattern, re.IGNORECASE)
        
        for asset in release_data["assets"]:
            name = asset["name"]
            if regex.match(name):
                return asset["browser_download_url"]
                
        return None
    
    def download_file(self, url: str, dest: Path) -> bool:
        """Download a single file."""
        if self.dry_run:
            self.log(f"[DRY] Would download: {url} -> {dest}", "INFO")
            return True
            
        try:
            self.log(f"Downloading: {dest.name}", "INFO")
            urlretrieve(url, dest)
            return True
        except Exception as e:
            self.log(f"Failed: {dest.name} - {e}", "ERROR")
            return False
    
    def download_kext(self, kext_name: str, kext_info: Dict, category: str) -> bool:
        """Download a single kext."""
        repo = kext_info.get("repo")
        pattern = kext_info.get("pattern")
        
        if not repo or not pattern:
            return False
            
        release = self.get_latest_release(repo)
        if not release:
            self.failed.append(kext_name)
            return False
            
        asset_url = self.find_asset_url(release, pattern)
        if not asset_url:
            self.log(f"No match for {kext_name} in {repo}", "WARN")
            self.failed.append(kext_name)
            return False
            
        output_path = self.output_dir / category / f"{kext_name}.zip"
        
        if self.download_file(asset_url, output_path):
            self.downloaded.append(kext_name)
            return True
            
        self.failed.append(kext_name)
        return False
    
    def download_all(self):
        """Download all kexts from all categories."""
        total = 0
        success = 0
        
        for category, kexts in KEXTS_DB.items():
            category_dir = category.lower().replace("-", "/")
            self.log(f"\n=== {category} ===", "INFO")
            
            for kext_name, kext_info in kexts.items():
                total += 1
                if self.download_kext(kext_name, kext_info, category_dir):
                    success += 1
                    self.log(f"OK: {kext_name}", "OK")
                else:
                    self.log(f"FAIL: {kext_name}", "ERROR")
                    
                time.sleep(0.5)  # Rate limiting
        
        return total, success
    
    def list_kexts(self):
        """List all available kexts."""
        print("\n" + "="*60)
        print("COMPLETE KEXT DATABASE")
        print("="*60)
        
        for category, kexts in KEXTS_DB.items():
            print(f"\n{category}:")
            for kext_name, kext_info in kexts.items():
                req = " [REQUIRED]" if kext_info.get("required") else ""
                print(f"  - {kext_name}{req}")
                print(f"    {kext_info.get('description', '')}")


def main():
    parser = argparse.ArgumentParser(
        description="TETRATOSH - Complete Kext Downloader"
    )
    parser.add_argument("-o", "--output-dir", default="kexts", help="Output directory")
    parser.add_argument("-n", "--dry-run", action="store_true", help="Dry run")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose")
    parser.add_argument("-l", "--list", action="store_true", help="List kexts")
    
    args = parser.parse_args()
    
    downloader = CompleteKextDownloader(
        output_dir=args.output_dir,
        dry_run=args.dry_run,
        verbose=args.verbose
    )
    
    if args.list:
        downloader.list_kexts()
        return 0
    
    downloader.create_directories()
    
    print("\n" + "="*60)
    print("TETRATOSH - COMPLETE KEXT DOWNLOADER")
    print("Downloading ALL community kexts for maximum compatibility")
    print("="*60 + "\n")
    
    total, success = downloader.download_all()
    
    print("\n" + "="*60)
    print("DOWNLOAD SUMMARY")
    print("="*60)
    print(f"Total kexts: {total}")
    print(f"Downloaded: {success}")
    print(f"Failed: {total - success}")
    print("="*60)
    
    return 0 if success > 0 else 1


if __name__ == "__main__":
    sys.exit(main())
