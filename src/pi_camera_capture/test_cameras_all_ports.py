import cv2

for i in range(5):
    print(f"Tentando abrir /dev/video{i}...")
    cap = cv2.VideoCapture(i)

    if not cap.isOpened():
        print(f"❌ Não foi possível abrir a câmera {i}.")
        continue

    ret, frame = cap.read()
    if not ret:
        print(f"❌ Câmera {i} abriu, mas não retornou frame.")
    else:
        print(f"✅ Frame capturado da câmera {i}. Salvando imagem...")
        cv2.imwrite(f"usb_camera_{i}.jpg", frame)
        break

    cap.release()
