import serial
import time

try:
    ser = serial.Serial('/dev/serial0', 115200, timeout=1)
    print("✅ Porta serial da Pi 3 aberta com sucesso.")
    time.sleep(2)
    
    command_to_send = b'FOTO_OK\n'
    ser.write(command_to_send)
    print(f"📤 Sinal '{command_to_send.decode().strip()}' enviado.")

except Exception as e:
    print(f"❌ Erro na Pi 3: {e}")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("🚪 Porta serial da Pi 3 fechada.")

