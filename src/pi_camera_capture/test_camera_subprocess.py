import subprocess
import os

# Caminhos
usuario_pi = "lmbernardo"
ip_pi = "192.168.1.195"
caminho_imagem_remoto = "/home/lmbernardo/captura_autodetectada.jpg"
caminho_imagem_local = os.path.expanduser("~/captura_autodetectada.jpg")

# 1. Executa o script remoto que detecta a câmera e captura a imagem
print("📸 Executando captura de imagem na Raspberry Pi...")
result = subprocess.run([
    "ssh", f"{usuario_pi}@{ip_pi}",
    "source ~/embedded_vision_project_venv/bin/activate && "
    "python3 ~/test_camera_autodetect_and_capture.py"
], capture_output=True, text=True)
print(result.stdout)
print(result.stderr)

# 2. Copia a imagem da Pi para o notebook via SCP
print("🔁 Copiando imagem da Raspberry Pi para o notebook...")
scp_result = subprocess.run([
    "scp",
    f"{usuario_pi}@{ip_pi}:{caminho_imagem_remoto}",
    caminho_imagem_local
], capture_output=True, text=True)

# Exibe resultado do SCP
if scp_result.returncode == 0:
    print(f"✅ Imagem copiada com sucesso para: {caminho_imagem_local}")
else:
    print("❌ Falha ao copiar a imagem:")
    print(scp_result.stderr)
