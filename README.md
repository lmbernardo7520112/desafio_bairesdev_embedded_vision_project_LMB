# 🔍 Inferência de Visão Computacional com Raspberry Pi Pico (RP2040)

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![TFLM](https://img.shields.io/badge/TensorFlow_Lite_Micro-RP2040-yellow?style=for-the-badge&logo=tensorflow)
![PicoSDK](https://img.shields.io/badge/PicoSDK-1.5.1-lightgrey?style=for-the-badge&logo=raspberrypi)
![CMSIS-NN](https://img.shields.io/badge/CMSIS--NN-optimized-brightgreen?style=for-the-badge&logo=arm)

## 📄 Descrição do Projeto

Este projeto realiza inferência de imagens em tempo real utilizando um microcontrolador **Raspberry Pi Pico (RP2040)** com modelo embarcado do **TensorFlow Lite for Microcontrollers (TFLM)**, otimizado com **CMSIS-NN**. O modelo classifica expressões faciais a partir de imagens 48×48 processadas previamente no host (Raspberry Pi 3).

> 📸 A captura é feita por **câmera USB** conectada ao Raspberry Pi 3 (não à Pico).  
> 📤 A imagem é processada no Pi 3 e enviada via **UART** à Pico.  
> 📊 O resultado da inferência é visualizado via **monitor serial (PuTTY)**.

---

## 🧠 Arquitetura Geral

```text
[Câmera USB] ─┬─> [Raspberry Pi 3 (host)]
              │       └─> Captura + Redimensionamento + Normalização
              │       └─> Envio via UART
              ↓
        [Raspberry Pi Pico (RP2040)]
              └─> Recebe dados normalizados
              └─> Executa inferência com TFLM
              └─> Exibe resultado no monitor serial (ex: PuTTY)
```

> 💬 A exibição em display OLED SSD1306 **ainda não foi implementada**, embora a biblioteca esteja pronta no projeto.

---

## 🛠️ Tecnologias e Ferramentas

- **Hardware**
  - Raspberry Pi Pico (RP2040, Cortex-M0+)
  - Raspberry Pi 3 (Host USB/UART)
  - Câmera USB
  - OLED SSD1306 (planejado)
  - ESP32 (modo analisador lógico, opcional)
- **Software**
  - TensorFlow Lite Micro + CMSIS-NN
  - C++17 com Pico SDK
  - `gcc-arm-none-eabi` toolchain
  - PuTTY / Minicom (monitor serial)
  - sigrok/PulseView (para análise de sinais UART)

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
- ✅ Comunicação UART configurada com `uart0`, `baudrate 115200`, RX/TX definidos.
- ✅ Recepção dos dados de imagem 48×48 do Pi 3 via serial.
- ✅ Inferência executa sem falhas com `MicroInterpreter`.
- ✅ Logs da inferência são exibidos via **monitor serial (PuTTY)**.
- ⚠️ **UART apresenta ruído ou dados corrompidos** — em depuração.
- ⏳ A exibição em OLED ainda será implementada.

---

## 🧪 Próximas Etapas

- [ ] Resolver problema de integridade dos dados UART (desalinhamento, sincronização)
- [ ] Adicionar parsing robusto do buffer UART (start/stop delimiters ou checksum)
- [ ] Medir latência real por frame
- [ ] Ligar e testar exibição com display OLED SSD1306
- [ ] Dividir UART/Inferência entre os dois cores do RP2040 (multi-core)
- [ ] Avaliar podas/extensões de modelo (CNN separável, pruning)

---

## 🧰 Debug com ESP32 como Analisador Lógico

Para diagnóstico mais preciso da UART (ruído, start bits, tempos), o ESP32 foi configurado como **analisador lógico compatível com protocolo SUMP**, com o firmware `logic_analyzer-pico.ino`.

- Sinais RX e TX são capturados diretamente do Pi 3 ou da Pico.
- Visualização no **PulseView** com clock de amostragem personalizado.

---

## 📎 Complementos

- 📁 `modelo_expressoes_TFLite.ipynb` — Treinamento e quantização do modelo
- 📁 `model_data.cc` — Modelo como array embarcado
- 🔧 `env.sh` — Exporta variáveis de ambiente do SDK e toolchain
- 📸 Câmera USB gerenciada 100% pelo **Raspberry Pi 3** (não pela Pico)

---

## 📢 Aviso Final

> A câmera utilizada neste projeto é uma **USB conectada ao Raspberry Pi 3**, responsável pela captura, preprocessamento e envio serial. A **Raspberry Pi Pico não realiza captura direta de imagem** nem usa câmera SPI, CSI ou PIO.
