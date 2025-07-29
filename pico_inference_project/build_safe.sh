#!/bin/bash
# build_safe.sh - Script de build robusto para Pico W + TensorFlow Lite

set -e  # Para na primeira falha

echo "🚀 Iniciando build do projeto Pico W TensorFlow Lite..."

# ============================================================
# VALIDAÇÕES INICIAIS
# ============================================================

# Verificar se variáveis estão definidas
if [ -z "$PICO_SDK_PATH" ]; then
    echo "❌ ERRO: PICO_SDK_PATH não definido!"
    echo "Execute: source env.sh"
    exit 1
fi

if [ ! -d "$PICO_SDK_PATH" ]; then
    echo "❌ ERRO: Diretório do Pico SDK não encontrado: $PICO_SDK_PATH"
    exit 1
fi

# Verificar se toolchain está no PATH
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "❌ ERRO: arm-none-eabi-gcc não encontrado no PATH!"
    echo "Execute: source env.sh"
    exit 1
fi

echo "✅ Variáveis de ambiente OK"
echo "   PICO_SDK_PATH: $PICO_SDK_PATH"
echo "   Toolchain: $(which arm-none-eabi-gcc)"

# ============================================================
# VERIFICAR INTEGRIDADE DO SDK
# ============================================================

echo "🔍 Verificando integridade do Pico SDK..."

LINKER_SCRIPT="$PICO_SDK_PATH/src/rp2_common/pico_crt0/rp2040/memmap_default.ld"
if [ ! -f "$LINKER_SCRIPT" ]; then
    echo "⚠️  Linker script não encontrado, tentando reparar SDK..."
    cd "$PICO_SDK_PATH"
    git submodule update --init --recursive
    echo "✅ SDK reparado"
else
    echo "✅ Linker scripts encontrados"
fi

# ============================================================
# PREPARAR DIRETÓRIO DE BUILD
# ============================================================

PROJECT_DIR="$HOME/desafio_bairesdev_embedded_vision_project_LMB/pico_inference_project"
cd "$PROJECT_DIR"

echo "🧹 Limpando build anterior..."
rm -rf build/
mkdir -p build
cd build

# ============================================================
# CONFIGURAR CMAKE
# ============================================================

echo "⚙️  Configurando CMAKE..."

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPICO_BOARD=pico_w \
    -DPICO_SDK_PATH="$PICO_SDK_PATH" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ $? -ne 0 ]; then
    echo "❌ ERRO na configuração do CMAKE!"
    exit 1
fi

echo "✅ CMAKE configurado com sucesso"

# EXECUTAR BUILD (MODO DE DIAGNÓSTICO DETALHADO)
# ============================================================

echo "🔨 Iniciando compilação em modo de diagnóstico..."

# Tenta compilar o projeto. Se falhar, o script continuará.
make -j$(nproc)

# Verifica se o make falhou
if [ $? -ne 0 ]; then
    echo "-----------------------------------------------------------------"
    echo "🚨 BUILD FALHOU. Focando no arquivo problemático: src/inference.cpp"
    echo "-----------------------------------------------------------------"
    
    # Executa o comando de compilação para APENAS o arquivo problemático
    # com a flag VERBOSE=1 para mostrar o comando exato e o erro.
    # Este comando vai falhar, mas nos dará o erro exato e completo.
    make CMakeFiles/pico_inference_project.dir/src/inference.cpp.o VERBOSE=1
    
    echo "-----------------------------------------------------------------"
    echo "❌ ERRO DETALHADO ACIMA. A compilação foi interrompida."
    echo "-----------------------------------------------------------------"
    exit 1
fi


# ============================================================
# VERIFICAR SAÍDAS
# ============================================================

echo "✅ Compilação concluída com sucesso!"

if [ -f "pico_inference_project.uf2" ]; then
    echo "📁 Arquivo UF2 gerado: $(pwd)/pico_inference_project.uf2"
    ls -lh pico_inference_project.*
    
    echo ""
    echo "🎯 PRÓXIMOS PASSOS:"
    echo "1. Conecte o Pico W em modo BOOTSEL (segure BOOTSEL + conecte USB)"
    echo "2. Copie o arquivo .uf2 para o drive RPI-RP2"
    echo "3. O Pico reiniciará automaticamente com o firmware"
    echo ""
    echo "📊 Tamanho do firmware:"
    arm-none-eabi-size pico_inference_project.elf
else
    echo "⚠️  Arquivo UF2 não foi gerado, mas build passou"
    ls -la pico_inference_project.*
fi

echo "🏁 Build concluído!"
