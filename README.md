# 🔍 Inferência de Visão Computacional com Raspberry Pi Pico (RP2040)

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![TFLM](https://img.shields.io/badge/TensorFlow_Lite_Micro-RP2040-yellow?style=for-the-badge&logo=tensorflow)
![PicoSDK](https://img.shields.io/badge/PicoSDK-1.5.1-lightgrey?style=for-the-badge&logo=raspberrypi)
![CMSIS-NN](https://img.shields.io/badge/CMSIS--NN-optimized-brightgreen?style=for-the-badge&logo=arm)

## 📄 Descrição do Projeto

Este repositório implementa um sistema completo de inferência embarcada utilizando a microcontroladora **Raspberry Pi Pico (RP2040)**. O sistema é capaz de realizar classificação em tempo real com um modelo leve baseado em **TensorFlow Lite for Microcontrollers (TFLM)** otimizado com **CMSIS-NN**.

A aplicação embarcada foi desenvolvida em C++ com uso intensivo do **Pico SDK** e executa inferência utilizando um modelo `.tflite` especializado para classificação de expressões faciais com imagens 48×48. O modelo foi treinado com **Transfer Learning** no Google Colab (vide notebook anexo), e convertido para um array C (`model_data.cc`) para embarque no firmware.

> 📌 **Latência alvo:** < 100ms por inferência  
> ⚙️ **Plataforma alvo:** Raspberry Pi Pico (264KB RAM / 2MB Flash)  
> 🧠 **Modelo:** CNN (convolucional) treinado com imagens de expressões faciais

---

## 🛠️ Tecnologias e Ferramentas Utilizadas

- **Microcontrolador:** Raspberry Pi Pico (RP2040, Cortex-M0+ dual-core)
- **Framework Principal:** TensorFlow Lite for Microcontrollers (TFLM)
- **Backend de Otimização:** CMSIS-NN
- **SDK:** Pico SDK 1.5.1
- **Compilador:** `gcc-arm-none-eabi-10.3-2021.10`
- **Display:** SSD1306 I²C OLED (para saída da classe prevista)
- **Treinamento do Modelo:** Google Colab com GPU T4 (modelo `.tflite` gerado com Transfer Learning)

---

## ⚙️ Pipeline do Projeto

### 1. 🎓 Treinamento e Conversão do Modelo (`Colab`)

- O modelo original foi treinado no notebook **`modelo_expressoes_TFLite.ipynb`** usando Transfer Learning.
- Após atingir precisão satisfatória nas classes de interesse, o modelo foi quantizado e exportado como `.tflite`.
- A ferramenta `xxd` foi usada para converter o modelo em array C (`model_data.cc`) para embarque estático.

### 2. 🧠 Integração com TFLM no RP2040

- O código embarcado integra:
  - Um **pipeline de inferência com `MicroInterpreter`** da TFLM
  - Operações essenciais como `Conv2D`, `Relu`, `MaxPool2D`, `Softmax`, `FullyConnected`, `Reshape`, `Pad`, `StridedSlice`
  - `tensor_arena` estático com 20KB configurado para evitar `malloc`

### 3. 🖼️ Captura e Pré-processamento

- Um módulo `image_provider.cpp` simula a aquisição da imagem e gera os dados normalizados para inferência.
- Os dados são inseridos manualmente no tensor de entrada.

### 4. 📟 Exibição no Display SSD1306

- O resultado da classificação (índice da classe) é exibido diretamente no display OLED I²C.
- O display é gerenciado por uma biblioteca leve baseada no protocolo SSD1306.

---

## 📋 Estrutura do Código
pico_inference_project/
│
├── CMakeLists.txt # Configuração de build (otimizações -Os, flags ARM)
├── src/
│ ├── inference.cpp # Pipeline de inferência TFLM
│ ├── image_provider.cpp/.h # Simulação de aquisição de imagem
│ ├── model_data.cc # Modelo embarcado como array
│
├── lib/ssd1306/ # Biblioteca do display OLED
├── external/pico-tflmicro/ # Submódulo com TFLM + CMSIS-NN
├── include/ # Headers locais
├── toolchain-arm-none-eabi.cmake
└── env.sh # Setup de variáveis de ambiente

---

## ✅ Status e Resultados Esperados

- [x] Build com `pico-sdk` e `pico-tflmicro` com sucesso
- [x] Display I²C funcional
- [x] Pipeline de inferência funcional com `model_data.cc`
- [ ] Acurácia validada com imagens reais
- [ ] Migração futura para captura de imagem via câmera SPI/UART (ex: ArduCam)

---

## 🧪 Próximos Passos

- [ ] Medição real da latência (< 100ms alvo)
- [ ] Compressão adicional do modelo com quantização agressiva
- [ ] Uso de segundo core da Pico para desacoplar aquisição/inferência

---

## 📎 Anexos

- 📓 `modelo_expressoes_TFLite.ipynb` (Google Colab): Treinamento via Transfer Learning
- 📘 `arquivo-para-reduzir-modelo.pdf`: Análise de footprint e sugestões de compressão

---

## 💡 Contribuição

Este projeto é voltado para entusiastas e profissionais de sistemas embarcados que desejam executar redes neurais diretamente em microcontroladores de baixo custo e baixa potência sem depender de conectividade ou edge gateways.
