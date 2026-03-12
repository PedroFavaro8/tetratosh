#!/usr/bin/env python3

import json
import argparse
import sys
import ssl
import re
import time
import os
from pathlib import Path
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError

GITHUB_API = "https://api.github.com/repos/{repo}/releases/latest"


KEXTS_DB = {

"Essential": {

"Lilu": {
"repo": "acidanthera/Lilu",
"pattern": r"Lilu.*\.zip",
"required": True
},

"VirtualSMC": {
"repo": "acidanthera/VirtualSMC",
"pattern": r"VirtualSMC.*\.zip",
"required": True
}

},

"GPU": {

"WhateverGreen": {
"repo": "acidanthera/WhateverGreen",
"pattern": r"WhateverGreen.*\.zip",
"required": True
}

},

"Audio": {

"AppleALC": {
"repo": "acidanthera/AppleALC",
"pattern": r"AppleALC.*\.zip",
"required": True
}

},

"Network": {

"IntelMausi": {
"repo": "Mieze/IntelMausiEthernet",
"pattern": r"IntelMausi.*\.zip"
},

"RealtekRTL8111": {
"repo": "Mieze/RTL8111_driver_for_OS_X",
"pattern": r"RealtekRTL8111.*\.zip"
}

},

"Wireless": {

"AirportItlwm": {
"repo": "OpenIntelWireless/itlwm",
"pattern": r"AirportItlwm.*\.kext\.zip"
},

"IntelBluetoothFirmware": {
"repo": "OpenIntelWireless/IntelBluetoothFirmware",
"pattern": r"IntelBluetooth.*\.zip"
}

},

"Bluetooth": {

"BrcmPatchRAM": {
"repo": "acidanthera/BrcmPatchRAM",
"pattern": r"BrcmPatchRAM.*\.zip"
}

},

"Storage": {

"NVMeFix": {
"repo": "acidanthera/NVMeFix",
"pattern": r"NVMeFix.*\.zip"
}

},

"USB": {

"USBInjectAll": {
"repo": "Sniki/OS-X-USB-Inject-All",
"pattern": r"USBInjectAll.*\.zip"
}

},

"Input": {

"VoodooPS2Controller": {
"repo": "acidanthera/VoodooPS2",
"pattern": r"VoodooPS2Controller.*\.zip"
},

"VoodooI2C": {
"repo": "VoodooI2C/VoodooI2C",
"pattern": r"VoodooI2C.*\.zip"
}

},

"Utility": {

"CPUFriend": {
"repo": "acidanthera/CPUFriend",
"pattern": r"CPUFriend.*\.zip"
},

"FeatureUnlock": {
"repo": "acidanthera/FeatureUnlock",
"pattern": r"FeatureUnlock.*\.zip"
}

}

}


class KextDownloader:

    def __init__(self, output_dir="kexts", dry_run=False, token=None):

        self.output_dir = Path(output_dir)
        self.dry_run = dry_run
        self.ctx = ssl.create_default_context()
        self.token = token or os.environ.get("GITHUB_TOKEN")

        self.downloaded = []
        self.failed = []

    def log(self, msg):

        print("[TETRATOSH]", msg)

    def request(self, url):

        req = Request(url, headers={"User-Agent": "Tetratosh"})
        if self.token:
            req.add_header("Authorization", f"token {self.token}")
        return urlopen(req, context=self.ctx, timeout=30)

    def get_release(self, repo):

        url = GITHUB_API.format(repo=repo)

        try:

            r = self.request(url)
            return json.loads(r.read().decode())

        except Exception as e:

            self.log(f"failed to fetch {repo} : {e}")
            return None

    def find_asset(self, release, pattern):

        if not release:
            return None

        regex = re.compile(pattern, re.IGNORECASE)

        for asset in release.get("assets", []):

            name = asset.get("name")

            if regex.search(name):
                return asset.get("browser_download_url")

        return None

    def download(self, url, dest):

        if self.dry_run:

            self.log(f"DRY {url}")
            return True

        try:

            with self.request(url) as r:
                with open(dest, "wb") as f:
                    f.write(r.read())

            return True

        except Exception as e:

            self.log(f"download failed {dest} : {e}")
            return False

    def download_kext(self, name, info, category):

        repo = info["repo"]
        pattern = info["pattern"]

        release = self.get_release(repo)

        url = self.find_asset(release, pattern)

        if not url:

            self.failed.append(name)
            return

        path = self.output_dir / category
        path.mkdir(parents=True, exist_ok=True)

        dest = path / f"{name}.zip"

        if self.download(url, dest):

            self.downloaded.append(name)
            self.log(f"OK {name}")

        else:

            self.failed.append(name)

    def run(self):

        total = 0

        for category, kexts in KEXTS_DB.items():

            self.log(f"=== {category} ===")

            for name, info in kexts.items():

                total += 1
                self.download_kext(name, info, category.lower())

                time.sleep(0.5)

        print("\nSummary")
        print("Downloaded:", len(self.downloaded))
        print("Failed:", len(self.failed))
        print("Total:", total)


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-o",
        "--output-dir",
        default="kexts"
    )

    parser.add_argument(
        "--dry-run",
        action="store_true"
    )

    parser.add_argument(
        "-t",
        "--token",
        default=os.environ.get("GITHUB_TOKEN"),
        help="GitHub token for higher rate limits"
    )

    args = parser.parse_args()

    d = KextDownloader(
        output_dir=args.output_dir,
        dry_run=args.dry_run,
        token=args.token
    )

    d.run()


if __name__ == "__main__":

    main()