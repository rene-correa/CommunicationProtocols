#include <Arduino.h>
#include "driver/spi_master.h"

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_SCLK 18
#define PIN_CS   5
#define LED_PIN  2
#define BUTTON_PIN 0

spi_device_handle_t spiSlave;

static uint8_t txBuf[4] __attribute__((aligned(4)));
static uint8_t rxBuf[4] __attribute__((aligned(4)));

// Inicializa o SPI master e adiciona o slave
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,                // modo SPI 0
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .spics_io_num = PIN_CS,
        .queue_size = 1
    };

    esp_err_t ret;
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        Serial.printf("Erro ao iniciar barramento SPI: %d\n", ret);
        while (true);
    }

    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spiSlave);
    if (ret != ESP_OK) {
        Serial.printf("Erro ao adicionar dispositivo SPI: %d\n", ret);
        while (true);
    }

    Serial.println("SPI Master iniciado!");
}

void loop() {
    uint8_t sendData, recvData;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    int buttonValue = digitalRead(BUTTON_PIN);
    sendData = (buttonValue == HIGH) ? 1 : 0;

    Serial.printf("sendData: %u", sendData);

    t.length = 8;  // 8 bits = 1 byte
    t.tx_buffer = &sendData;
    t.rx_buffer = &recvData;

    esp_err_t ret = spi_device_transmit(spiSlave, &t);

    if (ret == ESP_OK) {
        if (recvData == 0) {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED Mestre LIGADO");
        } else {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED Mestre APAGADO");
        }
    } else {
        Serial.printf("Erro na transação SPI: %d\n", ret);
    }

    delay(1000);
}