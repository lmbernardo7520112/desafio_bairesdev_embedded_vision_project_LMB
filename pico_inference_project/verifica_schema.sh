#!/bin/bash

# Caminho do projeto (ajuste se necessário)
PROJ_DIR="$HOME/desafio_bairesdev_embedded_vision_project_LMB/pico_inference_project"

# Caminho do arquivo problemático
SRC_FILE="$PROJ_DIR/tflm/tensorflow/lite/core/api/flatbuffer_conversions.cc"

# Compilador
COMPILER="arm-none-eabi-g++"  # ou apenas g++ se estiver usando emulação

# Flags do compilador (ajuste se necessário)
FLAGS="-std=c++17 -I$PROJ_DIR/tflm -I$PROJ_DIR/tflm/third_party/flatbuffers/include -I$PROJ_DIR/tflm/tensorflow -DDEBUG -E"

# Executa pré-processador e procura o header incluído
echo "⏳ Verificando onde schema_generated.h está sendo incluído..."
$COMPILER $FLAGS $SRC_FILE 2>/dev/null | grep "schema_generated.h"
