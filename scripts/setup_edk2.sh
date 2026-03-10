#!/bin/bash
#
# TETRATOSH - Setup EDK2
# Baixa e configura o EDK2 para compilação
#

set -e

echo "=========================================="
echo "TETRATOSH - Setup EDK2"
echo "=========================================="
echo ""

EDK2_DIR="/tmp/edk2"

# Verificar se já existe
if [ -d "$EDK2_DIR" ]; then
    echo "EDK2 já existe em $EDK2_DIR"
    read -p "Deseja atualizar? (s/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Ss]$ ]]; then
        echo "Usando EDK2 existente..."
    else
        echo "Atualizando EDK2..."
        cd $EDK2_DIR
        git pull
        git submodule update --init --recursive
    fi
else
    echo "Clonando EDK2..."
    cd /tmp
    git clone https://github.com/tianocore/edk2.git --depth 1
    
    echo "Inicializando submodulos..."
    cd $EDK2_DIR
    git submodule update --init --recursive
fi

echo ""
echo "Instalando dependências Python..."
pip3 install -r $EDK2_DIR/pip-requirements.txt || true

echo ""
echo "Compilando BaseTools..."
cd $EDK2_DIR
source ./edksetup.sh BaseTools
make -C BaseTools

echo ""
echo "=========================================="
echo "Setup concluído!"
echo "EDK2 em: $EDK2_DIR"
echo ""
echo "Para compilar o Tetratosh:"
echo "  ./scripts/build_tetratosh.sh"
echo "=========================================="
