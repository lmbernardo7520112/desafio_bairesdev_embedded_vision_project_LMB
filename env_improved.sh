#!/bin/bash

# Carrega configurações padrão do bash
if [ -f ~/.bashrc ]; then
    source ~/.bashrc
fi

# Configuração do ambiente para Raspberry Pi Pico
export PICO_SDK_PATH=/home/lmbernardo/embarcatech/raspberry_pi_pico_sdk/pico-sdk
export PICO_TOOLCHAIN_PATH=/home/lmbernardo/toolchains/gcc-arm-none-eabi-10.3-2021.10
export PICO_EXAMPLES_PATH=/home/lmbernardo/embarcatech/pico/pico-examples

# Adiciona o toolchain ao PATH
export PATH=/usr/bin:$PICO_TOOLCHAIN_PATH/bin:$PATH

# Variável condicional para a toolchain personalizada (apenas no projeto principal)
if [[ "$(pwd)" == *desafio_bairesdev_embedded_vision_project_LMB* ]]; then
    export CMAKE_TOOLCHAIN_FILE="$(pwd)/toolchain-arm-none-eabi.cmake"
else
    unset CMAKE_TOOLCHAIN_FILE
fi

# Personaliza o prompt para mostrar que está no ambiente Pico
export PS1="\[\033[01;32m\]🔧[PICO]\[\033[00m\] \[\033[01;34m\]\w\[\033[00m\]\$ "

# Aliases úteis para desenvolvimento
alias build-pico='cd build && make -j$(nproc)'
alias clean-build='rm -rf build && mkdir build'
alias pico-cmake='cmake -DCMAKE_TOOLCHAIN_FILE=../pico_inference_project/toolchain-arm-none-eabi.cmake -DPICO_SDK_PATH=$PICO_SDK_PATH -DCMAKE_BUILD_TYPE=Release ..'

# Função para verificar ambiente
check_pico_env() {
    echo "🔍 Verificando ambiente Pico..."
    echo "   SDK: $([ -d "$PICO_SDK_PATH" ] && echo "✅" || echo "❌") $PICO_SDK_PATH"
    echo "   Toolchain: $([ -d "$PICO_TOOLCHAIN_PATH" ] && echo "✅" || echo "❌") $PICO_TOOLCHAIN_PATH"
    echo -n "   Compilador: "
    if command -v arm-none-eabi-gcc >/dev/null; then
        echo "✅ $(command -v arm-none-eabi-gcc)"
    else
        echo "❌ Não encontrado"
    fi
    echo "   Toolchain file: ${CMAKE_TOOLCHAIN_FILE:-❌ Não definido}"
}

# Mostra status na inicialização
echo "🚀 Ambiente Pico carregado!"
check_pico_env
