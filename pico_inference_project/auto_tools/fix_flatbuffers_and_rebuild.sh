#!/bin/bash

# Caminho base do seu projeto
PROJ_DIR=~/desafio_bairesdev_embedded_vision_project_LMB/pico_inference_project
FLATBUFFERS_VERSION=23.5.26

cd "$PROJ_DIR" || exit 1

echo "🔄 Removendo flatbuffers antigo..."
rm -rf tflm/third_party/flatbuffers

echo "⬇️ Clonando flatbuffers versão ${FLATBUFFERS_VERSION}..."
git clone --depth=1 --branch v${FLATBUFFERS_VERSION} https://github.com/google/flatbuffers.git tflm/third_party/flatbuffers

echo "🔍 Baixando flatc versão ${FLATBUFFERS_VERSION} pré-compilado..."
cd /tmp
wget https://github.com/google/flatbuffers/releases/download/v${FLATBUFFERS_VERSION}/Linux.flatc.binary.clang++-12.zip -O flatc.zip
unzip -o flatc.zip
chmod +x flatc
mv flatc /usr/local/bin/flatc

echo "✅ Verificando flatc..."
flatc --version

echo "🧼 Limpando arquivos antigos gerados..."
cd "$PROJ_DIR/tflm/tensorflow/lite"
rm -f schema/schema_generated.h

echo "🛠️ Regenerando schema_generated.h com flatc ${FLATBUFFERS_VERSION}..."
flatc --cpp schema/schema.fbs

echo "🔁 Limpando e reconstruindo projeto..."
cd "$PROJ_DIR"
rm -rf build
mkdir build && cd build

cmake .. -G "Unix Makefiles"
make -j$(nproc) > build_log_13.txt 2>&1

echo "✅ Processo concluído. Verifique se houve erros no build."
