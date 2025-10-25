#include <Arduino.h>
#include "driver/spi_slave.h"

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_SCLK 18
#define PIN_CS 5
#define LED_PIN 2
#define BUTTON_PIN 0

static uint8_t recvBuf[4] __attribute__((aligned(4)));
static uint8_t sendBuf[4] __attribute__((aligned(4)));

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = PIN_CS,
        .flags = 0,
        .queue_size = 1,
        .mode = 0};

    esp_err_t ret = spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        Serial.printf("Erro ao iniciar SPI Slave: %d\n", ret);
        while (true)
            ;
    }

    Serial.println("SPI Slave iniciado!");
}

void loop()
{
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.rx_buffer = recvBuf;
    t.tx_buffer = sendBuf;
    t.length = 8;

    esp_err_t ret = spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY);

    if (ret == ESP_OK)
    {
        uint8_t received = recvBuf[0];

        if (received == 0)
        {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("Slave LED ON");
        }
        else
        {
            digitalWrite(LED_PIN, LOW);
            Serial.println("Slave LED OFF");
        }

        uint8_t buttonState = digitalRead(BUTTON_PIN);
        sendBuf[0] = (buttonState == HIGH) ? 1 : 0;

        Serial.printf("sendBuf: %u", sendBuf[0]);

        delay(50);
    }
}
