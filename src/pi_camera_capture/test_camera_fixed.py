import cv2

# Use a câmera correta (ajuste para o índice que funcionou)
cap = cv2.VideoCapture(1)

# (opcional) definir resolução padrão
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

if not cap.isOpened():
    print("❌ Não foi possível abrir a câmera.")
    exit()

ret, frame = cap.read()

if not ret:
    print("❌ Não foi possível capturar o frame.")
    cap.release()
    exit()

cv2.imwrite("captura_final.jpg", frame)
print("Imagem capturada e salva como captura_final.jpg")

cap.release()
