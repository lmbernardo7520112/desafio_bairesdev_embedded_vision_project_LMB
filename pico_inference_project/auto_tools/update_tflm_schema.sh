#!/usr/bin/env bash
set -euo pipefail

# ---- Ajuste o caminho do projeto se necessário ----
PROJ=$HOME/desafio_bairesdev_embedded_vision_project_LMB/pico_inference_project
TFLM=$PROJ/tflm
SCHEMA_DIR=$TFLM/tensorflow/lite/schema
SCHEMA_URL=https://raw.githubusercontent.com/tensorflow/tensorflow/main/tensorflow/lite/schema/schema.fbs

echo "➡  Voltando ao repositório TFLM..."
cd "$TFLM"

# 1) Limpa ou guarda alterações locais para permitir o pull
echo "➡  Stash de alterações locais (se existirem)..."
git stash push -u -m "auto‑stash‑update‑schema" || true

echo "➡  Sincronizando com a branch main..."
git pull --ff-only origin main

# 2) Baixa a versão oficial do schema.fbs
echo "➡  Baixando schema.fbs atualizado..."
curl -L -o "$SCHEMA_DIR/schema.fbs" "$SCHEMA_URL"

# 3) Gera novamente o header com flatc
echo "➡  Gerando schema_generated.h..."
flatc --cpp --scoped-enums -o "$SCHEMA_DIR" "$SCHEMA_DIR/schema.fbs"

# 4) Reconstrói o projeto do zero
echo "➡  Limpando build/ e recompilando..."
cd "$PROJ"
rm -rf build
mkdir build && cd build
cmake ..           # acrescente -DPICO_SDK_PATH=... se precisar
make -j"$(nproc)"

echo "✅  Concluído com sucesso!"

