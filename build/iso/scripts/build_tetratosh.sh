#!/bin/bash
#
# TETRATOSH Build Script
# Compila o bootloader com EDK2
#

set -e

echo "=========================================="
echo "TETRATOSH Build System"
echo "=========================================="
echo ""

# Verificar EDK2
if [ ! -d "/tmp/edk2" ]; then
    echo "ERRO: EDK2 não encontrado em /tmp/edk2"
    echo "Execute primeiro: ./scripts/setup_edk2.sh"
    exit 1
fi

cd /tmp/edk2

# Setup environment
export WORKSPACE=/tmp/edk2
export GCC_BIN=$(which gcc)
source ./edksetup.sh BaseTools

echo "Compilando Tetratosh..."
echo ""

# Build
./BaseTools/BinWrappers/PosixLike/build \
    -p /workspaces/tetratosh/bootloader/Tetratosh.dsc \
    -a X64 \
    -b DEBUG \
    -t GCC \
    -n 3

echo ""
echo "=========================================="
echo "Build concluído!"
echo "Resultado em: Build/Tetratosh/"
echo "=========================================="
