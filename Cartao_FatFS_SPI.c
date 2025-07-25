#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"
#include "hw_config.h"

static FATFS fs;
static FIL file;
static bool cartao_ok = false;
void atualizar_display(const char *status, int contador);

void montar_cartao() {
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Erro ao montar o cartão SD: %d\n", res);
        atualizar_display("No SD Detected", 0);
        while (1) tight_loop_contents(); // trava o sistema
    } else {
        cartao_ok = true; // ← ESSENCIAL
    }
}


void criar_arquivo_csv() {
    if (!cartao_ok) return;

    FRESULT fr = f_open(&file, "ArquivosDados/dados_mpu.csv", FA_WRITE | FA_CREATE_ALWAYS);
    if (fr == FR_OK) {
        f_printf(&file, "tempo,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z\n");
        f_close(&file);
    } else {
        printf("Erro ao criar arquivo CSV: %d\n", fr);
    }
}

void salvar_dado_csv(int16_t ax, int16_t ay, int16_t az,
                     int16_t gx, int16_t gy, int16_t gz) {
    if (!cartao_ok) return;

    char linha[128];
    UINT bw;
    uint32_t tempo = to_ms_since_boot(get_absolute_time());
    sprintf(linha, "%lu,%d,%d,%d,%d,%d,%d\n", tempo, ax, ay, az, gx, gy, gz);

    FRESULT fr = f_open(&file, "ArquivosDados/dados_mpu.csv", FA_WRITE | FA_OPEN_APPEND);
    if (fr == FR_OK) {
        f_write(&file, linha, strlen(linha), &bw);
        f_close(&file);
    } else {
        printf("Erro ao abrir arquivo CSV: %d\n", fr);
    }
}
