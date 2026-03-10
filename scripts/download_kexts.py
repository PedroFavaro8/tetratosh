#!/usr/bin/env python3
"""
TETRATOSH - Kext Downloader
Downloads all community kexts needed for Hackintosh builds.

Usage:
    python3 download_kexts.py [--output-dir DIR] [--dry-run]

Author: TETRATOSH Team
"""

import os
import sys
import json
import argparse
import hashlib
import subprocess
from pathlib import Path
from typing import Dict, List, Optional
from urllib.request import urlretrieve, urlopen
from urllib.error import URLError, HTTPError
import ssl
import re


# Kexts configuration with GitHub release URLs
# Format: "kext_name": {"url": "github_api_url", "pattern": "regex_to_match_filename"}

KEXTS_DATABASE = {
    # Essential kexts (always required)
    "Essential": {
        "Lilu": {
            "repo": "acidanthera/Lilu",
            "pattern": r"Lilu-v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Arbitrary kext and process patching"
        },
        "VirtualSMC": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"VirtualSMC-v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Advanced Apple SMC emulator"
        },
    },
    
    # GPU kexts
    "GPU": {
        "WhateverGreen": {
            "repo": "acidanthera/WhateverGreen",
            "pattern": r"WhateverGreen-v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "GPU patches for various GPUs"
        },
    },
    
    # Audio kexts
    "Audio": {
        "AppleALC": {
            "repo": "acidanthera/AppleALC",
            "pattern": r"AppleALC-v?(\d+\.\d+\.\d+)\.zip",
            "required": True,
            "description": "Native AppleALC audio patcher"
        },
    },
    
    # Network kexts - Intel
    "Network-Intel": {
        "IntelMausi": {
            "repo": "Mieze/IntelMausiEthernet",
            "pattern": r"IntelMausi-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel LAN driver for I219/I225"
        },
        "IntelGBMausi": {
            "repo": "Mieze/IntelMausiEthernet",
            "pattern": r"IntelGBMausi-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Gigabit Ethernet driver"
        },
    },
    
    # Network kexts - Realtek
    "Network-Realtek": {
        "RealtekRTL8111": {
            "repo": "Mieze/RTL8111_driver_for_Mac_OS",
            "pattern": r"RTL8111-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek 8111/8168 driver"
        },
        "RealtekRTL8125": {
            "repo": "Mieze/RTL8125_driver_for_Mac_OS",
            "pattern": r"RTL8125-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek 8125 2.5GbE driver"
        },
        "RealtekRTL8152": {
            "repo": "Mieze/RTL8152_driver_for_Mac_OS",
            "pattern": r"RTL8152-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Realtek RTL8152/RTL8153 driver"
        },
    },
    
    # Network kexts - Broadcom
    "Network-Broadcom": {
        "AppleBCM5701": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"AppleBCM5701-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom BCM57xx driver"
        },
        "BrcmPatchRAM": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BrcmPatchRAM-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom RAM patcher"
        },
        "BrcmFirmwareData": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BrcmFirmwareData-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom firmware data"
        },
    },
    
    # Wireless kexts - Intel
    "Wireless-Intel": {
        "AirportItlwm": {
            "repo": "OpenIntelWireless/itlwm",
            "pattern": r"AirportItlwm-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Wi-Fi driver"
        },
        "IntelBluetoothFirmware": {
            "repo": "OpenIntelWireless/IntelBluetoothFirmware",
            "pattern": r"IntelBluetoothFirmware-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Bluetooth driver"
        },
        "itlwm": {
            "repo": "OpenIntelWireless/itlwm",
            "pattern": r"itlwm-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Intel Wi-Fi driver (legacy)"
        },
    },
    
    # Wireless kexts - Broadcom
    "Wireless-Broadcom": {
        "AirportBrcmFixup": {
            "repo": "Acidanthera/AirportBrcmFixup",
            "pattern": r"AirportBrcmFixup-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Broadcom Wi-Fi patcher"
        },
    },
    
    # Wireless kexts - Other
    "Wireless-Other": {
        "IO80211Elan": {
            "repo": "Acidanthera/AirportBrcmFixup",
            "pattern": r"IO80211Elan-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "ELAN Wi-Fi support"
        },
    },
    
    # Bluetooth kexts
    "Bluetooth": {
        "BlueToolFixup": {
            "repo": "Acidanthera/BrcmPatchRAM",
            "pattern": r"BlueToolFixup-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Bluetooth fixup"
        },
    },
    
    # Storage kexts
    "Storage": {
        "NVMeFix": {
            "repo": "Acidanthera/NVMeFix",
            "pattern": r"NVMeFix-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "NVMe drive fix"
        },
        "AHCIPortInjector": {
            "repo": "Acidanthera/AHCI_3rdParty_EFI",
            "pattern": r"AHCIPortInjector-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "AHCI port injector"
        },
    },
    
    # USB kexts
    "USB": {
        "USBInjectAll": {
            "repo": "RehabMan/OS-X-USB-Inject-All",
            "pattern": r"USBInjectAll-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "USB inject all"
        },
        "XHCI-unsupported": {
            "repo": "RehabMan/OS-X-USB-Inject-All",
            "pattern": r"XHCI-unsupported-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Unsupported XHCI controller"
        },
    },
    
    # Input kexts
    "Input": {
        "VoodooPS2Controller": {
            "repo": "RehabMan/OS-X-Voodoo-PS2-Controller",
            "pattern": r"VoodooPS2-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "PS/2 keyboard and mouse"
        },
        "VoodooI2C": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2C-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "I2C trackpad and touchscreen"
        },
        "VoodooI2CHID": {
            "repo": "VoodooI2C/VoodooI2C",
            "pattern": r"VoodooI2CHID-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "I2C HID driver"
        },
    },
    
    # SMC kexts
    "SMC": {
        "SMCProcessor": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCProcessor-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "SMC processor plugin"
        },
        "SMCSuperIO": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCSuperIO-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "SMC super I/O plugin"
        },
    },
    
    # Other kexts
    "Other": {
        "HibernationFixup": {
            "repo": "acidanthera/HibernationFixup",
            "pattern": r"HibernationFixup-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Hibernation fix"
        },
        "CPUFriend": {
            "repo": "acidanthera/CPUFriend",
            "pattern": r"CPUFriend-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "CPU power management"
        },
        "CPUFriendDataProvider": {
            "repo": "acidanthera/CPUFriendDataProvider",
            "pattern": r"CPUFriendDataProvider-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "CPU friend data provider"
        },
        "FeatureUnlock": {
            "repo": "acidanthera/FeatureUnlock",
            "pattern": r"FeatureUnlock-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Unlock macOS features"
        },
        "RTCMemoryFixup": {
            "repo": "acidanthera/RTCMemoryFixup",
            "pattern": r"RTCMemoryFixup-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "RTC memory fix"
        },
        "SMCDellSensors": {
            "repo": "acidanthera/VirtualSMC",
            "pattern": r"SMCDellSensors-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Dell SMC sensors"
        },
        "NullCPUPowerManagement": {
            "repo": "RehabMan/OS-X-NULL-CPU-Power-Management",
            "pattern": r"NullCPUPowerManagement-v?(\d+\.\d+\.\d+)\.zip",
            "required": False,
            "description": "Null CPU power management"
        },
    }
}


class KextDownloader:
    """Downloads and manages Hackintosh kexts."""
    
    def __init__(self, output_dir: str = "kexts", dry_run: bool = False, verbose: bool = False):
        self.output_dir = Path(output_dir)
        self.dry_run = dry_run
        self.verbose = verbose
        self.ctx = ssl.create_default_context()
        self.ctx.check_hostname = False
        self.ctx.verify_mode = ssl.CERT_NONE
        
        # Create output directories
        self._create_directories()
        
    def _create_directories(self):
        """Create directory structure for kexts."""
        categories = [
            "essential",
            "gpu",
            "audio", 
            "network/intel",
            "network/realtek",
            "network/broadcom",
            "wireless/intel",
            "wireless/broadcom",
            "wireless/other",
            "bluetooth",
            "storage",
            "usb",
            "input",
            "smc",
            "other"
        ]
        
        for category in categories:
            (self.output_dir / category).mkdir(parents=True, exist_ok=True)
            
        # Also create community kexts directory
        (self.output_dir / "community").mkdir(parents=True, exist_ok=True)
        
    def log(self, message: str, level: str = "INFO"):
        """Log a message."""
        if self.verbose or level in ["INFO", "ERROR", "WARN"]:
            prefix = {
                "INFO": "[KEXT]",
                "WARN": "[KEXT WARN]",
                "ERROR": "[KEXT ERROR]",
                "SUCCESS": "[KEXT OK]"
            }
            print(f"{prefix.get(level, '[KEXT]')} {message}")
    
    def get_latest_release(self, repo: str) -> Optional[Dict]:
        """Get the latest release info from a GitHub repository."""
        url = f"https://api.github.com/repos/{repo}/releases/latest"
        
        try:
            request = urlopen(url, context=self.ctx)
            data = json.loads(request.read().decode('utf-8'))
            return data
        except (URLError, HTTPError, json.JSONDecodeError) as e:
            self.log(f"Failed to fetch release from {repo}: {e}", "ERROR")
            return None
    
    def find_asset_url(self, release_data: Dict, pattern: str) -> Optional[str]:
        """Find the asset URL matching the pattern."""
        if not release_data or "assets" not in release_data:
            return None
            
        regex = re.compile(pattern)
        
        for asset in release_data["assets"]:
            name = asset["name"]
            if regex.match(name):
                self.log(f"Found matching asset: {name}", "INFO")
                return asset["browser_download_url"]
                
        return None
    
    def download_file(self, url: str, dest: Path) -> bool:
        """Download a file from URL to destination."""
        if self.dry_run:
            self.log(f"[DRY RUN] Would download: {url} -> {dest}", "INFO")
            return True
            
        try:
            self.log(f"Downloading: {url}", "INFO")
            urlretrieve(url, dest)
            return True
        except Exception as e:
            self.log(f"Failed to download {url}: {e}", "ERROR")
            return False
    
    def download_kext(self, kext_name: str, kext_info: Dict, category: str) -> bool:
        """Download a single kext."""
        repo = kext_info.get("repo")
        pattern = kext_info.get("pattern")
        
        if not repo or not pattern:
            self.log(f"Invalid kext config for {kext_name}", "ERROR")
            return False
            
        # Get latest release
        release = self.get_latest_release(repo)
        if not release:
            return False
            
        # Find asset
        asset_url = self.find_asset_url(release, pattern)
        if not asset_url:
            self.log(f"No matching asset found for {kext_name}", "ERROR")
            return False
            
        # Determine output path
        # Extract version from filename
        version_match = re.search(pattern.replace("(", "(?:").replace(")", ")?"), 
                                  asset_url.split("/")[-1])
        if version_match:
            filename = f"{kext_name}.zip"
        else:
            filename = f"{kext_name}.zip"
            
        output_path = self.output_dir / category / filename
        
        # Download
        return self.download_file(asset_url, output_path)
    
    def download_all(self, categories: Optional[List[str]] = None) -> Dict[str, bool]:
        """Download all kexts."""
        results = {}
        
        # If no categories specified, download all
        if categories is None:
            categories = list(KEXTS_DATABASE.keys())
        
        for category in categories:
            if category not in KEXTS_DATABASE:
                self.log(f"Unknown category: {category}", "WARN")
                continue
                
            self.log(f"\n=== Downloading {category} kexts ===", "INFO")
            
            for kext_name, kext_info in KEXTS_DATABASE[category].items():
                self.log(f"Processing: {kext_name}", "INFO")
                
                success = self.download_kext(kext_name, kext_info, category.lower())
                results[kext_name] = success
                
                if success:
                    self.log(f"Downloaded: {kext_name}", "SUCCESS")
                else:
                    self.log(f"Failed: {kext_name}", "ERROR")
                    
        return results
    
    def list_kexts(self):
        """List all available kexts."""
        print("\n=== Available Kexts ===\n")
        
        for category, kexts in KEXTS_DATABASE.items():
            print(f"\n{category}:")
            for kext_name, kext_info in kexts.items():
                required = " [REQUIRED]" if kext_info.get("required", False) else ""
                print(f"  - {kext_name}{required}")
                print(f"    {kext_info.get('description', '')}")
                print(f"    Repo: {kext_info.get('repo', 'N/A')}")


def main():
    parser = argparse.ArgumentParser(
        description="TETRATOSH Kext Downloader - Download Hackintosh kexts"
    )
    parser.add_argument(
        "-o", "--output-dir",
        default="kexts",
        help="Output directory for kexts (default: kexts)"
    )
    parser.add_argument(
        "-n", "--dry-run",
        action="store_true",
        help="Show what would be downloaded without downloading"
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Verbose output"
    )
    parser.add_argument(
        "-l", "--list",
        action="store_true",
        help="List available kexts and exit"
    )
    parser.add_argument(
        "-c", "--categories",
        nargs="+",
        help="Specific categories to download"
    )
    
    args = parser.parse_args()
    
    downloader = KextDownloader(
        output_dir=args.output_dir,
        dry_run=args.dry_run,
        verbose=args.verbose
    )
    
    if args.list:
        downloader.list_kexts()
        return 0
    
    # Download kexts
    results = downloader.download_all(args.categories)
    
    # Summary
    success = sum(1 for v in results.values() if v)
    total = len(results)
    
    print(f"\n=== Download Summary ===")
    print(f"Total: {total}")
    print(f"Success: {success}")
    print(f"Failed: {total - success}")
    
    return 0 if success == total else 1


if __name__ == "__main__":
    sys.exit(main())
