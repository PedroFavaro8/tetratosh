#!/usr/bin/env python3
"""
TETRATOSH - ACPI Downloader
Downloads all ACPI files (SSDTs) from Acidanthera and other sources
"""

import os
import sys
import json
import subprocess
from pathlib import Path
from urllib.request import urlretrieve, urlopen
from urllib.error import URLError, HTTPError
from typing import Optional
import ssl
import re
import time


ACPI_REPOSITORIES = {
    "acidanthera": {
        "base_url": "https://github.com/acidanthera/{repo}/releases/download/{tag}/{file}",
        "releases": {
            "AcpiMath": {
                "repo": "AcpiMath",
                "files": ["AcpiMath.kext.zip"]
            },
            "AirportBrcmFixup": {
                "repo": "AirportBrcmFixup",
                "files": ["AirportBrcmFixup-v*.zip"]
            },
            "AppleALC": {
                "repo": "AppleALC",
                "files": ["AppleALC-v*.zip"]
            },
            "CPUFriend": {
                "repo": "CPUFriend",
                "files": ["CPUFriend-v*.zip"]
            },
            "FeatureUnlock": {
                "repo": "FeatureUnlock",
                "files": ["FeatureUnlock-v*.zip"]
            },
            "HibernationFixup": {
                "repo": "HibernationFixup",
                "files": ["HibernationFixup-v*.zip"]
            },
            "Lilu": {
                "repo": "Lilu",
                "files": ["Lilu-v*.zip"]
            },
            "NVMeFix": {
                "repo": "NVMeFix",
                "files": ["NVMeFix-v*.zip"]
            },
            "RTCMemoryFixup": {
                "repo": "RTCMemoryFixup",
                "files": ["RTCMemoryFixup-v*.zip"]
            },
            "VirtualSMC": {
                "repo": "VirtualSMC",
                "files": ["VirtualSMC-v*.zip"]
            },
            "WhateverGreen": {
                "repo": "WhateverGreen",
                "files": ["WhateverGreen-v*.zip"]
            }
        }
    }
}

SSDT_TEMPLATES = {
    "Intel": {
        "SSDT-PLUG": {
            "description": "CPU Power Management Plugin",
            "cpu_generation": "all"
        },
        "SSDT-EC": {
            "description": "Embedded Controller",
            "cpu_generation": "all"
        },
        "SSDT-EC-USBX": {
            "description": "EC + USB Power",
            "cpu_generation": "Skylake+"
        },
        "SSDT-PMC": {
            "description": "PMC Fix",
            "cpu_generation": "Skylake+"
        },
        "SSDT-AWAC": {
            "description": "RTC Fix",
            "cpu_generation": "300 series+"
        },
        "SSDT-XOSI": {
            "description": "OSI Workaround",
            "cpu_generation": "AMD"
        },
        "SSDT-USB-Reset": {
            "description": "USB Reset",
            "cpu_generation": "all"
        },
        "SSDT-DGPU-OFF": {
            "description": "Disable Discrete GPU",
            "cpu_generation": "laptops"
        },
        "SSDT-PNLF": {
            "description": "Panel Brightness",
            "cpu_generation": "all"
        },
        "SSDT-XCPM": {
            "description": "XCPM Mode",
            "cpu_generation": "all"
        }
    },
    "AMD": {
        "SSDT-AGESA": {
            "description": "AMD AGESA Fix",
            "cpu_family": "Zen"
        },
        "SSDT-AMD-CPU-PM": {
            "description": "AMD CPU Power Management",
            "cpu_family": "all"
        },
        "SSDT-AMD-RX": {
            "description": "AMD Radeon RX Fix",
            "gpu_family": "Polaris+"
        },
        "SSDT-XOSI": {
            "description": "Windows OS Identification",
            "cpu_family": "all"
        }
    }
}


class ACPIDownloader:
    def __init__(self, output_dir: str = "acpi", dry_run: bool = False, verbose: bool = False):
        self.output_dir = Path(output_dir)
        self.dry_run = dry_run
        self.verbose = verbose
        self.ctx = ssl.create_default_context()
        self.ctx.check_hostname = False
        self.ctx.verify_mode = ssl.CERT_NONE
        self.downloaded = []
        
    def log(self, message: str, level: str = "INFO"):
        prefix = {"INFO": "[ACPI]", "OK": "[ACPI OK]", "ERROR": "[ACPI ERROR]"}
        print(f"{prefix.get(level, '[ACPI]')} {message}")
    
    def create_directories(self):
        """Create ACPI directory structure."""
        dirs = [
            "intel/ssdt",
            "intel/patches",
            "amd/ssdt",
            "amd/patches",
            "templates",
            "precompiled"
        ]
        for d in dirs:
            (self.output_dir / d).mkdir(parents=True, exist_ok=True)
    
    def get_latest_tag(self, repo: str) -> Optional[str]:
        """Get latest release tag from GitHub."""
        url = f"https://api.github.com/repos/{repo}/releases/latest"
        try:
            request = urlopen(url, context=self.ctx, timeout=30)
            data = json.loads(request.read().decode('utf-8'))
            return data.get('tag_name', '')
        except Exception as e:
            self.log(f"Failed to get tag for {repo}: {e}", "ERROR")
            return None
    
    def find_asset(self, repo: str, pattern: str) -> Optional[str]:
        """Find matching asset in latest release."""
        url = f"https://api.github.com/repos/{repo}/releases/latest"
        try:
            request = urlopen(url, context=self.ctx, timeout=30)
            data = json.loads(request.read().decode('utf-8'))
            
            regex = re.compile(pattern.replace('*', '.*'), re.IGNORECASE)
            
            for asset in data.get('assets', []):
                name = asset['name']
                if regex.match(name):
                    return asset['browser_download_url']
        except Exception as e:
            self.log(f"Failed to find asset: {e}", "ERROR")
        return None
    
    def download_file(self, url: str, dest: Path) -> bool:
        """Download a file."""
        if self.dry_run:
            self.log(f"[DRY] Would download: {url} -> {dest}")
            return True
        try:
            self.log(f"Downloading: {dest.name}")
            urlretrieve(url, dest)
            return True
        except Exception as e:
            self.log(f"Failed: {dest.name} - {e}", "ERROR")
            return False
    
    def download_acpi_kexts(self):
        """Download ACPI-related kexts."""
        self.log("Downloading ACPI kexts...")
        
        repos = [
            ("acidanthera/Lilu", "Lilu-v*.zip"),
            ("acidanthera/WhateverGreen", "WhateverGreen-v*.zip"),
            ("acidanthera/VirtualSMC", "VirtualSMC-v*.zip"),
            ("acidanthera/AppleALC", "AppleALC-v*.zip"),
            ("acidanthera/CPUFriend", "CPUFriend-v*.zip"),
            ("acidanthera/NVMeFix", "NVMeFix-v*.zip"),
            ("acidanthera/RTCMemoryFixup", "RTCMemoryFixup-v*.zip"),
            ("acidanthera/HibernationFixup", "HibernationFixup-v*.zip"),
            ("acidanthera/FeatureUnlock", "FeatureUnlock-v*.zip"),
            ("acidanthera/AirportBrcmFixup", "AirportBrcmFixup-v*.zip"),
        ]
        
        for repo, pattern in repos:
            url = self.find_asset(repo, pattern)
            if url:
                filename = url.split('/')[-1]
                dest = self.output_dir / "precompiled" / filename
                if self.download_file(url, dest):
                    self.downloaded.append(filename)
    
    def generate_ssdt_configs(self):
        """Generate SSDT configuration files."""
        self.log("Generating SSDT configurations...")
        
        config = {
            "ssdt_templates": SSDT_TEMPLATES,
            "cpu_compatibility": {
                "Intel": {
                    "Nehalem": {"year": 2008, "ssdt": ["SSDT-PLUG", "SSDT-EC"]},
                    "Westmere": {"year": 2010, "ssdt": ["SSDT-PLUG", "SSDT-EC"]},
                    "SandyBridge": {"year": 2011, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-USB-Reset"]},
                    "IvyBridge": {"year": 2012, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-USB-Reset"]},
                    "Haswell": {"year": 2013, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX"]},
                    "Broadwell": {"year": 2014, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-AWAC"]},
                    "Skylake": {"year": 2015, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC"]},
                    "KabyLake": {"year": 2016, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC"]},
                    "CoffeeLake": {"year": 2017, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC", "SSDT-AWAC"]},
                    "CometLake": {"year": 2019, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC", "SSDT-AWAC"]},
                    "IceLake": {"year": 2019, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC"]},
                    "RocketLake": {"year": 2020, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC", "SSDT-AWAC"]},
                    "AlderLake": {"year": 2021, "ssdt": ["SSDT-PLUG", "SSDT-EC", "SSDT-EC-USBX", "SSDT-PMC", "SSDT-AWAC"]}
                },
                "AMD": {
                    "Bulldozer": {"year": 2011, "ssdt": ["SSDT-PLUG", "SSDT-XOSI"]},
                    "Piledriver": {"year": 2012, "ssdt": ["SSDT-PLUG", "SSDT-XOSI"]},
                    "Excavator": {"year": 2014, "ssdt": ["SSDT-PLUG", "SSDT-XOSI"]},
                    "Zen": {"year": 2017, "ssdt": ["SSDT-PLUG", "SSDT-XOSI", "SSDT-AGESA"]},
                    "Zen+": {"year": 2018, "ssdt": ["SSDT-PLUG", "SSDT-XOSI", "SSDT-AGESA"]},
                    "Zen2": {"year": 2019, "ssdt": ["SSDT-PLUG", "SSDT-XOSI", "SSDT-AGESA"]},
                    "Zen3": {"year": 2020, "ssdt": ["SSDT-PLUG", "SSDT-XOSI", "SSDT-AGESA"]},
                    "Zen4": {"year": 2022, "ssdt": ["SSDT-PLUG", "SSDT-XOSI", "SSDT-AGESA"]}
                }
            }
        }
        
        config_path = self.output_dir / "ssdt_config.json"
        with open(config_path, 'w') as f:
            json.dump(config, f, indent=2)
        
        self.log(f"Generated: {config_path}")
    
    def download_all(self):
        """Download all ACPI files."""
        self.create_directories()
        self.download_acpi_kexts()
        self.generate_ssdt_configs()
        
        print(f"\n=== ACPI Download Complete ===")
        print(f"Files downloaded: {len(self.downloaded)}")


def main():
    import argparse
    parser = argparse.ArgumentParser(description="TETRATOSH ACPI Downloader")
    parser.add_argument("-o", "--output-dir", default="acpi", help="Output directory")
    parser.add_argument("-n", "--dry-run", action="store_true", help="Dry run")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose")
    
    args = parser.parse_args()
    
    downloader = ACPIDownloader(
        output_dir=args.output_dir,
        dry_run=args.dry_run,
        verbose=args.verbose
    )
    
    downloader.download_all()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
