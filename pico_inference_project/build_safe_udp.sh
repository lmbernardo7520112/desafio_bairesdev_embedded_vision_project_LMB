#!/bin/bash
# ============================================================
# build_safe.sh - Script de build SEGURO para Raspberry Pi Pico W
# Versão: UDP (sem I2C/SSD1306)
# ============================================================

PROJECT_NAME="pico_inference_project"
BUILD_DIR="build"

# --- Função para mensagens coloridas ---
green() { echo -e "\033[1;32m$1\033[0m"; }
red()   { echo -e "\033[1;31m$1\033[0m"; }
blue()  { echo -e "\033[1;34m$1\033[0m"; }
yellow(){ echo -e "\033[1;33m$1\033[0m"; }

echo ""
blue "🚀 Iniciando build do projeto $PROJECT_NAME..."

# --- Checagem do PICO_SDK_PATH ---
if [ -z "$PICO_SDK_PATH" ]; then
    red "❌ ERRO: PICO_SDK_PATH não está definido!"
    echo "Defina com:"
    echo "    export PICO_SDK_PATH=/caminho/para/pico-sdk"
    exit 1
fi
green "✅ PICO_SDK_PATH: $PICO_SDK_PATH"

# --- Checagem do Toolchain ---
TOOLCHAIN=$(which arm-none-eabi-gcc 2>/dev/null)
if [ -z "$TOOLCHAIN" ]; then
    red "❌ ERRO: Toolchain arm-none-eabi-gcc não encontrado!"
    echo "Instale com:"
    echo "    sudo apt install gcc-arm-none-eabi cmake build-essential"
    exit 1
fi
green "✅ Toolchain: $TOOLCHAIN"

# --- Limpeza do diretório de build ---
if [ -d "$BUILD_DIR" ]; then
    yellow "🧹 Limpando diretório de build antigo..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# --- Rodando CMake ---
blue "🔧 Gerando arquivos com CMake..."
cmake .. -DPICO_BOARD=pico_w
if [ $? -ne 0 ]; then
    red "❌ Erro no CMake. Abortando."
    exit 1
fi
green "✅ CMake concluído com sucesso."

# --- Compilando ---
blue "⚙️ Compilando projeto..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    red "❌ Erro na compilação. Abortando."
    exit 1
fi
green "✅ Compilação concluída com sucesso."

# --- Verificando UF2 ---
UF2_FILE=$(find . -name "*.uf2" | head -n 1)
if [ -f "$UF2_FILE" ]; then
    green "�� Build finalizado! Arquivo gerado:"
    echo "    $UF2_FILE"
else
    red "❌ UF2 não encontrado!"
    exit 1
fi

echo ""
blue "💡 Para carregar no Pico W:"
echo "  1. Conecte segurando BOOTSEL"
echo "  2. Copie o arquivo .uf2 para a unidade RPI-RP2"
echo ""
