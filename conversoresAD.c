#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font.h"
#include "hardware/pwm.h"
#include <string.h>
#include <stdlib.h>
#include "ws2818b.pio.h"
#include "hardware/adc.h"

// Definições para comunicação I2C com o display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

// Definição de pinos para botões e joystick
#define BUTTON_A_PIN 5       // Botão A
#define JOYSTICK_BUTTON 22   // Botão do joystick
#define JOYSTICK_X 26        // Eixo X do joystick (entrada ADC)
#define JOYSTICK_Y 27        // Eixo Y do joystick (entrada ADC)

// Definição de pinos para controle do LED RGB
#define LED_RGB_GREEN 11     // LED verde
#define LED_RGB_RED 12       // LED vermelho (PWM)
#define LED_RGB_BLUE 13      // LED azul (PWM)

// Parâmetros do display OLED
#define WIDTH 128
#define HEIGHT 64
#define QUADRADO_SIZE 10 // Tamanho do quadrado desenhado na tela

// Variáveis de estado dos LEDs e da interface
bool led_green_state = false;
bool led_pwm_state = true;
int border_style = 0;

ssd1306_t ssd; // Estrutura para controle do display
volatile uint32_t last_interrupt_time = 0;
#define DEBOUNCE_DELAY 300   // Delay para debounce de botões em milissegundos

// Posição inicial do quadrado no display
int pos_x = WIDTH / 2;
int pos_y = HEIGHT / 2;

// Função chamada na interrupção dos botões para alternar estados
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    if (current_time - last_interrupt_time > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time = current_time;
        
        if (gpio == BUTTON_A_PIN) {
            led_pwm_state = !led_pwm_state;  // Alterna estado do controle PWM
        } 
        else if (gpio == JOYSTICK_BUTTON) {
            led_green_state = !led_green_state; // Alterna estado do LED verde
            gpio_put(LED_RGB_GREEN, led_green_state);
            ssd1306_toggle_border(&ssd, led_green_state); // Alterna a borda do display
            if(led_green_state){
                ssd1306_toggle_border(&ssd, led_green_state);
            }
            ssd1306_fill(&ssd, false);
        }
    }
}

// Atualiza o nível de PWM de um pino específico
void atualizar_pwm(uint pin, uint16_t valor) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), valor);
}

// Configura o PWM para um pino específico
void configurar_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 4095);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), 0);
    pwm_set_enabled(slice_num, true);
}

// Atualiza o display OLED com a posição do quadrado
void atualizar_display() {
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, pos_y, pos_x, QUADRADO_SIZE, QUADRADO_SIZE, true, false);
    ssd1306_send_data(&ssd);
}

int main() {
    stdio_init_all();

    // Configuração da comunicação I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    atualizar_display();
    
    // Configuração dos botões com interrupção
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    // Configuração do LED verde
    gpio_init(LED_RGB_GREEN);
    gpio_set_dir(LED_RGB_GREEN, GPIO_OUT);
    gpio_put(LED_RGB_GREEN, led_green_state);
    
    // Configuração dos LEDs PWM (vermelho e azul)
    configurar_pwm(LED_RGB_RED);
    configurar_pwm(LED_RGB_BLUE);
    
    // Configuração do ADC para leitura do joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    
    while (true) {
        // Leitura dos valores do joystick
        adc_select_input(0);
        uint16_t valor_x = adc_read();
        adc_select_input(1);
        uint16_t valor_y = adc_read();

        printf("%u \n", valor_y);

        // Cálculo do deslocamento do quadrado na tela
        int desloc_x = ((int)valor_x - 2048) / 400;
        int desloc_y = ((int)valor_y - 2048) / 400;
        
        pos_x += desloc_y;
        pos_y += -desloc_x;

        // Impede que o quadrado saia da tela
        if (pos_x < 0) pos_x = 0;
        if (pos_x > WIDTH - QUADRADO_SIZE) pos_x = WIDTH - QUADRADO_SIZE;
        if (pos_y < 0) pos_y = 0;
        if (pos_y > HEIGHT - QUADRADO_SIZE) pos_y = HEIGHT - QUADRADO_SIZE;
        
        atualizar_display();
        

        // Controle de brilho dos LEDs PWM com base no joystick
        if (led_pwm_state) {
            uint16_t brilho_vermelho = 0;
            uint16_t brilho_azul = 0;
            
            if (abs(2048 - valor_x) > 300) {
                brilho_vermelho = abs(2048 - valor_x) * 2;
            }
            
            if (abs(2048 - (valor_y + 50)) > 300) {
                brilho_azul = abs(2048 - valor_y) * 2;
            }
            
            atualizar_pwm(LED_RGB_RED, brilho_vermelho);
            atualizar_pwm(LED_RGB_BLUE, brilho_azul);
        } else {
            atualizar_pwm(LED_RGB_RED, 0);
            atualizar_pwm(LED_RGB_BLUE, 0);
        }
        
        sleep_ms(50);
    }
}
