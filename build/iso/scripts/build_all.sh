#!/bin/bash
#
# TETRATOSH - Master Build Script
# Downloads ALL required files for offline boot
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
echo "================================================"
echo "  TETRATOSH - Complete Offline Build System"
echo "================================================"
echo -e "${NC}"

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Step 1: Download all kexts
log_info "Step 1/4: Downloading community kexts..."
python3 "$SCRIPT_DIR/download_all_kexts.py" -v || log_warn "Some kexts may have failed"

# Step 2: Download ACPI files
log_info "Step 2/4: Downloading ACPI files..."
python3 "$SCRIPT_DIR/download_acpi.py" -v || log_warn "Some ACPI files may have failed"

# Step 3: Create structure
log_info "Step 3/4: Creating ISO structure..."

# Create output directories
mkdir -p "$PROJECT_ROOT/output/EFI/BOOT"
mkdir -p "$PROJECT_ROOT/output/EFI/OC/ACPI"
mkdir -p "$PROJECT_ROOT/output/EFI/OC/Drivers"
mkdir -p "$PROJECT_ROOT/output/EFI/OC/Kexts"
mkdir -p "$PROJECT_ROOT/output/EFI/OC/Resources"
mkdir -p "$PROJECT_ROOT/output/ACPI/Intel"
mkdir -p "$PROJECT_ROOT/output/ACPI/AMD"
mkdir -p "$PROJECT_ROOT/output/DATABASE"

# Copy device database
if [ -f "$PROJECT_ROOT/device_database/devices.json" ]; then
    cp "$PROJECT_ROOT/device_database/devices.json" "$PROJECT_ROOT/output/DATABASE/"
fi

if [ -f "$PROJECT_ROOT/device_database/devices_v2.json" ]; then
    cp "$PROJECT_ROOT/device_database/devices_v2.json" "$PROJECT_ROOT/output/DATABASE/"
fi

# Copy ACPI database
if [ -f "$PROJECT_ROOT/acpi_database/acpi_patches.json" ]; then
    cp "$PROJECT_ROOT/acpi_database/acpi_patches.json" "$PROJECT_ROOT/output/DATABASE/"
fi

# Step 4: List downloaded files
log_info "Step 4/4: Summary..."

echo ""
echo -e "${GREEN}=== Build Complete! ===${NC}"
echo ""
echo "Downloaded files:"
echo "  - Kexts: $PROJECT_ROOT/kexts/"
echo "  - ACPI: $PROJECT_ROOT/acpi/"
echo ""
echo "Output structure:"
echo "  - $PROJECT_ROOT/output/"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "  1. Download OpenCore files to output/EFI/OC/"
echo "  2. Run: ./scripts/build_iso.sh"
echo ""
