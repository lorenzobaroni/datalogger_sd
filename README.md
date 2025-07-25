
# 📈 Datalogger de Movimento com MPU6050 + SD + OLED (RP2040 - BitDogLab)

Este projeto é um sistema embarcado para coleta de dados de movimento utilizando o sensor MPU6050 com uma Raspberry Pi Pico W (BitDogLab). Os dados de aceleração e giroscópio são armazenados em um cartão SD no formato `.csv` e exibidos em tempo real no display OLED SSD1306.

---

## 🧠 Funcionalidades

- ✅ Leitura de dados do sensor MPU6050 (aceleração e giroscópio)
- ✅ Gravação dos dados em um cartão SD no formato `.csv`
- ✅ Exibição do status e contador de amostras em tempo real no display OLED
- ✅ Interação por botões físicos (montar cartão, iniciar gravação, etc.)
- ✅ Feedback visual com LED RGB e buzzer
- ✅ Geração de gráficos a partir dos dados usando Python

---

## 🔌 Hardware Utilizado

| Componente         | Modelo                      |
|--------------------|-----------------------------|
| Microcontrolador   | Raspberry Pi Pico W (BitDogLab) |
| Sensor IMU         | MPU6050 (I2C)               |
| Display OLED       | SSD1306 (128x64, I2C)       |
| Armazenamento      | Cartão SD com FAT32 (SPI)   |
| Sinalização        | LED RGB e buzzer            |
| Entradas           | Botões A (GPIO 5) e B (GPIO 6) |

---

## 🧪 Comandos Disponíveis via Terminal

Durante a execução, o sistema aguarda comandos seriais:

- `a` → Montar cartão SD
- `b` → Criar novo arquivo CSV
- `c` → Mostrar conteúdo do CSV no terminal
- `d` → Iniciar gravação de 100 amostras
- `h` → Mostrar ajuda com os comandos

---

## 📺 Feedback Visual

- **Display OLED:** mostra o status do sistema e contador de amostras.
- **LED RGB:**
  - Amarelo → Inicializando
  - Verde → Pronto para gravar
  - Vermelho → Gravando dados
  - Azul piscando → Gravando (durante gravação)
  - Roxo piscando → Erro (ex: falha ao montar SD)
- **Buzzer:**
  - 1 beep → Início de gravação
  - 2 beeps → Fim da gravação

---

## 🖥️ Visualização dos Dados

Use o script Python para gerar gráficos a partir dos dados gravados:

### Instalar dependências (uma vez):
```bash
pip install pandas matplotlib
```

### Executar o script:
```bash
python3 PlotaDados.py
```

Isso irá gerar dois arquivos:
- `grafico_acelerometro.png`
- `grafico_giroscopio.png`

---

## 📜 Licença

Este projeto é de uso livre para fins educacionais e acadêmicos.
