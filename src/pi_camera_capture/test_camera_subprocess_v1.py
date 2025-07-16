import subprocess

# Envia o comando para rodar o script remotamente na Pi
result = subprocess.run([
    "ssh", "lmbernardo@192.168.1.195",
    "source ~/embedded_vision_project_venv/bin/activate && "
    "python3 ~/desafio_bairesdev_embedded_vision_project_LMB/src/pi_camera_capture/test_camera_fixed.py"
], capture_output=True, text=True)
print(result.stdout)
print(result.stderr)
