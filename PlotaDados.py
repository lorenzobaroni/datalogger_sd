import pandas as pd
import matplotlib.pyplot as plt
import os

# Caminho do CSV
caminho_csv = "ArquivosDados/dados_mpu.csv"

# Verifica se o arquivo existe
if not os.path.exists(caminho_csv):
    print(f"Arquivo não encontrado: {caminho_csv}")
    exit(1)

# Lê os dados
df = pd.read_csv(caminho_csv)
df['tempo'] = df['tempo'] / 1000  # Converte tempo para segundos

# Caminho para salvar imagens
caminho_saida = "ArquivosDados/"
os.makedirs(caminho_saida, exist_ok=True)

# === Gráfico de Aceleração ===
plt.figure(figsize=(12, 6))
plt.plot(df['tempo'], df['acc_x'], label='Acc X')
plt.plot(df['tempo'], df['acc_y'], label='Acc Y')
plt.plot(df['tempo'], df['acc_z'], label='Acc Z')
plt.xlabel('Tempo (s)')
plt.ylabel('Aceleração (raw)')
plt.title('Aceleração nos 3 eixos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(caminho_saida, "grafico_acelerometro.png"))
plt.show()

# === Gráfico de Giroscópio ===
plt.figure(figsize=(12, 6))
plt.plot(df['tempo'], df['gyro_x'], label='Gyro X')
plt.plot(df['tempo'], df['gyro_y'], label='Gyro Y')
plt.plot(df['tempo'], df['gyro_z'], label='Gyro Z')
plt.xlabel('Tempo (s)')
plt.ylabel('Velocidade Angular (raw)')
plt.title('Velocidade Angular nos 3 eixos')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(caminho_saida, "grafico_giroscopio.png"))
plt.show()
