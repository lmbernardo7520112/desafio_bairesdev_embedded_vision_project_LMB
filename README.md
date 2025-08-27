# 🔍 Inferência de Visão Computacional com Raspberry Pi Pico (RP2040)

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![TFLM](https://img.shields.io/badge/TensorFlow_Lite_Micro-RP2040-yellow?style=for-the-badge&logo=tensorflow)
![PicoSDK](https://img.shields.io/badge/PicoSDK-1.5.1-lightgrey?style=for-the-badge&logo=raspberrypi)
![CMSIS-NN](https://img.shields.io/badge/CMSIS--NN-optimized-brightgreen?style=for-the-badge&logo=arm)

## 📄 Descrição do Projeto

Este projeto realiza inferência de imagens em tempo real utilizando um microcontrolador **Raspberry Pi Pico (RP2040)** com modelo embarcado do **TensorFlow Lite for Microcontrollers (TFLM)**, otimizado com **CMSIS-NN**. O modelo classifica expressões faciais a partir de imagens 48×48 processadas previamente no host (Raspberry Pi 3).

> 📸 A captura é feita por **câmera USB** conectada ao Raspberry Pi 3 (não à Pico).  
> 📤 A imagem é processada no Pi 3 e enviada via **Wi-Fi** à Pico. 
> 📊 O resultado da inferência é visualizado via **monitor serial (Minicom)**.

---

## 🧠 Arquitetura Geral

```text
[Câmera USB] ─┬─> [Raspberry Pi 3 (host)]
              │       └─> Captura + Redimensionamento + Normalização
              │       └─> Envio via Wi-Fi
              ↓
        [Raspberry Pi Pico (RP2040)]
              └─> Recebe dados normalizados
              └─> Executa inferência com TFLM
              └─> Exibe resultado no monitor serial (Minicom)
```

> 💬 A exibição em display OLED SSD1306 **ainda não foi implementada**, embora a biblioteca esteja pronta no projeto.

---

## 🛠️ Tecnologias e Ferramentas

- **Hardware**
  - Raspberry Pi Pico (RP2040, Cortex-M0+)
  - Raspberry Pi 3 (Host USB/Wi-Fi)
  - Câmera USB
  - OLED SSD1306 (planejado)
- **Software**
  - TensorFlow Lite Micro
  - C++17 com Pico SDK
  - `gcc-arm-none-eabi` toolchain
  - PuTTY / Minicom (monitor serial)

---

## 📂 Estrutura do Código

```
pico_inference_project/
│
├── CMakeLists.txt              # Configuração do projeto (compilador, flags ARM)
├── src/
│   ├── main.cpp                # Função principal: UART, inferência, logs
│   ├── inference.cpp/.h       # Pipeline da TFLM (tensores, operador, arena)
│   ├── image_provider.cpp/.h  # Interface para imagem (buffer UART → tensor)
│   ├── model_data.cc/.h       # Array C do modelo .tflite quantizado
│
├── lib/
│   └── ssd1306/                # Biblioteca SSD1306 (OLED) – ainda não usada
├── include/                   # Headers comuns
├── external/pico-tflmicro/    # Submódulo TFLM + CMSIS-NN (otimizado ARM)
├── env.sh                     # Script para configurar toolchain e SDK
└── toolchain-arm-none-eabi.cmake
```

---

## 🔎 Estado Atual

- ✅ Código da Pico compila corretamente com suporte completo ao TFLM.
- ✅ Conexão Wi-Fi configurada com cyw43_arch, escaneia redes "moto" e conecta.
- ✅ Recepção dos dados de imagem 48×48 do Pi 3 via serial.
- ✅ Inferência executa sem falhas com `MicroInterpreter`.
- ✅ Logs da inferência são exibidos via monitor serial (Minicom).
- ⏳ A exibição em OLED ainda será implementada.

---

## 🧪 Próximas Etapas

- [ ] Refatorar modelo para reduzir latência (pruning, quantização adicional).
- [ ] Ligar e testar exibição com display OLED SSD1306
- [ ] Melhorar precisão com ajustes na CNN (camadas separáveis).
- [ ] Otimizar tamanho do modelo para menor uso de memória.
      
---

## 📎 Complementos

- 📁 `modelo_expressoes_TFLite.ipynb` — Treinamento e quantização do modelo
- 📁 `model_data.cc` — Modelo como array embarcado
- 🔧 `env.sh` — Exporta variáveis de ambiente do SDK e toolchain
- 📸 Câmera USB gerenciada 100% pelo **Raspberry Pi 3** (não pela Pico)

---

## 📢 Aviso Final

> A câmera utilizada neste projeto é uma **USB conectada ao Raspberry Pi 3**, responsável pela captura, preprocessamento e envio serial. A **Raspberry Pi Pico não realiza captura direta de imagem** nem usa câmera SPI, CSI ou PIO.
