import cv2

# Abre a webcam USB no /dev/video0
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("❌ Não foi possível abrir a câmera.")
    exit()

ret, frame = cap.read()

if not ret:
    print("❌ Não foi possível capturar um frame.")
    cap.release()
    exit()

# Salva a imagem
cv2.imwrite("captura_usb.jpg", frame)
print("✅ Imagem capturada e salva como captura_usb.jpg")

cap.release()
