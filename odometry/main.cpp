#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <math.h>

#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1

struct Data {
    int16_t x;
    int16_t y;
};

enum class Stage {
    WaitSOF,
    RecvData,
    WaitEOF,
    Ready
};

enum Stage stage = Stage::WaitSOF;
uint8_t buffer[8];
uint8_t i = 0;

void on_uart_rx() {
    uint8_t byte = uart_getc(UART_ID);
    switch (stage) {
        case Stage::WaitSOF:
            if (byte == 0x55) {
                stage = Stage::RecvData;
            }
            break;
        case Stage::RecvData:
            buffer[i] = byte;
            i++;
            if (i == 8) {
                i = 0;
                stage = Stage::WaitEOF;
            }
            break;
        case Stage::WaitEOF:
            if (byte == 0xAA) {
                stage = Stage::Ready;
            } else {
                stage = Stage::WaitSOF;
            }
            break;
        case Stage::Ready:
            break;
    }
}

int main() {
    stdio_init_all();

    uart_init(UART_ID, 115200);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);
    uint8_t UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    double x = 0;
    double y = 0;
    double theta = 0;
    const double L = 205.; // マウス間距離[mm]
    const double N = 23; // mm単位に直せる謎の値

    while (true) {
        uart_putc_raw(UART_ID, 0x5A);
        sleep_ms(5);
        if (stage == Stage::Ready) {
            double lx_count = *((int16_t*)(buffer + 0)) / N;
            double ly_count = (*((int16_t*)(buffer + 2)) * -1) / N;
            double rx_count = (*((int16_t*)(buffer + 4)) * -1) / N;
            double ry_count = *((int16_t*)(buffer + 6)) / N;
            double local_dx = (lx_count + rx_count) / 2.;
            double local_dy = (ry_count + ly_count) / 2.;
            double local_d_theta = (ry_count - ly_count) / L;

            x += local_dx * cos(theta) - local_dy * sin(theta);
            y += local_dy * cos(theta) + local_dx * sin(theta);
            theta += local_d_theta;

            printf("%lf, %lf, %lf\n", x, y, (theta * 180.) / M_PI);
            stage = Stage::WaitSOF;
        }
    }
}
