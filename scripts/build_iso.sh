#!/bin/bash
#
# TETRATOSH ISO Builder
# Creates a bootable ISO with Tetratosh bootloader
#

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
KEKTS_DIR="$PROJECT_ROOT/kexts"
OPENCORE_DIR="$PROJECT_ROOT/OpenCore"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_dependencies() {
    log_info "Checking dependencies..."
    
    local missing=()
    
    command -v xorriso >/dev/null 2>&1 || missing+=("xorriso")
    command -v mtools >/dev/null 2>&1 || missing+=("mtools")
    command -v grub-mkimage >/dev/null 2>&1 || missing+=("grub-common")
    
    if [ ${#missing[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing[*]}"
        echo "Install with: apt-get install ${missing[*]}"
        exit 1
    fi
    
    log_info "All dependencies found"
}

# Create directory structure
create_structure() {
    log_info "Creating ISO structure..."
    
    mkdir -p "$BUILD_DIR/iso/EFI/BOOT"
    mkdir -p "$BUILD_DIR/iso/EFI/OC/ACPI"
    mkdir -p "$BUILD_DIR/iso/EFI/OC/Drivers"
    mkdir -p "$BUILD_DIR/iso/EFI/OC/Kexts"
    mkdir -p "$BUILD_DIR/iso/EFI/OC/Resources"
    mkdir -p "$BUILD_DIR/iso/EFI/OC/Tools"
    mkdir -p "$BUILD_DIR/iso/KEKSTS/REQUIRED"
    mkdir -p "$BUILD_DIR/iso/KEKSTS/COMMUNITY"
    mkdir -p "$BUILD_DIR/iso/TOOLS"
    mkdir -p "$BUILD_DIR/iso/DATABASE"
}

# Copy bootloader
copy_bootloader() {
    log_info "Copying Tetratosh bootloader..."
    
    if [ -f "$BUILD_DIR/bootloader/tetratosh.efi" ]; then
        cp "$BUILD_DIR/bootloader/tetratosh.efi" "$BUILD_DIR/iso/EFI/BOOT/BOOTx64.efi"
    else
        log_warn "Bootloader not found, using placeholder"
        # Create a minimal EFI for testing
        # In production, this should be the actual Tetratosh bootloader
    fi
}

# Copy OpenCore
copy_opencore() {
    log_info "Setting up OpenCore..."
    
    if [ -d "$OPENCORE_DIR" ]; then
        # Copy OpenCore files
        cp -r "$OPENCORE_DIR/"* "$BUILD_DIR/iso/EFI/OC/" 2>/dev/null || true
        log_info "OpenCore copied"
    else
        log_warn "OpenCore directory not found"
        log_warn "Please place OpenCore files in $OPENCORE_DIR"
    fi
}

# Copy kexts
copy_kexts() {
    log_info "Copying kexts..."
    
    if [ -d "$KEKTS_DIR" ]; then
        # Copy all kexts
        find "$KEKTS_DIR" -name "*.kext" -type d -exec cp -r {} "$BUILD_DIR/iso/EFI/OC/Kexts/" \; 2>/dev/null || true
        log_info "Kexts copied"
    else
        log_warn "Kexts directory not found"
    fi
}

# Copy database
copy_database() {
    log_info "Copying device database..."
    
    if [ -d "$PROJECT_ROOT/device_database" ]; then
        cp -r "$PROJECT_ROOT/device_database/"* "$BUILD_DIR/iso/DATABASE/"
    fi
}

# Create EFI partition image
create_efi_image() {
    log_info "Creating EFI partition image..."
    
    # Create a FAT32 image for EFI partition
    dd if=/dev/zero of="$BUILD_DIR/efi.img" bs=1M count=16
    mkfs.fat -F32 "$BUILD_DIR/efi.img"
    
    # Mount and copy files
    mkdir -p "$BUILD_DIR/efi_mount"
    mount -o loop "$BUILD_DIR/efi.img" "$BUILD_DIR/efi_mount"
    cp -r "$BUILD_DIR/iso/"* "$BUILD_DIR/efi_mount/"
    umount "$BUILD_DIR/efi_mount"
    rmdir "$BUILD_DIR/efi_mount"
}

# Create bootable ISO
create_iso() {
    log_info "Creating bootable ISO..."
    
    local iso_path="$DIST_DIR/tetratosh.iso"
    
    mkdir -p "$DIST_DIR"
    
    # Create ISO with EFI partition
    xorriso \
        -as mkisofs \
        -no-emul-boot \
        -boot-load-size 4 \
        -append_partition 2 0xEF "$BUILD_DIR/efi.img" \
        -output "$iso_path" \
        "$BUILD_DIR/iso/" \
        -graft-points \
        /EFI/="$BUILD_DIR/iso/EFI"
    
    log_info "ISO created: $iso_path"
}

# Main function
main() {
    log_info "Starting TETRATOSH ISO build..."
    
    check_dependencies
    create_structure
    copy_bootloader
    copy_opencore
    copy_kexts
    copy_database
    create_efi_image
    create_iso
    
    log_info "Build complete!"
    log_info "ISO location: $DIST_DIR/tetratosh.iso"
}

# Run main
main "$@"
