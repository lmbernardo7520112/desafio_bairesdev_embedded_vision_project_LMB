import serial
import time
import subprocess
import os

# Configuração da UART correta
UART_PORT = "/dev/serial0"  # Aponta para a UART liberada pela config.txt
BAUD_RATE = 115200

def send_uart_signal(message):
    print("📡 Inicializando UART para envio...")
    try:
        with serial.Serial(UART_PORT, BAUD_RATE, timeout=2) as ser:
            time.sleep(0.1)  # Tempo para estabilização
            ser.write((message + "\n").encode('utf-8'))  # Envia como string UTF-8 com quebra de linha
            print(f"📤 Enviado via UART: {message}")
    except Exception as e:
        print(f"❌ Erro UART: {e}")

def capture_image(filename="captura_pi3.jpg"):
    print("📸 Capturando imagem com câmera USB...")
    try:
        subprocess.run(["fswebcam", "-r", "640x480", "--no-banner", filename], check=True)
        print(f"✅ Imagem salva: {filename}")
        return os.path.exists(filename) # type: ignore
    except subprocess.CalledProcessError:
        print("❌ Erro ao capturar imagem.")
        return False

if __name__ == "__main__":
    print("\n==== INÍCIO DO SCRIPT DA PI 3 ====\n")
    
    # Etapa 1: Teste UART
    send_uart_signal("TESTE_UART")
    time.sleep(1)

    # Etapa 2: Captura de imagem
    if capture_image():
        time.sleep(1)
        send_uart_signal("FOTO_OK")
        print("✅ Sinal de captura enviado.")
    else:
        print("❌ Captura falhou. Abortando envio.")
