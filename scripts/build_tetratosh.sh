#!/bin/bash
#
# TETRATOSH Build Script
# Compila o bootloader com Makefile (standalone)
#

set -e

echo "=========================================="
echo "TETRATOSH Build System"
echo "=========================================="
echo ""

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BOOTLOADER_DIR="$PROJECT_ROOT/bootloader"
BUILD_DIR="$PROJECT_ROOT/build"

echo "Building with standalone Makefile..."
cd "$BOOTLOADER_DIR"
make clean
make
mkdir -p "$BUILD_DIR/bootloader"
cp build/Tetratosh.efi "$BUILD_DIR/bootloader/"

echo ""
echo "=========================================="
echo "Build concluído!"
echo "Resultado em: $BUILD_DIR/bootloader/Tetratosh.efi"
echo "=========================================="
