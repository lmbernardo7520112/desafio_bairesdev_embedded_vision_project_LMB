# send_uart_signal.py
import serial # type: ignore
import time

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
time.sleep(2)
ser.write(b'FOTO_OK\n')
print("📤 Sinal enviado via UART.")
ser.close()
