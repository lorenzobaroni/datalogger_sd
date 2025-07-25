#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"
#include "ff.h"               // Para FATFS, FIL, FRESULT, etc.
#include "hardware/gpio.h"    // Para GPIO_IRQ_EDGE_FALL
#include "pico/time.h"        // Para to_ms_since_boot()
#include "lib/ssd1306.h"

#define I2C_DISP_PORT i2c1
#define SDA_DISP 14
#define SCL_DISP 15
#define OLED_ADDR 0x3C
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define BOTAO_A 5
#define BOTAO_B 6
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define BUZZER 21

ssd1306_t oled; 
static int addr = 0x68;
volatile uint32_t ultimo_tempo_A = 0;
volatile uint32_t ultimo_tempo_B = 0;
const uint32_t debounce = 200;

// Prototipos do Cartao_FatFS_SPI.c
void montar_cartao();
void criar_arquivo_csv();
void salvar_dado_csv(int16_t ax, int16_t ay, int16_t az,
                     int16_t gx, int16_t gy, int16_t gz);
void read_file(const char *filename);

void atualizar_display(const char *status, int contador);

// === LED RGB ===
void set_led_rgb(bool r, bool g, bool b) {
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
}

// === Buzzer ===
void beep(int vezes, int duracao_ms) {
    for (int i = 0; i < vezes; i++) {
        gpio_put(BUZZER, 1);
        sleep_ms(duracao_ms);
        gpio_put(BUZZER, 0);
        sleep_ms(100);
    }
}

static void mpu6050_reset() {
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(I2C_PORT, addr, buf, 2, false);
    sleep_ms(100);
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, addr, buf, 2, false);
    sleep_ms(10);
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t buffer[6];
    uint8_t val = 0x3B;
    i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
    i2c_read_blocking(I2C_PORT, addr, buffer, 6, false);
    for (int i = 0; i < 3; i++) accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);

    val = 0x43;
    i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
    i2c_read_blocking(I2C_PORT, addr, buffer, 6, false);
    for (int i = 0; i < 3; i++) gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);

    val = 0x41;
    i2c_write_blocking(I2C_PORT, addr, &val, 1, true);
    i2c_read_blocking(I2C_PORT, addr, buffer, 2, false);
    *temp = buffer[0] << 8 | buffer[1];
}

// Callback
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    if (gpio == BOTAO_A && (tempo_atual - ultimo_tempo_A > debounce)) {
        ultimo_tempo_A = tempo_atual;
        printf("Cartão montado com sucesso.\n");
        atualizar_display("Aguardando", 0);
        set_led_rgb(0, 1, 0);  // Verde = pronto
    } else if (gpio == BOTAO_B && (tempo_atual - ultimo_tempo_B > debounce)) {
        ultimo_tempo_B = tempo_atual;
        printf(">> Arquivo CSV Criado\n");
        atualizar_display("Aguardando", 0);
        set_led_rgb(0, 1, 0); // Verde = pronto
    }
}

void mostrar_menu() {
    printf("\nComandos disponíveis:\n");
    printf("a - Montar cartão SD\n");
    printf("b - Criar arquivo CSV\n");
    printf("c - Mostrar conteúdo do CSV\n");
    printf("d - Iniciar gravação de dados da IMU\n");
    printf("h - Mostrar este menu\n");
    printf("Escolha uma opção: ");
}

void read_file(const char *filename) {
    FIL file;
    FRESULT res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return;
    }

    char buffer[128];
    UINT br;
    printf("Conteúdo do arquivo %s:\n", filename);
    while (f_read(&file, buffer, sizeof(buffer) - 1, &br) == FR_OK && br > 0) {
        buffer[br] = '\0';
        printf("%s", buffer);
    }

    f_close(&file);
    printf("\n\n");
}

void atualizar_display(const char *status, int contador) {
    char linha1[22];
    char linha2[22];

    snprintf(linha1, sizeof(linha1), "%s", status);
    snprintf(linha2, sizeof(linha2), "Amostras: %d", contador);

    ssd1306_fill(&oled, 0);
    ssd1306_draw_string(&oled, linha1, 0, 0);
    ssd1306_draw_string(&oled, linha2, 0, 16);
    ssd1306_send_data(&oled);
}

int main() {
    stdio_init_all();
    sleep_ms(5000);

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);

    // Estado inicial: LED amarelo = R+G
    set_led_rgb(1, 1, 0);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


    printf("Hello, MPU6050 + Cartao SD\n");

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));

    i2c_init(I2C_DISP_PORT, 400 * 1000);
    gpio_set_function(SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_DISP);
    gpio_pull_up(SCL_DISP);

    ssd1306_init(&oled, 128, 64, true, OLED_ADDR, I2C_DISP_PORT);
    ssd1306_config(&oled);
    ssd1306_draw_string(&oled, "Inicializando...", 0, 0);
    ssd1306_send_data(&oled);

    mpu6050_reset();
    montar_cartao();

    int16_t acc[3], gyr[3], temp;
    mostrar_menu();

    while (true) {
        int c = getchar_timeout_us(0);

        if (c == 'a') {
            beep(1, 100);
            set_led_rgb(0, 1, 0);
            atualizar_display("Aguardando", 0);
            printf("Cartão montado.\n");
        } else if (c == 'b') {
            criar_arquivo_csv();
            beep(1, 100);
            set_led_rgb(0, 1, 0);
            printf("Arquivo CSV criado.\n");
        } else if (c == 'c') {
            read_file("ArquivosDados/dados_mpu.csv");
        } else if (c == 'd') {
            printf("Gravando dados da IMU...\n");
            for (int i = 0; i < 100; i++) {
                set_led_rgb(0, 0, 1);
                mpu6050_read_raw(acc, gyr, &temp);
                salvar_dado_csv(acc[0], acc[1], acc[2], gyr[0], gyr[1], gyr[2]);
                atualizar_display("Gravando...", i + 1);
                set_led_rgb(0, 0, 0);
                sleep_ms(50);
                set_led_rgb(0, 0, 1);
                sleep_ms(50);
            }
            set_led_rgb(0, 1, 0);  // verde = pronto
            beep(2, 100);
            printf(">> Gravação encerrada\n");
            printf("Gravação concluída.\n");
        } else if (c == 'h') {
            mostrar_menu();
        }

        sleep_ms(100);
    }

}