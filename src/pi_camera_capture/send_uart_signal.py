import serial
import time

try:
    ser = serial.Serial('/dev/serial0', 115200, timeout=1)
    print("✅ Porta serial da Pi 3 aberta com sucesso.")
    time.sleep(2)

    command = "FOTO_OK\n"
    ser.write(command.encode('utf-8'))  # codifica corretamente
    print(f"📤 Sinal '{command.strip()}' enviado.")

except Exception as e:
    print(f"❌ Erro na Pi 3: {e}")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("🚪 Porta serial da Pi 3 fechada.")
